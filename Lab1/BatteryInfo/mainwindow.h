#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMessageBox>
#include "batteryinformation.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateInfo();

private slots:
    void onPushButtonClicked();
    void onPushButton2Clicked();

private:
    Ui::MainWindow *ui;

    BatteryInfo batteryInfo;
};
#endif // MAINWINDOW_H
