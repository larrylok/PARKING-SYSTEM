#include "../include/ParkingRecord.h"

ParkingRecord::ParkingRecord(const Vehicle &vehicle,
                             int firstSlot, int secondSlot)
    : vehicle(vehicle), firstSlot(firstSlot),
      secondSlot(secondSlot),
      entryTime(std::chrono::system_clock::now())
{
}

Vehicle ParkingRecord::getVehicle() const
{
    return vehicle;
}

int ParkingRecord::getFirstSlot() const
{
    return firstSlot;
}

int ParkingRecord::getSecondSlot() const
{
    return secondSlot;
}

std::chrono::system_clock::time_point ParkingRecord::getEntryPoint() const
{
    return entryTime;
}
