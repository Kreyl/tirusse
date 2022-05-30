/* 
 * File:   cc2500.h
 * Author: Laurelindo
 *
 * Created on 11 Декабрь 2009 г., 2:08
 */

#ifndef _CC1101_H
#define	_CC1101_H

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "cc1101defins.h"
#include "cc_rf_settings.h"
#include "../common/common.h"

// ================================== Defins ===================================
// Ports
#define CC_DDR  DDRB
#define CC_PORT PORTB
#define CC_PIN  PINB

#define CC_GDO0 PB2
#define CC_GDO2 PB3
#define CC_CS   PB4
#define CC_MOSI PB5
#define CC_MISO PB6
#define CC_SCLK PB7

// =========================== Pseudo functions ================================
#define CC_SCLK_HI  CC_PORT |=  (1<<CC_SCLK)
#define CC_SCLK_LO  CC_PORT &= ~(1<<CC_SCLK)
#define CC_MOSI_HI  CC_PORT |=  (1<<CC_MOSI)
#define CC_MOSI_LO  CC_PORT &= ~(1<<CC_MOSI)
#define CC_CS_HI    CC_PORT |=  (1<<CC_CS)
#define CC_CS_LO    CC_PORT &= ~(1<<CC_CS)

#define CC_MISO_IS_HI( )    bit_is_set(CC_PIN, CC_MISO)
#define CC_GDO0_IS_HI( )    bit_is_set(CC_PIN, CC_GDO0)
#define CC_GDO2_IS_HI( )    bit_is_set(CC_PIN, CC_GDO2)


#ifdef __AVR_ATmega324P__
#define CC_GDO0_IRQ_ENABLE( )   EIMSK |=  (1<<INT2)
#define CC_GDO0_IRQ_DISABLE( )  EIMSK &= ~(1<<INT2)
#elif defined (__AVR_ATmega16__) || defined (__AVR_ATmega16A__)
#define CC_GDO0_IRQ_ENABLE( )   GICR |=  (1<<INT2)
#define CC_GDO0_IRQ_DISABLE( )  GICR &= ~(1<<INT2)
#endif

// =============================== Variables ===================================
#define CC_PKT_DATA_LENGTH  CC_PKT_LENGTH-3
struct CC_Packet_t{
    uint8_t Address;
    uint8_t PacketID;
    uint8_t CommandID;
    uint8_t Data[CC_PKT_DATA_LENGTH];
    uint8_t RSSI;
    uint8_t LQI;
};

struct CC_t {
    uint8_t State;
    union {
        uint8_t RX_PktArray[sizeof(struct CC_Packet_t)];
        struct CC_Packet_t RX_Pkt;
    };
    union {
        uint8_t TX_PktArray[sizeof(struct CC_Packet_t)];
        struct CC_Packet_t TX_Pkt;
    };
    bool NewPacketReceived;
};

extern struct CC_t CC;


// ================================ Prototypes =================================
// High level
void CC_Init(void);
void CC_SetChannel(uint8_t AChannel);
void CC_SetAddress(uint8_t AAdress);

// Middle level
uint8_t CC_ReadRegister (uint8_t ARegAddr);
void CC_WriteRegister (uint8_t ARegAddr, uint8_t AData);
void CC_WriteStrobe (uint8_t AStrobe);

// Inner use
void CC_WriteBurst(uint8_t ARegAddr, uint8_t *PData, uint8_t ALength);
void CC_WriteBurst_P(uint8_t ARegAddr, prog_uint8_t *PData, uint8_t ALength);
void CC_WriteTX (uint8_t *PData, uint8_t ALength);
void CC_ReadRX  (uint8_t *PData, uint8_t ALength);
void CC_RfConfig(void);

// low-level
uint8_t CC_ReadWriteByte(uint8_t AByte);
#define CC_ReadByte( )      CC_ReadWriteByte(0)
#define CC_WriteByte(AByte) CC_ReadWriteByte(AByte)

// ========================== Hi-level pseudo functions ========================
// Strobes
#define CC_RESET( )         CC_WriteStrobe(CC_SRES)
#define CC_ENTER_TX( )      CC_WriteStrobe(CC_STX)
#define CC_ENTER_RX( )      CC_WriteStrobe(CC_SRX)
#define CC_ENTER_IDLE( )    CC_WriteStrobe(CC_SIDLE)
#define CC_POWERDOWN( )     CC_WriteStrobe(CC_SPWD)
#define CC_CALIBRATE()      CC_WriteStrobe(CC_SCAL)
#define CC_FLUSH_RX_FIFO()  CC_WriteStrobe(CC_SFRX)
#define CC_FLUSH_TX_FIFO()  CC_WriteStrobe(CC_SFTX)
#define CC_GET_STATE()      CC_WriteStrobe(CC_SNOP)

#endif	/* _CC1101_H */

