#include "../include/Vehicle.h"

Vehicle::Vehicle(const std::string &registrationNO,
                 VehicleSize vehiclesize)
    : registrationNO(registrationNO),
      vehiclesize(vehiclesize)
{
}

std::string Vehicle::getRegistrationNo() const
{
    return registrationNO;
}

VehicleSize Vehicle::getVehicleSize() const
{
    return vehiclesize;
}
