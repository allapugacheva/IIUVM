#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <winsock2.h>
#include <ws2bth.h>
#include <windows.h>
#include <bthsdpdef.h>
#include <bluetoothapis.h>
#include <QMainWindow>
#include <QStandardItem>
#include <QStandardItemModel>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <QMap>
#include <QListView>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;

    QMap<QString, ULONGLONG> devices;
    QString currentSelectedDevice;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void listDevices();
    void onSendButtonClicked();
    void onItemClicked(const QModelIndex &index);
};
#endif // MAINWINDOW_H
