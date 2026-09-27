#ifndef PARKING_SLOT_H
#define PARKING_SLOT_H

class ParkingSlot
{
private:
    int slotNumber;
    bool available;

public:
    ParkingSlot(int slotNumber);
    int getSlotNumber() const;
    bool isAvailable() const;
    void occupy();
    void release();
};

#endif
