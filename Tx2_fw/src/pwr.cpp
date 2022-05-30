/*
 * pwr.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: g.kruglov
 */

#include "adc_f100.h"
#include "pwr.h"

Adc_t Adc;
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
//    // ExtPwr
//    if(!IExtPwrConnected and ExtPwrOn()) {
//        IExtPwrConnected = true;
//        chEvtBroadcast(&IEvtSrcPwr);
//    }
//    else if(IExtPwrConnected and !ExtPwrOn()) {
//        IExtPwrConnected = false;
//        chEvtBroadcast(&IEvtSrcPwr);
//    }
//    // Charging
//    if(IExtPwrConnected) {
//        if(!ICharging and Charging()) {
//            ICharging = true;
//            chEvtBroadcast(&IEvtSrcPwr);
//        }
//        else if(ICharging and !Charging()) {
//            ICharging = false;
//            chEvtBroadcast(&IEvtSrcPwr);
//        }
//    }

    // Battery
    uint32_t rslt = 0;
    Adc.Enable();
    Adc.Calibrate();
    for(uint8_t i=0; i<8; i++) {
        Adc.StartConversion();
        while(!Adc.ConversionCompleted()) chThdSleepMilliseconds(20);
        rslt += Adc.Result();
    }
    Adc.Disable();
    rslt >>= 3;
    //Uart.Printf("Adc=%u; ", rslt);
    // Convert adc to mv
    rslt = (rslt * 16225) / 10000;
    Uart.Printf("mv=%u\r", rslt);
    BatteryDischarged = (rslt < BATTERY_EMPTY_MV);
}

void Pwr_t::Init() {
//    PinSetupIn(EXT_PWR_GPIO, EXT_PWR_PIN, pudPullDown);
//    PinSetupIn(CHARGE_GPIO, CHARGE_PIN, pudPullUp);
//    IExtPwrConnected = false;
//    ICharging = false;
//    chEvtInit(&IEvtSrcPwr);
    Adc.Init();
    // Init thread
    chThdCreateStatic(warPwrThread, sizeof(warPwrThread), NORMALPRIO, PwrThread, NULL);
}



