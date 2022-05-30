#include <stdbool.h>
#include "time_utils.h"
#include "battery.h"
#include "common.h"

#define BAT_IS_CHARGING()   bit_is_clear (BAT_CHRG_PIN, BAT_CHRG_P)

struct Battery_t Battery;

// ============================= Implementation ================================
FORCE_INLINE void Battery_Task (void) {
    #ifdef BAT_CHARGER
    if (TimerDelayElapsed(&Battery.ChargeTimer, BAT_CHRG_POLL_PERIOD)) {
        // Check if charging just has begun
        if (BAT_IS_CHARGING()) {
            if (!Battery.IsCharging) {
                Battery.IsCharging = true;
                EVENT_ChargeStarted();
            }
        }
        else {
            if (Battery.IsCharging) {
                Battery.IsCharging = false;
                EVENT_ChargeEnded();
            }
        } // if charging
    } // Timer
    #endif
    #ifdef BAT_ADC
    switch (Battery.ADCState) {
        case ADCNoMeasure:
            if (TimerDelayElapsed(&Battery.ADCTimer, BAT_MEASURE_PERIOD)) { // Check if timeout has passed
                // Start meter
                BATTERY_METER_ON();
                ADC_REF_ENABLE();           // Start ADC
                ADC_START_MEASUREMENT();    // Start new measure - dummy one for the first time
                Battery.ADCState = ADCInit;
            }
            break;

        case ADCInit:
            // Check if dummy measurement completed & timeout passed
            if (ADC_MEASUREMENT_COMPLETED() && TimerDelayElapsed (&Battery.ADCTimer, ADC_PREPARE_TIMEOUT)) {
                ADC_START_MEASUREMENT();    // Start new measure
                Battery.ADCValue = 0;
                Battery.MeasuresCounter = 0;
                Battery.ADCState = ADCMeasuring;
            }
            break;

        case ADCMeasuring:
            if (ADC_MEASUREMENT_COMPLETED()) {  // Check if current measurement completed
                Battery.MeasuresCounter++;
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    Battery.ADCValue += ADC;    // Add ADC result
                } // atomic
                // Check if series of measurements is completed
                if (Battery.MeasuresCounter >= BAT_NUMBER_OF_MEASURES) {
                    // Disable ADC to save energy
                    ADC_DISABLE();
                    ADC_REF_DISABLE();
                    BATTERY_METER_OFF();
                    // Prepare result
                    Battery.ADCValue >>= BAT_POWER_OF_MEASURES;
                    Battery.ADCState = ADCNoMeasure;
                    TimerResetDelay(&Battery.ADCTimer);
                    EVENT_ADCMeasureCompleted();
                }
                // if not, start measurement again
                else ADC_START_MEASUREMENT();
            } // if current measurement completed
            break;
    } // switch
    #endif
}

FORCE_INLINE void BatteryInit(void) {
    #ifdef BAT_CHARGER
    BAT_CHRG_DDR  &= ~(1<<BAT_CHRG_P);
    BAT_CHRG_PORT |=  (1<<BAT_CHRG_P);
    Battery.IsCharging = false;
    TimerResetDelay(&Battery.ChargeTimer);
    #endif
    #ifdef BAT_ADC
    BAT_ADC_DDR |=  (1<<BAT_SWITCH);
    BAT_ADC_DDR &= ~(1<<BAT_ADC_P);
    BATTERY_METER_OFF();
    TimerResetDelay(&Battery.ADCTimer);
    Battery.ADCState = ADCNoMeasure;
    #endif
}