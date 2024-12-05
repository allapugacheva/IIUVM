#include "mainwindow.h"
#include "./ui_mainwindow.h"

QString MainWindow::receiveFileOverBluetooth(SOCKET listenSocket) {

    SOCKADDR_BTH clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);

    if (clientSocket == INVALID_SOCKET)
        return "";

    char buffer[1024];
    int bytesReceived;
    recv(clientSocket, buffer, sizeof(buffer), 0);
    QString fileName(buffer);

    std::ofstream file(QString(ui->lineEdit->text() + fileName).toStdString().c_str(), std::ios::binary);
    if (!file.is_open()) {
        closesocket(clientSocket);
        return "";
    }

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        file.write(buffer, bytesReceived);

    file.close();
    closesocket(clientSocket);
    return fileName;
}

bool MainWindow::waitFile() {

    WSADATA wsaData;
    SOCKET listenSocket;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    listenSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (listenSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    SOCKADDR_BTH serverAddr = { 0 };
    serverAddr.addressFamily = AF_BTH;
    serverAddr.btAddr = BTH_ADDR(0x0);
    serverAddr.port = 4;

    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    if (listen(listenSocket, 1) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    while (true) {
        QString fileName = receiveFileOverBluetooth(listenSocket);

        if (fileName == "")
            emit fileReceived("");
        else
            emit fileReceived(fileName);
    }

    closesocket(listenSocket);
    WSACleanup();

    return true;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QThread* thread = new QThread(this);
    QObject::connect(thread, &QThread::started, [=]() {
        waitFile();
    });
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onPushButtonClicked);
    connect(this, &MainWindow::fileReceived, this, &MainWindow::onFileReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPushButtonClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this,
                                                           tr("Выберите папку"),
                                                           QString(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folderPath.isEmpty())
        ui->lineEdit->setText(folderPath);
}

void MainWindow::onFileReceived(const QString& file)
{
    if (file != "") {
        QMessageBox::information(this, "Получение файла", "Получен файл " + file, QMessageBox::Ok);

        if(ui->checkBox->isChecked()) {

            QMediaPlayer* player = new QMediaPlayer(this);
            QAudioOutput* output = new QAudioOutput(this);
            player->setAudioOutput(output);
            player->setSource(QUrl::fromLocalFile(ui->lineEdit->text() + file));
            output->setVolume(50);
            player->play();
        }
    }
    else
        QMessageBox::information(this, "Получение файла", "Ошибка при получении файла", QMessageBox::Ok);
}
