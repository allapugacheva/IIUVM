#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <setupapi.h>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <QTimer>
#include <initguid.h>
#include <usbiodef.h>
#include <QList>
#include <QSet>
#include <QStringList>
#include <QStringListModel>
#include <QTime>
#include <cfgmgr32.h>
#include <dbt.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

typedef struct _Device{
    QString deviceName;
    bool ejectable;
    DWORD devInst;

    bool operator== (const _Device& other) const {
        return deviceName == other.deviceName && ejectable == other.ejectable && devInst == other.devInst;
    }
} Device;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class MainWindow : public QMainWindow
{
    Q_OBJECT

    HDEVNOTIFY hDevNotify;
    HWND hwnd;

    Ui::MainWindow *ui;
    int currentSelectedDevice;
    QList<Device> currentDevicesList;

signals:
    void devicesChanged(bool refusal);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateDevices();
    void addEvent(const QString& deviceName, bool connected, bool rejected = false);

private slots:
    void onItemClicked(const QModelIndex &index);
    void onExtractButtonClicked();
    void onDevicesChanged(bool refusal);
};
#endif // MAINWINDOW_H
