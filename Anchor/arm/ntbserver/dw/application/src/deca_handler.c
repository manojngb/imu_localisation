// Includes
#include "deca_handler.h"
#include "leds.h"
#include "usbd_cdc_vcp.h"
#include <string.h>

// variables
uint8_t seq_num = 0;

// channel configuration (taken from mode 3 of EVK1000)
dwt_config_t ch_config = {
	.chan = 2,
	.rxCode = 9,
	.txCode = 9,
	.prf = DWT_PRF_64M,
	.dataRate = DWT_BR_110K,
	.txPreambLength = DWT_PLEN_1024,
	.rxPAC = DWT_PAC32,
	.nsSFD = 1,
	.phrMode = DWT_PHRMODE_STD,
	.sfdTO = (1025+64-32),
	.smartPowerEn = 0
};

// power configuration
dwt_txconfig_t tx_config = {
	.PGdly = 0x00,
	.power = 0x00
};

// calibrated tx values
tx_struct tx_calib = {
	.PGdelay = 0xC5,
	.txPwr[0] = 0x0f2f4f6f,
	.txPwr[1] = 0x2b4b6b8b
};


// other variables
uint8_t eui_local[8];
uint16_t tx_antennadelay;
uint16_t tx_power = 0;


// Initialize DecaWave
void dw_init()
{
	// ----------- Turn on orange LED to say we're busy -------------
	led_set(LED_ORANGE);

	// ------------------ Configure Hardware GPIO -------------------
	// Configure DW reset line
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStruct.GPIO_Pin = DW_RESET_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(DW_RESET_PORT, &GPIO_InitStruct);

	// Configure the DW IRQ line
	GPIO_InitStruct.GPIO_Pin = DW_IRQ_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(DW_IRQ_PORT, &GPIO_InitStruct);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource2);

	// Configure the EXTI for DW IRQ
	EXTI_InitStruct.EXTI_Line = EXTI_Line2;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	// Set up the interrupt vector for DW IRQ
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	// ----------- Initialize SPI and wait a little bit -------------
	dw_spi_init();
	Sleep(1000);

	// reset the DW1000
	dwt_softreset();


	// ------------- Instance-specific Init. of DW1000 --------------
	//dw_setup(DWT_LOADUCODE | DWT_LOADLDO | DWT_LOADTXCONFIG | DWT_LOADANTDLY| DWT_LOADXTALTRIM);
	// customer calibration: xtal trim, ant delay, txconfig, 
	dw_setup(DWT_LOADUCODE | DWT_LOADLDO );
	Sleep(1000);

	// ------------- Channel configuration for DW1000 ---------------
	//dw_chconfig(ch_config, DWT_LOADANTDLY | DWT_LOADXTALTRIM);
	dw_chconfig(ch_config, 0x00);

	// ----------------- Configure Rx / Tx behavior -----------------
	dwt_setrxmode(DWT_RX_NORMAL, 0, 0);
	dwt_setrxtimeout(0); // disabled

	// ------------- Configure IRQ routines for DW1000 --------------
	dwt_setcallbacks(dw_handle_txdone, dw_handle_rxdone);
	dwt_setinterrupt( DWT_INT_RFCG | DWT_INT_TFRS, 1 );

	// ---------- Turn off orange LED to say we're done -------------
	led_clear(LED_ORANGE);

}

void dw_reset()
{
	int i;
	GPIO_ResetBits( DW_RESET_PORT, DW_RESET_PIN );
	Sleep(100);
	GPIO_SetBits( DW_RESET_PORT, DW_RESET_PIN );
	Sleep(100);
}

int dw_setup(uint16_t load_flags)
{
    int result;

	// load from OTP/ROM on initialization
    result = dwt_initialise(load_flags) ;

    //this is platform dependant - only use if DW EVK/EVB
    dwt_setleds(3);

    if (DWT_SUCCESS != result)
    {
        return (-1) ;   // device initialise has failed
    }

    dwt_geteui(eui_local);

    return 0 ;
}

int dw_chconfig(dwt_config_t config, int use_otpdata)
{
	int instance = 0 ;
    uint32_t power = 0;

    // enable gating gain for 6.81Mbps data rate
    if( config.dataRate == DWT_BR_6M8 )
        config.smartPowerEn = 1;
    else
        config.smartPowerEn = 0;

    // configure the channel parameters
    dwt_configure(&config, use_otpdata);

    // firstly check if there are calibrated TX power value in the DW1000 OTP
    power = dwt_getotptxpower(config.prf, config.chan);

    if((power == 0x0) || (power == 0xFFFFFFFF)) //if there are no calibrated values... need to use defaults
    {
        power = tx_calib.txPwr[0]; // 16M
    }

    //Configure TX power
    //if smart power is used then the value as read from OTP is used directly
    //if smart power is used the user needs to make sure to transmit only one frame per 1ms or TX spectrum power will be violated
    if(config.smartPowerEn == 1)
    {
        tx_config.power = power;
    }
	else //if the smart power is not used, then the low byte value (repeated) is used for the whole TX power register
    {
        uint8_t p = power & 0xFF ;
        tx_config.power = (p | (p << 8) | (p << 16) | (p << 24));
    }
    dwt_setsmarttxpower(config.smartPowerEn);

    //configure the tx spectrum parameters (power and PG delay)
    dwt_configuretxrf(&tx_config);

    //check if to use the antenna delay calibration values as read from the OTP
    if((use_otpdata & DWT_LOADANTDLY) == 0)
    {
        tx_antennadelay = (uint16_t)( (DWT_PRF_64M_RFDLY/2.0)*1e-9/DWT_TIME_UNITS );
        // set the antenna delay, we assume that the RX is the same as TX.
        dwt_setrxantennadelay(tx_antennadelay);
        dwt_settxantennadelay(tx_antennadelay);
    }
    else
    {
        //get the antenna delay that was read from the OTP calibration area
        tx_antennadelay = dwt_readantennadelay(config.prf) >> 1;

        // if nothing was actually programmed then set a reasonable value anyway
        if (tx_antennadelay == 0)
        {
            tx_antennadelay = (uint16_t)( (DWT_PRF_64M_RFDLY/2.0)*1e-9/DWT_TIME_UNITS );
            // -------------------------------------------------------------------------------------------------------------------
            // set the antenna delay, we assume that the RX is the same as TX.
            dwt_setrxantennadelay(tx_antennadelay);
            dwt_settxantennadelay(tx_antennadelay);
        }


    }

    if(config.txPreambLength == DWT_PLEN_64) //if preamble length is 64
	{
    	dw_spi_configprescaler(SPI_BaudRatePrescaler_16); //reduce SPI to < 3MHz
		dwt_loadopsettabfromotp(0);
		dw_spi_configprescaler(SPI_BaudRatePrescaler_4); //increase SPI to max
    }

}

void dw_config_tx()
{

}

void dw_config_rx()
{

}

void dw_sendbeacon()
{
	// construct beacon frame
	iso_IEEE_EUI64_blink_msg msg;
	msg.frameCtrl = 0xC5;
	msg.seqNum = seq_num++;
	memcpy(msg.tagID, eui_local, ADDR_BYTE_SIZE_L);

	uint8_t txdata[1] = { DW_MSG_BEACON };
	// frame length w/ 2-byte CRC, byte pointer, offset
	dwt_writetxdata(BLINK_FRAME_CRTL_AND_ADDRESS + FRAME_CRC, (uint8_t*)(&msg), 0);
	// frame length w/ 2-byte CRC, offset
	dwt_writetxfctrl(BLINK_FRAME_CRTL_AND_ADDRESS + FRAME_CRC, 0) ;
	// send tx -- immediate with no RSP expected
	dwt_starttx(DWT_START_TX_IMMEDIATE);

}

void dw_listen()
{
	dwt_rxenable(0);
}

void dw_handle_txdone()
{
	led_toggle(LED_RED);
}

void dw_handle_rxdone()
{
	led_toggle(LED_GREEN);
}
