/*
 * LedEffects.h
 *
 *  Created on: 29 янв. 2022 г.
 *      Author: layst
 */

#pragma once

#include "color.h"

namespace Eff {
void Init();

void SetGem(Color_t Clr, uint32_t ASmooth);
void SetBlade(Color_t Clr, uint32_t ASmooth);
void StartBatteryIndication(uint32_t ABattery_mV);

}
