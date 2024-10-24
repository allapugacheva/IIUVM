#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Windows.h>
#include "IO_DRIVER_HEADER.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

typedef struct _PCI_DEVICE_INFO {
    unsigned short vendorId;
    unsigned short deviceId;
} PCI_DEVICE_INFO;

typedef struct _PCI_DEVICES {
    PCI_DEVICE_INFO devices[256];
    unsigned int count;
} PCI_DEVICES;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool getPCIDevicesUsingIO(HANDLE& h);

    PCI_DEVICES pciDevices;
};
#endif // MAINWINDOW_H
