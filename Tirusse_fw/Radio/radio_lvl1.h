/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#pragma once

#include "kl_lib.h"
#include "ch.h"
#include "cc1101.h"
#include "kl_buf.h"
#include "uart2.h"
#include "MsgQ.h"

__unused
static const uint8_t PwrTable[12] = {
        CC_PwrMinus30dBm, // 0
        CC_PwrMinus27dBm, // 1
        CC_PwrMinus25dBm, // 2
        CC_PwrMinus20dBm, // 3
        CC_PwrMinus15dBm, // 4
        CC_PwrMinus10dBm, // 5
        CC_PwrMinus6dBm,  // 6
        CC_Pwr0dBm,       // 7
        CC_PwrPlus5dBm,   // 8
        CC_PwrPlus7dBm,   // 9
        CC_PwrPlus10dBm,  // 10
        CC_PwrPlus12dBm   // 11
};

#if 1 // =========================== Pkt_t =====================================
union rPkt_t {
    uint32_t DW32[2];
    struct {
        uint8_t ID; // Required to distinct packets from same src
        uint8_t TimeSrc;
        uint8_t HopCnt;
        uint16_t iTime;
        // Payload
        uint8_t Type;
        int8_t Rssi; // Will be set after RX. Transmitting is useless, but who cares.
        uint8_t Salt;
    } __attribute__((__packed__));
    rPkt_t& operator = (const rPkt_t &Right) {
        DW32[0] = Right.DW32[0];
        DW32[1] = Right.DW32[1];
        return *this;
    }
} __attribute__ ((__packed__));
#endif

#define RPKT_LEN    sizeof(rPkt_t)
#define RPKT_SALT   0xCA

#if 1 // =================== Channels, cycles, Rssi  ===========================
#define RCHNL_EACH_OTH      2

#define RX_DURATION_MS      252
#define SLEEP_DURATION_MS   999

#endif

#if 1 // ============================= RX Table ================================
struct rPayload_t {
    uint8_t IsValid = 0;
    uint8_t Type;
} __attribute__ ((__packed__));

#define RXTABLE_SZ          104

// RxTable must be cleared during processing
class RxTable_t {
private:
    rPayload_t IBuf[RXTABLE_SZ];
public:
    void AddOrReplaceExistingPkt(rPkt_t *APkt) {
        chSysLock();
        AddOrReplaceExistingPktI(APkt);
        chSysUnlock();
    }
    void AddOrReplaceExistingPktI(rPkt_t *pPkt) {
        if(pPkt->ID < RXTABLE_SZ) {
            IBuf[pPkt->ID].Type = pPkt->Type;
            IBuf[pPkt->ID].IsValid = 1;
        }
    }

    rPayload_t& operator[](const int32_t Indx) {
        return IBuf[Indx];
    }

    void ProcessCountingDistinctTypes(uint32_t *TypeTable, uint8_t TableSz) {
        for(auto &Payload : IBuf) {
            if(Payload.IsValid and Payload.Type < TableSz) {
                TypeTable[Payload.Type]++;
                Payload.IsValid = 0; // Clear item for future use
            }
        }
    }

    void Print() {
        Printf("RxTable\r");
        for(uint32_t i=0; i<RXTABLE_SZ; i++) {
            if(IBuf[i].IsValid) Printf("ID: %u; Type: %u\r", i, IBuf[i].Type);
        }
    }
};
#endif

class rLevel1_t {
private:
    RxTable_t RxTable1, RxTable2, *RxTableW = &RxTable1;
    bool Enabled = true;
public:
    uint8_t TxPower;

    void AddPktToRxTableI(rPkt_t *pPkt) { RxTableW->AddOrReplaceExistingPktI(pPkt); }

    RxTable_t* GetRxTable() {
        chSysLock();
        RxTable_t* RxTableR;
        // Switch tables
        if(RxTableW == &RxTable1) {
            RxTableW = &RxTable2;
            RxTableR = &RxTable1;
        }
        else {
            RxTableW = &RxTable1;
            RxTableR = &RxTable2;
        }
        chSysUnlock();
        return RxTableR;
    }

    uint8_t Init();
    void Stop();

    // Inner use
    void ITask();
};

extern rLevel1_t Radio;
