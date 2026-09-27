#include <iostream>
#include <limits>

#include "../include/Vehicle.h"
#include "../include/ParkingSlot.h"
#include "../include/ParkingSystem.h"
#include "../include/Database.h"

namespace
{
void clearInputLine()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void printMenu()
{
    std::cout << "\n==============================\n";
    std::cout << " PARKING MANAGEMENT SYSTEM\n";
    std::cout << "==============================\n";
    std::cout << "1. Park a vehicle\n";
    std::cout << "2. Exit a vehicle\n";
    std::cout << "3. View available spaces\n";
    std::cout << "4. View parked vehicles\n";
    std::cout << "5. View waiting queue\n";
    std::cout << "6. Quit\n";
    std::cout << "Choose an option: ";
}

void handlePark(ParkingSystem &parkingSystem)
{
    std::string registration;
    std::cout << "Registration number: ";
    std::getline(std::cin, registration);

    int sizeChoice = 0;
    std::cout << "Vehicle size (1 = Standard, 2 = Large): ";
    std::cin >> sizeChoice;
    clearInputLine();

    VehicleSize size = (sizeChoice == 2) ? VehicleSize::LARGE : VehicleSize::STANDARD;
    Vehicle vehicle(registration, size);

    bool parked = parkingSystem.processVehicleEntry(vehicle);

    if (parked)
    {
        std::cout << "Parked " << registration << ".\n";
        return;
    }

    bool nowWaiting = false;
    for (const std::string &reg : parkingSystem.getWaitingRegistrations())
    {
        if (reg == registration)
        {
            nowWaiting = true;
            break;
        }
    }

    if (nowWaiting)
    {
        std::cout << "No slot available. " << registration << " added to the waiting queue.\n";
    }
    else
    {
        std::cout << "Could not park " << registration << ". It may already be active.\n";
    }
}

void handleExit(ParkingSystem &parkingSystem)
{
    std::string registration;
    std::cout << "Registration number to exit: ";
    std::getline(std::cin, registration);

    long long fee = parkingSystem.getVehicleFee(registration);

    if (fee == -1)
    {
        std::cout << registration << " is not currently parked.\n";
        return;
    }

    std::cout << "Fee due: KSh " << fee << "\n";
    std::cout << "Confirm payment and exit? (y/n): ";

    std::string confirmation;
    std::getline(std::cin, confirmation);

    bool confirmed = (!confirmation.empty() && (confirmation[0] == 'y' || confirmation[0] == 'Y'));

    bool exited = parkingSystem.exitVehicle(registration, confirmed);

    if (exited)
    {
        std::cout << registration << " exited. Fee collected: KSh " << fee << ".\n";
    }
    else
    {
        std::cout << "Exit not completed for " << registration << ".\n";
    }
}

void handleViewParked(const ParkingSystem &parkingSystem)
{
    std::vector<ParkedVehicleInfo> parked = parkingSystem.getParkedVehiclesInfo();

    if (parked.empty())
    {
        std::cout << "No vehicles currently parked.\n";
        return;
    }

    std::cout << "\nRegistration\tSize\tSlot(s)\tDuration(min)\tFee(KSh)\n";
    for (const ParkedVehicleInfo &info : parked)
    {
        std::cout << info.registrationNo << "\t" << info.sizeLabel << "\t";
        std::cout << info.firstSlot;
        if (info.secondSlot != -1)
        {
            std::cout << " & " << info.secondSlot;
        }
        std::cout << "\t" << info.durationMinutes << "\t" << info.fee << "\n";
    }
}

void handleViewWaiting(const ParkingSystem &parkingSystem)
{
    std::vector<std::string> waiting = parkingSystem.getWaitingRegistrations();

    if (waiting.empty())
    {
        std::cout << "No vehicles waiting.\n";
        return;
    }

    std::cout << "Waiting queue:\n";
    for (const std::string &reg : waiting)
    {
        std::cout << "  " << reg << "\n";
    }
}
}

int main()
{
    Database database;

    if (!database.open())
    {
        return 1;
    }

    ParkingSystem parkingSystem(database);

    bool running = true;

    while (running)
    {
        printMenu();

        int choice = 0;
        std::cin >> choice;
        clearInputLine();

        switch (choice)
        {
        case 1:
            handlePark(parkingSystem);
            break;
        case 2:
            handleExit(parkingSystem);
            break;
        case 3:
            std::cout << "Available spaces: " << parkingSystem.getAvailableSpaces() << " / 100\n";
            break;
        case 4:
            handleViewParked(parkingSystem);
            break;
        case 5:
            handleViewWaiting(parkingSystem);
            break;
        case 6:
            running = false;
            break;
        default:
            std::cout << "Invalid option.\n";
            break;
        }
    }

    std::cout << "Goodbye.\n";
    return 0;
}