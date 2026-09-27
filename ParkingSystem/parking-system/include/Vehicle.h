#ifndef VEHICLE_H
#define VEHICLE_H
#include <string>

enum class VehicleSize
{
    STANDARD,
    LARGE
};

class Vehicle
{
private:
    std::string registrationNO;
    VehicleSize vehiclesize;

public:
    Vehicle(const std::string &registrationNO,
            VehicleSize vehiclesize);

    std::string getRegistrationNo() const;
    VehicleSize getVehicleSize() const;
};

#endif
