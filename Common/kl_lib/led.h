/*
 * led.h
 *
 *  Created on: 03.11.2011
 *      Author: g.kruglov
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f10x.h"
#include "ch.h"
#include "hal.h"
#include "kl_lib_f100.h"

#define LED_SMOOTH  TRUE
#define LED_RGB     FALSE


/*
class Led_t {
protected:
    uint16_t IPinNumber;
    GPIO_TypeDef* IGPIO;
public:
    void Init(GPIO_TypeDef *PGpioPort, uint16_t APinNumber) { IGPIO = PGpioPort; IPinNumber = APinNumber; PinSetupOut(PGpioPort, APinNumber, GPIO_Mode_Out_PP); }
    void On(void)  { PinSet  (IGPIO, IPinNumber); }
    void Off(void) { PinClear(IGPIO, IPinNumber); }
};

class LedBlink_t : public Led_t {
private:
    uint32_t ITimer, IBlinkDelay;
    bool IsInsideBlink;
public:
    void Disable(void) { Off(); IsInsideBlink = false; }
    void Blink(uint32_t ABlinkDelay);
    void Task(void);
};

class LedBlinkInverted_t {
private:
    uint32_t ITimer, IBlinkDelay;
    bool IsInsideBlink;
    uint16_t IPinNumber;
    GPIO_TypeDef* IGPIO;
public:
    void Init(GPIO_TypeDef *PGpioPort, uint16_t APinNumber) { IGPIO = PGpioPort; IPinNumber = APinNumber; klGpioSetupByN(PGpioPort, APinNumber, GPIO_Mode_Out_OD); }
    void On(void)  { klGpioClearByN(IGPIO, IPinNumber); }
    void Off(void) { klGpioSetByN  (IGPIO, IPinNumber); }
    void Disable(void) { Off(); IsInsideBlink = false; }
    void Blink(uint32_t ABlinkDelay) {
        IsInsideBlink = true;
        IBlinkDelay = ABlinkDelay;
        On();
        Delay.Reset(&ITimer);
    }
    void Task(void) {
        if (!IsInsideBlink) return;
        if (Delay.Elapsed(&ITimer, IBlinkDelay)) {
            IsInsideBlink = false;
            Off();
        }
    }
};
*/

#if LED_SMOOTH

#define LED_GPIO            GPIOB
#define LED_GPIO_N          0
#define LED_TIM_N           3
#define LED_CCR_N           3
#define LED_TOP_VALUE       1998
#define LED_BOTTOM_VALUE    0
#define LED_INITIAL_VALUE   LED_TOP_VALUE
#define LED_PWM_FREQ_HZ     999             // No more than 3 kHz

enum SmLedState_t {slsNone, slsGlimmer};
class LedSmooth_t {
private:
    VirtualTimer ITmr;
    PwmPin_t IPin;
    SmLedState_t IState;
    uint16_t IMax, IMin;
    uint16_t ICurrentValue, INeededValue;
    uint32_t ISetupDelay(uint16_t AValue) {
        return (uint32_t)((810 / (AValue+4)) + 1);
//        if (IValue < 11) IDelay = 81;
//        else if (IValue < 27) IDelay = 36;
//        else IDelay = 9;
    }
public:
    void Init();
    void Set(uint16_t AValue) { IPin.Set(AValue); ICurrentValue = AValue; }
    void SetSmoothly(uint16_t AValue);
    bool IsEqOrAbove(uint16_t AValue) { return (ICurrentValue >= AValue); }
    bool IsEqOrBelow(uint16_t AValue) { return (ICurrentValue <= AValue); }
    void Glimmer(uint16_t AMax, uint16_t AMin);
    // Inner use
    void IrqHandlerI();
};

extern LedSmooth_t LedSmooth;
#endif

// Colors
struct Color_t {
    uint8_t Red, Green, Blue;
    bool operator == (const Color_t AColor) { return ((Red == AColor.Red) and (Green == AColor.Green) and (Blue == AColor.Blue)); }
    //bool operator != (const Color_t AColor) { return ((this->Red != AColor.Red) || (this->Green != AColor.Green) || (this->Blue != AColor.Blue)); }
};
#define clBlack     ((Color_t){0,   0,   0})
#define clRed       ((Color_t){255, 0,   0})
#define clGreen     ((Color_t){0,   255, 0})
#define clBlue      ((Color_t){0,   0,   255})
#define clYellow    ((Color_t){255, 255, 0})
#define clViolet    ((Color_t){255, 0, 255})
#define clCyan      ((Color_t){0, 255, 255})
#define clWhite     ((Color_t){255, 255, 255})

#if LED_RGB
/*
 * RGB LED
 * Provides smooth color change.
 */
#define LED_SETUP_DELAY_MS  45

#define LED_TIM         TIM2
#define LED_RED_CCR     CCR3
#define LED_GREEN_CCR   CCR4
#define LED_BLUE_CCR    CCR2
#define LED_RCC_EN()    rccEnableTIM2()

class LedRGB_t {
private:
	Color_t ICurrentColor, INeededColor;
	Thread *PThread;
	bool IsSleeping;
	void ISetRed  (uint8_t AValue) {LED_TIM->LED_RED_CCR   = AValue;}
	void ISetGreen(uint8_t AValue) {LED_TIM->LED_GREEN_CCR = AValue;}
	void ISetBlue (uint8_t AValue) {LED_TIM->LED_BLUE_CCR  = AValue;}
public:
	void Init();
	void Task();
	void SetColor(Color_t AColor) {
		ISetRed(AColor.Red);
		ISetGreen(AColor.Green);
		ISetBlue(AColor.Blue);
		ICurrentColor = AColor;
	}
	void SetColorSmoothly(Color_t AColor);
	bool IsOff() { return (ICurrentColor == INeededColor) and (ICurrentColor == clBlack); }
//	void Blink(uint32_t ABlinkDelay, Color_t AColor) {
//	        IsInsideBlink = true;
//	        IBlinkDelay = ABlinkDelay;
//	        INeededColor = AColor;
//	        SetColor(AColor);
//	        Delay.Reset(&ITimer);
//	    }
};

extern LedRGB_t Led;

#endif // LED_RGB

#endif /* LED_H_ */
