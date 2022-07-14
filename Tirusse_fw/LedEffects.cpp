/*
 * LedEffects.cpp
 *
 *  Created on: 29 янв. 2022 г.
 *      Author: layst
 */

#include "LedEffects.h"
#include "ws2812b.h"
#include "board.h"
#include "ch.h"

extern Neopixels_t Leds;

#define LED_CNT     NPX_LED_CNT

static void TmrCallbackUpdate(void *p);

class ParentEff_t {
private:
    virtual_timer_t Tmr;
protected:
    void StartTimerI(systime_t Delay_st) { chVTSetI(&Tmr, Delay_st, TmrCallbackUpdate, this); }
    void StartTimer(systime_t Delay) {
        chSysLock();
        StartTimerI(Delay);
        chSysUnlock();
    }
    bool TimerIsStarted() { return chVTIsArmed(&Tmr); }
    void StopTimerI() { chVTResetI(&Tmr); }
public:
    bool IsIdle = true;
    virtual void UpdateI() = 0;
    ParentEff_t() { Tmr.func = nullptr; }
    virtual ~ParentEff_t() {}
};

// Universal Updating callback
static void TmrCallbackUpdate(void *p) {
    chSysLockFromISR();
//    PrintfI("i");
    ((ParentEff_t*)p)->UpdateI();
    chSysUnlockFromISR();
}

class Blade_t : public ParentEff_t {
private:
    Color_t ClrCurr = clBlack;
public:
    Color_t ClrTarget = clBlack;
    uint32_t Smooth = 0;
    void UpdateI() { ClrCurr.Adjust(ClrTarget); }
    void Draw() {
        if(ClrCurr != ClrTarget and !TimerIsStarted()) StartTimer(ClrCurr.DelayToNextAdj(ClrTarget, Smooth));
//        chSysLock();
        for(uint32_t i=1; i<(LED_CNT-1); i++) Leds.ClrBuf[i] = ClrCurr;
//        chSysUnlock();
    }
};



static Blade_t Blade;
static thread_reference_t ThdRef;

//static void SetGemClr(Color_t Clr) {
//    Leds.ClrBuf[0] = Clr;
//    Leds.ClrBuf[LED_CNT-1] = Clr;
//}

static THD_WORKING_AREA(waNpxThread, 256);
__noreturn
static void NpxThread(void *arg) {
    systime_t Strt = 0;
    chRegSetThreadName("NpxThread");
    while(true) {
        // Wait if too fast
        while(chVTTimeElapsedSinceX(Strt) < TIME_MS2I(4)) chThdSleepMilliseconds(2);
        Strt = chVTGetSystemTimeX();
        // Enter sleep if Leds are busy
        chSysLock();
        if(!Leds.TransmitDone) chThdSuspendS(&ThdRef); // Wait Leds done
        chSysUnlock();
        Leds.SetCurrentColors(); // Send current buf to leds
        // ==== Process effects ====
        Blade.Draw();
    } // while true
}

// Npx-DMA-completed callback
static void OnLedsTransmitEnd() {
    chSysLockFromISR();
    if(ThdRef->state == CH_STATE_SUSPENDED) chThdResumeI(&ThdRef, MSG_OK);
    chSysUnlockFromISR();
}

namespace Eff {

void Init() {
    Leds.OnTransmitEnd = OnLedsTransmitEnd;
    chThdCreateStatic(waNpxThread, sizeof(waNpxThread), NORMALPRIO, (tfunc_t)NpxThread, NULL);
}

void SetGem(Color_t Clr, uint32_t ASmooth) {
//    ClrGemTarget = Clr;
//    SmoothGem = ASmooth;
//    chThdResume(&ThdRefGem, MSG_OK);
}

void SetBlade(Color_t Clr, uint32_t ASmooth) {
    Blade.ClrTarget = Clr;
    Blade.Smooth = ASmooth;
}

} // namespace
