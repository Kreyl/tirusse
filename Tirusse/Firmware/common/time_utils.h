/* 
 * File:   time_utils.h
 * Author: Laurelindo
 *
 * Created on 5 Октябрь 2009 г., 0:40
 */

#ifndef _TIME_UTILS_H
#define	_TIME_UTILS_H

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/atomic.h>

// ============================== Types ========================================
//struct Time_t {
//    uint8_t Hour;
//    uint8_t Minute;
//    uint8_t Second;
//    uint16_t Timer;
//};

// =========================== Global variables ================================
extern volatile uint16_t TickCounter;

// ========================== Function Prototypes ==============================
void TimerInit (void);
bool TimerDelayElapsed (uint16_t *AVar, const uint16_t ADelay);
void TimerResetDelay (uint16_t *AVar);

//void PrintTime (struct Time_t *ATime);

//void IncSecond (struct Time_t *ATime);

#endif	/* _TIME_UTILS_H */

