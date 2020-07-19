
#include "utility/SimpleAgent.h"
#include "device/PyCubed.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <net/if.h>
#include <linux/if_tun.h>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 0.25

#define PYCUBED_UART 0
#define PYCUBED_BAUD 9600

#define MAXBUFFERSIZE 2560 // comm buffer for agents
#define TUN_BUF_SIZE 2000

using namespace std;
using namespace cubesat;


// The agent object which allows for communication with COSMOS
SimpleAgent *agent;
Device *imu, *gps, *battery, *radio, *pycubed;

// Tunnel stuff
static queue<vector<uint8_t> > tun_fifo;
static queue<vector<uint8_t> > tcv_fifo;

static condition_variable tcv_fifo_check;
static condition_variable tun_fifo_check;

static int tun_fd;

static string rxr_devname;
static string txr_devname;
static uint16_t tunnel_mtu = 512;

int tun_out, tun_in, gl_in, gl_out = 0;


// The PyCubed device handler
struct {
	PyCubed *device;
	uint8_t uart_device;
	int baud_rate = 9600;
	
	bool sent_startup_confirmation = false;
} pycubed2;

struct {
	struct {
		int cindex;
		int dindex;
	} obc;
	
	struct {
		int cindex;
		int dindex;
	} imu;
	
	
	struct {
		int cindex;
		int dindex;
	} gps;
	
	struct {
		int cindex;
		int dindex;
	} radio;
	
	struct {
		int cindex;
		int dindex;
	} batt;
	
	PyCubedIMUInfo imu_info;
	PyCubedGPSInfo gps_info;
	PyCubedPowerInfo power_info;
	PyCubedTempInfo temp_info;
} pycubed_info;

// =========== Function Prototypes ===========

/**
 * @brief Sets up the agent. Prints a message and exits with code 1 if an error occurs during setup.
 */
void InitAgent();

/**
 * @brief Main loop for this agent.
 */
void run_agent();


/**
 * @brief Sets up the pycubed.
 */
void init_pycubed();
/**
 * @brief Handles operations of the PyCubed
 */
void handle_pycubed();


void update_namespace();

bool reconnect_pycubed();




/**
 * @brief Callback function for received files from PyCubed.
 * @param name The file location
 */
void file_received(const std::string &name);

/**
 * @brief Shutdown callback function
 */
void shutdown();


// Tunnel stuff
void tcv_read_loop();
void tcv_write_loop();
void tun_read_loop();
void tun_write_loop();

int32_t request_tunnel_queue_size(char *request, char *response, Agent *);
int32_t request_serial_queue_size(char *request, char *response, Agent *);

// ===========================================









int main(int argc, char** argv) {
	char tunnel_ip[20];
	vector<uint8_t> buffer;
	std::string dev;
	int32_t iretn;
	bool do_tunnel = true;
	
	switch ( argc ) {
		case 3: {
			dev = argv[2];
			// Get address for tunnel
			strcpy(tunnel_ip,argv[1]);
		} break;
		default:
			printf("To establish tunnel: agent_pycubed ip_address device\n");
			do_tunnel = false;
			break;
	}
	
	// Initialize the Agent
	InitAgent();
	
	// Initialize the PyCubed
	init_pycubed();
	
	if ( !do_tunnel ) {
		// Run the main loop for this agent
		run_agent();
		
		exit(0);
	}
	
	// Initialize Gamalink
	//	gamalink = new GamaLink();
	//	gamalink->set_debug_level(GamaLink::debugLevel::OFF);
	//	iretn = gamalink->init(dev);
	//	if(iretn < 0 ){
	//		printf("Could not intialize \n");
	//		exit(-1);
	//	}
	
	// Start serial threads
	thread tcv_read_thread(tcv_read_loop);
	thread tcv_write_thread(tcv_write_loop);
	
	// Open tunnel device
	int tunnel_sock;
	struct ifreq ifr1, ifr2;
	struct sockaddr_in *addr = (struct sockaddr_in *)&ifr2.ifr_addr;
	
	if ((tun_fd=open("/dev/net/tun", O_RDWR)) < 0)
	{
		perror("Error opening tunnel device");
		exit (-1);
	}
	
	memset(&ifr1, 0, sizeof(ifr1));
	ifr1.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr1.ifr_name, agent->GetComplexAgent()->cinfo->agent[0].beat.proc, IFNAMSIZ);
	if (ioctl(tun_fd, TUNSETIFF, static_cast<void *>(&ifr1)) < 0)
	{
		perror("Error setting tunnel interface");
		exit (-1);
	}
	
	if((tunnel_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
	{
		perror("Error opening tunnel socket");
		exit (-1);
	}
	
	// Get ready to set things
	strncpy(ifr2.ifr_name, agent->GetComplexAgent()->cinfo->agent[0].beat.proc, IFNAMSIZ);
	ifr2.ifr_addr.sa_family = AF_INET;
	
	// Set interface address
	
	inet_pton(AF_INET, tunnel_ip, &addr->sin_addr);
	if (ioctl(tunnel_sock, SIOCSIFADDR, &ifr2) < 0 )
	{
		perror("Error setting tunnel address");
		exit (-1);
	}
	
	// Set interface netmask
	inet_pton(AF_INET, static_cast<const char *>("255.255.255.0"), &addr->sin_addr);
	if (ioctl(tunnel_sock, SIOCSIFNETMASK, &ifr2) < 0 )
	{
		perror("Error setting tunnel netmask");
		exit (-1);
	}
	
	if (ioctl(tunnel_sock, SIOCGIFFLAGS, &ifr2) < 0 )
	{
		perror("Error getting tunnel interface flags");
		exit (-1);
	}
	
	// Bring interface up
	ifr2.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if (ioctl(tunnel_sock, SIOCSIFFLAGS, &ifr2) < 0 )
	{
		perror("Error setting tunnel interfa ce flags");
		exit (-1);
	}
	
	// Set interface MTU
	ifr2.ifr_mtu = tunnel_mtu;
	if (ioctl(tunnel_sock, SIOCSIFMTU, &ifr2) < 0 )
	{
		perror("Error setting tunnel interface MTU");
		exit (-1);
	}
	
	
	close(tunnel_sock);
	
	// Start tunnel threads
	thread tun_read_thread(tun_read_loop);
	thread tun_write_thread(tun_write_loop);
	
	
	// Run the main loop for this agent
	run_agent();
	
	
	return 0;
}


void InitAgent() {
	// Create the agent
	agent = new SimpleAgent(CUBESAT_AGENT_PYCUBED_NAME);
	agent->SetLoopPeriod(SLEEP_TIME);
	
	pycubed = agent->NewDevice<CPU>("pycubed");
	pycubed->AddProperty<CPU::UTC>(0);
	pycubed->SetProperty<bool>("sent_startup_confirmation", false);
	
	battery = agent->NewDevice<Battery>("battery");
	battery->AddProperty<Battery::Temperature>(273.15);
	battery->AddProperty<Battery::UTC>(0);
	battery->AddProperty<Battery::Capacity>(3.5f * 4);
	battery->AddProperty<Battery::Efficiency>(0.85f);
	
	imu = agent->NewDevice<IMU>("imu");
	imu->AddProperty<IMU::UTC>(0);
	imu->AddProperty<IMU::Temperature>(273.15);
	imu->AddProperty<IMU::Acceleration>(Vec3(0, 0, 0));
	imu->AddProperty<IMU::Magnetometer>(Vec3(0, 0, 0));
	imu->AddProperty<IMU::Gyroscope>(Vec3(0, 0, 0));
	
	gps = agent->NewDevice<GPS>("gps");
	gps->AddProperty<GPS::UTC>(0);
	gps->AddProperty<GPS::Location>(Location(0, 0, 0));
	gps->AddProperty<GPS::SatellitesUsed>(0);
}

void run_agent() {
	
	// Start executing the agent
	while ( agent->StartLoop() ) {
		
		// Try to connect to the PyCubed if not already connected
		//reconnect_pycubed();
		
		// Update PyCubed communication
		handle_pycubed();
		
		// Update the COSMOS namespace
		update_namespace();
	}
}


void init_pycubed() {
	
	// Create a new PyCubed device
	PyCubed *handler = new PyCubed(PYCUBED_UART, PYCUBED_BAUD);
	handler->SetShutdownCallback(shutdown);
	
	pycubed->SetProperty<PyCubed*>("handler", handler);
}

bool connect_pycubed() {
	
	PyCubed *handler = pycubed->GetProperty<PyCubed*>("handler");
	
	if ( handler->IsOpen() )
		return true;
	
	if ( handler->Open() >= 0 ) {
		printf("Successfully opened PyCubed device at %s\n", handler->GetDevicePath().c_str());
		return true;
	}
	else {
		printf("Failed to open PyCubed device at %s\n", handler->GetDevicePath().c_str());
		handler->Close();
		
		return false;
	}
}

void handle_pycubed() {
	PyCubed *handler = pycubed->GetProperty<PyCubed*>("handler");
	
	// Make sure the PyCubed is connected
	if ( !handler->IsOpen() ) {
		return;
	}
	
	
	
	// Send startup confirmation
	if ( !pycubed->GetProperty<bool>("sent_startup_confirmation") ) {
		handler->StartupConfirmation();
		pycubed->SetProperty<bool>("sent_startup_confirmation", true);
	}
	
	
	
	// Receive messages from pycubed
	handler->ReceiveMessages();
	
	// Update information
	pycubed_info.imu_info = handler->GetIMUInfo();
	pycubed_info.gps_info = handler->GetGPSInfo();
	pycubed_info.power_info = handler->GetPowerInfo();
	pycubed_info.temp_info = handler->GetTempInfo();
}

void update_namespace() {
	
	pycubed->Timestamp<CPU>();
	pycubed->SetProperty<CPU::Voltage>(pycubed_info.power_info.sys_voltage);
	pycubed->SetProperty<CPU::Current>(pycubed_info.power_info.sys_current);
	pycubed->SetProperty<CPU::Temperature>(pycubed_info.temp_info.cpu_temp);
	
	battery->Timestamp<Battery>();
	battery->SetProperty<Battery::Voltage>(pycubed_info.power_info.batt_voltage);
	battery->SetProperty<Battery::Current>(pycubed_info.power_info.batt_current);
	battery->SetProperty<Battery::Temperature>(pycubed_info.temp_info.batt_temp);
	
	imu->Timestamp<IMU>();
	imu->SetProperty<IMU::Temperature>(pycubed_info.temp_info.cpu_temp);
	imu->SetProperty<IMU::Magnetometer>(pycubed_info.imu_info.mag);
	imu->SetProperty<IMU::Acceleration>(pycubed_info.imu_info.accel);
	imu->SetProperty<IMU::Gyroscope>(pycubed_info.imu_info.omega);
	
	
	gps->Timestamp<GPS>();
	gps->SetProperty<GPS::Location>(Location(pycubed_info.gps_info.latitude, pycubed_info.gps_info.longitude, pycubed_info.gps_info.altitude));
	gps->SetProperty<GPS::SatellitesUsed>(pycubed_info.gps_info.sats_used);
	
}


void shutdown() {
	
	PyCubed *handler = pycubed->GetProperty<PyCubed*>("handler");
	
	// 1. Shut down agents (not agent_switch)
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_TEMP_NAME " shutdown");
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_SUNSENSOR_NAME " shutdown");
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_HEATER_NAME " shutdown");
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_RASPI_NAME " shutdown");
	
	// 2. Turn off hardware components (agent_switch)
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_SWITCH_NAME " disable all");
	
	// 3. Give PyCubed permission to take over
	handler->Handoff();
	agent->Shutdown();
	
	exit(0);
}

void tun_read_loop() {
	vector<uint8_t> buffer;
	ssize_t nbytes;
	
	while ( agent->IsRunning() ) {
		
		buffer.resize(TUN_BUF_SIZE);
		nbytes = read(tun_fd, &buffer[0], TUN_BUF_SIZE);
		if (nbytes > 0)
		{	// Start of mutex for tcv fifo
			tun_in ++;
			
			buffer.resize(static_cast<size_t>(nbytes));
			tcv_fifo.push(buffer);
			tcv_fifo_check.notify_one();
		}	// End of mutex for tcv fifo
	}
}

void tun_write_loop() {
	vector<uint8_t> buffer;
	ssize_t nbytes;
	std::mutex tun_fifo_lock;
	std::unique_lock<std::mutex> locker(tun_fifo_lock);
	
	while ( agent->IsRunning() ) {
		
		tun_fifo_check.wait(locker);
		// End of mutex for tun fifo
		
		while (!tun_fifo.empty())
		{
			buffer = tun_fifo.front();
			nbytes = write(tun_fd, &buffer[0], buffer.size());
			if (nbytes > 0)
			{
				tun_fifo.pop();
				tun_out ++;
			}
		}
	}
}

void tcv_read_loop() { // reading from gamalink packet buffer (incoming)
	vector<uint8_t> buffer;
	ssize_t nbytes;
	PyCubedPacket packet;
	PyCubed *handler = nullptr;
	
	
	while ( agent->IsRunning() ) {
		
		if ( handler == nullptr )
			handler = pycubed->GetProperty<PyCubed*>("handler");
		
		// Read data from receiver port
		buffer.resize(TUN_BUF_SIZE);
		
		if ( handler == nullptr )
			nbytes = 0;
		else
			nbytes = handler->IsOpen() ? handler->PopIncoming(packet) : 0;
		
		if ( nbytes > 0 ) { // Start of mutex for tun FIFO
			gl_in ++;
			buffer.resize(0);
			
			for(uint16_t i = 0; i < nbytes; i++){
				buffer.push_back(packet.content.data[i+6]);
			}
			
			tun_fifo.push(buffer);
			tun_fifo_check.notify_one();
			
		} // End of mutex for tun FIFO
	}
}

void tcv_write_loop() { // telecommand send gamalink packet (outgoing)
	std::mutex tcv_fifo_lock;
	std::unique_lock<std::mutex> locker(tcv_fifo_lock);
	vector<uint8_t> buffer;
	PyCubed *handler;
	
	PyCubedDataPacket out_packet;
	while ( agent->IsRunning()) {
		tcv_fifo_check.wait(locker);
		
		if ( handler == nullptr )
			handler = pycubed->GetProperty<PyCubed*>("handler");
		
		while ( !tcv_fifo.empty() ) {
			// Get next packet from transceiver FIFO
			buffer = tcv_fifo.front();
			tcv_fifo.pop();
			gl_out ++;
			// Write data to transmitter port
			out_packet.length = buffer.size();
			out_packet.addr = 255;
			out_packet.data = buffer;
			
			if ( handler != nullptr && handler->IsOpen() )
				handler->TelecommandOutboundPacket(out_packet);
			else
				printf("Attempted to send packets, but PyCubed device is not open.\n");
			//gamalink->telecommand_outbound_packet(out_packet);
		}
	}
}

