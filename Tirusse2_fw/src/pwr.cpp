/*
 * pwr.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: g.kruglov
 */

#include "pwr.h"

Pwr_t Pwr;

// ================================= Thread ====================================
static WORKING_AREA(warPwrThread, 128);
static msg_t PwrThread(void *arg) {
    (void)arg;
    chRegSetThreadName("Pwr");
    while(1) Pwr.Task();
    return 0;
}

void Pwr_t::Task() {
    chThdSleepMilliseconds(999);
    // ExtPwr
    if(!IExtPwrConnected and ExtPwrOn()) {
        IExtPwrConnected = true;
        chEvtBroadcast(&IEvtSrcPwr);
    }
    else if(IExtPwrConnected and !ExtPwrOn()) {
        IExtPwrConnected = false;
        chEvtBroadcast(&IEvtSrcPwr);
    }
    // Charging
    if(IExtPwrConnected) {
        if(!ICharging and Charging()) {
            ICharging = true;
            chEvtBroadcast(&IEvtSrcPwr);
        }
        else if(ICharging and !Charging()) {
            ICharging = false;
            chEvtBroadcast(&IEvtSrcPwr);
        }
    }
}

void Pwr_t::Init() {
    PinSetupIn(EXT_PWR_GPIO, EXT_PWR_PIN, pudPullDown);
    PinSetupIn(CHARGE_GPIO, CHARGE_PIN, pudPullUp);
    IExtPwrConnected = false;
    ICharging = false;
    chEvtInit(&IEvtSrcPwr);
    // Init thread
    chThdCreateStatic(warPwrThread, sizeof(warPwrThread), NORMALPRIO, PwrThread, NULL);
}
