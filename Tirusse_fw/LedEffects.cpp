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

void LedPwrOn();
void LedPwrOff();

#define LED_CNT     NPX_LED_CNT

static void TmrCallbackUpdate(void *p);

class ParentEff_t {
private:
    virtual_timer_t Tmr;
protected:
    void StartTimerI(uint32_t Delay_ms) { chVTSetI(&Tmr, TIME_MS2I(Delay_ms), TmrCallbackUpdate, this); }
    void StartTimer(uint32_t Delay_ms) {
        chSysLock();
        StartTimerI(Delay_ms);
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
    ((ParentEff_t*)p)->UpdateI();
    chSysUnlockFromISR();
}

class Blade_t : public ParentEff_t {
private:
    Color_t ClrCurr = clBlack;
    enum State_t {staIdle, staIndicateBattery, staEnterIdle} State;
    Color_t ClrBattery;
    uint32_t Sz;
public:
    Color_t ClrTarget = clBlack;
    uint32_t Smooth = 0;
    void UpdateI() override {
        if(State == staIndicateBattery) State = staEnterIdle; // Stop indicating battery
        else ClrCurr.Adjust(ClrTarget);
    }

    void Draw() {
        switch(State) {
            case staIdle:
                if(ClrCurr != ClrTarget and !TimerIsStarted()) StartTimer(ClrCurr.DelayToNextAdj(ClrTarget, Smooth));
                for(uint32_t i=1; i<(LED_CNT-1); i++) Leds.ClrBuf[i] = ClrCurr;
                break;

            case staIndicateBattery:
                if(!TimerIsStarted()) StartTimer(BATTERY_INDICATION_DURATION_MS);
                for(uint32_t i=1; i<(LED_CNT/2); i++) {
                    Leds.ClrBuf[i] = (i <= Sz)? ClrBattery : clBlack;
                    Leds.ClrBuf[LED_CNT - i - 1] = Leds.ClrBuf[i];
                }
                break;

            case staEnterIdle:
                if(ClrCurr != ClrTarget and !TimerIsStarted()) StartTimer(ClrCurr.DelayToNextAdj(ClrTarget, Smooth));
                for(uint32_t i=1; i<(LED_CNT-1); i++) Leds.ClrBuf[i] = clBlack;
                State = staIdle;
                break;
        } // switch
    }

    void DoBatteryIndication(Color_t AClr, uint32_t ASz) {
        chSysLock();
        StopTimerI();
        ClrBattery = AClr;
        Sz = ASz;
        State = staIndicateBattery;
        ClrCurr = clBlack;
        chSysUnlock();
    }
};

class Gem_t : public ParentEff_t {
private:
    Color_t ClrCurr = clBlack, ClrBlink = clBlack;
    enum State_t {staIdle, staBlinkFadeIn, staBlinkFadeOut} State;
    bool BlinkForever = false;
public:
    Color_t ClrTarget = clBlack;
    uint32_t Smooth = 0;

    void UpdateI() override { ClrCurr.Adjust(ClrTarget); }

    void Draw() {
        Leds.ClrBuf[0] = ClrCurr;
        Leds.ClrBuf[LED_CNT-1] = ClrCurr;
        if(ClrCurr == ClrTarget) {
            switch(State) {
                case staBlinkFadeIn:
                    State = staBlinkFadeOut;
                    ClrTarget = clBlack;
                    break;
                case staBlinkFadeOut:
                    State = BlinkForever? staBlinkFadeIn : staIdle;
                    ClrTarget = ClrBlink;
                    break;
                default: break;
            }
        }
        else if(!TimerIsStarted()) StartTimer(ClrCurr.DelayToNextAdj(ClrTarget, Smooth));
    }

    void BlinkForeverOrContinue(Color_t Clr, uint32_t ASmooth) {
        chSysLock();
        if(State == staIdle) {
            State = staBlinkFadeIn;
            ClrTarget = Clr;
            Smooth = ASmooth;
            ClrCurr = clBlack;
            StartTimerI(ClrCurr.DelayToNextAdj(ClrTarget, Smooth));
        }
        ClrBlink = Clr;
        BlinkForever = true;
        chSysUnlock();
    }

    void BlinkOnce(Color_t Clr, uint32_t ASmooth) {
        chSysLock();
        StopTimerI();
        State = staBlinkFadeIn;
        ClrTarget = Clr;
        Smooth = ASmooth;
        ClrCurr = clBlack;
        ClrBlink = clBlack;
        BlinkForever = false;
        StartTimerI(ClrCurr.DelayToNextAdj(ClrTarget, Smooth));
        chSysUnlock();
    }

    void Set(Color_t Clr, uint32_t ASmooth) {
        chSysLock();
        if(State != staIdle) {
            StopTimerI();
            State = staIdle;
        }
        ClrTarget = Clr;
        Smooth = ASmooth;
        chSysUnlock();
    }

    bool IsIdle() { return State == staIdle and ClrCurr == clBlack; }
};

static Blade_t Blade;
static Gem_t Gem;
static thread_reference_t ThdRef;

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
        // Draw or switch off
        if(Leds.AreOff()) LedPwrOff();
        else {
            LedPwrOn();
            Leds.SetCurrentColors(); // Send current buf to leds
        }
        // Process effects
        Blade.Draw();
        Gem.Draw();
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

void SetBlade(Color_t Clr, uint32_t ASmooth) {
    Blade.ClrTarget = Clr;
    Blade.Smooth = ASmooth;
}

void StartBatteryIndication(uint32_t ABattery_mV) {
    Printf("VBat: %u mV\r", ABattery_mV);
    // Calculate color and size
    Color_t Clr;
    uint32_t Sz;
    if(ABattery_mV <= 3300) {
        Clr.FromRGB(255, 0, 30);
        Sz = 2;
    }
    else if(ABattery_mV <= 3400) {
        Clr.FromRGB(255, 0, 0);
        Sz = 4;
    }
    else if(ABattery_mV <= 3600) {
        Clr.FromRGB(255, 30, 0);
        Sz = 6;
    }
    else if(ABattery_mV <= 3800) {
        Clr.FromRGB(255, 100, 0);
        Sz = 8;
    }
    else if(ABattery_mV <= 4000) {
        Clr.FromRGB(255, 255, 0);
        Sz = 10;
    }
    else {
        Clr.FromRGB(0, 255, 0);
        Sz = 12;
    }
    Blade.DoBatteryIndication(Clr, Sz);
}


void SetGem(Color_t Clr, uint32_t ASmooth) { Gem.Set(Clr, ASmooth); }
void GemBlinkOnce(Color_t Clr, uint32_t ASmooth) { Gem.BlinkOnce(Clr, ASmooth); }
void GemBlinkForeverOrContinue(Color_t Clr, uint32_t ASmooth) { Gem.BlinkForeverOrContinue(Clr, ASmooth); }
bool GemIsIdle() { return Gem.IsIdle(); }

void WaitLedsOff() {
    while(!Gem.IsIdle()) {
        Iwdg::Reload();
        chThdSleepMilliseconds(9);
    }
}

} // namespace
