
#ifndef CUBESAT_DEVICE_DEFS
#define CUBESAT_DEVICE_DEFS

#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"

namespace cubesat {
	
	struct Vec3 {
		double x, y, z;
		
		Vec3() : x(0), y(0), z(0) {}
		Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
		Vec3(const rvector &vec) : x(vec.col[0]), y(vec.col[1]), z(vec.col[2]) {}
		
		operator rvector() const {
			rvector value;
			value.col[0] = x;
			value.col[1] = y;
			value.col[2] = z;
			return value;
		}
		
	};
	
	struct Location {
		double latitude;
		double longitude;
		double altitude;
		
		Location() : latitude(0), longitude(0), altitude(0) {}
		Location(double latitude, double longitude, double altitude)
			: latitude(latitude), longitude(longitude), altitude(altitude) {}
		Location(const gvector &vec) : latitude(vec.lat), longitude(vec.lon), altitude(vec.h) {}
		
		operator gvector() const {
			gvector value;
			value.lat = latitude;
			value.lon = longitude;
			value.h = altitude;
			return value;
		}
	};
	
	template <typename T, size_t _offset>
	struct DeviceProperty {
		using ValueType = T;
		static constexpr size_t offset = _offset;
	};
	
	template <DeviceType T>
	struct _Device {};
	
	
	
	
	
	
	
	
	
	
	
	
	
	template <>
	struct _Device<DeviceType::CPU> {
		static constexpr DeviceType type = DeviceType::CPU;
		static constexpr const char *name = "CPU";
		
		struct UTC : DeviceProperty<double, offsetof(cpustruc, utc)> {
			static constexpr auto key = "device_cpu_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(cpustruc, volt)> {
			static constexpr auto key = "device_cpu_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(cpustruc, amp)> {
			static constexpr auto key = "device_cpu_amp";
			static constexpr const char *name = "Current";
		};
		struct MemoryUsed : DeviceProperty<float, offsetof(cpustruc, gib)> {
			static constexpr auto key = "device_cpu_gib";
			static constexpr const char *name = "Memory Used";
		};
		struct MaxMemory : DeviceProperty<float, offsetof(cpustruc, maxgib)> {
			static constexpr auto key = "device_cpu_maxgib";
			static constexpr const char *name = "Max Memory";
		};
		struct Load : DeviceProperty<float, offsetof(cpustruc, load)> {
			static constexpr auto key = "device_cpu_load";
			static constexpr const char *name = "Load";
		};
		struct BootCount : DeviceProperty<uint32_t, offsetof(cpustruc, boot_count)> {
			static constexpr auto key = "device_cpu_boot_count";
			static constexpr const char *name = "Boot Count";
		};
		struct UpTime : DeviceProperty<uint32_t, offsetof(cpustruc, uptime)> {
			static constexpr auto key = "device_cpu_uptime";
			static constexpr const char *name = "Up Time";
		};
		struct Temperature : DeviceProperty<float, offsetof(cpustruc, temp)> {
			static constexpr auto key = "device_cpu_temp";
			static constexpr const char *name = "Temperature";
		};
	};
	
	template <>
	struct _Device<DeviceType::BATT> {
		static constexpr DeviceType type = DeviceType::BATT;
		static constexpr const char *name = "Battery";
		
		struct UTC : DeviceProperty<double, offsetof(battstruc, utc)> {
			static constexpr auto key = "device_batt_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(battstruc, volt)> {
			static constexpr auto key = "device_batt_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(battstruc, amp)> {
			static constexpr auto key = "device_batt_amp";
			static constexpr const char *name = "Current";
		};
		struct Percentage : DeviceProperty<float, offsetof(battstruc, percentage)> {
			static constexpr auto key = "device_batt_percentage";
			static constexpr const char *name = "Percentage";
		};
		struct Capacity : DeviceProperty<float, offsetof(battstruc, percentage)> {
			static constexpr auto key = "device_batt_capacity"; // node_battcap?
			static constexpr const char *name = "Capacity";
		};
		struct Efficiency : DeviceProperty<float, offsetof(battstruc, percentage)> {
			static constexpr auto key = "device_batt_efficiency";
			static constexpr const char *name = "Efficiency";
		};
		struct Temperature : DeviceProperty<float, offsetof(battstruc, temp)> {
			static constexpr auto key = "device_batt_temp";
			static constexpr const char *name = "Temperature";
		};
	};
	
	template <>
	struct _Device<DeviceType::SSEN> {
		static constexpr DeviceType type = DeviceType::SSEN;
		static constexpr const char *name = "Sun Sensor";
		
		struct UTC : DeviceProperty<double, offsetof(ssenstruc, utc)> {
			static constexpr auto key = "device_ssen_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(ssenstruc, volt)> {
			static constexpr auto key = "device_ssen_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(ssenstruc, amp)> {
			static constexpr auto key = "device_ssen_amp";
			static constexpr const char *name = "Current";
		};
		struct Temperature : DeviceProperty<float, offsetof(ssenstruc, temp)> {
			static constexpr auto key = "device_ssen_temp";
			static constexpr const char *name = "Temperature";
		};
	};
	
	template <>
	struct _Device<DeviceType::TSEN> {
		static constexpr DeviceType type = DeviceType::TSEN;
		static constexpr const char *name = "Temperature Sensor";
		
		struct UTC : DeviceProperty<double, offsetof(tsenstruc, utc)> {
			static constexpr auto key = "device_tsen_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(tsenstruc, volt)> {
			static constexpr auto key = "device_tsen_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(tsenstruc, amp)> {
			static constexpr auto key = "device_tsen_amp";
			static constexpr const char *name = "Current";
		};
		struct Temperature : DeviceProperty<float, offsetof(tsenstruc, temp)> {
			static constexpr auto key = "device_tsen_temp";
			static constexpr const char *name = "Temperature";
		};
		struct Enabled : DeviceProperty<bool, offsetof(tsenstruc, enabled)> {
			static constexpr auto key = "device_tsen_enabled";
			static constexpr const char *name = "Enabled";
		};
	};
	
	template <>
	struct _Device<DeviceType::IMU> {
		static constexpr DeviceType type = DeviceType::IMU;
		static constexpr const char *name = "IMU";
		
		struct UTC : DeviceProperty<double, offsetof(imustruc, utc)> {
			static constexpr auto key = "device_imu_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(imustruc, volt)> {
			static constexpr auto key = "device_imu_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(imustruc, amp)> {
			static constexpr auto key = "device_imu_amp";
			static constexpr const char *name = "Current";
		};
		struct Acceleration : DeviceProperty<rvector, offsetof(imustruc, accel)> {
			static constexpr auto key = "device_imu_accel";
			static constexpr const char *name = "Acceleration";
		};
		struct Magnetometer : DeviceProperty<rvector, offsetof(imustruc, mag)> {
			static constexpr auto key = "device_imu_mag";
			static constexpr const char *name = "Magnetometer";
		};
		struct Gyroscope : DeviceProperty<rvector, offsetof(imustruc, omega)> {
			static constexpr auto key = "device_imu_omega";
			static constexpr const char *name = "Gyroscope";
		};
		struct Temperature : DeviceProperty<float, offsetof(imustruc, temp)> {
			static constexpr auto key = "device_imu_temp";
			static constexpr const char *name = "Temperature";
		};
	};
	
	template <>
	struct _Device<DeviceType::GPS> {
		static constexpr DeviceType type = DeviceType::GPS;
		static constexpr const char *name = "GPS";
		
		struct UTC : DeviceProperty<double, offsetof(gpsstruc, utc)> {
			static constexpr auto key = "device_gps_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(gpsstruc, volt)> {
			static constexpr auto key = "device_gps_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(gpsstruc, amp)> {
			static constexpr auto key = "device_gps_amp";
			static constexpr const char *name = "Current";
		};
		struct Location : DeviceProperty<gvector, offsetof(gpsstruc, geods)> {
			static constexpr auto key = "device_gps_geods";
			static constexpr const char *name = "Location";
		};
		struct SatellitesUsed : DeviceProperty<uint16_t, offsetof(gpsstruc, sats_used)> {
			static constexpr auto key = "device_gps_sats_used";
			static constexpr const char *name = "Satellites Used";
		};
	};
	
	template <>
	struct _Device<DeviceType::TCV> {
		static constexpr DeviceType type = DeviceType::TCV;
		static constexpr const char *name = "Radio Transceiver";
		
		struct UTC : DeviceProperty<double, offsetof(tcvstruc, utc)> {
			static constexpr auto key = "device_tcv_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(tcvstruc, volt)> {
			static constexpr auto key = "device_tcv_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(tcvstruc, amp)> {
			static constexpr auto key = "device_tcv_amp";
			static constexpr const char *name = "Current";
		};
		struct Enabled : DeviceProperty<bool, offsetof(tcvstruc, enabled)> {
			static constexpr auto key = "device_tcv_enabled";
			static constexpr const char *name = "Enabled";
		};
		struct Band : DeviceProperty<float, offsetof(tcvstruc, band)> {
			static constexpr auto key = "device_tcv_band";
			static constexpr const char *name = "Band";
		};
		struct Frequency : DeviceProperty<float, offsetof(tcvstruc, freq)> {
			static constexpr auto key = "device_tcv_freq";
			static constexpr const char *name = "Frequency";
		};
		struct MaxFrequency : DeviceProperty<float, offsetof(tcvstruc, maxfreq)> {
			static constexpr auto key = "device_tcv_maxfreq";
			static constexpr const char *name = "Max Frequency";
		};
		struct MinFrequency : DeviceProperty<float, offsetof(tcvstruc, minfreq)> {
			static constexpr auto key = "device_tcv_minfreq";
			static constexpr const char *name = "Min Frequency";
		};
		struct PowerIn : DeviceProperty<float, offsetof(tcvstruc, powerin)> {
			static constexpr auto key = "device_tcv_powerin";
			static constexpr const char *name = "Power In";
		};
		struct PowerOut : DeviceProperty<float, offsetof(tcvstruc, powerout)> {
			static constexpr auto key = "device_tcv_powerout";
			static constexpr const char *name = "Power Out";
		};
		struct MaxPower : DeviceProperty<float, offsetof(tcvstruc, maxpower)> {
			static constexpr auto key = "device_tcv_maxpower";
			static constexpr const char *name = "Max Power";
		};
		struct RSSI : DeviceProperty<uint16_t, offsetof(tcvstruc, rssi)> {
			static constexpr auto key = "device_tcv_rssi";
			static constexpr const char *name = "RSSI";
		};
		struct BadPacketCount : DeviceProperty<uint32_t, offsetof(tcvstruc, badcnt)> {
			static constexpr auto key = "device_tcv_badcnt";
			static constexpr const char *name = "Bad Packet Count";
		};
		struct GoodPacketCount : DeviceProperty<uint32_t, offsetof(tcvstruc, goodcnt)> {
			static constexpr auto key = "device_tcv_goodcnt";
			static constexpr const char *name = "Good Packet Count";
		};
		struct PacketSize : DeviceProperty<uint16_t, offsetof(tcvstruc, pktsize)> {
			static constexpr auto key = "device_tcv_pktsize";
			static constexpr const char *name = "Packet Size";
		};
		struct OperatingMode : DeviceProperty<uint16_t, offsetof(tcvstruc, opmode)> {
			static constexpr auto key = "device_tcv_opmode";
			static constexpr const char *name = "Operating Mode";
		};
	};
	
	template <>
	struct _Device<DeviceType::HTR> {
		static constexpr DeviceType type = DeviceType::HTR;
		static constexpr const char *name = "Heater";
		
		struct UTC : DeviceProperty<double, offsetof(htrstruc, utc)> {
			static constexpr auto key = "device_htr_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(htrstruc, volt)> {
			static constexpr auto key = "device_htr_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(htrstruc, amp)> {
			static constexpr auto key = "device_htr_amp";
			static constexpr const char *name = "Current";
		};
		struct Enabled : DeviceProperty<bool, offsetof(htrstruc, enabled)> {
			static constexpr auto key = "device_htr_enabled";
			static constexpr const char *name = "Enabled";
		};
	};
	
	template <>
	struct _Device<DeviceType::SWCH> {
		static constexpr DeviceType type = DeviceType::SWCH;
		static constexpr const char *name = "Switch";
		
		struct UTC : DeviceProperty<double, offsetof(swchstruc, utc)> {
			static constexpr auto key = "device_swch_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(swchstruc, volt)> {
			static constexpr auto key = "device_swch_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(swchstruc, amp)> {
			static constexpr auto key = "device_swch_amp";
			static constexpr const char *name = "Current";
		};
		struct Enabled : DeviceProperty<bool, offsetof(swchstruc, enabled)> {
			static constexpr auto key = "device_swch_enabled";
			static constexpr const char *name = "Enabled";
		};
	};
	
	template <>
	struct _Device<DeviceType::CAM> {
		static constexpr DeviceType type = DeviceType::CAM;
		static constexpr const char *name = "Camera";
		
		struct UTC : DeviceProperty<double, offsetof(camstruc, utc)> {
			static constexpr auto key = "device_cam_utc";
			static constexpr const char *name = "UTC";
		};
		struct Voltage : DeviceProperty<float, offsetof(camstruc, volt)> {
			static constexpr auto key = "device_cam_volt";
			static constexpr const char *name = "Voltage";
		};
		struct Current : DeviceProperty<float, offsetof(camstruc, amp)> {
			static constexpr auto key = "device_cam_amp";
			static constexpr const char *name = "Current";
		};
		struct Enabled : DeviceProperty<bool, offsetof(camstruc, enabled)> {
			static constexpr auto key = "device_cam_enabled";
			static constexpr const char *name = "Enabled";
		};
		struct PixelWidth : DeviceProperty<uint16_t, offsetof(camstruc, pwidth)> {
			static constexpr auto key = "device_cam_pwidth";
			static constexpr const char *name = "Pixel Width";
		};
		struct PixelHeight : DeviceProperty<uint16_t, offsetof(camstruc, pheight)> {
			static constexpr auto key = "device_cam_pheight";
			static constexpr const char *name = "Pixel Height";
		};
		struct Width : DeviceProperty<float, offsetof(camstruc, width)> {
			static constexpr auto key = "device_cam_width";
			static constexpr const char *name = "Width";
		};
		struct Height : DeviceProperty<float, offsetof(camstruc, height)> {
			static constexpr auto key = "device_cam_height";
			static constexpr const char *name = "Height";
		};
		struct FocalLength : DeviceProperty<float, offsetof(camstruc, flength)> {
			static constexpr auto key = "device_cam_flength";
			static constexpr const char *name = "Focal Length";
		};
	};
	
	template <>
	struct _Device<DeviceType::PLOAD> {
		static constexpr DeviceType type = DeviceType::PLOAD;
		static constexpr const char *name = "Custom Device";
		
		struct UTC : DeviceProperty<double, offsetof(ploadstruc, utc)> {
			static constexpr auto key = "device_pload_utc";
			static constexpr const char *name = "UTC";
		};
	};
	
	using CustomDevice = _Device<DeviceType::PLOAD>;
	using CPU = _Device<DeviceType::CPU>;
	using Battery = _Device<DeviceType::BATT>;
	using SunSensor = _Device<DeviceType::SSEN>;
	using TemperatureSensor = _Device<DeviceType::TSEN>;
	using IMU = _Device<DeviceType::IMU>;
	using GPS = _Device<DeviceType::GPS>;
	using Heater = _Device<DeviceType::HTR>;
	using Transceiver = _Device<DeviceType::TCV>;
	using Switch = _Device<DeviceType::SWCH>;
	using Camera = _Device<DeviceType::CAM>;
	
	
	
	using DevicePropertyID = intptr_t;
	
	template <typename T>
	DevicePropertyID GetDevicePropertyID() {
		static char key;
		return reinterpret_cast<DevicePropertyID>(&key);
	}
	
	template <typename ValueType>
	struct PropertyValueString {
		inline static std::string ToString(ValueType value) {
			return std::to_string(value);
		}
	};
	template <>
	struct PropertyValueString<bool> {
		inline static std::string ToString(bool value) {
			return value ? "true" : "false";
		}
	};
	template <>
	struct PropertyValueString<rvector> {
		inline static std::string ToString(const rvector &value) {
			return "(" + std::to_string(value.col[0]) + ", " + std::to_string(value.col[1]) + ", " + std::to_string(value.col[2]) + ")";
		}
	};
	template <>
	struct PropertyValueString<gvector> {
		inline static std::string ToString(const gvector &value) {
			return "(" + std::to_string(value.lat) + ", " + std::to_string(value.lon) + ", " + std::to_string(value.h) + ")";
		}
	};
	
	
	
//	enum class Devices : int {
//		Payload = (int):DeviceType::PLOAD,
//		SunSensor = (int)DeviceType::SSEN,
//		TempSensor = (int)DeviceType::TSEN,
//		TorqueRod = (int)DeviceType::MTR,
//		Antenna = (int)DeviceType::ANT,
//		CPU = (int)DeviceType::CPU,
//		IMU = (int)DeviceType::IMU,
//		GPS = (int)DeviceType::GPS,
//		ReactionWheel = (int)DeviceType::RW,
//		RadioReceiver = (int)DeviceType::RXR,
//		RadioTransmitter = (int)DeviceType::TXR,
//		RadioTransceiver = (int)DeviceType::TCV,
//		Motor = (int)DeviceType::MOTR,
//		Thruster = (int)DeviceType::THST,
//		PropellantTank = (int)DeviceType::PROP,
//		Switch = (int)DeviceType::SWCH,
//		Rotor = (int)DeviceType::ROT,
//		StarTracker = (int)DeviceType::STT,
//		MotionCaptureCamera = (int)DeviceType::MCC,
//		TorqueRodControlUnit = (int)DeviceType::TCU,
//		PowerBus = (int)DeviceType::BUS,
//		PressureSensor = (int)DeviceType::PSEN,
//		Camera = (int)DeviceType::CAM,
//		Telemetry = (int)DeviceType::TELEM,
//		DiskDrive = (int)DeviceType::DISK,
		
//	};
	
	const char* GetDeviceTypeString(DeviceType type);
	
	template <typename ValueType, size_t Offset>
	using NodeProperty = DeviceProperty<ValueType, Offset>;
	
	
	namespace Node {
		struct UTC : DeviceProperty<double, offsetof(nodestruc, utc)> {
			static constexpr auto key = "node_utc";
			static constexpr const char *name = "UTC";
		};
		struct MomentOfInertia : DeviceProperty<rvector, offsetof(nodestruc, moi)> {
			static constexpr auto key = "node_moi";
			static constexpr const char *name = "Moment of Inertia";
		};
		struct Mass : DeviceProperty<float, offsetof(nodestruc, mass)> {
			static constexpr auto key = "node_mass";
			static constexpr const char *name = "Mass";
		};
		struct PowerGeneration : DeviceProperty<float, offsetof(nodestruc, powgen)> {
			static constexpr auto key = "node_powgen";
			static constexpr const char *name = "Power Generation";
		};
		struct PowerUse : DeviceProperty<float, offsetof(nodestruc, powuse)> {
			static constexpr auto key = "node_powuse";
			static constexpr const char *name = "Power Use";
		};
		struct BatteryCapacity : DeviceProperty<float, offsetof(nodestruc, battcap)> {
			static constexpr auto key = "node_battcap";
			static constexpr const char *name = "Battery Capacity";
		};
	}

}

#endif
