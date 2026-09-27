#ifndef PARKING_RECORD_H
#define PARKING_RECORD_H
#include "Vehicle.h"
#include <chrono>

class ParkingRecord
{
private:
    Vehicle vehicle;
    int firstSlot;
    int secondSlot;
    std::chrono::system_clock::time_point entryTime;

public:
    ParkingRecord(const Vehicle &vehicle,
                  int firstSlot, int secondSlot = -1);

    Vehicle getVehicle() const;
    int getFirstSlot() const;
    int getSecondSlot() const;
    std::chrono::system_clock::time_point getEntryPoint() const;
};

#endif
