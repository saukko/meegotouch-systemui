#include "batterygconf.h"

#include <DuiGConfItem>
#include <QDebug>

namespace {    
    QString Dir = "/battery/settings"; //TODO: Change the dir value if necessary
}

BatteryGConf::BatteryGConf()
{
    // init the gconf keys    
    duiGConfItems.insert(BatteryGConf::PSMToggleKey, new DuiGConfItem(mapGConfKey(BatteryGConf::PSMToggleKey)));
    duiGConfItems.insert(BatteryGConf::PSMDisabledKey, new DuiGConfItem(mapGConfKey(BatteryGConf::PSMDisabledKey)));
    duiGConfItems.insert(BatteryGConf::PSMThresholdKey, new DuiGConfItem(mapGConfKey(BatteryGConf::PSMThresholdKey)));

    QHash<BatteryGConf::GConfKey, DuiGConfItem *>::iterator i;
    for (i = duiGConfItems.begin(); i != duiGConfItems.end(); ++i)
        connect( i.value(), SIGNAL(valueChanged()), this, SLOT(keyValueChanged()));    

}

BatteryGConf::~BatteryGConf()
{ 
    QHash<BatteryGConf::GConfKey, DuiGConfItem *>::iterator i;
    for (i = duiGConfItems.begin(); i != duiGConfItems.end(); ++i) {
        delete i.value();
        i.value() = NULL;
    } 
}

int BatteryGConf::keyCount()
{
    DuiGConfItem duiGConfItem(Dir);
    QList<QString> list = duiGConfItem.listEntries();
    return list.size();
}

void BatteryGConf::setValue(BatteryGConf::GConfKey key, QVariant value)
{
    duiGConfItems.value(key)->set(value);
}

QVariant BatteryGConf::value(BatteryGConf::GConfKey key)
{    
    return duiGConfItems.value(key)->value();
}

void BatteryGConf::keyValueChanged()
{
    DuiGConfItem *item = static_cast<DuiGConfItem *>(this->sender());
    emit valueChanged(duiGConfItems.key(item), item->value());
}

QString BatteryGConf::mapGConfKey(BatteryGConf::GConfKey key)
{
    QString keyStr("%1%2");
    switch(key) {
        case BatteryGConf::PSMToggleKey:
            keyStr = keyStr.arg(Dir).arg("/BatteryPSMToggle");
            break;
        case BatteryGConf::PSMDisabledKey:
            keyStr = keyStr.arg(Dir).arg("/BatteryPSMDisabled");
            break;
        case BatteryGConf::PSMThresholdKey:
            keyStr = keyStr.arg(Dir).arg("/BatteryPSMThreshold");
            break;        
        default:
            break;
    }
    return keyStr;
}
