#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <winsock2.h>
#include <ws2bth.h>
#include <fstream>
#include <QThread>
#include <QFileDialog>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QAudioOutput>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow *ui;

    QString receiveFileOverBluetooth(SOCKET listenSocket);
    bool waitFile();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void fileReceived(const QString& file);

private slots:
    void onPushButtonClicked();
    void onFileReceived(const QString& file);

};
#endif // MAINWINDOW_H
