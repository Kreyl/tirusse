/* 
 * File:   main.h of Tirusse
 * Author: Kreyl Laurelindo
 *
 * Created on 7 Апрель 2010 г., 19:44
 */

#ifndef _MAIN_H
#define	_MAIN_H

#include <avr/io.h>

// ================================= LED =======================================
#define LED_DDR   DDRC
#define LED_PORT  PORTC
#define LED_P     PC6

#define LED_ON()  LED_PORT |=  (1<<LED_P)
#define LED_OFF() LED_PORT &= ~(1<<LED_P)

#define LED_ON_PERIOD   99 // ms
#define LED_OFF_PERIOD  999 // ms

// ================================ Common =====================================
#define EE_CHANNEL_ADDRESS  (uint8_t*)4

// =============================== Prototypes ==================================
void GeneralInit(void);

void CC_Task (void);
void LED_Task(void);

#endif	/* _MAIN_H */

