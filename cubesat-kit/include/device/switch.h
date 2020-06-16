
#ifndef CUBESAT_DEVICE_SWITCH
#define CUBESAT_DEVICE_SWITCH


namespace cubesat {
	
	
	class Switch {
	public:
		Switch();
		Switch(int pin);
		
		
		void SetHigh();
		void SetLow();
		
	private:
		int pin;
	};
	
	
}


#endif
