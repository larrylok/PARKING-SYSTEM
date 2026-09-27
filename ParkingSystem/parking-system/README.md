# Parking Management System

This is a C++ parking management system for a parking lot with 100 spaces.

It supports:
- Standard vehicles (1 space)
- Large vehicles (2 adjacent spaces)
- Vehicle entry and exit
- A waiting queue when the lot is full
- Parking time and fee calculation
- SQLite database records
- A simple console version
- A Qt desktop GUI

## Project structure

```text
include/     Header files for the main classes
src/         C++ source files and the console program
gui/         Qt GUI files
third_party/ SQLite source files used by the project
parking.db   SQLite database used by the program
```

## How to build

The project uses CMake and C++17.

```text
mkdir build
cd build
cmake ..
cmake --build .
```

The console program can run without Qt. If Qt5 or Qt6 is installed, CMake also builds the GUI program.

## Programs

`parking_console` is the console version.

`parking_gui` is the desktop version. It shows the parking spaces, parked vehicles, fees and waiting queue.

## Parking fees

- First 30 minutes: Free
- 31 to 60 minutes: KSh 100
- 61 to 120 minutes: KSh 200
- After 2 hours: KSh 50 for each additional hour started

The database keeps the vehicle, parking session and payment information.

## Notes

The database file included with the project is kept empty so the program starts with a clean parking lot.

If the GUI is not needed, the console program is enough to test the main parking system.
