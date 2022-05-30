/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "led.h"
#include "kl_lib_f100.h"
#include "ch.h"
#include "hal.h"
#include "pwr.h"
#include "lvl1_lckt.h"

#define LED_LOW     63
#define LED_HIGH    999

// Events
#define EVTMASK_APPEARANCE  EVENT_MASK(0)
#define EVTMASK_PWR         EVENT_MASK(1)

static EventListener EvtLstnrAppearance, EvtListenerPwr;

static inline void Init();

int main(void) {
    // ==== Init clock system ====
    Clk.SetupBusDividers(ahbDiv8, apbDiv1, apbDiv1);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();
    // ==== Init Hard & Soft ====
    Init();

    // Events
    uint32_t EvtMsk;
    rLevel1.RegisterEvtAppearance(&EvtLstnrAppearance, EVTMASK_APPEARANCE);
    Pwr.RegisterEvtPwr(&EvtListenerPwr, EVTMASK_PWR);

    while(TRUE) {
        EvtMsk = chEvtWaitAny(EVTMASK_APPEARANCE | EVTMASK_PWR);
        // Power
        if(EvtMsk & EVTMASK_PWR) {
            if(Pwr.ExtPwrOn()) {
                rLevel1.Enabled = false;
                // Demonstrate charge status
                if(Pwr.Charging()) LedSmooth.Glimmer(LED_HIGH, LED_LOW);
                else LedSmooth.SetSmoothly(LED_LOW);
            }
            else {
                rLevel1.Enabled = true;
                LedSmooth.SetSmoothly(0);
            }
        }
        // Appear/disappear
        else if(EvtMsk & EVTMASK_APPEARANCE) {
            if(rLevel1.SomethingIsNear) LedSmooth.SetSmoothly(LED_TOP_VALUE);
            else LedSmooth.SetSmoothly(0);
        }
    } // while
}

void Init() {
    JtagDisable();
    Uart.Init(57600);
    LedSmooth.Init();
    LedSmooth.SetSmoothly(0);
    Pwr.Init();
    rLevel1.Init();
    Uart.Printf("\rTirusse2  AHB=%u; APB1=%u; APB2=%u\r", Clk.AHBFreqHz, Clk.APB1FreqHz, Clk.APB2FreqHz);
}
