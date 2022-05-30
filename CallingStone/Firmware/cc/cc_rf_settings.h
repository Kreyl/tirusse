/* 
 * File:   cc_rf_settings.h
 * Author: Kreyl Laurelindo
 *
 * Created on 7 Март 2010 г., 12:42
 */

#ifndef _CC_RF_SETTINGS_H
#define	_CC_RF_SETTINGS_H

// Bitrate
//#define CC_BITRATE_10K
#define CC_BITRATE_38K4

// ============================ Common use values ==============================
//#define CC_PKT_LENGTH       30    // Data size = 27
#define CC_PKT_LENGTH       7   // Data size = 4
#define CC_TX_FIFO_SIZE     33
#define CC_RX_FIFO_SIZE     32

// =================================== Power ===================================
#define CC_PATABLE0_VALUE   0x51    // 0 dBm
//#define CC_PATABLE0_VALUE   0x85    // +5 dBm
//#define CC_PATABLE0_VALUE   0xCB    // +7 dBm
//#define CC_PATABLE0_VALUE   0xC2    // +10 dBm


// ================================= Frequency =================================
#define CC_FREQ2_VALUE      0x0C        // Frequency control word, high byte.
#define CC_FREQ1_VALUE      0x1D        // Frequency control word, middle byte.
#define CC_FREQ0_VALUE      0x89        // Frequency control word, low byte.

// =================================== Common ==================================
#define CC_MDMCFG1_VALUE    0b10100010  // FEC=1, Preamble length=010 => 4bytes, Ch spacing=10=2
//#define CC_MDMCFG1_VALUE    0b11000010  // FEC=1, Preamble length=100 => 8bytes, Ch spacing=10=2

#define CC_MCSM0_VALUE      0x18        // Calibrate at IDLE->RX,TX
#define CC_MCSM1_VALUE      0b00001100  // Channel is always clear, RX->RX,   TX->IDLE
//#define CC_MCSM1_VALUE      0b00001111  // Channel is always clear, RX->RX,   TX->RX
//#define CC_MCSM1_VALUE      0b00000000  // Channel is always clear, RX->IDLE, TX->IDLE
//#define CC_MCSM1_VALUE      0b00000011  // Channel is always clear, RX->IDLE, TX->RX

#define CC_FIFOTHR_VALUE    0b00000111  // RXFIFO and TXFIFO thresholds: TX 33, RX 32
#define CC_IOCFG2_VALUE     0x0E        // GDO2 - Carrier sence
#define CC_IOCFG0_VALUE     0x07        // GDO0 - Asserts when a packet has been received with CRC OK. De-asserts when the first byte is read from the RX FIFO.

#define CC_PKTCTRL1_VALUE   0b00001110  // PQT=0, CRC autoflush=1, Append=1, Address check = 10 (check, 0 is broadcast)
#define CC_PKTCTRL0_VALUE   0b01000100  // WhiteData=1, PKTFormat=normal, CRC enabled, Fixed Length
#define CC_ADDR_VALUE       0x01        // Device address.
#define CC_PKTLEN_VALUE     CC_PKT_LENGTH  // Packet length.


// ========================= Bitrate-specific ==================================
#ifdef CC_BITRATE_10K
#define CC_FSCTRL1_VALUE    0x06        // Frequency synthesizer control: IF - RF studio
#define CC_FSCTRL0_VALUE    0x00        // Frequency synthesizer control: freq offset

#define CC_MDMCFG4_VALUE    0xC8        // Modem configuration: channel bandwidth
#define CC_MDMCFG3_VALUE    0x93        // Modem configuration.
#define CC_MDMCFG2_VALUE    0x03        // Filter, modulation format, Manchester coding, SYNC_MODE=011 => 30/32 sync word bits
#define CC_MDMCFG0_VALUE    0xF8        // Modem configuration.

#define CC_DEVIATN_VALUE    0x34        // Modem deviation setting - RF studio
#define CC_FREND1_VALUE     0x56        // Front end RX configuration - RF studio
#define CC_FREND0_VALUE     0x10        // Front end TX configuration.

#define CC_MCSM2_VALUE      0b00000111

#define CC_FOCCFG_VALUE     0x16        // Frequency Offset Compensation Configuration - RF studio
#define CC_BSCFG_VALUE      0x6C        // Bit synchronization Configuration - RF studio
#define CC_AGCCTRL2_VALUE   0x43        // }
#define CC_AGCCTRL1_VALUE   0x40        // }
#define CC_AGCCTRL0_VALUE   0x91        // } AGC control: RF studio

#define CC_FSCAL3_VALUE     0xE9        // }
#define CC_FSCAL2_VALUE     0x2A        // }
#define CC_FSCAL1_VALUE     0x00        // }
#define CC_FSCAL0_VALUE     0x1F        // } Frequency synthesizer calibration: RF studio

#define CC_FSTEST_VALUE     0x59        // Frequency synthesizer calibration: RF studio
#define CC_TEST2_VALUE      0x81        // Various test settings: RF studio
#define CC_TEST1_VALUE      0x35        // Various test settings: RF studio
#define CC_TEST0_VALUE      0x0B        // Various test settings: RF studio

// ********************
#elif defined CC_BITRATE_38K4
#define CC_FSCTRL1_VALUE    0x06        // Frequency synthesizer control: IF - RF studio
#define CC_FSCTRL0_VALUE    0x00        // Frequency synthesizer control: freq offset

#define CC_MDMCFG4_VALUE    0xCA        // Modem configuration: channel bandwidth
#define CC_MDMCFG3_VALUE    0x83        // Modem configuration.
#define CC_MDMCFG2_VALUE    0x03        // Filter, modulation format, Manchester coding, SYNC_MODE=011 => 30/32 sync word bits
#define CC_MDMCFG0_VALUE    0xF8        // Modem configuration.

#define CC_DEVIATN_VALUE    0x34        // Modem deviation setting - RF studio
#define CC_FREND1_VALUE     0x56        // Front end RX configuration - RF studio
#define CC_FREND0_VALUE     0x10        // Front end TX configuration.

#define CC_MCSM2_VALUE      0b00000111

#define CC_FOCCFG_VALUE     0x16        // Frequency Offset Compensation Configuration - RF studio
#define CC_BSCFG_VALUE      0x6C        // Bit synchronization Configuration - RF studio
#define CC_AGCCTRL2_VALUE   0x43        // }
#define CC_AGCCTRL1_VALUE   0x40        // }
#define CC_AGCCTRL0_VALUE   0x91        // } AGC control: RF studio

#define CC_FSCAL3_VALUE     0xE9        // }
#define CC_FSCAL2_VALUE     0x2A        // }
#define CC_FSCAL1_VALUE     0x00        // }
#define CC_FSCAL0_VALUE     0x1F        // } Frequency synthesizer calibration: RF studio

#define CC_FSTEST_VALUE     0x59        // Frequency synthesizer calibration: RF studio
#define CC_TEST2_VALUE      0x81        // Various test settings: RF studio
#define CC_TEST1_VALUE      0x35        // Various test settings: RF studio
#define CC_TEST0_VALUE      0x0B        // Various test settings: RF studio

// ********************
#endif




// Rare use settings
#define CC_SYNC1_VALUE      0xD3
#define CC_SYNC0_VALUE      0x91

#define CC_CHANNR_VALUE     0x00        // Channel number.


#endif	/* _CC_RF_SETTINGS_H */

