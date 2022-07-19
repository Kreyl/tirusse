#if 1 // ============================ Includes =================================
#include "hal.h"
#include "MsgQ.h"
#include "uart.h"
#include "shell.h"
#include "ChunkTypes.h"
#include "SaveToFlash.h"
#include "adc_f100.h"
#include "battery_consts.h"
#include "radio_lvl1.h"
#include "led.h"
#include "Sequences.h"
#include "Config.h"
#endif
#if 1 // ======================== Variables & prototypes =======================
// Forever
bool OsIsInitialized = false;
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};
void OnCmd(Shell_t *PShell);
void ITask();

LedSmooth_t Lumos { LED_PIN };
uint8_t GetID();
#endif

#if 1 // === ADC ===
#define BATTERY_LOW_mv  3500
extern Adc_t Adc;
TmrKL_t TmrOneS {TIME_MS2I(999), evtIdEverySecond, tktPeriodic};

Config_t Cfg;
#endif

int main(void) {
#if 1 // ==== Iwdg, Clk, Os, EvtQ, Uart ====
    // Start Watchdog. Will reset in main thread by periodic 1 sec events.
    Iwdg::InitAndStart(4500);
    Iwdg::DisableInDebug();
    Clk.SetupBusDividers(ahbDiv2, apbDiv1, apbDiv1);
    Clk.UpdateFreqValues();
    // Init OS
    halInit();
    chSysInit();
    OsIsInitialized = true;
    EvtQMain.Init();
#endif

    // ==== Init hardware ====
    Uart.Init();
    AFIO->MAPR = (0b010UL << 24); // Remap pins: disable JTAG leaving SWD
    Cfg.ID = 50 + GetID();
    Printf("\r%S %S\rID=%u; Type=%u; Pwr=%S\r", APP_NAME, XSTRINGIFY(BUILD_TIME),
            Cfg.ID, Cfg.Type, CC_PwrToString(Cfg.TxPower));
    Clk.PrintFreqs();

    Lumos.Init();

    // Battery measurement
    PinSetupAnalog(ADC_BAT_PIN);
    Adc.Init();

    // ==== Radio ====
    if(Radio.Init() == retvOk) Lumos.StartOrRestart(lsqLStart);
    else Lumos.StartOrRestart(lsqFailure);

    TmrOneS.StartOrRestart();

    // Main cycle
    ITask();
}

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdEverySecond:
//                Printf("Second\r");
                Iwdg::Reload();
                Adc.StartMeasurement();
                break;

            case evtIdAdcRslt: {
//                Printf("%u %u\r", Adc.GetResult(0), Adc.GetResult(1));
                uint32_t Battery_mV = 2 * Adc.Adc2mV(Adc.GetResult(0), Adc.GetResult(1)); // *2 because of resistor divider
//                Printf("VBat: %umV\r", Battery_mV);
                if(Battery_mV < BATTERY_LOW_mv) Lumos.StartOrContinue(lsqDischarged);
            } break;

            default: break;
        } // switch
    } // while true
}

uint8_t GetID() {
    uint8_t id = 0;
    for(uint8_t i=3; i<=9; i++) {
        id <<= 1;
        PinSetupInput(GPIOB, i, pudPullUp);
        chThdSleepMilliseconds(1);
        if(PinIsLo(GPIOB, i)) id |= 1; // Fuse detected
        PinSetupAnalog(GPIOB, i);
    }
    return id;
}

#if 1 // ======================= Command processing ============================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ok();
    else if(PCmd->NameIs("Version")) PShell->Print("%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));

    else PShell->CmdUnknown();
}
#endif
