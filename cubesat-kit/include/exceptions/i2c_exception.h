#ifndef I2C_EXCEPTION_AID_H
#define I2C_EXCEPTION_AID_H

#include <exception>
#include <iostream>
#include <string>

namespace cubesat {
    using namespace std;

    class I2CSetupException : public exception {
    public:
        I2CSetupException( string errMessage ):errMessage_(errMessage){}
        const char* what() const throw() { return errMessage_.c_str( ); }

    private:
        string errMessage_;
    };
}

#endif
