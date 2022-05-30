/* 
 * File:   battery.h
 * Author: Kreyl Laurelindo
 *
 * Created on 9 Июнь 2010 г., 23:43
 */

#ifndef _BATTERY_H
#define	_BATTERY_H

#include <avr/io.h>

// ============================== Defins =======================================
//#define BAT_CHARGER   // Uncomment this if charger is enabled
#define BAT_ADC

#ifdef BAT_CHARGER
#define BAT_CHRG_P      PB1
#define BAT_CHRG_DDR    DDRB
#define BAT_CHRG_PORT   PORTB
#define BAT_CHRG_PIN    PINB

#define BAT_CHRG_POLL_PERIOD 500 // ms
#endif

#ifdef BAT_ADC
#define BAT_ADC_P       PA7
#define BAT_ADC_DDR     DDRA
#define BAT_ADC_PORT    PORTA
#define BAT_ADC_PIN     PINA
#define BAT_SWITCH      PA6

// Number of measures as power of 2: 0 means 1, 1 means 2, 3 means 8 and so on
// Needed for averaging of values
#define BAT_POWER_OF_MEASURES   3   // Power of 2
#define BAT_NUMBER_OF_MEASURES  (1<<BAT_POWER_OF_MEASURES)

#define BAT_U_DISCHARGED    720 // About 3.6V
#define BAT_MEASURE_PERIOD  999 // ms; Period of battery measurement
#define ADC_PREPARE_TIMEOUT 50  // ms; Time for ADC to prepare

enum Bat_ADC_State_t {ADCNoMeasure, ADCInit, ADCMeasuring};
#endif

// ============================== Types ========================================
struct Battery_t {
    #ifdef BAT_CHARGER
    uint16_t ChargeTimer;
    bool IsCharging;
    #endif
    #ifdef BAT_ADC
    uint16_t ADCTimer;
    uint8_t MeasuresCounter;
    uint16_t ADCValue;
    enum Bat_ADC_State_t ADCState;
    #endif
};
extern struct Battery_t Battery;

#ifdef BAT_ADC
#define BATTERY_IS_DISCHARGED()     Battery.ADCValue <= BAT_U_DISCHARGED
#define ADC_MEASUREMENT_COMPLETED() bit_is_clear (ADCSRA, ADSC)
#define BATTERY_METER_ON()          BAT_ADC_PORT &= ~(1<<BAT_SWITCH)
#define BATTERY_METER_OFF()         BAT_ADC_PORT |=  (1<<BAT_SWITCH)
// Enable ADC, no IRQ, CLK/8 = 1 Mhz/8 = 125 kHz
#define ADC_START_MEASUREMENT()     ADCSRA = (1<<ADEN)|(1<<ADSC)|(0<<ADATE)|(0<<ADIE)|(0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
#define ADC_DISABLE()               ADCSRA = 0
// Internal 2.56 V, not left-adj, ADC7
#define ADC_REF_ENABLE()            ADMUX = (1<<REFS1)|(1<<REFS0)|(0<<ADLAR)|(0<<MUX4)|(0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)
#define ADC_REF_DISABLE()           ADMUX = 0
#endif

// ============================ Prototypes =====================================
void Battery_Task (void);

void BatteryInit(void);

#ifdef BAT_CHARGER
void EVENT_ChargeStarted(void);
void EVENT_ChargeEnded(void);
#endif
#ifdef BAT_ADC
void EVENT_ADCMeasureCompleted(void);
#endif

#endif	/* _BATTERY_H */

