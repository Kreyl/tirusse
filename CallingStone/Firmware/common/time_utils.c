#include "time_utils.h"

volatile uint16_t TickCounter;

void TimerInit (void) {
    // Millisecond timer initialization, with output compare interrupt enabled
    #ifdef TCCR0A
    TCCR0A = (1<<WGM01);                    // CTC mode
    TCCR0B = (0<<CS02)|(1<<CS01)|(0<<CS00); // 1 MHz/8 = 125 kHz
    OCR0A  = 125;                           // 125 kHz / 125 = 1000 compares per second
    TIMSK0 |= (1<<OCIE0A);                  // Enable interrupt
    #else
    TCCR0 = (1<<WGM01)|(0<<WGM00)|(0<<CS02)|(1<<CS01)|(0<<CS00);// CTC mode, 1 MHz/8 = 125 kHz
    OCR0  = 125;                                                // 125 kHz / 125 = 1000 compares per second
    TIMSK |= (1<<OCIE0);                                        // Enable interrupt
    #endif
}

bool TimerDelayElapsed (uint16_t *AVar, const uint16_t ADelay) {
    uint16_t ttmp;
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        ttmp = TickCounter;
    }
    if ((ttmp - *AVar) >= ADelay) {
        *AVar = ttmp;   // Reset delay
        return true;
    }
    else return false;
}

void TimerResetDelay (uint16_t *AVar) {
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        *AVar = TickCounter;
    }
}

/*
void PrintTime (struct Time_t *ATime) {
    printf_P (PSTR("\r\n %d:%d:%d"), ATime->Hour, ATime->Minute, ATime->Second);
}

void IncSecond (struct Time_t *ATime) {
    if (ATime->Second == 59) {
        ATime->Second = 0;
        if (ATime->Minute == 59) {
            ATime->Minute = 0;
            if (ATime->Hour == 23) ATime->Hour = 0;
            else ATime->Hour++;
        } // if min==59
        else ATime->Minute++;
    } // if s==59
    else ATime->Second++;
}
*/

// ================================ Interrupts =================================
// ISR for the timer 0 compare vector. This ISR fires once each millisecond,
// and increments the tick counter.
#ifdef TIMER0_COMPA_vect
ISR (TIMER0_COMPA_vect, ISR_BLOCK) {
    TickCounter++;
}
#else
ISR (TIMER0_COMP_vect, ISR_BLOCK) {
    TickCounter++;
}
#endif
