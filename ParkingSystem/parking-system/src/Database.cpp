#include "../include/Database.h"
#include <iostream>

Database::Database()
    : db(nullptr)
{
}
Database::~Database()
{
    close();
}
bool Database::open(const std::string &dbPath)
{
    int result = sqlite3_open(dbPath.c_str(), &db);
    if (result != SQLITE_OK)
    {
        std::cout << "Failed to open database!" << std::endl;
        return false;
    }
    std::cout << "Database connected successfully!" << std::endl;
    return true;
}
void Database::close()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}
bool Database::execute(const std::string &sql)
{
    char *errorMessage = nullptr;
    int result = sqlite3_exec(db, sql.c_str(),
                              nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        std::cout << "SQL error: "
                  << errorMessage
                  << std::endl;

        sqlite3_free(errorMessage);

        return false;
    }
    return true;
}
int Database::addVehicle(
    const std::string &registrationNo,
    const std::string &vehicleSize)
{
    // Reuse the vehicle record if this registration is already in the database.
    int existingId = getVehicleId(registrationNo);
    if (existingId != -1)
    {
        return existingId;
    }

    std::string sql =
        "INSERT INTO vehicles (registration_no, vehicle_size) "
        "VALUES (?, ?);";

    sqlite3_stmt *statement;

    int result = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        -1,
        &statement,
        nullptr);

    if (result != SQLITE_OK)
    {
        std::cout << "Failed to prepare vehicle insert."
                  << std::endl;

        return -1;
    }

    sqlite3_bind_text(
        statement,
        1,
        registrationNo.c_str(),
        -1,
        SQLITE_TRANSIENT);

    sqlite3_bind_text(
        statement,
        2,
        vehicleSize.c_str(),
        -1,
        SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE)
    {
        std::cout << "Failed to add vehicle."
                  << std::endl;

        sqlite3_finalize(statement);

        return -1;
    }

    sqlite3_finalize(statement);

    return static_cast<int>(sqlite3_last_insert_rowid(db));
}
int Database::getVehicleId(
    const std::string &registrationNo)
{
    std::string sql =
        "SELECT vehicle_id "
        "FROM vehicles "
        "WHERE registration_no = ?;";

    sqlite3_stmt *statement;

    int result = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        -1,
        &statement,
        nullptr);

    if (result != SQLITE_OK)
    {
        return -1;
    }

    sqlite3_bind_text(
        statement,
        1,
        registrationNo.c_str(),
        -1,
        SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    if (result == SQLITE_ROW)
    {
        int vehicleId =
            sqlite3_column_int(statement, 0);

        sqlite3_finalize(statement);

        return vehicleId;
    }

    sqlite3_finalize(statement);

    return -1;
}
bool Database::completeParkingSession(
    int vehicleId,
    const std::string &exitTime,
    long long durationMinutes,
    long long amount)
{
    std::string sql =
        "UPDATE parking_sessions "
        "SET exit_time = ?, "
        "duration_minutes = ?, "
        "amount = ?, "
        "payment_status = 'PAID' "
        "WHERE vehicle_id = ? "
        "AND exit_time IS NULL;";

    sqlite3_stmt *statement;

    int result = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        -1,
        &statement,
        nullptr);

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(
        statement,
        1,
        exitTime.c_str(),
        -1,
        SQLITE_TRANSIENT);

    sqlite3_bind_int64(
        statement,
        2,
        durationMinutes);

    sqlite3_bind_int64(
        statement,
        3,
        amount);

    sqlite3_bind_int(
        statement,
        4,
        vehicleId);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}
bool Database::addPayment(
    int vehicleId,
    long long amount,
    const std::string &paymentMethod,
    const std::string &paymentTime)
{
    std::string sql =
        "INSERT INTO payments "
        "(session_id, amount, payment_method, payment_time, payment_status) "
        "SELECT session_id, ?, ?, ?, 'PAID' "
        "FROM parking_sessions "
        "WHERE vehicle_id = ? "
        "AND exit_time IS NOT NULL "
        "ORDER BY session_id DESC "
        "LIMIT 1;";

    sqlite3_stmt *statement;

    int result = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        -1,
        &statement,
        nullptr);

    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int64(
        statement,
        1,
        amount);

    sqlite3_bind_text(
        statement,
        2,
        paymentMethod.c_str(),
        -1,
        SQLITE_TRANSIENT);

    sqlite3_bind_text(
        statement,
        3,
        paymentTime.c_str(),
        -1,
        SQLITE_TRANSIENT);

    sqlite3_bind_int(
        statement,
        4,
        vehicleId);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}
bool Database::createParkingSession(
    int vehicleId,
    const std::string &entryTime)
{
    std::string sql =
        "INSERT INTO parking_sessions "
        "(vehicle_id, entry_time) "
        "VALUES (?, ?);";

    sqlite3_stmt *statement;

    int result = sqlite3_prepare_v2(
        db,
        sql.c_str(),
        -1,
        &statement,
        nullptr);

    if (result != SQLITE_OK)
    {
        std::cout << "Failed to prepare parking session insert."
                  << std::endl;

        return false;
    }

    sqlite3_bind_int(
        statement,
        1,
        vehicleId);

    sqlite3_bind_text(
        statement,
        2,
        entryTime.c_str(),
        -1,
        SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        std::cout << "Failed to create parking session."
                  << std::endl;

        return false;
    }

    return true;
}
sqlite3 *Database::getConnection()
{
    return db;
}