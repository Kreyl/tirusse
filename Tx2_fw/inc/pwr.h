/*
 * pwr.h
 *
 *  Created on: Jun 12, 2013
 *      Author: g.kruglov
 */

#ifndef PWR_H_
#define PWR_H_

#include "kl_lib_f100.h"

#define CHARGE_GPIO     GPIOA
#define CHARGE_PIN      12
#define EXT_PWR_GPIO    GPIOA
#define EXT_PWR_PIN     0

#define BATTERY_EMPTY_MV    3600

class Pwr_t {
private:
//    EventSource IEvtSrcPwr;
//    bool IExtPwrConnected, ICharging;
public:
    bool BatteryDischarged;
    void Init();
//    bool ExtPwrOn() { return PinIsSet(EXT_PWR_GPIO, EXT_PWR_PIN); }
//    bool Charging() { return !PinIsSet(CHARGE_GPIO, CHARGE_PIN); }
//    void RegisterEvtPwr(EventListener *PEvtLstnr, uint8_t EvtMask) { chEvtRegisterMask(&IEvtSrcPwr, PEvtLstnr, EvtMask); }
    // Inner use
    void Task();
};

extern Pwr_t Pwr;


#endif /* PWR_H_ */
