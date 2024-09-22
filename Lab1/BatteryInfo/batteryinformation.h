#ifndef BATTERYINFORMATION_H
#define BATTERYINFORMATION_H

#include <QString>
#include <windows.h>
#include <powrprof.h>
#include <iostream>
#include <batclass.h>
#include <setupapi.h>
#include <devguid.h>

enum BatteryType {
    PbAc,
    LIon,
    NiCd,
    NiMh,
    NiZn,
    RAM
};

const QString batteryTypes[] = {"Lead Acid", "Lithium Ion", "Nickel Cadmium", "Nickel Metal Hydride", "Nickel Zinc", "Rechargeable Alkaline-Manganese"};

class BatteryInfo
{
    int isFromAC;
    BatteryType batteryType;
    int batteryPercentage;
    int isSaverOn;
    QString batteryLifeTime;
    QString batteryFullLifeTime;

    void findBatteryType();

public:
    BatteryInfo();

    QString getIsFromAC() const;
    void setIsFromAC(int newIsFromAC);
    QString getBatteryType() const;
    void setBatteryType(BatteryType newBatteryType);
    int getBatteryPercentage() const;
    void setBatteryPercentage(int newBatteryPercentage);
    QString getIsSaverOn() const;
    void setIsSaverOn(int newIsSaverOn);
    QString getBatteryLifeTime() const;
    void setBatteryLifeTime(const QString &newBatteryLifeTime);
    QString getBatteryFullLifeTime() const;
    void setBatteryFullLifeTime(const QString &newBatteryFullLifeTime);

    void findBatteryInformation();
};

#endif // BATTERYINFORMATION_H
