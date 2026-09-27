#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QMessageBox>
#include <QTimer>

namespace
{
constexpr int RefreshIntervalMs = 3000;
constexpr int SlotMapColumns = 10; // 100 slots -> 10x10 grid

enum TableColumn
{
    ColumnRegistration = 0,
    ColumnSize,
    ColumnSlots,
    ColumnDuration,
    ColumnFee,
    ColumnCount
};

QString slotTileStyle(bool available)
{
    return available
        ? "background-color: #c8e6c9; border: 1px solid #81c784; border-radius: 4px;"
        : "background-color: #ef9a9a; border: 1px solid #e57373; border-radius: 4px; font-weight: bold;";
}
}

MainWindow::MainWindow(Database &database, QWidget *parent)
    : QMainWindow(parent),
      parkingSystem(database)
{
    buildUi();
    refreshDisplays();

    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDisplays);
    refreshTimer->start(RefreshIntervalMs);
}

void MainWindow::buildUi()
{
    setWindowTitle("Parking Management System");
    resize(900, 780);

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *summaryLayout = new QHBoxLayout();
    availableSpacesLabel = new QLabel();
    waitingCountLabel = new QLabel();
    availableSpacesLabel->setStyleSheet("font-weight: bold;");
    waitingCountLabel->setStyleSheet("font-weight: bold;");
    summaryLayout->addWidget(availableSpacesLabel);
    summaryLayout->addStretch();
    summaryLayout->addWidget(waitingCountLabel);
    mainLayout->addLayout(summaryLayout);

    auto *mapGroup = new QGroupBox("Parking Lot Map (green = free, red = occupied)");
    auto *mapLayout = new QVBoxLayout();
    auto *slotGrid = new QGridLayout();
    slotGrid->setSpacing(3);
    buildSlotMap(slotGrid);
    mapLayout->addLayout(slotGrid);
    mapGroup->setLayout(mapLayout);
    mainLayout->addWidget(mapGroup);

    auto *entryGroup = new QGroupBox("Park a Vehicle");
    auto *entryLayout = new QFormLayout();
    registrationInput = new QLineEdit();
    registrationInput->setPlaceholderText("e.g. KDA 123A");
    vehicleSizeInput = new QComboBox();
    vehicleSizeInput->addItem("Standard", static_cast<int>(VehicleSize::STANDARD));
    vehicleSizeInput->addItem("Large", static_cast<int>(VehicleSize::LARGE));
    parkButton = new QPushButton("Park Vehicle");
    entryLayout->addRow("Registration No.", registrationInput);
    entryLayout->addRow("Vehicle Size", vehicleSizeInput);
    entryLayout->addRow(parkButton);
    entryGroup->setLayout(entryLayout);
    mainLayout->addWidget(entryGroup);
    connect(parkButton, &QPushButton::clicked, this, &MainWindow::onParkVehicle);

    auto *parkedGroup = new QGroupBox("Currently Parked");
    auto *parkedLayout = new QVBoxLayout();
    parkedVehiclesTable = new QTableWidget(0, ColumnCount);
    parkedVehiclesTable->setHorizontalHeaderLabels(
        {"Registration", "Size", "Slot(s)", "Duration (min)", "Fee (KSh)"});
    parkedVehiclesTable->horizontalHeader()->setStretchLastSection(true);
    parkedVehiclesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    parkedVehiclesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    parkedVehiclesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    exitButton = new QPushButton("Exit Selected Vehicle");
    parkedLayout->addWidget(parkedVehiclesTable);
    parkedLayout->addWidget(exitButton);
    parkedGroup->setLayout(parkedLayout);
    mainLayout->addWidget(parkedGroup);
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::onExitSelectedVehicle);

    auto *waitingGroup = new QGroupBox("Waiting Queue");
    auto *waitingLayout = new QVBoxLayout();
    waitingQueueList = new QListWidget();
    waitingLayout->addWidget(waitingQueueList);
    waitingGroup->setLayout(waitingLayout);
    mainLayout->addWidget(waitingGroup);

    statusLabel = new QLabel();
    mainLayout->addWidget(statusLabel);

    setCentralWidget(central);
}

void MainWindow::buildSlotMap(QGridLayout *slotGrid)
{
    std::vector<SlotStatus> statuses = parkingSystem.getAllSlotStatuses();
    int totalSlots = static_cast<int>(statuses.size());

    slotTiles.reserve(totalSlots);

    for (int i = 0; i < totalSlots; i++)
    {
        int slotNumber = statuses[static_cast<std::size_t>(i)].slotNumber;

        auto *tile = new QLabel(QString::number(slotNumber));
        tile->setAlignment(Qt::AlignCenter);
        tile->setFixedSize(48, 32);
        tile->setStyleSheet(slotTileStyle(true));
        tile->setToolTip("Free");

        int row = i / SlotMapColumns;
        int col = i % SlotMapColumns;
        slotGrid->addWidget(tile, row, col);

        slotTiles.push_back(tile);
    }
}

void MainWindow::refreshSlotMap()
{
    std::vector<SlotStatus> statuses = parkingSystem.getAllSlotStatuses();

    for (std::size_t i = 0; i < statuses.size() && i < slotTiles.size(); i++)
    {
        const SlotStatus &status = statuses[i];
        QLabel *tile = slotTiles[i];

        tile->setStyleSheet(slotTileStyle(status.available));

        if (status.available)
        {
            tile->setToolTip("Free");
        }
        else
        {
            tile->setToolTip(
                QString("Occupied by %1").arg(QString::fromStdString(status.registrationNo)));
        }
    }
}

void MainWindow::onParkVehicle()
{
    QString registration = registrationInput->text().trimmed();

    if (registration.isEmpty())
    {
        showStatus("Enter a registration number before parking.", true);
        return;
    }

    VehicleSize size = static_cast<VehicleSize>(vehicleSizeInput->currentData().toInt());
    Vehicle vehicle(registration.toStdString(), size);

    bool parked = parkingSystem.processVehicleEntry(vehicle);

    if (parked)
    {
        showStatus(QString("Parked %1.").arg(registration), false);
        registrationInput->clear();
    }
    else
    {
        bool nowWaiting = false;
        for (const std::string &reg : parkingSystem.getWaitingRegistrations())
        {
            if (reg == vehicle.getRegistrationNo())
            {
                nowWaiting = true;
                break;
            }
        }

        if (nowWaiting)
        {
            showStatus(QString("No slot available. %1 added to the waiting queue.").arg(registration), true);
        }
        else
        {
            showStatus(QString("Could not park %1. It may already be active, or a system error occurred.").arg(registration), true);
        }
        registrationInput->clear();
    }

    refreshDisplays();
}

void MainWindow::onExitSelectedVehicle()
{
    int row = parkedVehiclesTable->currentRow();

    if (row < 0)
    {
        showStatus("Select a parked vehicle to exit first.", true);
        return;
    }

    QString registration = parkedVehiclesTable->item(row, ColumnRegistration)->text();
    std::string registrationStd = registration.toStdString();

    long long fee = parkingSystem.getVehicleFee(registrationStd);

    auto answer = QMessageBox::question(
        this,
        "Confirm Payment",
        QString("Fee due for %1 is KSh %2. Confirm payment and exit?")
            .arg(registration)
            .arg(fee),
        QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes)
    {
        return;
    }

    bool exited = parkingSystem.exitVehicle(registrationStd, true);

    if (exited)
    {
        showStatus(QString("%1 exited. Fee collected: KSh %2.").arg(registration).arg(fee), false);
    }
    else
    {
        showStatus(QString("Failed to process exit for %1.").arg(registration), true);
    }

    refreshDisplays();
}

void MainWindow::refreshDisplays()
{
    refreshSlotMap();

    availableSpacesLabel->setText(
        QString("Available spaces: %1 / 100").arg(parkingSystem.getAvailableSpaces()));
    waitingCountLabel->setText(
        QString("Vehicles waiting: %1").arg(parkingSystem.getVehicleWaitingCount()));

    QString selectedRegistration;
    int currentRow = parkedVehiclesTable->currentRow();
    if (currentRow >= 0)
    {
        selectedRegistration = parkedVehiclesTable->item(currentRow, ColumnRegistration)->text();
    }

    std::vector<ParkedVehicleInfo> parked = parkingSystem.getParkedVehiclesInfo();
    parkedVehiclesTable->setRowCount(static_cast<int>(parked.size()));

    for (int row = 0; row < static_cast<int>(parked.size()); row++)
    {
        const ParkedVehicleInfo &info = parked[row];

        QString slotText = QString::number(info.firstSlot);
        if (info.secondSlot != -1)
        {
            slotText += QString(" & %1").arg(info.secondSlot);
        }

        parkedVehiclesTable->setItem(row, ColumnRegistration, new QTableWidgetItem(QString::fromStdString(info.registrationNo)));
        parkedVehiclesTable->setItem(row, ColumnSize, new QTableWidgetItem(QString::fromStdString(info.sizeLabel)));
        parkedVehiclesTable->setItem(row, ColumnSlots, new QTableWidgetItem(slotText));
        parkedVehiclesTable->setItem(row, ColumnDuration, new QTableWidgetItem(QString::number(info.durationMinutes)));
        parkedVehiclesTable->setItem(row, ColumnFee, new QTableWidgetItem(QString::number(info.fee)));

        if (!selectedRegistration.isEmpty() &&
            QString::fromStdString(info.registrationNo) == selectedRegistration)
        {
            parkedVehiclesTable->selectRow(row);
        }
    }

    waitingQueueList->clear();
    for (const std::string &reg : parkingSystem.getWaitingRegistrations())
    {
        waitingQueueList->addItem(QString::fromStdString(reg));
    }
}

void MainWindow::showStatus(const QString &message, bool isError)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(isError ? "color: #b00020;" : "color: #1b5e20;");
}
