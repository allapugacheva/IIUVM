#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    HANDLE hDevice = CreateFile(L"\\\\.\\PCIDriver",
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                0,
                                nullptr);

    if (hDevice == INVALID_HANDLE_VALUE) {

        QMessageBox::critical(this, "Ошибка", "Не удалось связаться с драйвером");

        CloseHandle(hDevice);
    } else {

        //bool success = getPCIDevicesUsingReadConfig(hDevice);
        bool success = getPCIDevicesUsingIO(hDevice);

        if (success) {

            QStandardItemModel* model = new QStandardItemModel(pciDevices.count, 2);
            QStandardItem* item;

            model->setHeaderData(0, Qt::Horizontal, "Device Id");
            model->setHeaderData(1, Qt::Horizontal, "Vendor Id");

            this->ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

            for(int i = 0; i<pciDevices.count; i++) {

                item = new QStandardItem(QString::number(pciDevices.devices[i].deviceId, 16).toUpper());
                model->setItem(i, 0, item);

                item = new QStandardItem(QString::number(pciDevices.devices[i].vendorId, 16).toUpper());
                model->setItem(i, 1, item);
            }

            this->ui->tableView->setModel(model);
        } else
            QMessageBox::critical(this, "Ошибка", "Не удалось получить данные от драйвера");

        CloseHandle(hDevice);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::getPCIDevicesUsingIO(HANDLE& h)
{
    DWORD returned;
    unsigned short vendorID, deviceID;
    int count = 0;

    WRITE_ADDR_DATA wad;
    wad.addr = (unsigned long*)0xCF8;

    READ_ADDR ra;
    ra.addr = (unsigned long*)0xCFC;

    READ_DATA rd;

    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {

                wad.data = 0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (0x00);
                if(!DeviceIoControl(h,
                                    IOCTL_WRITE_DATA,
                                    &wad,
                                    sizeof(wad),
                                    nullptr,
                                    0,
                                    &returned,
                                    nullptr))
                    return false;

                if(!DeviceIoControl(h,
                                    IOCTL_READ_DATA,
                                    &ra,
                                    sizeof(ra),
                                    &rd,
                                    sizeof(rd),
                                    &returned,
                                    nullptr))
                    return false;

                vendorID = rd.data & 0xFFFF;
                deviceID = (rd.data >> 16) & 0xFFFF;

                if (vendorID != 0xFFFF) {
                    pciDevices.devices[count].deviceId = deviceID;
                    pciDevices.devices[count].vendorId = vendorID;
                    count++;
                }
            }
        }
    }

    pciDevices.count = count;
    return true;
}

bool MainWindow::getPCIDevicesUsingReadConfig(HANDLE& h)
{
    DWORD returned;
    return DeviceIoControl(h,
                           IOCTL_PCI_READ_CONFIG,
                           nullptr,
                           0,
                           &pciDevices,
                           sizeof(pciDevices),
                           &returned,
                           nullptr);
}
