#include <QApplication>
#include <QMessageBox>

#include "MainWindow.h"
#include "../include/Database.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Database database;

    if (!database.open())
    {
        QMessageBox::critical(nullptr, "Database Error", "Failed to open parking.db");
        return 1;
    }

    MainWindow window(database);
    window.show();

    return app.exec();
}
