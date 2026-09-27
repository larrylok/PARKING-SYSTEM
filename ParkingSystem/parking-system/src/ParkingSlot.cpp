#include "../include/ParkingSlot.h"

ParkingSlot::ParkingSlot(int slotNumber)
    : slotNumber(slotNumber),
      available(true)
{
}

int ParkingSlot::getSlotNumber() const
{
    return slotNumber;
}

bool ParkingSlot::isAvailable() const
{
    return available;
}

void ParkingSlot::occupy()
{
    available = false;
}

void ParkingSlot::release()
{
    available = true;
}
