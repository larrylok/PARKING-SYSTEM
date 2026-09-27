#ifndef DATABASE_H
#define DATABASE_H
#include <sqlite3.h>
#include <string>

class Database
{
private:
    sqlite3 *db;

public:
    Database();
    ~Database();
    bool open(const std::string &dbPath = "parking.db");
    void close();
    bool execute(const std::string &sql);
    int addVehicle(const std::string &registrationNo,
                   const std::string &vehicleSize);
    int getVehicleId(const std::string &registrationNo);
    bool completeParkingSession(
        int vehicleId,
        const std::string &exitTime,
        long long durationMinutes,
        long long amount);
    bool addPayment(
        int vehicleId,
        long long amount,
        const std::string &paymentMethod,
        const std::string &paymentTime);
    bool createParkingSession(
        int vehicleId,
        const std::string &entryTime);
    sqlite3 *getConnection();
};

#endif