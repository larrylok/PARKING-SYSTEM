#include "../include/ParkingSystem.h"

// Gets the current date and time.
std::string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();

    std::time_t currentTime =
        std::chrono::system_clock::to_time_t(now);

    std::tm *localTime = std::localtime(&currentTime);

    std::stringstream timestamp;

    timestamp << std::put_time(
        localTime,
        "%Y-%m-%d %H:%M:%S");

    return timestamp.str();
}

// Set up the 100 parking spaces
ParkingSystem::ParkingSystem(Database &database)
    : database(&database)
{
    for (int i = 1; i <= 100; i++)
    {
        parkingSlots.push_back(ParkingSlot(i));
    }
}
// Count the free spaces
int ParkingSystem::getAvailableSpaces() const
{
    int availableSpaces = 0;
    for (const ParkingSlot &slot : parkingSlots)
    {
        if (slot.isAvailable())
        {
            availableSpaces++;
        }
    }
    return availableSpaces;
}
ParkingSlot *ParkingSystem::findAvailableSlot()
{
    for (ParkingSlot &slot : parkingSlots)
    {
        if (slot.isAvailable())
        {
            return &slot;
        }
    }
    return nullptr;
}
bool ParkingSystem::parkVehicle(const Vehicle &vehicle)
{
    // Do not allow the same registration to be parked twice.
    if (parkedVehicles.find(vehicle.getRegistrationNo()) != parkedVehicles.end())
    {
        return false;
    }

    if (vehicle.getVehicleSize() == VehicleSize::STANDARD)
    {

        ParkingSlot *slot = findAvailableSlot();
        if (slot != nullptr)
        {
            slot->occupy();
            parkedVehicles.insert({vehicle.getRegistrationNo(),
                                   ParkingRecord(vehicle, slot->getSlotNumber())});
            return true;
        }
        return false;
    }
    // A large vehicle needs two free slots next to each other.
    if (vehicle.getVehicleSize() == VehicleSize::LARGE)
    {
        int startIndex = findLargeVehicleSlot();

        if (startIndex != -1)
        {
            parkingSlots[startIndex].occupy();
            parkingSlots[startIndex + 1].occupy();
            int slotNumber = parkingSlots[startIndex].getSlotNumber();
            parkedVehicles.insert({vehicle.getRegistrationNo(),
                                   ParkingRecord(vehicle,
                                                 parkingSlots[startIndex].getSlotNumber(),
                                                 parkingSlots[startIndex + 1].getSlotNumber())});
            return true;
        }

        return false;
    }
    return false;
}
int ParkingSystem::findLargeVehicleSlot()
{
    for (std::size_t i = 0; i + 1 < parkingSlots.size(); i++)
    {
        if (parkingSlots[i].isAvailable() &&
            parkingSlots[i + 1].isAvailable())
        {
            return i;
        }
    }
    return -1;
}
void ParkingSystem::addToWaitingQueue(const Vehicle &vehicle)
{
    waitingVehicles.push(vehicle);
}
Vehicle ParkingSystem::getNextWaitingVehicle()
{
    Vehicle vehicle = waitingVehicles.front();
    waitingVehicles.pop();
    return vehicle;
}
bool ParkingSystem::hasWaitingVehicles() const
{
    return !waitingVehicles.empty();
}
long long ParkingSystem::getVehicleDuration(const std::string &registrationNo) const
{
    auto it = parkedVehicles.find(registrationNo);
    if (it == parkedVehicles.end())
    {
        return -1;
    }
    auto currentTime = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(currentTime - it->second.getEntryPoint());
    return duration.count();
}
long long ParkingSystem::getVehicleFee(const std::string &registrationNo) const
{
    long long duration = getVehicleDuration(registrationNo);
    if (duration == -1)
    {
        return -1;
    }
    return calculateFee(duration);
}
long long ParkingSystem::calculateFee(long long durationMinutes) const
{
    // 0–30 minutes
    if (durationMinutes <= 30)
    {
        return 0;
    }
    // 31–60 minutes
    if (durationMinutes <= 60)
    {
        return 100;
    }
    // 61–120 minutes
    if (durationMinutes <= 120)
    {
        return 200;
    }
    // Additional hour commenced
    long long fee = 200;
    long long remainingMinutes = durationMinutes - 120;
    long long additionalHours = (remainingMinutes + 59) / 60;
    fee += additionalHours * 50;
    return fee;
}
bool ParkingSystem::processVehicleEntry(const Vehicle &vehicle)
{
    if (parkedVehicles.find(vehicle.getRegistrationNo()) != parkedVehicles.end())
    {
        return false;
    }

    if (!parkVehicle(vehicle))
    {
        // No space right now, so keep the vehicle in the queue.
        waitingVehicles.push(vehicle);
        return false;
    }

    std::string vehicleSize;

    if (vehicle.getVehicleSize() == VehicleSize::STANDARD)
    {
        vehicleSize = "STANDARD";
    }
    else
    {
        vehicleSize = "LARGE";
    }

    int vehicleId =
        database->addVehicle(
            vehicle.getRegistrationNo(),
            vehicleSize);

    if (vehicleId == -1)
    {
        // the in-memory slot was already claimed by parkVehicle() above;
        // since the database write failed, undo that claim instead of
        // leaving a slot permanently locked with no matching record.
        auto it = parkedVehicles.find(vehicle.getRegistrationNo());
        if (it != parkedVehicles.end())
        {
            parkingSlots[it->second.getFirstSlot() - 1].release();
            if (it->second.getSecondSlot() != -1)
            {
                parkingSlots[it->second.getSecondSlot() - 1].release();
            }
            parkedVehicles.erase(it);
        }
        return false;
    }

    std::string entryTime =
        getCurrentTimestamp();

    bool sessionCreated =
        database->createParkingSession(
            vehicleId,
            entryTime);

    if (!sessionCreated)
    {
        // same rollback as above: the vehicle row now exists but the
        // session doesn't, so don't leave the slot claimed either.
        auto it = parkedVehicles.find(vehicle.getRegistrationNo());
        if (it != parkedVehicles.end())
        {
            parkingSlots[it->second.getFirstSlot() - 1].release();
            if (it->second.getSecondSlot() != -1)
            {
                parkingSlots[it->second.getSecondSlot() - 1].release();
            }
            parkedVehicles.erase(it);
        }
        return false;
    }

    return true;
}
// Number of vehicles waiting
int ParkingSystem::getVehicleWaitingCount() const
{
    return waitingVehicles.size();
}
bool ParkingSystem::exitVehicle(
    const std::string &registrationNo,
    bool paymentConfirmed)
{
    auto it =
        parkedVehicles.find(registrationNo);

    if (it == parkedVehicles.end())
    {
        return false;
    }

    if (!paymentConfirmed)
    {
        return false;
    }

    ParkingRecord record = it->second;

    long long duration =
        getVehicleDuration(registrationNo);

    long long fee =
        calculateFee(duration);

    int vehicleId =
        database->getVehicleId(registrationNo);

    if (vehicleId == -1)
    {
        return false;
    }

    std::string exitTime =
        getCurrentTimestamp();

    bool sessionUpdated =
        database->completeParkingSession(
            vehicleId,
            exitTime,
            duration,
            fee);

    if (!sessionUpdated)
    {
        return false;
    }

    bool paymentRecorded =
        database->addPayment(
            vehicleId,
            fee,
            "CASH",
            exitTime);

    if (!paymentRecorded)
    {
        return false;
    }

    parkingSlots[record.getFirstSlot() - 1].release();

    if (record.getSecondSlot() != -1)
    {
        parkingSlots[record.getSecondSlot() - 1].release();
    }

    parkedVehicles.erase(it);

    if (hasWaitingVehicles())
    {
        Vehicle nextVehicle =
            waitingVehicles.front();

        if (processVehicleEntry(nextVehicle))
        {
            waitingVehicles.pop();
        }
    }

    return true;
}
bool ParkingSystem::fillOneAvailableSlot()
{

    ParkingSlot *slot = findAvailableSlot();

    if (slot != nullptr)
    {
        slot->occupy();
        return true;
    }

    return false;
}
// Get information for the parked vehicle list
std::vector<ParkedVehicleInfo> ParkingSystem::getParkedVehiclesInfo() const
{
    std::vector<ParkedVehicleInfo> info;

    for (const auto &entry : parkedVehicles)
    {
        const ParkingRecord &record = entry.second;
        long long duration = getVehicleDuration(entry.first);
        long long fee = calculateFee(duration);

        std::string sizeLabel =
            (record.getVehicle().getVehicleSize() == VehicleSize::STANDARD)
                ? "STANDARD"
                : "LARGE";

        info.push_back(ParkedVehicleInfo{
            entry.first,
            sizeLabel,
            record.getFirstSlot(),
            record.getSecondSlot(),
            duration,
            fee});
    }

    return info;
}
// Get the status of each parking slot
std::vector<SlotStatus> ParkingSystem::getAllSlotStatuses() const
{
    std::unordered_map<int, std::string> slotToRegistration;
    for (const auto &entry : parkedVehicles)
    {
        const ParkingRecord &record = entry.second;
        slotToRegistration[record.getFirstSlot()] = entry.first;
        if (record.getSecondSlot() != -1)
        {
            slotToRegistration[record.getSecondSlot()] = entry.first;
        }
    }

    std::vector<SlotStatus> statuses;
    statuses.reserve(parkingSlots.size());

    for (const ParkingSlot &slot : parkingSlots)
    {
        SlotStatus status;
        status.slotNumber = slot.getSlotNumber();
        status.available = slot.isAvailable();

        if (!status.available)
        {
            auto it = slotToRegistration.find(status.slotNumber);
            if (it != slotToRegistration.end())
            {
                status.registrationNo = it->second;
            }
        }

        statuses.push_back(status);
    }

    return statuses;
}

// Get the registrations in the waiting queue
std::vector<std::string> ParkingSystem::getWaitingRegistrations() const
{
    std::vector<std::string> registrations;
    std::queue<Vehicle> copy = waitingVehicles;

    while (!copy.empty())
    {
        registrations.push_back(copy.front().getRegistrationNo());
        copy.pop();
    }

    return registrations;
}
