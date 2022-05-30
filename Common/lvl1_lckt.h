/*
 * lvl1_lckt.h
 *
 *  Created on: 12.06.2013
 *      Author: kreyl
 */

/*
 * Level 1 of locket protocol.
 * Level 1 is responsible to sync rpkt exchange and therefore has
 * high priority.
 */

#ifndef LVL1_LCKT_H_
#define LVL1_LCKT_H_

#include "ch.h"
#include "hal.h"
#include "main.h"   // To distinct between TX2 and Tirusse
#include "kl_lib_f100.h"

// ============================== Pkt_t ========================================
struct rPkt_t {
    uint8_t TheByte;
    int8_t RSSI;        // Received signal level, RX use only
} __attribute__ ((__packed__));
#define RPKT_LEN    (sizeof(rPkt_t)-1)  // Exclude RSSI

#define THE_BYTE        0xCA

// =========================== Address space ===================================
// Devices
#define RDEV_BOTTOM_ID  1
#define RDEV_TOP_ID     50

#define RDEVICE_CNT     ((1+RDEV_TOP_ID)-RDEV_BOTTOM_ID)
#define CHANNEL_ZERO    10

// ============================== Timings ======================================
#define RX_DURATION_MS          18  // How long to listen
#define DISAPPEAR_TIMEOUT_MS    3600

// ================================ Level1 =====================================
class rLevel1_t {
private:
#ifdef TIRUSSE
    rPkt_t PktRx;
    EventSource IEvtSrcAppearance;
    VirtualTimer DisappearTmr;
#endif
public:
#ifdef TX2
    void Init(uint16_t ASelfID, uint8_t TxPwr);
    rPkt_t PktTx;       // Local rPkt to transmit
    inline void TxDone();
#else
    bool SomethingIsNear;
    bool Enabled;
    void Init();
    void RegisterEvtAppearance(EventListener *PEvtLstnr, uint8_t EvtMask) { chEvtRegisterMask(&IEvtSrcAppearance, PEvtLstnr, EvtMask); }
    // Inner use
    void Task();
    void TimeoutHandler();
#endif
};

extern rLevel1_t rLevel1;

#endif /* LVL1_LCKT_H_ */
