/*
 * File:   main.c of Tirusse of Tirusse project
 * Author: Kreyl Laurelindo
 *
 * Created on 2010.04.07
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/eeprom.h>

#include "calling_stone.h"
#include "cc/cc1101.h"
#include "common/time_utils.h"
#include "common/common.h"
#include "common/battery.h"

// ================================= Types =====================================
struct {
    uint16_t Timer;
    bool IsOn;
    bool Blink;
} ELED;
struct {
    uint16_t Timer;
} CC_Srv;

// =============================== General =====================================
int main(void) {
    GeneralInit();
    sei();

    // ******** Main cycle *********
    while (1){
        wdt_reset();    // Reset watchdog
        CC_Task();
        LED_Task();
        Battery_Task();
    } // while 1
}

FORCE_INLINE void GeneralInit(void){
    wdt_enable(WDTO_2S);
    ACSR = 1<<ACD;  // Disable analog comparator
    TimerInit();

    // LED init
    LED_DDR |= (1<<LED_P);
    TimerResetDelay(&ELED.Timer);
    ELED.IsOn = false;
    ELED.Blink = true;

    // CC init
    TimerResetDelay(&CC_Srv.Timer);
    CC_Init();
    uint8_t Channel = eeprom_read_byte(EE_CHANNEL_ADDRESS);
    if (Channel == 0xFF) Channel = 0;   // Erased eeprom
    CC_SetChannel(Channel);             // Set needed channel
    CC_SetAddress(4);                   //Never changes in CC itself

    // Battery
    BatteryInit();
}

// ================================ Tasks ======================================
void LED_Task(void) {
    if (ELED.Blink) {
        if (ELED.IsOn) {
            if (TimerDelayElapsed(&ELED.Timer, LED_ON_PERIOD)) {
                ELED.IsOn = false;
                LED_OFF();
            }
        }
        else {
            if (TimerDelayElapsed(&ELED.Timer, LED_OFF_PERIOD)) {
                ELED.IsOn = true;
                LED_ON();
            }
        }
    } // if blink
    else LED_ON();  // Light continuously
}

void CC_Task (void){
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
            // Transmit at once if IDLE
            // Prepare CALL packet
            CC.TX_Pkt.Address = 0;     // Broadcast
            CC.TX_Pkt.PacketID++;      // Current packet ID, to avoid repeative treatment
            CC.TX_Pkt.CommandID = 0xCA;// }
            CC.TX_Pkt.Data[0] = 0x11;  // } CALL packet

            CC_WriteTX (&CC.TX_PktArray[0], CC_PKT_LENGTH); // Write bytes to FIFO
            CC_ENTER_TX();
            break;
        
        default: // Just get out in case of RX, TX, FSTXON, CALIBRATE, SETTLING
            break;
    }//Switch
}


// =============================== Events ======================================
void EVENT_ADCMeasureCompleted() {
    // Choose mode of LED
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (Battery.ADCValue <= BAT_U_DISCHARGED) {
            ELED.Blink = false;
        }
        else ELED.Blink = true;
    }
}