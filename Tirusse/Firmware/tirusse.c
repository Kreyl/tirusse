/*
 * File:   main.c of Tirusse of Tirusse project
 * Author: Kreyl Laurelindo
 *
 * Created on 2010.04.07
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "tirusse.h"
#include "cc/cc1101.h"
#include "common/time_utils.h"
#include "common/battery.h"
#include "common/common.h"

// ================================= Types =====================================
struct {
    uint16_t Timer;
    uint8_t PWM, PWMDesired;
} ELED;
struct {
    bool Detected;
    bool IsHere;
    uint16_t Timer;
} CStone;
struct {
    uint16_t Timer;
    bool JustEnteredRX;
    bool DeepSleep;
    uint8_t Channel;
} CC_Srv;

// =============================== General =====================================
int main(void) {
    GeneralInit();

    sei();

    // ******** Main cycle *********
    while (1){
        wdt_reset();    // Reset watchdog
        CC_Task();
        Stone_Task();
        LED_Task();
        Battery_Task();
        IndicateCharging_TASK();
    } // while 1
}

FORCE_INLINE void GeneralInit(void){
    wdt_enable(WDTO_2S);
    #ifdef LED_DEBUG
    LED_DDR |= (1<<LED_P);
    #endif
    ACSR = 1<<ACD;  // Disable analog comparator
    TimerInit();

    // PWM init
    ELED.PWMDesired = 0;
    ELED.PWM = PWM_MAX; // Light-up at power-on
    PWMEnable();
    TimerResetDelay(&ELED.Timer);

    // Stone init
    CStone.Detected = false;
    CStone.IsHere = false;
    TimerResetDelay(&CStone.Timer);

    // CC init
    CC_Srv.Channel = CC_CHANNEL_START;
    CC_Srv.JustEnteredRX = false;
    CC_Srv.DeepSleep = false;
    TimerResetDelay(&CC_Srv.Timer);
    CC_Init();
    CC_SetAddress(4);   //Never changes in CC itself

    // Battery
    BatteryInit();
}

// ============================= PWM functions =================================
bool MayChangePWM(void) {
    if (ELED.PWM <= PWMStepOver1) {
        return TimerDelayElapsed(&ELED.Timer, PWMDelayLong);        // Low speed
    }
    else if (ELED.PWM > PWMStepOver1 && ELED.PWM <= PWMStepOver2) {
        return TimerDelayElapsed(&ELED.Timer, PWMDelayMid);         // Mid-speed
    }
    else if (ELED.PWM > PWMStepOver2) {
        return TimerDelayElapsed(&ELED.Timer, PWMDelayFast);        // High-speed
    }
    return false;
}

FORCE_INLINE void PWMEnable(void) {
    // Fast PWM, 10 bit, no prescaling, OC1A disconnected, OC1B inverted
    TCCR1A = (1<<COM1B1)|(1<<COM1B0)|(1<<WGM11)|(1<<WGM10);
    TCCR1B = (0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);
    // Setup russe LED
    DDRD  |= (1<<PD4);
    PORTD |= (1<<PD4);
}
FORCE_INLINE void PWMDisable(void) {
    TCCR1A = 0;
    TCCR1B = 0;
}


// ================================ Tasks ======================================
FORCE_INLINE void Stone_Task(void) {
    if (!TimerDelayElapsed(&CStone.Timer, DETECTOR_TIMEOUT)) return;
    // Here is only hide event. Detected event fires within packet handler.
    if (CStone.IsHere && !CStone.Detected) {
        CStone.IsHere = false;
        EVENT_Hide();
    }
    CStone.Detected = false;         // reset detecor
}

FORCE_INLINE void LED_Task(void) {
    if (ELED.PWM != ELED.PWMDesired) {
        if (ELED.PWMDesired < ELED.PWM) {   // Lower PWM
            if (MayChangePWM()) {
                ELED.PWM--;
                OCR1BL = ELED.PWM;
            }
            // Workaround hardware PWM bug: LED does not switches off totally
            if (ELED.PWM == 0) PWMDisable();
        }
        else {
            if (ELED.PWM == 0) PWMEnable();
            if (MayChangePWM()) {
                ELED.PWM++;
                OCR1BL = ELED.PWM;
            } // if may change
        } // Fade or brighten
    } // if not desired
}

FORCE_INLINE void CC_Task (void){
    if (Battery.IsCharging) return;

    // Handle packet if received
    if (CC.NewPacketReceived) {
        CC.NewPacketReceived = false;
        EVENT_NewPacket();
    }

    if (CC_Srv.DeepSleep) {
        if (TimerDelayElapsed(&CC_Srv.Timer, CC_RX_OFF_DELAY)) CC_Srv.DeepSleep = false;
        else return;
    }
    //PORTA |= (1<<PA2); // DEBUG
    // Do with CC what needed
    CC_GET_STATE();
    switch (CC.State){
        case CC_STB_RX_OVF:
            CC_FLUSH_RX_FIFO();
            break;
        case CC_STB_TX_UNDF:
            CC_FLUSH_TX_FIFO();
            break;

        case CC_STB_IDLE:
            // Set needed channel and enter RX if time has come
            CC_SetChannel(CC_Srv.Channel);
            CC_ENTER_RX();
            CC_Srv.JustEnteredRX = true;
            // Calculate next channel
            CC_Srv.Channel++;
            if (CC_Srv.Channel > CC_CHANNEL_END) CC_Srv.Channel = CC_CHANNEL_START;
            break;
        
        case CC_STB_RX:
            // Reset timer if just entered RX
            if (CC_Srv.JustEnteredRX) {
                CC_Srv.JustEnteredRX = false;
                TimerResetDelay(&CC_Srv.Timer);
            }
            else {
                // Check if CC_RX_ON_DELAY delay elapsed
                if (TimerDelayElapsed(&CC_Srv.Timer, CC_RX_ON_DELAY)) { // Time to switch off
                    CC_ENTER_IDLE();
                    CC_POWERDOWN();
                    CC_Srv.DeepSleep = true;
                    //PORTA &= ~(1<<PA2); // DEBUG
                }// if timer
            }// if not RX
            break;
            
        default: // Just get out in case of RX, TX, FSTXON, CALIBRATE, SETTLING
            break;
    }//Switch
}

FORCE_INLINE void IndicateCharging_TASK(void) {
    if (!Battery.IsCharging) return;
    // Fade and brighten russe
    if      (ELED.PWM == CHARGING_PWM_MIN) ELED.PWMDesired = CHARGING_PWM_MAX;
    else if (ELED.PWM == CHARGING_PWM_MAX) ELED.PWMDesired = CHARGING_PWM_MIN;
}

// =============================== Events ======================================
void EVENT_Detected(void) {
    ELED.PWMDesired = PWM_MAX;
}
void EVENT_Hide(void) {
    ELED.PWMDesired = 0;
}

void EVENT_NewPacket(void) {
    if (CC.RX_Pkt.CommandID == PKT_ID_CALL) {
        CStone.Detected = true;
        // Send event if needed
        if (!CStone.IsHere) {
            CStone.IsHere = true;
            EVENT_Detected();
        }
    } // if PKT_ID_CALL
}

void EVENT_ChargeStarted(void) {
    #ifdef LED_DEBUG
    LED_ON();   // DEBUG
    #endif
    // Shutdown CC
    CC_ENTER_IDLE();
    CC_POWERDOWN();
    // Set brightness
    ELED.PWMDesired = CHARGING_PWM_MIN;
}
void EVENT_ChargeEnded(void) {
    #ifdef LED_DEBUG
    LED_OFF();   // DEBUG
    #endif
    // Set brightness
    ELED.PWMDesired = 0;
}