
#ifndef CUBESAT_SIMPLEAGENT
#define CUBESAT_SIMPLEAGENT

#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"

#include "cubesat_defs.h"
#include "utility/device_defs.h"

#include <unordered_map>

#define STRICT_MODE true


namespace cubesat {
	
	
	int32_t RequestProxy(char *request, char* response, Agent *agent_);
	
	using ArgumentedRequest = std::function<std::string(std::vector<std::string>)>;
	using NonArgumentedRequest = std::function<std::string(void)>;
	std::string _Request_DebugPrint(std::vector<std::string> args);
	
	class SimpleAgent;
	
	
	class RemoteAgent {
	public:
		RemoteAgent() {
			beat.utc = 0.;
			beat.exists = false;
		}
		RemoteAgent(const std::string &node_name, const std::string &agent_name, Agent *agent, beatstruc beat) : agent(agent), beat(beat), node_name(node_name), agent_name(agent_name) {
			
		}
		
		/**
		 * @brief Checks if this agent was connected to successfully.
		 * @return True if connected
		 */
		inline bool IsOpen() const {
			return beat.utc != 0. && beat.exists;
		}
		
		
		bool Reconnect(float wait_sec = 2.0f, bool crash_if_failed = false) {
			
			// Indicate success if the agent is already open
			if ( IsOpen() )
				return true;
			
			
			beat = agent->find_agent(node_name, agent_name, wait_sec);
			
			// Check if an error occurred
			if ( !IsOpen() ) {
				printf("Could not find agent '%s' running on node '%s'\n", agent_name.c_str(), node_name.c_str());
				
				if ( crash_if_failed )
					exit(1);
			}
			
			return IsOpen();
		}
		
		
		std::unordered_map<std::string, Json::Value> GetValues(std::vector<std::string> keys) {
			std::unordered_map<std::string, Json::Value> values;
			
			// Return default values if the agent isn't connected
			if ( !IsOpen() ) {
				return values;
			}
			
			
			// Generate the request string
			std::stringstream request;
			request << "getvalue {";
			for (const std::string &key : keys)
				request << "\"" << key << "\", ";
			request << "}";
			
			// Call the request
			std::string output;
			int status = agent->send_request(beat, request.str(), output, wait_time);
			
			// Check if an error occurred
			if ( status < 0 ) {
				beat.utc = 0.;
				beat.exists = false;
				printf("Error sending request to [%s:%s]: %s\n", node_name.c_str(), agent_name.c_str(), cosmos_error_string(status).c_str());
				return values;
			}
			
			// Parse the output
			Json jresult;
			status = jresult.extract_object(output);
			
			if ( status > 0 ) {
				for (Json::Member member : jresult.Members) {
					values[member.value.name] = member.value;
				}
			}
			
			return values;
		}
		
		template <typename... Args>
		std::string SendRequest(std::string request_name, Args... args) {
			
			// Return default values if the agent isn't connected
			if ( !IsOpen() ) {
				return "";
			}
			
			// Generate the request string
			std::stringstream request;
			request << request_name;
			
			for (auto arg : {args...}) {
				request << " " << arg;
			}
			
			// Call the request
			std::string output;
			int status = agent->send_request(beat, request.str(), output, wait_time);
			
			// Check if an error occurred
			if ( status < 0 ) {
				beat.utc = 0.;
				beat.exists = false;
				printf("Error sending request to [%s:%s]: %s\n", node_name.c_str(), agent_name.c_str(), cosmos_error_string(status).c_str());
				return "";
			}
			
			return output;
		}
		
	protected:
		Agent *agent;
		beatstruc beat;
		float wait_time = 5.0f;
		std::string node_name;
		std::string agent_name;
		
	};
	
	
	class Device {
		struct Property {
			std::string value_string;
			std::string cosmos_name;
			std::string readable_name;
			bool post;
			
			Property() {}
			Property (const std::string &value_string, const std::string &cosmos_name, const std::string &readable_name, bool post)
				: value_string(value_string), cosmos_name(cosmos_name), readable_name(readable_name), post(post) {}
		};
		
		class NonExistentPropertyException : public std::exception {
		public:
			NonExistentPropertyException(const std::string &prop_name) {
				error_msg = "Attempted to access nonexistent property '" + prop_name + "'";
			}
			
		  virtual const char* what() const throw() {
			return error_msg.c_str();
		  }
			
		private:
			std::string error_msg;
		};
		
	public:
		Device()
			: agent(nullptr), type(DeviceType::NONE),
			  pindex(-1), cindex(-1), dindex(-1) {}
		Device(Agent *agent, DeviceType type, const std::string &device_name,
			   int pindex, int cindex, int dindex)
			: agent(agent), type(type), device_name(device_name),
			  pindex(pindex), cindex(cindex), dindex(dindex) {}
		~Device() {
			// Free the custom properties
			for (auto pair : custom_properties) {
				free(pair.second);
			}
		}
		
		template <typename _DeviceType>
		void Timestamp() {
			SetProperty<typename _DeviceType::UTC>(currentmjd());
		}
		
		
		template <typename _DeviceProperty, typename T = typename _DeviceProperty::ValueType, size_t offset = _DeviceProperty::offset>
		typename std::enable_if<std::is_base_of<DeviceProperty<T, offset>, _DeviceProperty>::value, bool>::type
		PropertyExists() {
			return cosmos_properties.find(GetDevicePropertyID<_DeviceProperty>()) != cosmos_properties.end();
		}
		
		template <typename T>
		bool PropertyExists(const std::string &property_name) {
			return custom_properties.find(property_name) != custom_properties.end();
		}
		
		/**
		 * @brief Adds and posts a property. Equivalent to SetProperty<...>(..., true)
		 * @tparam _DeviceProperty The property type
		 * @param value The value
		 */
		template <typename _DeviceProperty>
		void AddProperty(typename _DeviceProperty::ValueType value = _DeviceProperty::ValueType()) {
			SetProperty<_DeviceProperty>(value, true);
		}
		
		template <typename _DeviceProperty>
		void SetProperty(typename _DeviceProperty::ValueType value, bool post = false) {
			
			// Check if there's an old value set and if so, take its 'post' value
			if ( cosmos_properties.find(GetDevicePropertyID<_DeviceProperty>()) != cosmos_properties.end() ) {
				post = cosmos_properties[GetDevicePropertyID<_DeviceProperty>()].post;
			}
			
			// Create the property
			Property new_property(PropertyValueString<typename _DeviceProperty::ValueType>::ToString(value), GetCOSMOSPropertyName<_DeviceProperty>(), _DeviceProperty::name, post);
			
			// Store the device property
			cosmos_properties[GetDevicePropertyID<_DeviceProperty>()] = new_property;
			
			// Set the property value using the DeviceProperty offset
			// Beware: pointer magic ahead!
			*(typename _DeviceProperty::ValueType*)((void*)&agent->cinfo->device[cindex] + _DeviceProperty::offset) = value;
		}
		
		template <typename T>
		void SetProperty(const std::string &name, T value) {
			
			// Check if the property is already set
			if ( custom_properties.find(name) != custom_properties.end() ) {
				// Delete the old value
				free(custom_properties[name]);
			}
			
			// Store the new value
			custom_properties[name] = malloc(sizeof(T));
			*(T*)custom_properties[name] = value;
		}
		
		template <typename _DeviceProperty>
		typename _DeviceProperty::ValueType GetProperty() {
			
			if ( !PropertyExists<_DeviceProperty>() ) {
				printf("Attempted to retrieve property '%s', which does not exist\n", _DeviceProperty::name);
				return typename _DeviceProperty::ValueType();
				//throw NonExistentPropertyException(_DeviceProperty::name);
			}
			else {
				// Get the property value using the DeviceProperty offset
				// Beware: pointer magic ahead!
				return *(typename _DeviceProperty::ValueType*)((void*)&agent->cinfo->device[cindex] + _DeviceProperty::offset);
			}
		}
		
		template <typename T>
		T GetProperty(const std::string &name) {
			if ( !PropertyExists<T>(name) ) {
				printf("Attempted to retrieve property '%s', which does not exist\n", name.c_str());
				//throw NonExistentPropertyException(name);
				return T();
			}
			else {
				return *(T*)custom_properties[name];
			}
		}
		
		void StorePostedProperties(std::vector<std::string> &keys) {
			for (auto property_pair : cosmos_properties) {
				if ( property_pair.second.post ) {
					keys.push_back(property_pair.second.cosmos_name);
				}
			}
		}
		
		void DebugPrint(bool print_all = false) const {
			
			// Print the COSMOS properties
			for (auto property_pair : cosmos_properties) {
				if ( print_all || property_pair.second.post )
					printf("|\t|\t| COSMOS Property '%s' (aka %s): %s\n",
						   property_pair.second.readable_name.c_str(),
						   property_pair.second.cosmos_name.c_str(),
						   property_pair.second.value_string.c_str());
			}
			
			// Print the custom properties
			for (auto property_pair : custom_properties) {
				printf("|\t|\t| Custom Property '%s'\n", property_pair.first.c_str());
			}
		}
		
		void GetDebugString(std::stringstream &ss, bool print_all = false) const {
			// Print the COSMOS properties
			for (auto property_pair : cosmos_properties) {
				if ( print_all || property_pair.second.post )
					ss << "|\t|\t| COSMOS Property '" << property_pair.second.readable_name <<
						  "' (aka " << property_pair.second.cosmos_name <<
						  "): " << property_pair.second.value_string << "\n";
			}
			
			// Print the custom properties
			for (auto property_pair : custom_properties) {
				ss << "|\t|\t| Custom Property '" << property_pair.first << "'\n";
			}
		}
		
		/**
		 * @brief Returns the name of this device
		 * @return The device name
		 */
		inline const std::string& GetName() const {
			return device_name;
		}
		
		
	protected:
		DeviceType type;
		Agent *agent;
		std::string device_name;
		
		//! COSMOS piece index
		int pindex;
		//! COSMOS component index
		int cindex;
		//! COSMOS device index
		int dindex;
		
		//! Custom data for users to add, stored in dynamically-allocated memory
		//! to allow for flexible typing. Be careful!
		std::unordered_map<std::string, void*> custom_properties;
		
		//! A map of <property name, <property value string, cosmos name>>
		std::unordered_map<DevicePropertyID, Property> cosmos_properties;
		
		
		
		template <typename DeviceProperty>
		std::string GetCOSMOSPropertyName() {
			std::stringstream ss;
			ss << DeviceProperty::key << "_" << std::setw(3) << std::setfill('0') << dindex;
			return ss.str();
		}
	};
	
	
	
	
	class SimpleAgent : protected Agent {
	protected:
		struct ArgumentedRequestData {
			ArgumentedRequest callback;
		};
		
		struct NonArgumentedRequestData {
			NonArgumentedRequest callback;
		};
		
		class DeviceAlreadyExistsException : public std::exception {
		public:
			DeviceAlreadyExistsException(const std::string &device_name) {
				error_msg = "The device '" + device_name + "' already exists.";
			}
			
		  virtual const char* what() const throw() {
			return error_msg.c_str();
		  }
			
		private:
			std::string error_msg;
		};
		
		struct NodeProperty {
			std::string cosmos_name;
			std::string readable_name;
			std::string value_string;
			bool post;
			
			NodeProperty() {}
			NodeProperty(const std::string &value_string, const std::string &cosmos_name, const std::string &readable_name, bool post)
				: cosmos_name(cosmos_name), readable_name(readable_name), value_string(value_string), post(post) {}
		};
		
	public:
		
		/**
		 * @brief Creates a new simple agent with the given name
		 * @param name The agent name
		 * @param node The node name, defaults to 'cubesat'
		 */
		SimpleAgent(const std::string &name, std::string node = CUBESAT_NODE_NAME,
					bool crash_if_not_open = STRICT_MODE) : Agent(node, name) {
			
			SimpleAgent::instance = this;
			
			// Check if the agent started successfully
			if ( !IsOpen() ) {
				printf("Could not start agent '%s' on node '%s'\n", name.c_str(), node.c_str());
				
				if ( crash_if_not_open )
					exit(1);
			}
			else {
				AddRequest("print", _Request_DebugPrint, "Prints all added devices and requests");
			}
			
			this->loop_started = false;
			SetLoopPeriod(1);
		}
		//! Disallow copy construction for safety
		SimpleAgent(const SimpleAgent&) = delete;
		
		virtual ~SimpleAgent() {
			
			// Delete devices
			for (auto device_pair : devices)
				delete device_pair.second;
			
			devices.clear();
		}
		
		/**
		 * @brief Returns the original (more complex) version of this agent
		 * @return The original version of this agent
		 */
		inline Agent* GetComplexAgent() {
			// Just	upcast to the base class
			return (Agent*)this;
		}
		
		
		inline RemoteAgent FindAgent(const std::string &name, const std::string &node, float wait_sec = 2.0f, bool crash_if_failed = false) {
			beatstruc beat = this->find_agent(node, name, wait_sec);
			
			// Check if an error occurred
			if ( beat.utc == 0. || !beat.exists ) {
				printf("Could not find agent '%s' running on node '%s'\n", name.c_str(), node.c_str());
				
				if ( crash_if_failed )
					exit(1);
			}
			
			return RemoteAgent(node, name, GetComplexAgent(), beat);
		}
		
		inline RemoteAgent FindAgent(const std::string &name, bool crash_if_failed = false) {
			return FindAgent(name, this->nodeName, crash_if_failed);
		}
		
		
		/**
		 * @brief Checks if the agent has started successfully. This should be checked
		 * after creating a new SimpleAgent.
		 * @return True if the agent was created successfully
		 */
		inline bool IsOpen() {
			return this->cinfo != nullptr && this->running() && this->wait() >= 0;
		}
		
		inline void CrashIfNotOpen() {
			int status;
			
			// Crash if the agent isn't running
			if ( (status = this->wait()) < 0 ) {
				printf("Fatal: failed to open agent '%s' on node '%s': %s\n",
					   agentName.c_str(), nodeName.c_str(),
					   cosmos_error_string(status).c_str());
				exit(status);
			}
		}
		
		
		/**
		 * @brief Checks if a device with the given name already exists
		 * @param name The device name
		 * @return True if the device already exists
		 */
		inline bool DeviceExists(const std::string &name) const {
			return devices.find(name) != devices.end();
		}
		
		/**
		 * @brief Gets a device by name
		 * @param name The device name
		 * @return The device, or nullptr if it does not exist
		 */
		inline Device* GetDevice(const std::string &name) {
			
			// Make sure the device exists
			if ( DeviceExists(name) )
				return devices[name];
			else
				return nullptr;
		}
		
		/**
		 * @brief Checks if a request with the given name already exists
		 * @param request_name The request name
		 * @return True if the request already exists
		 */
		inline bool RequestExists(const std::string &request_name) {
			if ( argumented_requests.find(request_name) != argumented_requests.end() )
				return true;
			else
				return non_argumented_requests.find(request_name) != non_argumented_requests.end();
		}
		
		/**
		 * @brief Sets how long the agent should wait between iterations of the main loop (the period).
		 * If this method is not called, the period defaults to 1 second.
		 * @param period The period in seconds
		 */
		void SetLoopPeriod(double period) {
			this->cinfo->agent[0].aprd = period;
		}
		/**
		 * @brief Signals the beginning of an iteration of the main loop.
		 * @return True if the agent is running
		 */
		bool StartLoop() {
			// Check if the agent is actually running
			if ( !this->IsRunning() )
				return false;
			
			// Check if this is the first time this function has been called
			if ( !loop_started ) {
				loop_started = true;
				
				// Since this isn't the first iteration, idle until the next one should start
				this->finish_active_loop();
			}
			
			// Idle until the next loop should start
			this->finish_active_loop();
			
			// Update the node timestamp
			this->cinfo->node.utc = currentmjd();
			
			// Start the active loop
			this->start_active_loop();
			
			return true;
		}
		
		
		/**
		 * @brief Checks if the agent is currently running
		 * @return True if the agent is running
		 */
		bool IsRunning() {
			return this->running();
		}
		
		
		/**
		 * @brief Adds a request to this agent.
		 * @param request_name The name of the request
		 * @param request_callback The callback function
		 * @param synopsis A brief description of the request
		 * @param description A detailed description of the request
		 * @return True if successful
		 */
		bool AddRequest(const std::string &request_name, ArgumentedRequest request_callback, std::string synopsis = "", std::string description = "", bool crash_on_error = STRICT_MODE) {
			
			// Add the request proxy to the agent instead, using the given request name
			int status = this->add_request(request_name, RequestProxy, synopsis, description);
			
			// Check for errors
			if ( status < 0 ) {
				printf("Failed to add request (alias '%s'): %s\n", request_name.c_str(), cosmos_error_string(status).c_str());
				
				if ( crash_on_error )
					exit(status);
			}
			else {
				// Create a new request data object
				ArgumentedRequestData request;
				request.callback = request_callback;
				
				// Store the request data by name
				argumented_requests[request_name] = request;
			}
			
			return status >= 0;
		}
		
		/**
		 * @brief Adds a no-argument request to this agent.
		 * @param request_name The name of the request
		 * @param request_callback The callback function
		 * @param synopsis A brief description of the request
		 * @param description A detailed description of the request
		 * @return True if successful
		 */
		bool AddRequest(const std::string &request_name, NonArgumentedRequest request_callback, std::string synopsis = "", std::string description = "", bool crash_on_error = STRICT_MODE) {
			
			// Add the request proxy to the agent instead, using the given request name
			int status = this->add_request(request_name, RequestProxy, synopsis, description);
			
			// Check for errors
			if ( status < 0 ) {
				printf("Failed to add request (alias '%s'): %s\n", request_name.c_str(), cosmos_error_string(status).c_str());
				
				if ( crash_on_error )
					exit(status);
			}
			else {
				// Create a new request data object
				NonArgumentedRequestData request;
				request.callback = request_callback;
				
				// Store the request data by name
				non_argumented_requests[request_name] = request;
			}
			
			return status >= 0;
		}
		
		/**
		 * @brief Adds a request to this agent using alias names
		 * @param request_names The alias names of this request
		 * @param request_callback The callback function
		 * @param synopsis A brief description of the request
		 * @param description A detailed description of the request
		 * @return True if successful
		 */
		bool AddRequest(std::initializer_list<std::string> request_names, ArgumentedRequest request_callback, std::string synopsis = "", std::string description = "", bool crash_on_error = STRICT_MODE) {
			
			// Add aliases for the request
			bool success = true;
			
			// Add the requests
			// Agh why doesn't std::initializer_list have a subscript operator??
			int i = 0;
			std::string first_name;
			for (const std::string &name : request_names) {
				// After the first request, set the synopsis as 'Alias of ...'
				if ( i == 0 ) {
					success = success && AddRequest(name, request_callback, synopsis, description, crash_on_error);
					first_name = name;
				}
				else
					success = success && AddRequest(name, request_callback, "Alias of " + first_name, "", crash_on_error);
				
				++i;
			}
			
			return success;
		}
		
		bool AddRequest(std::initializer_list<std::string> request_names, NonArgumentedRequest request_callback, std::string synopsis = "", std::string description = "", bool crash_on_error = STRICT_MODE) {
			// Add aliases for the request
			bool success = true;
			
			// Add the requests
			// Agh why doesn't std::initializer_list have a subscript operator??
			int i = 0;
			std::string first_name;
			for (const std::string &name : request_names) {
				// After the first request, set the synopsis as 'Alias of ...'
				if ( i == 0 ) {
					success = success && AddRequest(name, request_callback, synopsis, description, crash_on_error);
					first_name = name;
				}
				else
					success = success && AddRequest(name, request_callback, "Alias of " + first_name, "", crash_on_error);
				
				++i;
			}
			
			return success;
		}
		
		template <typename _DeviceType>
		Device* NewDevice(const std::string &name, bool crash_on_error = STRICT_MODE) {
			
			if ( DeviceExists(name) ) {
				printf("The device '%s' already exists\n", name.c_str());
				throw DeviceAlreadyExistsException(name);
			}
			else {
				
				// Create the piece
				int pindex = json_createpiece(this->cinfo, name, _DeviceType::type);
				
				// Check if the piece was succesfully created
				if ( pindex < 0 ) {
					
					printf("Failed to add %s device named '%s': %s\n", GetDeviceTypeString(_DeviceType::type), name.c_str(), cosmos_error_string(pindex).c_str());
					if ( crash_on_error )
						exit(pindex);
					
					return nullptr;
				}
				
				// Get COSMOS indices
				int cindex = this->cinfo->pieces[pindex].cidx;
				int dindex = this->cinfo->device[cindex].all.didx;
				
				// Create the new device
				Device *device = new Device(GetComplexAgent(), _DeviceType::type, name, pindex, cindex, dindex);
				devices[name] = device;
				
				return device;
			}
		}
		
		
		/**
		 * @brief Updates this agent's state of health message using
		 * the list of posted properties.
		 */
		void FinalizeDevices() {
			
			// Add the posted properties from each device
			std::vector<std::string> keys;
			for (auto device_pair : devices) {
				device_pair.second->StorePostedProperties(keys);
			}
			
			
			// Format the state of health string using a stringstream
			std::stringstream ss;
			
			// Add all posted properties
			ss << "{";
			for (const std::string &key : keys) {
				ss << "\"" << key << "\",";
			}
			for (auto node_property_pair : node_properties) {
				if ( node_property_pair.second.post )
					ss << "\"" << node_property_pair.second.cosmos_name << "\", ";
			}
			ss << "}";
			
			// Set the state of health string
			this->set_sohstring(ss.str());
		}
		
		/**
		 * @brief Returns an argumented request callback function
		 * @param name The request name
		 * @return The request callback, or nullptr if it does not exist
		 */
		inline ArgumentedRequest GetArgumentedRequest(const std::string &name) {
			if ( argumented_requests.find(name) == argumented_requests.end() )
				return nullptr;
			
			return argumented_requests[name].callback;
		}
		
		/**
		 * @brief Returns a non-argumented request callback function
		 * @param name The request name
		 * @return The request callback, or nullptr if it does not exist
		 */
		inline NonArgumentedRequest GetNonArgumentedRequest(const std::string &name) {
			if ( non_argumented_requests.find(name) == non_argumented_requests.end() )
				return nullptr;
			
			return non_argumented_requests[name].callback;
		}
		
		template <typename _NodeProperty>
		void AddNodeProperty(typename _NodeProperty::ValueType value = _NodeProperty::ValueType()) {
			SetNodeProperty<_NodeProperty>(value, true);
		}
		
		template <typename _NodeProperty>
		void SetNodeProperty(typename _NodeProperty::ValueType value, bool post = false) {
			
			// Check if there's an old value set and if so, take its 'post' value
			if ( node_properties.find(GetDevicePropertyID<_NodeProperty>()) != node_properties.end() ) {
				post = node_properties[GetDevicePropertyID<_NodeProperty>()].post;
			}
			
			// Create the property
			NodeProperty new_property(PropertyValueString<typename _NodeProperty::ValueType>::ToString(value), _NodeProperty::key, _NodeProperty::name, post);
			
			// Store the device property
			node_properties[GetDevicePropertyID<_NodeProperty>()] = new_property;
			
			// Set the property value using the NodeProperty offset
			// Beware: pointer magic ahead!
			*(typename _NodeProperty::ValueType*)((void*)&this->cinfo->node + _NodeProperty::offset) = value;
		}
		
		template <typename _NodeProperty>
		typename _NodeProperty::ValueType GetNodeProperty() {
			
			if ( node_properties.find(GetDevicePropertyID<_NodeProperty>()) == node_properties.end() ) {
				printf("Attempted to retrieve nonexistent node property '%s'\n", _NodeProperty::name);
				return _NodeProperty::ValueType();
			}
			
			// Get the property value using the NodeProperty offset
			// Beware: pointer magic ahead!
			return *(typename _NodeProperty::ValueType*)((void*)&this->cinfo->node + _NodeProperty::offset);
		}
		
		
		
		/**
		 * @brief Prints the COSMOS names of all devices added
		 */
		void DebugPrint(bool print_all = false) const {
			
			// Print all devices
			printf("Devices\n");
			for (auto device_pair : devices) {
				printf("|\t| Device '%s'\n", device_pair.first.c_str());
				device_pair.second->DebugPrint(print_all);
			}
			
			// Print all requests
			printf("Requests\n");
			for (auto request_pair : argumented_requests)
				printf("|\t| Request '%s'\n", request_pair.first.c_str());
			for (auto request_pair : non_argumented_requests)
				printf("|\t| Request '%s'\n", request_pair.first.c_str());
			
			// Print all node properties
			printf("Node Properties\n");
			for (auto node_property_pair : node_properties) {
				if ( print_all || node_property_pair.second.post )
					printf("|\t| Property '%s' (aka %s): %s\n",
						   node_property_pair.second.readable_name.c_str(),
						   node_property_pair.second.cosmos_name.c_str(),
						   node_property_pair.second.value_string.c_str());
			}
			
		}
		
		/**
		 * @brief Prints the COSMOS names of all devices added
		 */
		std::string GetDebugString(bool print_all = false) const {
			
			std::stringstream ss;
			
			// Print all devices
			ss << "Devices\n";
			for (auto device_pair : devices) {
				ss << "|\t| Device '" << device_pair.first << "'\n";
				device_pair.second->GetDebugString(ss, print_all);
			}
			
			// Print all requests
			ss << "Requests\n";
			for (auto request_pair : argumented_requests) {
				ss << "|\t| Request '" << request_pair.first << "'\n";
			}
			for (auto request_pair : non_argumented_requests) {
				ss << "|\t| Request '" << request_pair.first << "'\n";
			}
			
			// Print all node properties
			ss << "Node Properties\n";
			for (auto node_property_pair : node_properties) {
				if ( print_all || node_property_pair.second.post )
					ss << "|\t| Property '" << node_property_pair.second.readable_name
						  << "' (aka " << node_property_pair.second.cosmos_name
						  << "): " << node_property_pair.second.value_string << "\n";
			}
			
			return ss.str();
		}
		
		
		inline static SimpleAgent* GetInstance() {
			return SimpleAgent::instance;
		}
		
		
		inline void Shutdown() {
			this->shutdown();
		}
		
		
	protected:
		//! A table of user-defined request callbacks (those which have arguments)
		std::unordered_map<std::string, ArgumentedRequestData> argumented_requests;
		//! A table of user-defined request callbacks (this without arguments)
		std::unordered_map<std::string, NonArgumentedRequestData> non_argumented_requests;
		//! A table of user-defined devices
		std::unordered_map<std::string, Device*> devices;
		//! A table of user-defined node properties
		std::unordered_map<DevicePropertyID, NodeProperty> node_properties;
		//! Indicates whether the first iteration of the main loop has occurred
		bool loop_started;
		
		
		static SimpleAgent *instance;
		
	};
	
	/**
	 * @brief A request which redirects itself to requests added to a SimpleAgent
	 * @param request The request string. This begins with the request name, so we can look up the callback
	 * @param response The output response string
	 * @param agent_ The (complex) agent calling this request
	 * @return 0 upon success, or a negative value on failure
	 */
	inline int32_t RequestProxy(char *request, char* response, Agent *agent_) {
		
		// Split the request string into arguments
		std::vector<std::string> arguments;
		istringstream iss(request);
		string arg;
		
		while ( getline(iss, arg, ' ') )
			arguments.push_back(arg);
		
		// Get the request name and remove it from the argument list
		std::string request_name = arguments[0];
		arguments.erase(arguments.begin());
		
		// Get the SimpleAgent version so we can find the proper request callback
		SimpleAgent *agent = SimpleAgent::GetInstance();
		
		ArgumentedRequest argumented_callback;
		NonArgumentedRequest non_argumented_callback;
		std::string response_str;
		
		// Check if an argumented version of the request exists
		if ( (argumented_callback = agent->GetArgumentedRequest(request_name)) != nullptr ) {
			
			response_str = argumented_callback(arguments);
		}
		// Check if a non-argumented version of the request exists
		else if ( (non_argumented_callback = agent->GetNonArgumentedRequest(request_name)) != nullptr ) {
			
			response_str = non_argumented_callback();
		}
		// If this occurs, the request does not exist
		else {
			
			// Indicate an error since the user-defined request doesn't exist
			sprintf(response, "User-defined request '%s' does not exist", request_name.c_str());
			return -1;
		}
		
		// Print the response string to the output field
		sprintf(response, "%s", response_str.c_str());
		
		// Inidicate success if the output is not empty
		return !response_str.empty();
	}
	
	
	inline std::string _Request_DebugPrint(std::vector<std::string> args) {
		if ( args.size() == 1 ) {
			if ( args[0] == "all" )
				return SimpleAgent::GetInstance()->GetDebugString(true);
		}
		return SimpleAgent::GetInstance()->GetDebugString();
	}
	
	
	
	
	
	
	}

#endif
