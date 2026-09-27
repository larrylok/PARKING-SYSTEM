#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <vector>
#include "../include/ParkingSystem.h"
#include "../include/Database.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QTableWidget;
class QListWidget;
class QTimer;
class QGridLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Database &database, QWidget *parent = nullptr);

private slots:
    void onParkVehicle();
    void onExitSelectedVehicle();
    void refreshDisplays();

private:
    void buildUi();
    void buildSlotMap(QGridLayout *slotGrid);
    void refreshSlotMap();
    void showStatus(const QString &message, bool isError);

    ParkingSystem parkingSystem;

    QLabel *availableSpacesLabel;
    QLabel *waitingCountLabel;
    QLabel *statusLabel;

    QLineEdit *registrationInput;
    QComboBox *vehicleSizeInput;
    QPushButton *parkButton;

    QTableWidget *parkedVehiclesTable;
    QPushButton *exitButton;

    QListWidget *waitingQueueList;

    std::vector<QLabel *> slotTiles;

    QTimer *refreshTimer;
};

#endif
