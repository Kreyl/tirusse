/*
 * LedEffects.h
 *
 *  Created on: 29 янв. 2022 г.
 *      Author: layst
 */

#pragma once

#include "color.h"

#define BATTERY_INDICATION_DURATION_MS  1530

namespace Eff {
void Init();

void SetBlade(Color_t Clr, uint32_t ASmooth);
void StartBatteryIndication(uint32_t ABattery_mV);

void SetGem(Color_t Clr, uint32_t ASmooth);
void GemBlinkOnce(Color_t Clr, uint32_t ASmooth);
void GemBlinkForeverOrContinue(Color_t Clr, uint32_t ASmooth);
bool GemIsIdle();

void WaitLedsOff();

}
