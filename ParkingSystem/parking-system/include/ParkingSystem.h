#ifndef PARKING_SYSTEM_H
#define PARKING_SYSTEM_H
#include "ParkingSlot.h"
#include "Vehicle.h"
#include "ParkingRecord.h"
#include "Database.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <iomanip>
#include <sstream>

// Gets the current date and time.
std::string getCurrentTimestamp();

struct ParkedVehicle
{
    Vehicle vehicle;
    int firstSlot;
    std::chrono::system_clock::time_point entryTime;
};

// Information used when displaying parked vehicles.
struct ParkedVehicleInfo
{
    std::string registrationNo;
    std::string sizeLabel;
    int firstSlot;
    int secondSlot;
    long long durationMinutes;
    long long fee;
};

// Information used by the GUI to show each slot.
struct SlotStatus
{
    int slotNumber;
    bool available;
    std::string registrationNo;
};

class ParkingSystem
{
private:
    std::vector<ParkingSlot> parkingSlots;
    std::unordered_map<std::string, ParkingRecord> parkedVehicles;
    std::queue<Vehicle> waitingVehicles;
    Database *database;

public:
    // constructor declaration
    ParkingSystem(Database &database);
    // getter declaration
    int getAvailableSpaces() const;
    ParkingSlot *findAvailableSlot();
    bool parkVehicle(const Vehicle &vehicle);
    int findLargeVehicleSlot();
    void addToWaitingQueue(const Vehicle &vehicle);
    Vehicle getNextWaitingVehicle();
    bool hasWaitingVehicles() const;
    long long getVehicleDuration(const std::string &registrationNo) const;
    long long getVehicleFee(const std::string &registrationNo) const;
    long long calculateFee(long long durationMinutes) const;
    bool processVehicleEntry(const Vehicle &vehicle);
    int getVehicleWaitingCount() const;
    bool exitVehicle(const std::string &registrationNo, bool confirmedPayment);

    bool fillOneAvailableSlot();

    // Information used by the GUI
    std::vector<ParkedVehicleInfo> getParkedVehiclesInfo() const;
    std::vector<std::string> getWaitingRegistrations() const;
    // Status of all parking slots for the GUI
    std::vector<SlotStatus> getAllSlotStatuses() const;
};

#endif
