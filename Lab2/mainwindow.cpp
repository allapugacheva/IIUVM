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
        PCI_DEVICES pciDevices;

        DWORD returned;
        BOOL success = DeviceIoControl(hDevice,
                                       IOCTL_PCI_READ_CONFIG,
                                       nullptr,
                                       0,
                                       &pciDevices,
                                       sizeof(pciDevices),
                                       &returned,
                                       nullptr);

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
