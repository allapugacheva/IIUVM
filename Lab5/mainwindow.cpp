#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentSelectedDevice(-1), ui(new Ui::MainWindow)
{
    WNDCLASSEXW wx;
    ZeroMemory(&wx, sizeof(wx));
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = (WNDPROC)WindowProc;
    wx.lpszClassName = L"NONE";

    HWND hWnd = NULL;
    if (RegisterClassExW(&wx))
        hWnd = CreateWindowA("NONE", "DevNotifWnd", WS_ICONIC, 0, 0, CW_USEDEFAULT, 0, 0, NULL, GetModuleHandle(nullptr), NULL);

    DEV_BROADCAST_DEVICEINTERFACE_A filter;
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    RegisterDeviceNotificationA(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    ui->setupUi(this);
    this->setFixedSize(400, 500);

    connect(this, &MainWindow::devicesChanged, this, &MainWindow::updateDevices);

    QStandardItemModel* model = new QStandardItemModel();
    ui->eventsList->setModel(model);

    updateDevices();

    ui->extractButton->setEnabled(false);

    connect(ui->itemsList, &QListView::clicked, this, &MainWindow::onItemClicked);
    connect(ui->extractButton, &QPushButton::clicked, this, &MainWindow::onExtractButtonClicked);
}

MainWindow::~MainWindow()
{
    UnregisterDeviceNotification(hDevNotify);
    DestroyWindow(hwnd);

    delete ui;
}

void MainWindow::updateDevices()
{
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        &GUID_DEVINTERFACE_USB_DEVICE,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
        );

    if (deviceInfoSet == INVALID_HANDLE_VALUE)
        return;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {};
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    DWORD deviceIndex = 0;

    QList<Device> newDevicesList;

    while (SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &GUID_DEVINTERFACE_USB_DEVICE, deviceIndex, &deviceInterfaceData)) {
        deviceIndex++;

        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);

        PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);

        deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        SP_DEVINFO_DATA deviceInfoData = {};
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceDetailData, requiredSize, nullptr, &deviceInfoData)) {

            WCHAR deviceName[256];
            if (!SetupDiGetDeviceRegistryProperty(
                    deviceInfoSet,
                    &deviceInfoData,
                    SPDRP_DEVICEDESC,
                    nullptr,
                    (PBYTE)deviceName,
                    sizeof(deviceName),
                    nullptr
                    ))
                continue;

            DWORD properties;
            if(!SetupDiGetDeviceRegistryPropertyA(
                    deviceInfoSet,
                    &deviceInfoData,
                    SPDRP_CAPABILITIES,
                    nullptr,
                    (PBYTE)&properties,
                    sizeof(DWORD),
                    nullptr
                    ))
                continue;

            newDevicesList.append(Device{QString::fromWCharArray(deviceName), properties & CM_DEVCAP_REMOVABLE, deviceInfoData.DevInst});
        }

        free(deviceDetailData);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    QList<QString> onlyConnected;
    for(auto& dev : currentDevicesList)
        if(!newDevicesList.contains(dev))
            onlyConnected.append(dev.deviceName);

    QList<QString> onlyDisconnected;
    for(auto& dev : newDevicesList)
        if(!currentDevicesList.contains(dev))
            onlyDisconnected.append(dev.deviceName);

    if(onlyDisconnected.size() != 0 || onlyConnected.size() != 0) {

        if(currentDevicesList.size() != 0) {
            for(auto& ev : onlyDisconnected)
                addEvent(ev, true);
            for(auto& ev : onlyConnected)
                addEvent(ev, false);
        }

        QStandardItemModel* model = new QStandardItemModel();
        for(auto& dev : newDevicesList) {
            QStandardItem *item = new QStandardItem(dev.deviceName);
            model->appendRow(item);
        }

        ui->itemsList->setModel(model);

        currentDevicesList = newDevicesList;
    }
}

void MainWindow::addEvent(const QString &deviceName, bool connected, bool rejected)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->eventsList->model());

    QStandardItem* item;
    if(!rejected)
        item = new QStandardItem(QTime::currentTime().toString("[hh:mm:ss]: ") + deviceName + (connected ? " подключено." : " отключено."));
    else
        item = new QStandardItem(QTime::currentTime().toString("[hh:mm:ss]: ") + "Отказ в безопасном извлечении " + (deviceName == "" ? "устройства." : deviceName));

    model->appendRow(item);
}

void MainWindow::onItemClicked(const QModelIndex &index)
{
    currentSelectedDevice = index.row();
    ui->extractButton->setEnabled(currentDevicesList[currentSelectedDevice].ejectable ? true : false);
}

void MainWindow::onExtractButtonClicked()
{
    if (CM_Request_Device_EjectW(currentDevicesList[currentSelectedDevice].devInst, nullptr, nullptr, 0, 0) != CR_SUCCESS)
        addEvent(currentDevicesList[currentSelectedDevice].deviceName, false, true);
    else
        ui->extractButton->setEnabled(false);
}

void MainWindow::onDevicesChanged(bool refusal)
{
    if(refusal)
        addEvent("", false, true);
    else
        updateDevices();
}
