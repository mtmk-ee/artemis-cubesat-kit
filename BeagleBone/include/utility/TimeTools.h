
#ifndef CUBESAT_TIME_TOOLS
#define CUBESAT_TIME_TOOLS


#include "support/timelib.h"


namespace cubesat {
	
	class Time {
	public:
		
		Time() : utc_mjd(currentmjd()) {
			
		}
		Time(double utc_mjd) : utc_mjd(utc_mjd) {
			
		}
		
		inline static Time Now() {
			return Time();
		}
		
		//! Converts this time to seconds
		inline double Seconds() const {
			return utc2unixseconds(utc_mjd);
		}
		//! Converts this time to minutes
		inline double Minutes() const {
			return Seconds() / 60.0;
		}
		//! Converts this time to hours
		inline double Hours() const {
			return Seconds() / 3600.0;
		}
		//! Converts this time to days
		inline double Days() const {
			return Seconds() / 86400.0;
		}
		
		//! Returns the modified Julian date (UTC)
		inline double GetModifiedJulianDate() const {
			return utc_mjd;
		}
		
		//! Returns the modified Julian date (UTC)
		inline operator double() const {
			return utc_mjd;
		}
		
		inline Time operator +(const Time &other) const {
			return Time(utc_mjd + other.utc_mjd);
		}
		inline Time operator -(const Time &other) const {
			return Time(utc_mjd - other.utc_mjd);
		}
		
	private:
		double utc_mjd;
	};
	
	
	class Timer {
	public:
		//! Starts the timer
		inline void Start() {
			start_time = Time::Now();
		}
		
		//! Returns the duration between when Timer::Start() was called,
		//! and when this function was called
		inline double Seconds() {
			return Time::Now().Seconds() - start_time.Seconds();
		}
		
	private:
		Time start_time;
	};
	
	
}

#endif
