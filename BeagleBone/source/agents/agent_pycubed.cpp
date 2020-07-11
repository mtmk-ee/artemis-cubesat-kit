
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"


// Internal headers
#include "cubesat_defs.h"
#include "device/PyCubed.h"

// Standard headers
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <net/if.h>
#include <linux/if_tun.h>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

#define MAXBUFFERSIZE 2560 // comm buffer for agents
#define TUN_BUF_SIZE 2000

using namespace std;
using namespace cubesat;


// The agent object which allows for communication with COSMOS
Agent *agent;

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
PyCubed *pycubed;

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
void init_agent();
/**
 * @brief Sets the SOH string for the agent.
 */
void set_soh();
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
void init_tunnel();
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
	
	switch (argc)
	{
		case 3:
		{
			dev = argv[2];
			// Get address for tunnel
			strcpy(tunnel_ip,argv[1]);
		}
			break;
		default:
			printf("Usage: agent_pycubed ip_address device\n");
			exit (-1);
	}
	
	// Initialize the Agent
	init_agent();
	
	// Initialize the PyCubed
	init_pycubed();
	
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
	strncpy(ifr1.ifr_name, agent->cinfo->agent[0].beat.proc, IFNAMSIZ);
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
	strncpy(ifr2.ifr_name, agent->cinfo->agent[0].beat.proc, IFNAMSIZ);
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
	
	double nmjd = currentmjd(0.);
	int32_t sleept;
	
	// Set the SOH string
	set_soh();
	
	
	// Run the main loop for this agent
	run_agent();
	
	
	return 0;
}

void init_tunnel() {
	
}

void init_agent() {
	// Create the agent
	agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_PYCUBED_NAME);
	
	// Make sure the agent initialized successfully
	if ( agent->cinfo == nullptr || !agent->running() ) {
		
		// Notify that an error occurred and exit
		printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_PYCUBED_NAME);
		exit(1);
	}
	
	int status;
	
	// Add battery (4 physically, represented as 1 for now)
	status = json_createpiece(agent->cinfo, "batt", DeviceType::BATT);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add battery %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	
	int cindex = agent->cinfo->pieces[status].cidx;
	pycubed_info.batt.cindex = cindex;
	agent->cinfo->device[cindex].batt.capacity = 3.5f * 4;
	agent->cinfo->device[cindex].batt.nvolt = 3.635f * 2;
	agent->cinfo->device[cindex].batt.efficiency = .85f; // Not on datasheet
	
	// Add IMU
	status = json_createpiece(agent->cinfo, "imu", DeviceType::IMU);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add IMU %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	cindex = agent->cinfo->pieces[status].cidx;
	pycubed_info.imu.cindex = cindex;
	
	// Add GPS
	status = json_createpiece(agent->cinfo, "gps", DeviceType::GPS);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add GPS %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	cindex = agent->cinfo->pieces[status].cidx;
	pycubed_info.gps.cindex = cindex;
	
	
	
	// Add PyCubed OBC
	status = json_createpiece(agent->cinfo, "pycubedobc", DeviceType::CPU);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add PyCubed OBC %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	cindex = agent->cinfo->pieces[status].cidx;
	
	pycubed_info.obc.cindex = cindex;
	pycubed_info.obc.dindex = agent->cinfo->device[cindex].all.didx;
	
	
	json_dump_node(agent->cinfo);
	
	
	// Add request callbacks (TODO)
	
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	// Create the state of health string
	string soh = "{";
	
	// Add CPU info
	soh += "\"device_cpu_temp_00" + std::to_string(pycubed_info.obc.dindex) + "\",";
	soh += "\"device_cpu_volt_00" + std::to_string(pycubed_info.obc.dindex) + "\",";
	soh += "\"device_cpu_amp_00" + std::to_string(pycubed_info.obc.dindex) + "\",";
	
	// Add battery info
	soh += "\"device_batt_temp_000\",";
	soh += "\"device_batt_amp_000\",";
	soh += "\"device_batt_volt_000\",";
	soh += "\"device_batt_power_000\",";
	soh += "\"device_batt_charge_000\",";
	soh += "\"device_batt_percentage_000\",";
	soh += "\"device_batt_time_remaining_000\",";
	
	// Add IMU info
	soh += "\"device_imu_utc_000\",";
	soh += "\"device_imu_mag_000\",";
	soh += "\"device_imu_bdot_000\",";
	soh += "\"device_imu_accel_000\",";
	soh += "\"device_imu_alpha_000\",";
	soh += "\"device_imu_euler_000\",";
	soh += "\"device_imu_omega_000\",";
	soh += "\"device_imu_temp_000\",";
	
	// Add GPS info
	soh += "\"device_gps_utc_000\",";
	soh += "\"device_gps_geods_000\",";
	soh += "\"device_gps_sats_used_000\",";
	soh += "\"device_gps_sats_visible_000\",";
	soh += "\"device_gps_time_status_000\",";
	soh += "\"device_gps_solution_status_000\",";
	
	soh += "}";
	
	
	
	// set the soh string
	agent->set_sohstring(soh);
}

void run_agent() {
	
	
	double nmjd = currentmjd(0.);
    int32_t sleept;
	
	// Start executing the agent
	while ( agent->running() ) {
		
		handle_pycubed();
		
		update_namespace();
		
		// Sleep for a bit
		//COSMOS_SLEEP(SLEEP_TIME);
		
		
		// Set beginning of next cycle;
		nmjd += agent->cinfo->agent[0].aprd/86400.;
		
		sleept = (int32_t)((nmjd - currentmjd(0.))*86400000000.);
		if (sleept < 0)
		{
			sleept = 0;
		}
		COSMOS_USLEEP(sleept);
	}
}


void init_pycubed() {
	
	// Create a new PyCubed device
	pycubed = new PyCubed();
	
	// Set the callback functions
	pycubed->SetShutdownCallback(shutdown);
	
	// Notify the PyCubed that startup was successful
	pycubed->StartupConfirmation();
}


void handle_pycubed() {
	
	// Receive messages from pycubed
	pycubed->ReceiveMessages();
	
	// Update information
	pycubed_info.imu_info = pycubed->GetIMUInfo();
	pycubed_info.gps_info = pycubed->GetGPSInfo();
	pycubed_info.power_info = pycubed->GetPowerInfo();
	pycubed_info.temp_info = pycubed->GetTempInfo();
}

void update_namespace() {
	int cindex;
	
	// CPU
	cindex = pycubed_info.obc.cindex;
	agent->cinfo->device[cindex].cpu.utc = currentmjd();
	agent->cinfo->device[cindex].cpu.temp = pycubed_info.temp_info.cpu_temp;
	agent->cinfo->device[cindex].cpu.volt = pycubed_info.power_info.sys_voltage;
	agent->cinfo->device[cindex].cpu.amp = pycubed_info.power_info.sys_current;
	//agent->cinfo->device[cindex].cpu.amp = i3v3_draw + i5v_draw;
	//agent->cinfo->device[cindex].cpu.power = i3v3_draw * 3.3 + i5v_draw * 5;
	//agent->cinfo->device[cindex].cpu.volt = agent->cinfo->device[eps_cidx].cpu.power / agent->cinfo->device[eps_cidx].cpu.amp;
	//agent->cinfo->device[cindex].cpu.energy = tpower.bus[4].energy + tpower.bus[5].energy;
	
	
	// Battery
	cindex = pycubed_info.batt.cindex;
	agent->cinfo->device[cindex].batt.utc = currentmjd();
	agent->cinfo->device[cindex].batt.temp = pycubed_info.temp_info.batt_temp;
	agent->cinfo->device[cindex].batt.volt = pycubed_info.power_info.batt_voltage;
	agent->cinfo->device[cindex].batt.amp = pycubed_info.power_info.batt_current;
	
	// IMU
	cindex = pycubed_info.imu.cindex;
	agent->cinfo->device[cindex].imu.utc = currentmjd();
	agent->cinfo->device[cindex].imu.temp = pycubed_info.temp_info.cpu_temp;
	agent->cinfo->device[cindex].imu.mag = pycubed_info.imu_info.mag;
	agent->cinfo->device[cindex].imu.accel = pycubed_info.imu_info.accel;
	agent->cinfo->device[cindex].imu.omega = pycubed_info.imu_info.omega;
	
	// GPS
	agent->cinfo->device[cindex].gps.utc = currentmjd();
	agent->cinfo->device[cindex].gps.geods.lat = pycubed_info.gps_info.latitude;
	agent->cinfo->device[cindex].gps.geods.lon = pycubed_info.gps_info.longitude;
	agent->cinfo->device[cindex].gps.geods.h = pycubed_info.gps_info.altitude;
	agent->cinfo->device[cindex].gps.sats_used = pycubed_info.gps_info.sats_used;
	
	// Node
	agent->cinfo->node.utc = currentmjd();
	
	
}


void shutdown() {
	// 1. Shut down agents (not agent_switch)
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_RASPI_NAME " shutdown");
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_TEMP_NAME " shutdown");
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_SUNSENSOR_NAME " shutdown");
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_HEATER_NAME " shutdown");
	
	
	
	// 2. Turn off hardware components (agent_switch)
	system("agent " CUBESAT_NODE_NAME " " CUBESAT_AGENT_SWITCH_NAME " disable all");
	
	// 3. Give PyCubed permission to take over
	pycubed->Handoff();
	agent->shutdown();
	exit(0);
}

void tun_read_loop()
{
	vector<uint8_t> buffer;
	ssize_t nbytes;
	
	while (agent->running())
	{
		
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

void tun_write_loop()
{
	vector<uint8_t> buffer;
	ssize_t nbytes;
	std::mutex tun_fifo_lock;
	std::unique_lock<std::mutex> locker(tun_fifo_lock);
	
	while (agent->running())
	{
		
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

void tcv_read_loop() // reading from gamalink packet buffer (incoming)
{
	vector<uint8_t> buffer;
	ssize_t nbytes;
	PyCubedPacket packet;
	while (agent->running())
	{
		// Read data from receiver port
		buffer.resize(TUN_BUF_SIZE);
		
		nbytes = pycubed->PopIncoming(packet);
		if (nbytes > 0)
		{ // Start of mutex for tun FIFO
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

void tcv_write_loop() // telecommand send gamalink packet (outgoing)
{
	std::mutex tcv_fifo_lock;
	std::unique_lock<std::mutex> locker(tcv_fifo_lock);
	vector<uint8_t> buffer;
	
	PyCubedDataPacket out_packet;
	while (agent->running())
	{
		
		tcv_fifo_check.wait(locker);
		
		while (!tcv_fifo.empty())
		{
			// Get next packet from transceiver FIFO
			buffer = tcv_fifo.front();
			tcv_fifo.pop();
			gl_out ++;
			// Write data to transmitter port
			out_packet.length = buffer.size();
			out_packet.addr = 255;
			out_packet.data = buffer;
			pycubed->TelecommandOutboundPacket(out_packet);
			//gamalink->telecommand_outbound_packet(out_packet);
		}
	}
}

