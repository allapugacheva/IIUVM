#include "mainwindow.h"

#include <QApplication>

MainWindow* w;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if(uMsg == WM_DEVICECHANGE) {

        switch (wParam) {
        case DBT_DEVICEARRIVAL: // Устройство подключено
        case DBT_DEVICEREMOVECOMPLETE: // Устройство отключено
            emit w->devicesChanged(false);
            break;
        case DBT_DEVICEQUERYREMOVEFAILED: // Отказ в безопасном извлечении
            emit w->devicesChanged(true);
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    w = new MainWindow();
    w->show();
    return a.exec();
}
