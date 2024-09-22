#include "batteryinformation.h"

BatteryInfo::BatteryInfo() {

    findBatteryType();
    findBatteryInformation();
}

void BatteryInfo::findBatteryType() {

    // Получение дескриптора устройства
    HDEVINFO hdev =
        SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY,
                            0,
                            0,
                            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (INVALID_HANDLE_VALUE != hdev) {
        SP_DEVICE_INTERFACE_DATA did = {0};
        did.cbSize = sizeof(did);

        // Получение перечисление интерфейсов устройства.
        if (SetupDiEnumDeviceInterfaces(hdev,
                                        0,
                                        &GUID_DEVCLASS_BATTERY,
                                        0,
                                        &did)) {
            DWORD cbRequired = 0;

            // Получение размера структуры с информацией об устройстве.
            SetupDiGetDeviceInterfaceDetail(hdev,
                                            &did,
                                            0,
                                            0,
                                            &cbRequired,
                                            0);

            PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd =
                (PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LPTR,
                                                              cbRequired);
            if (pdidd) {
                pdidd->cbSize = sizeof(*pdidd);
                // Получение информации об устройстве.
                if (SetupDiGetDeviceInterfaceDetail(hdev,
                                                    &did,
                                                    pdidd,
                                                    cbRequired,
                                                    &cbRequired,
                                                    0)) {
                    // Открываем дескриптор файла для устройства.
                    HANDLE hBattery =
                        CreateFile(pdidd->DevicePath,
                                   GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
                    if (INVALID_HANDLE_VALUE != hBattery) {

                        BATTERY_QUERY_INFORMATION bqi = {0};

                        DWORD dwWait = 0;
                        DWORD dwOut;

                        // Получение тега батареи.
                        if (DeviceIoControl(hBattery,
                                            IOCTL_BATTERY_QUERY_TAG,
                                            &dwWait,
                                            sizeof(dwWait),
                                            &bqi.BatteryTag,
                                            sizeof(bqi.BatteryTag),
                                            &dwOut,
                                            NULL)
                            && bqi.BatteryTag) {

                            BATTERY_INFORMATION bi = {0};
                            bqi.InformationLevel = BatteryInformation;
                            // Получение информации о батарее.
                            if (DeviceIoControl(hBattery,
                                                IOCTL_BATTERY_QUERY_INFORMATION,
                                                &bqi,
                                                sizeof(bqi),
                                                &bi,
                                                sizeof(bi),
                                                &dwOut,
                                                NULL)) {

                                switch(bi.Chemistry[0]) {
                                case 'P':
                                    this->batteryType = PbAc;
                                    break;
                                case 'L':
                                    this->batteryType = LIon;
                                    break;
                                case 'N':
                                    switch(bi.Chemistry[2]) {
                                    case 'C':
                                        this->batteryType = NiCd;
                                        break;
                                    case 'M':
                                        this->batteryType = NiMh;
                                        break;
                                    case 'Z':
                                        this->batteryType = NiZn;
                                        break;
                                    }
                                    break;
                                case 'R':
                                    this->batteryType = RAM;
                                    break;
                                }
                            }
                        }
                        CloseHandle(hBattery);
                    }
                }
                LocalFree(pdidd);
            }
        }
        SetupDiDestroyDeviceInfoList(hdev);
    }
}

QString BatteryInfo::getIsFromAC() const
{
    switch(isFromAC) {

    case 0:
        return "от батареи";
    case 1:
        return "от сети";
    default:
        return "неизвестный источник";
    }
}

void BatteryInfo::setIsFromAC(int newIsFromAC)
{
    isFromAC = newIsFromAC;
}

QString BatteryInfo::getBatteryType() const
{
    return batteryTypes[batteryType];
}

void BatteryInfo::setBatteryType(BatteryType newBatteryType)
{
    batteryType = newBatteryType;
}

int BatteryInfo::getBatteryPercentage() const
{
    return batteryPercentage;
}

void BatteryInfo::setBatteryPercentage(int newBatteryPercentage)
{
    batteryPercentage = newBatteryPercentage;
}

QString BatteryInfo::getIsSaverOn() const
{
    switch(isSaverOn) {

    case 0:
        return "выкл";
    case 1:
        return "вкл";
    default:
        return "неизвестный статус";
    }
}

void BatteryInfo::setIsSaverOn(int newIsSaverOn)
{
    isSaverOn = newIsSaverOn;
}

QString BatteryInfo::getBatteryLifeTime() const
{
    if(batteryLifeTime == "-1")
        return "идёт зарядка";
    else if(batteryLifeTime == "-2")
        return "неизвестно";
    else
        return batteryLifeTime;
}

void BatteryInfo::setBatteryLifeTime(const QString &newBatteryLifeTime)
{
    batteryLifeTime = newBatteryLifeTime;
}

QString BatteryInfo::getBatteryFullLifeTime() const
{
    if(batteryFullLifeTime == "-1")
        return "идёт зарядка";
    else if(batteryFullLifeTime == "-2")
        return "неизвестно";
    else
        return batteryFullLifeTime;
}

void BatteryInfo::setBatteryFullLifeTime(const QString &newBatteryFullLifeTime)
{
    batteryFullLifeTime = newBatteryFullLifeTime;
}

void BatteryInfo::findBatteryInformation()
{
    int seconds, minutes, hours;
    SYSTEM_POWER_STATUS powerStatus;
    GetSystemPowerStatus(&powerStatus);

    this->isFromAC = powerStatus.ACLineStatus;
    this->batteryPercentage = powerStatus.BatteryLifePercent;
    this->isSaverOn = powerStatus.SystemStatusFlag;

    seconds = powerStatus.BatteryLifeTime;
    if(seconds == -1 && powerStatus.ACLineStatus == 1)
        this->batteryLifeTime = "-1";
    else if(seconds == -1)
        this->batteryLifeTime = "-2";
    else {

        hours = seconds / 3600;
        minutes = (seconds % 3600) / 60;
        seconds = (seconds % 3600) % 60;

        if(hours < 10)
            this->batteryLifeTime = "0";
        this->batteryLifeTime += QString::number(hours) + ":";
        if(minutes < 10)
            this->batteryLifeTime += "0";
        this->batteryLifeTime += QString::number(minutes) + ":";
        if(seconds < 10)
            this->batteryLifeTime += "0";
        this->batteryLifeTime += QString::number(seconds);
    }

    if(powerStatus.BatteryLifeTime == -1 && powerStatus.ACLineStatus == 1)
        this->batteryFullLifeTime = "-1";
    else if(seconds == -1)
        this->batteryFullLifeTime = "-2";
    else {

        seconds = powerStatus.BatteryLifeTime * 100 / powerStatus.BatteryLifePercent;
        hours = seconds / 3600;
        minutes = (seconds % 3600) / 60;
        seconds = (seconds % 3600) % 60;

        if(hours < 10)
            this->batteryFullLifeTime = "0";
        this->batteryFullLifeTime += QString::number(hours) + ":";
        if(minutes < 10)
            this->batteryFullLifeTime += "0";
        this->batteryFullLifeTime += QString::number(minutes) + ":";
        if(seconds < 10)
            this->batteryFullLifeTime += "0";
        this->batteryFullLifeTime += QString::number(seconds);
    }
}


