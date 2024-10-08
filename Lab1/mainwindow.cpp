#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(450,180);

    setWindowTitle("Battery Information");

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onPushButtonClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::onPushButton2Clicked);

    ui->label_4->setText(batteryInfo.getBatteryType());
    updateInfo();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateInfo);
    timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateInfo()
{
    batteryInfo.findBatteryInformation();

    ui->label_8->setText(batteryInfo.getBatteryLifeTime());
    ui->progressBar->setValue(batteryInfo.getBatteryPercentage());
    ui->label_10->setText(batteryInfo.getBatteryFullLifeTime());
    ui->label->setText(batteryInfo.getIsFromAC());
    ui->label_6->setText(batteryInfo.getIsSaverOn());
}

void MainWindow::onPushButtonClicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Режим сна", "Отключить события пробуждения?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes)
        SetSuspendState(FALSE, FALSE, TRUE);
    else if(reply == QMessageBox::No)
        SetSuspendState(FALSE, FALSE, FALSE);
}

void MainWindow::onPushButton2Clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Гибернация", "Отключить события пробуждения?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes)
        SetSuspendState(TRUE, FALSE, TRUE);
    else if(reply == QMessageBox::No)
        SetSuspendState(TRUE, FALSE, FALSE);
}
