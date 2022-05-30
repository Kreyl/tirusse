/* 
 * File:   cc1101.c
 * Author: Laurelindo
 *
 * Created on 06/03/2010 г., 2:08
 */
#include <avr/io.h>
#include <inttypes.h>
#include <util/atomic.h>
#include "cc1101.h"

struct CC_t CC;

void CC_Init(void){
    // ******** Hardware init section *******
    // Interrupts
    CC_GDO0_IRQ_DISABLE();
    #ifdef __AVR_ATmega324P__
    EICRA |= (1<<ISC21)|(1<<ISC20); // Rising edge generates an interrupt
    EIFR |= (1<<INTF2);             // Clear IRQ flag
    #elif defined (__AVR_ATmega16__) || defined (__AVR_ATmega16A__)
    MCUCSR |= (1<<ISC2);    // Rising edge generates an interrupt
    GIFR   |= (1<<INTF2);   // Clear IRQ flag
    #endif

    // Setup ports
    CC_DDR  &= ~((1<<CC_GDO0)|(1<<CC_GDO2)|(1<<CC_MISO));
    CC_DDR  |=   (1<<CC_CS)|(1<<CC_MOSI)|(1<<CC_SCLK);
    CC_PORT |=   (1<<CC_GDO0)|(1<<CC_GDO2)|(CC_MISO); // Enable pull-ups
    // Set initial values
    CC_SCLK_LO;
    CC_CS_HI;
    // Setup SPI: MSB first, master, SCK idle low, f/4
    SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
    SPSR = (1<<SPI2X);

    // ******* Firmware init section *******
    CC.NewPacketReceived = false;
    CC_RESET();
    CC_FLUSH_RX_FIFO();
    CC_RfConfig();

    CC_GDO0_IRQ_ENABLE();
}

FORCE_INLINE void CC_SetChannel(uint8_t AChannel){
    // CC must be in IDLE mode
    while (CC.State != CC_STB_IDLE) CC_ENTER_IDLE();
    // Now set channel
    CC_WriteRegister(CC_CHANNR, AChannel);
}

FORCE_INLINE void CC_SetAddress(uint8_t AAddress) {
    CC_WriteRegister(CC_ADDR, AAddress);
}

// ============================= Inner use =====================================
void CC_WriteBurst(uint8_t ARegAddr, uint8_t *PData, uint8_t ALength){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        CC_CS_LO;                                                   // Start transmission
        while (CC_MISO_IS_HI());                                    // Wait for chip to become ready
        CC_WriteByte(ARegAddr|CC_WRITE_FLAG|CC_BURST_FLAG);         // Address with write & burst flags
        for (uint8_t i=0; i<ALength; i++) CC_WriteByte(*PData++);   // Write bytes themselves
        CC_CS_HI;                                                   // End transmission
    } // atomic
}
void CC_WriteBurst_P(uint8_t ARegAddr, prog_uint8_t *PData, uint8_t ALength){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        CC_CS_LO;                                                   // Start transmission
        while (CC_MISO_IS_HI());                                    // Wait for chip to become ready
        CC_WriteByte(ARegAddr|CC_WRITE_FLAG|CC_BURST_FLAG);         // Address with write & burst flags
        for (uint8_t i=0; i<ALength; i++) CC_WriteByte(pgm_read_byte (PData++));   // Write bytes themselves
        CC_CS_HI;                                                   // End transmission
    } // atomic
}

void CC_WriteTX (uint8_t *PData, uint8_t ALength){
    CC_WriteBurst(CC_FIFO, PData, ALength);
}
FORCE_INLINE void CC_ReadRX  (uint8_t *PData, uint8_t ALength){
    CC_CS_LO;                                                   // Start transmission
    while (CC_MISO_IS_HI());                                    // Wait for chip to become ready
    CC_WriteByte(CC_FIFO|CC_READ_FLAG|CC_BURST_FLAG);           // Address with read & burst flags
    for (uint8_t i=0; i<ALength; i++) *PData++ = CC_ReadByte(); // Write bytes themselves
    CC_CS_HI;                                                   // End transmission
}

uint8_t CC_ReadRegister (uint8_t ARegAddr){
    uint8_t FReply;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        CC_CS_LO;                               // Start transmission
        while (CC_MISO_IS_HI());                // Wait for chip to become ready
        CC_WriteByte(ARegAddr | CC_READ_FLAG);  // Transmit header byte: set READ bit and BURST flag
        FReply = CC_ReadByte();                 // Read reply
        CC_CS_HI;                               // End transmission
    }//atomic
    return FReply;
}
void CC_WriteRegister (uint8_t ARegAddr, uint8_t AData){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        CC_CS_LO;               // Start transmission
        while (CC_MISO_IS_HI());// Wait for chip to become ready
        CC_WriteByte(ARegAddr); // Transmit header byte
        CC_WriteByte(AData);    // Write data
        CC_CS_HI;               // End transmission
    }//atomic
}
void CC_WriteStrobe (uint8_t AStrobe){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        CC_CS_LO;               // Start transmission
        while (CC_MISO_IS_HI());// Wait for chip to become ready
        CC.State = CC_ReadWriteByte(AStrobe);  // Write strobe
        CC_CS_HI;               // End transmission
        CC.State &= 0b01110000; // Mask needed bits
    } // atomic
}

// **** Used to setup CC with needed values ****
void CC_RfConfig(void){
    CC_WriteRegister(CC_FSCTRL1,  CC_FSCTRL1_VALUE);    // Frequency synthesizer control.
    CC_WriteRegister(CC_FSCTRL0,  CC_FSCTRL0_VALUE);    // Frequency synthesizer control.
    CC_WriteRegister(CC_FREQ2,    CC_FREQ2_VALUE);      // Frequency control word, high byte.
    CC_WriteRegister(CC_FREQ1,    CC_FREQ1_VALUE);      // Frequency control word, middle byte.
    CC_WriteRegister(CC_FREQ0,    CC_FREQ0_VALUE);      // Frequency control word, low byte.
    CC_WriteRegister(CC_MDMCFG4,  CC_MDMCFG4_VALUE);    // Modem configuration.
    CC_WriteRegister(CC_MDMCFG3,  CC_MDMCFG3_VALUE);    // Modem configuration.
    CC_WriteRegister(CC_MDMCFG2,  CC_MDMCFG2_VALUE);    // Modem configuration.
    CC_WriteRegister(CC_MDMCFG1,  CC_MDMCFG1_VALUE);    // Modem configuration.
    CC_WriteRegister(CC_MDMCFG0,  CC_MDMCFG0_VALUE);    // Modem configuration.
    CC_WriteRegister(CC_CHANNR,   CC_CHANNR_VALUE);     // Channel number.
    CC_WriteRegister(CC_DEVIATN,  CC_DEVIATN_VALUE);    // Modem deviation setting (when FSK modulation is enabled).
    CC_WriteRegister(CC_FREND1,   CC_FREND1_VALUE);     // Front end RX configuration.
    CC_WriteRegister(CC_FREND0,   CC_FREND0_VALUE);     // Front end RX configuration.
    CC_WriteRegister(CC_MCSM0,    CC_MCSM0_VALUE);      // Main Radio Control State Machine configuration.
    CC_WriteRegister(CC_FOCCFG,   CC_FOCCFG_VALUE);     // Frequency Offset Compensation Configuration.
    CC_WriteRegister(CC_BSCFG,    CC_BSCFG_VALUE);      // Bit synchronization Configuration.
    CC_WriteRegister(CC_AGCCTRL2, CC_AGCCTRL2_VALUE);   // AGC control.
    CC_WriteRegister(CC_AGCCTRL1, CC_AGCCTRL1_VALUE);   // AGC control.
    CC_WriteRegister(CC_AGCCTRL0, CC_AGCCTRL0_VALUE);   // AGC control.
    CC_WriteRegister(CC_FSCAL3,   CC_FSCAL3_VALUE);     // Frequency synthesizer calibration.
    CC_WriteRegister(CC_FSCAL2,   CC_FSCAL2_VALUE);     // Frequency synthesizer calibration.
    CC_WriteRegister(CC_FSCAL1,   CC_FSCAL1_VALUE);     // Frequency synthesizer calibration.
    CC_WriteRegister(CC_FSCAL0,   CC_FSCAL0_VALUE);     // Frequency synthesizer calibration.
    CC_WriteRegister(CC_FSTEST,   CC_FSTEST_VALUE);     // Frequency synthesizer calibration.
    CC_WriteRegister(CC_TEST2,    CC_TEST2_VALUE);      // Various test settings.
    CC_WriteRegister(CC_TEST1,    CC_TEST1_VALUE);      // Various test settings.
    CC_WriteRegister(CC_TEST0,    CC_TEST0_VALUE);      // Various test settings.
    CC_WriteRegister(CC_FIFOTHR,  CC_FIFOTHR_VALUE);    // fifo threshold
    CC_WriteRegister(CC_IOCFG2,   CC_IOCFG2_VALUE);     // GDO2 output pin configuration.
    CC_WriteRegister(CC_IOCFG0,   CC_IOCFG0_VALUE);     // GDO0 output pin configuration.
    CC_WriteRegister(CC_PKTCTRL1, CC_PKTCTRL1_VALUE);   // Packet automation control.
    CC_WriteRegister(CC_PKTCTRL0, CC_PKTCTRL0_VALUE);   // Packet automation control.
    CC_WriteRegister(CC_PKTLEN,   CC_PKTLEN_VALUE);     // Packet length.

    CC_WriteRegister(CC_PATABLE, CC_PATABLE0_VALUE);
    
    CC_WriteRegister(CC_MCSM2, CC_MCSM2_VALUE);
    CC_WriteRegister(CC_MCSM1, CC_MCSM1_VALUE);
}

// ============================= Low level =====================================
uint8_t CC_ReadWriteByte(uint8_t AByte){
    SPDR = AByte;	// Start transmission
    // Wait for transmission to complete
    while (bit_is_clear (SPSR, SPIF));
    uint8_t Response = SPDR;
    return Response;
}

// ============================ Interrupts =====================================
ISR(INT2_vect){
    // Packet has been successfully recieved
    //PORTA |= (1<<PA1); // DEBUG
    uint8_t FifoSize = CC_ReadRegister(CC_RXBYTES); // Get bytes in FIFO
    if (FifoSize > 0) {
        CC_ReadRX(&CC.RX_PktArray[0], FifoSize);
        CC.NewPacketReceived = true;
    }
    //PORTA &= ~(1<<PA1); // DEBUG
}