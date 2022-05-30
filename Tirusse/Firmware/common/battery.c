#include <stdbool.h>
#include "time_utils.h"
#include "battery.h"
#include "common.h"

#define BAT_IS_CHARGING()   bit_is_clear (BAT_CHRG_PIN, BAT_CHRG_P)

struct Battery_t Battery;

// ============================= Implementation ================================
FORCE_INLINE void Battery_Task (void) {
    if (!TimerDelayElapsed(&Battery.Timer, BAT_POLL_PERIOD)) return;
    #ifdef BAT_CHARGER
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
    }
    #endif

}

FORCE_INLINE void BatteryInit(void) {
    #ifdef BAT_CHARGER
    BAT_CHRG_DDR  &= ~(1<<BAT_CHRG_P);
    BAT_CHRG_PORT |=  (1<<BAT_CHRG_P);
    Battery.IsCharging = false;
    #endif
    TimerResetDelay(&Battery.Timer);
}