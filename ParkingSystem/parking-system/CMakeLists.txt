cmake_minimum_required(VERSION 3.16)
project(ParkingSystem CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SQLite3 -----------------------------------------------------------
# Bundled as the amalgamation (third_party/sqlite3/sqlite3.c + .h) so the
# build needs no pkg-config, no vcpkg, no system install of SQLite3.
set(SQLITE3_AMALGAMATION ${CMAKE_SOURCE_DIR}/third_party/sqlite3/sqlite3.c)
if(NOT EXISTS ${SQLITE3_AMALGAMATION})
    message(FATAL_ERROR
        "third_party/sqlite3/sqlite3.c not found.\n"
        "Download the SQLite amalgamation from https://www.sqlite.org/download.html "
        "and copy sqlite3.c + sqlite3.h into third_party/sqlite3/ "
        "(see third_party/sqlite3/README.txt).")
endif()

add_library(sqlite3 STATIC ${SQLITE3_AMALGAMATION})
target_include_directories(sqlite3 PUBLIC third_party/sqlite3)
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(sqlite3 PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()


set(CORE_SOURCES
    src/Database.cpp
    src/ParkingSystem.cpp
    src/Vehicle.cpp
    src/ParkingSlot.cpp
    src/ParkingRecord.cpp
)

# Console program
add_executable(parking_console src/main.cpp ${CORE_SOURCES})
target_include_directories(parking_console PRIVATE include)
target_link_libraries(parking_console PRIVATE sqlite3)

# GUI (built when Qt is installed)
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Widgets QUIET)
endif()

if(Qt6_FOUND OR Qt5_FOUND)
    add_executable(parking_gui
        gui/main_gui.cpp
        gui/MainWindow.cpp
        ${CORE_SOURCES}
    )
    target_include_directories(parking_gui PRIVATE include gui)
    set_target_properties(parking_gui PROPERTIES AUTOMOC ON)
    target_link_libraries(parking_gui PRIVATE sqlite3)

    if(Qt6_FOUND)
        target_link_libraries(parking_gui PRIVATE Qt6::Widgets)
    else()
        target_link_libraries(parking_gui PRIVATE Qt5::Widgets)
    endif()

    add_custom_command(TARGET parking_gui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/parking.db $<TARGET_FILE_DIR:parking_gui>/parking.db)
else()
    message(STATUS "Qt5/Qt6 not found. Skipping the GUI target.")
endif()

# Keep parking.db next to whichever binary is run.
add_custom_command(TARGET parking_console POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/parking.db $<TARGET_FILE_DIR:parking_console>/parking.db)
