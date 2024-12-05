#include "mainwindow.h"
#include "./ui_mainwindow.h"

bool sendFileOverBluetooth(const std::string& filePath, SOCKADDR_BTH remoteAddr, const QString& fileName) {

    WSADATA wsaData;
    SOCKET btSocket;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    btSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (btSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    if (connect(btSocket, (SOCKADDR*)&remoteAddr, sizeof(remoteAddr)) == SOCKET_ERROR) {
        closesocket(btSocket);
        WSACleanup();
        return false;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        closesocket(btSocket);
        WSACleanup();
        return false;
    }

    send(btSocket, fileName.toStdString().c_str(), fileName.size() + 1, 0);

    char buffer[1024];
    int i = 0;
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        int bytesRead = file.gcount();
        if (send(btSocket, buffer, bytesRead, 0) == SOCKET_ERROR)
            break;

        QApplication::processEvents();
    }

    file.close();
    closesocket(btSocket);
    WSACleanup();
    return true;
}

void MainWindow::listDevices() {

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = { 0 };
    BLUETOOTH_DEVICE_INFO deviceInfo = { 0 };
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
    searchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
    searchParams.fReturnAuthenticated = TRUE;
    searchParams.fReturnRemembered = TRUE;
    searchParams.fReturnUnknown = FALSE;
    searchParams.fReturnConnected = TRUE;
    searchParams.hRadio = NULL;

    HANDLE deviceHandle = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    if (deviceHandle != NULL) {

        QStandardItemModel* model = new QStandardItemModel;
        do {
            QStandardItem* item = new QStandardItem(QString::fromWCharArray(deviceInfo.szName));

            std::ostringstream oss;
            for (int i = 5; i >= 0; --i)
                oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(deviceInfo.Address.rgBytes[i]);

            devices.insert(QString::fromWCharArray(deviceInfo.szName), std::stoull(oss.str(), nullptr, 16));

            model->appendRow(item);
        } while (BluetoothFindNextDevice(deviceHandle, &deviceInfo));

        BluetoothFindDeviceClose(deviceHandle);

        ui->devicesView->setModel(model);
    }
}

void MainWindow::onSendButtonClicked() {

    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Выберите отправляемый файл",
        QDir::homePath(),
        "Файл (*.*)"
        );

    if (filePath.size() != 0) {

        QString fileName = filePath.mid(filePath.lastIndexOf('/') + 1);

        SOCKADDR_BTH remoteAddr = { 0 };
        remoteAddr.addressFamily = AF_BTH;
        remoteAddr.btAddr = BTH_ADDR(devices.value(currentSelectedDevice));
        remoteAddr.port = 4; // Порт 4 для RFCOMM

        sendFileOverBluetooth(filePath.toStdString(), remoteAddr, fileName);
        //QMessageBox::information(this, "Отправка файла", sendFileOverBluetooth(filePath.toStdString(), remoteAddr, fileName) ? "Файл успешно отправлен" : "Ошибка при отправке файла", QMessageBox::Ok);
    }
}

void MainWindow::onItemClicked(const QModelIndex &index) {

    currentSelectedDevice = ui->devicesView->model()->data(index).toString();
    ui->sendButton->setEnabled(true);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);

    listDevices();

    ui->sendButton->setEnabled(false);

    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::listDevices);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->devicesView, &QListView::clicked, this, &MainWindow::onItemClicked);
}

MainWindow::~MainWindow() {

    delete ui;
}
