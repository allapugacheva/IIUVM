#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Windows.h>
#include "PCI_DRIVER_HEADER.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool getPCIDevicesUsingIO(HANDLE& h);
    bool getPCIDevicesUsingReadConfig(HANDLE& h);

    PCI_DEVICES pciDevices;
};
#endif // MAINWINDOW_H
