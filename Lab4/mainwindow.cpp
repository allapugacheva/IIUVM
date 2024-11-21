#include "mainwindow.h"
#include "./ui_mainwindow.h"

HHOOK hHook = nullptr;
bool spy = false;

LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (wParam == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000) && pKeyboard->vkCode == VK_OEM_6) {
            spy = false;
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void MainWindow::fillData(IMFActivate *activator)
{
    // Try to get the display name.
    WCHAR *data = NULL;
    UINT32 len;
    if(!SUCCEEDED(activator->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &data, &len)))
        return;
    ui->deviceName->setText("Название: " + QString::fromWCharArray(data));

    if(!SUCCEEDED(activator->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &data, &len)))
        return;

    QString temp = QString::fromWCharArray(data);
    int i = temp.indexOf("vid_");
    ui->vendorId->setText("Идентификатор производителя: " + temp.mid(i + 4, 4));
    i = temp.indexOf("pid_");
    ui->deviceId->setText("Идентификатор продукта: " + temp.mid(i + 4, 4));

    GUID guidSourceType;
    if (!SUCCEEDED(activator->GetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, &guidSourceType)))
        return;
    ui->sourceType->setText(guidSourceType == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID ? "Тип устройства: устройство видеозахвата" : "Тип устройства: устройство аудиозахвата");

    // Категория устройства
    GUID guidCategory;
    if (!SUCCEEDED(activator->GetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_CATEGORY, &guidCategory)))
        return;
    ui->deviceCategory->setText(guidCategory == KSCATEGORY_CAMERA ? "Категория устройства: камера" : "Категория устройства: устройство видеозахвата");

    CoTaskMemFree(data);

    IMFMediaType* type;
    if(!SUCCEEDED(MFCreateSourceReaderFromMediaSource(device, NULL, &reader)))
        return;

    if(!SUCCEEDED(reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type)))
        return;

    UINT64 tmp;
    if(!SUCCEEDED(type->GetUINT64(MF_MT_FRAME_SIZE, &tmp)))
        return;

    type->Release();
    reader->Release();

    ui->deviceSize->setText("Размер: " + QString::number((UINT32)(tmp >> 32)) + 'x' + QString::number((UINT32)(tmp)));
}

void MainWindow::initReader(bool fill)
{
    IMFMediaType* type;

    UINT32 count = 0;
    IMFAttributes *pConfig = NULL;
    IMFActivate **ppDevices = NULL;

    if(!SUCCEEDED(MFCreateAttributes(&pConfig, 1)))
        return;

    // Request video capture devices.
    if(!SUCCEEDED(pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)))
        return;

    // Enumerate the devices,
    if (!SUCCEEDED(MFEnumDeviceSources(pConfig, &ppDevices, &count)))
        return;

    // Create a media source for the first device in the list.
    if (count > 0) {
        ppDevices[0]->ActivateObject(IID_PPV_ARGS(&device));
        if(fill) fillData(ppDevices[0]);

        if(!SUCCEEDED(MFCreateSourceReaderFromMediaSource(device, NULL, &reader)))
            return;

        if(!SUCCEEDED(MFCreateMediaType(&type)))
            return;

        if(!SUCCEEDED(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)))
            return;

        if(!SUCCEEDED(type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_MJPG)))
            return;

        if(!SUCCEEDED(reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type)))
            return;

        type->Release();
    }

    for (DWORD i = 0; i < count; i++)
        ppDevices[i]->Release();
    CoTaskMemFree(ppDevices);
}

void MainWindow::startSpy()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Выберите папку",
        QString(),  // Стартовая папка (по умолчанию)
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!folderPath.isEmpty()) {

        TimeDialog* dialog = new TimeDialog();
        if(dialog->exec() == QDialog::Accepted) {

            this->setWindowFlag(Qt::Tool);
            spy = true;

            QTimer* timer = new QTimer(this);
            timer->setInterval(dialog->getTime());
            connect(timer, &QTimer::timeout, [this, timer, folderPath](){

                takePhoto(folderPath + "/spy_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".jpg");
            });

            QTimer* checker = new QTimer(this);
            checker->setInterval(1000);
            connect(checker, &QTimer::timeout, [this, timer, checker](){

                if(!spy) {
                    timer->stop();
                    checker->stop();
                    this->setWindowFlag(Qt::Tool, false);
                    this->show();
                }
            });

            timer->start();
            checker->start();
        }
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(320, 245);

    MFStartup(MF_VERSION);

    initReader(true);

    connect(ui->photoButton, &QPushButton::clicked, this, &MainWindow::onPhotoButtonClicked);
    connect(ui->spyButton, &QPushButton::clicked, this, &MainWindow::startSpy);

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete device;
    delete reader;

    if (hHook)
        UnhookWindowsHookEx(hHook);

    MFShutdown();
}

void MainWindow::takePhoto(const QString& path)
{
    IMFSample* sample;
    DWORD stream;
    DWORD flags;
    LONGLONG timestamp;

    IMFMediaBuffer* buffer;
    BYTE* data;
    DWORD size;

    HANDLE h;
    DWORD written;

    initReader(false);

    for (;;)
    {
        // this is reading in syncronous blocking mode, MF supports also async calls
        if(!SUCCEEDED(reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &stream, &flags, &timestamp, &sample)))
            return;

        if (flags & MF_SOURCE_READERF_STREAMTICK)
            continue;

        break;
    }

    if(!SUCCEEDED(sample->ConvertToContiguousBuffer(&buffer)))
        return;

    if(!SUCCEEDED(buffer->Lock(&data, NULL, &size)))
        return;
    if((h = CreateFileA(path.toUtf8().constData(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        return;
    if(!WriteFile(h, data, size, &written, NULL))
        return;
    CloseHandle(h);

    buffer->Unlock();
    buffer->Release();
    sample->Release();
    reader->Release();
}

void MainWindow::stopSpy()
{
    spy = false;
}

void MainWindow::onPhotoButtonClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,
        "Сохранить файл как",
        QDir::homePath(),
        "Фотография (*.jpg)"
        );

    if (!filePath.isEmpty())
        takePhoto(filePath);
}
