/**
 * \file
 *
 * \brief FindMeSAT
 * main.c
 *
 * Created: 22.01.2017 13:13:47
 * Author : DF4IAH
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "conf_dac.h"

#include "main.h"


static uint8_t		runmode								= (uint8_t) 0;			// global runmode
static bool			my_flag_autorize_cdc_transfert		= false;

static uint16_t dac_io_dac0_buf[DAC_NR_OF_SAMPLES] = {
	32768, 35325, 37784, 40050, 42036, 43666, 44877, 45623,
	45875, 45623, 44877, 43666, 42036, 40050, 37784, 35325,
	32768, 30211, 27752, 25486, 23500, 21870, 20659, 19913,
	19661, 19913, 20659, 21870, 23500, 25486, 27752, 30211,
};

static uint16_t dac_io_dac1_buf[DAC_NR_OF_SAMPLES] = {
	32768, 35325, 37784, 40050, 42036, 43666, 44877, 45623,
	45875, 45623, 44877, 43666, 42036, 40050, 37784, 35325,
	32768, 30211, 27752, 25486, 23500, 21870, 20659, 19913,
	19661, 19913, 20659, 21870, 23500, 25486, 27752, 30211,
};



/* INIT section */

static void evsys_init(void)
{
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);

	/* ADC - event 7 */
	EVSYS.CH7MUX = EVSYS_CHMUX_TCC0_OVF_gc;

	/* DAC - event 6 */
	EVSYS.CH6MUX = EVSYS_CHMUX_TCE1_OVF_gc;
}


static void tc_init(void)
{
	/* DAC clock */
	tc_enable(&TCE1);
	tc_set_wgm(&TCE1, TC_WG_NORMAL);											// Internal clock for DAC convertion
	tc_write_period(&TCE1, (sysclk_get_per_hz() / DAC_RATE_OF_CONV) - 1);		// DAC clock 48 kHz for audio play-back

	/* VCTCXO PWM signal generation */
	struct pwm_config pwm_vctcxo_cfg;
	pwm_init(&pwm_vctcxo_cfg, PWM_TCC0, PWM_CH_D, 500);							// Init PWM structure and enable timer
	pwm_start(&pwm_vctcxo_cfg, 45);												// Start PWM. Percentage with 1% granularity is to coarse, use driver access instead
	tc_write_cc_buffer(&TCC0, TC_CCD, (uint16_t) (0.5f + 65536 * 1.5f/3.3f));	// Initial value for VCTCXO @ 1.5 V
}

static void tc_start(void)
{
	/* ADC clock */
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV1_gc);							// VCTCXO PWM start, output still is Z-state

	/* DAC clock */
	tc_write_clock_source(&TCE1, TC_CLKSEL_DIV1_gc);							// Internal clock
}


static void adc_init(void)
{
	struct adc_config					adc_5v0_conf;
	struct adc_config					adc_3v0_conf;
	struct adc_config					adc_vbat_conf;
	struct adc_config					adc_vctcxo_conf;
	struct adc_config					adc_io_adc4_conf;
	struct adc_config					adc_io_adc5_conf;
	struct adc_channel_config			adcch_5v0_conf;
	struct adc_channel_config			adcch_3v0_conf;
	struct adc_channel_config			adcch_vbat_conf;
	struct adc_channel_config			adcch_vctcxo_conf;
	struct adc_channel_config			adcch_io_adc4_conf;
	struct adc_channel_config			adcch_io_adc5_conf;

	/* Prepare the structures */
	adc_read_configuration(&ADC_5V0,						&adc_5v0_conf);
	adcch_read_configuration(&ADC_5V0, ADC_5V0_CH,			&adcch_5v0_conf);
	
	adc_read_configuration(&ADC_3V0,						&adc_3v0_conf);
	adcch_read_configuration(&ADC_3V0, ADC_3V0_CH,			&adcch_3v0_conf);
	
	adc_read_configuration(&ADC_VBAT,						&adc_vbat_conf);
	adcch_read_configuration(&ADC_VBAT, ADC_VBAT_CH,		&adcch_vbat_conf);
	
	adc_read_configuration(&ADC_VCTCXO,						&adc_vctcxo_conf);
	adcch_read_configuration(&ADC_VCTCXO, ADC_VCTCXO_CH,	&adcch_vctcxo_conf);
	
	adc_read_configuration(&ADC_IO_ADC4,					&adc_io_adc4_conf);
	adcch_read_configuration(&ADC_IO_ADC4, ADC_IO_ADC4_CH,	&adcch_io_adc4_conf);
	
	adc_read_configuration(&ADC_IO_ADC5,					&adc_io_adc5_conf);
	adcch_read_configuration(&ADC_IO_ADC5, ADC_IO_ADC5_CH,	&adcch_io_adc5_conf);

	/* Convertion and reference */
	adc_set_conversion_parameters(&adc_5v0_conf,		ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_conversion_parameters(&adc_3v0_conf,		ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_conversion_parameters(&adc_vbat_conf,		ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_conversion_parameters(&adc_vctcxo_conf,		ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_conversion_parameters(&adc_io_adc4_conf,	ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	adc_set_conversion_parameters(&adc_io_adc5_conf,	ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);

	/* Trigger */
	adc_set_conversion_trigger(&adc_5v0_conf, ADC_TRIG_EVENT_SYNCSWEEP, 4, 0);
	// ..
	adc_set_conversion_trigger(&adc_io_adc4_conf, ADC_TRIG_EVENT_SINGLE, 1, 0);
	adc_set_conversion_trigger(&adc_io_adc5_conf, ADC_TRIG_EVENT_SINGLE, 1, 0);

	/* ADC-clock request */
	adc_set_clock_rate(&adc_5v0_conf,		768000UL << 1);  // at LEAST 16x oversampling of 48kHz
	adc_set_clock_rate(&adc_3v0_conf,		768000UL << 1);  // at LEAST 16x oversampling of 48kHz
	adc_set_clock_rate(&adc_vbat_conf,		768000UL << 1);  // at LEAST 16x oversampling of 48kHz
	adc_set_clock_rate(&adc_vctcxo_conf,	768000UL << 1);  // at LEAST 16x oversampling of 48kHz
	adc_set_clock_rate(&adc_io_adc4_conf,	768000UL << 1);  // at LEAST 16x oversampling of 48kHz
	adc_set_clock_rate(&adc_io_adc5_conf,	768000UL << 1);  // at LEAST 16x oversampling of 48kHz

	/* Current limitation */
	adc_set_current_limit(&adc_5v0_conf,		ADC_CURRENT_LIMIT_MED);
	adc_set_current_limit(&adc_3v0_conf,		ADC_CURRENT_LIMIT_MED);
	adc_set_current_limit(&adc_vbat_conf,		ADC_CURRENT_LIMIT_MED);
	adc_set_current_limit(&adc_vctcxo_conf,		ADC_CURRENT_LIMIT_MED);
	adc_set_current_limit(&adc_io_adc4_conf,	ADC_CURRENT_LIMIT_MED);
	adc_set_current_limit(&adc_io_adc5_conf,	ADC_CURRENT_LIMIT_MED);

	/* ADC impedance */
	adc_set_gain_impedance_mode(&adc_5v0_conf,		ADC_GAIN_HIGHIMPEDANCE);
	adc_set_gain_impedance_mode(&adc_3v0_conf,		ADC_GAIN_HIGHIMPEDANCE);
	adc_set_gain_impedance_mode(&adc_vbat_conf,		ADC_GAIN_HIGHIMPEDANCE);
	adc_set_gain_impedance_mode(&adc_vctcxo_conf,	ADC_GAIN_HIGHIMPEDANCE);
	adc_set_gain_impedance_mode(&adc_io_adc4_conf,	ADC_GAIN_HIGHIMPEDANCE);
	adc_set_gain_impedance_mode(&adc_io_adc5_conf,	ADC_GAIN_HIGHIMPEDANCE);

	/* PIN assignment */
	adcch_set_input(&adcch_5v0_conf,		ADCCH_POS_PIN2, ADCCH_NEG_NONE, 1);
	adcch_set_input(&adcch_3v0_conf,		ADCCH_POS_PIN0, ADCCH_NEG_NONE, 1);
	adcch_set_input(&adcch_vbat_conf,		ADCCH_POS_PIN3, ADCCH_NEG_NONE, 1);
	adcch_set_input(&adcch_vctcxo_conf,		ADCCH_POS_PIN1, ADCCH_NEG_NONE, 1);
	adcch_set_input(&adcch_io_adc4_conf,	ADCCH_POS_PIN4, ADCCH_NEG_NONE, 1);
	adcch_set_input(&adcch_io_adc5_conf,	ADCCH_POS_PIN5, ADCCH_NEG_NONE, 1);

	/* Interrupt type */
	adcch_set_interrupt_mode(&adcch_5v0_conf,		ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&adcch_3v0_conf,		ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&adcch_vbat_conf,		ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&adcch_vctcxo_conf,	ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&adcch_io_adc4_conf,	ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&adcch_io_adc5_conf,	ADCCH_MODE_COMPLETE);

	/* PIN scan on ADC-channel 0 */
	adcch_set_pin_scan(&adcch_5v0_conf,		0, 3);
	adcch_set_pin_scan(&adcch_3v0_conf,		0, 3);
	adcch_set_pin_scan(&adcch_vbat_conf,	0, 3);

	/* Execute the new settings */
	adc_write_configuration(&ADC_5V0,						&adc_5v0_conf);
	adcch_write_configuration(&ADC_5V0, ADC_5V0_CH,			&adcch_5v0_conf);
	
	adc_write_configuration(&ADC_3V0,						&adc_3v0_conf);
	adcch_write_configuration(&ADC_3V0, ADC_3V0_CH,			&adcch_3v0_conf);
	
	adc_write_configuration(&ADC_VBAT,						&adc_vbat_conf);
	adcch_write_configuration(&ADC_VBAT, ADC_VBAT_CH,		&adcch_vbat_conf);
	
	adc_write_configuration(&ADC_VCTCXO,					&adc_vctcxo_conf);
	adcch_write_configuration(&ADC_VCTCXO, ADC_VCTCXO_CH,	&adcch_vctcxo_conf);
	
	adc_write_configuration(&ADC_IO_ADC4,					&adc_vctcxo_conf);
	adcch_write_configuration(&ADC_IO_ADC4, ADC_IO_ADC4_CH,	&adcch_vctcxo_conf);
	
	adc_write_configuration(&ADC_IO_ADC5,					&adc_vctcxo_conf);
	adcch_write_configuration(&ADC_IO_ADC5, ADC_IO_ADC5_CH,	&adcch_vctcxo_conf);
}


static void dac_init(void)
{
    struct dac_config dac_io_dac0_conf;
    struct dac_config dac_io_dac1_conf;

    dac_read_configuration(&DAC_IO_DAC0, &dac_io_dac0_conf);
    dac_read_configuration(&DAC_IO_DAC1, &dac_io_dac1_conf);
	
    dac_set_conversion_parameters(&dac_io_dac0_conf, DAC_REF_BANDGAP, DAC_ADJ_LEFT);
    dac_set_conversion_parameters(&dac_io_dac1_conf, DAC_REF_BANDGAP, DAC_ADJ_LEFT);
	
    dac_set_active_channel(&dac_io_dac0_conf, DAC_IO_DAC0_CHANNEL, 0);
    dac_set_active_channel(&dac_io_dac1_conf, DAC_IO_DAC1_CHANNEL, 0);
	
    dac_set_conversion_trigger(&dac_io_dac0_conf, DAC_IO_DAC0_CHANNEL, 6);
    dac_set_conversion_trigger(&dac_io_dac1_conf, DAC_IO_DAC1_CHANNEL, 6);
	
    #ifdef XMEGA_DAC_VERSION_1
    dac_set_conversion_interval(&dac_io_dac0_conf, 2);
    dac_set_conversion_interval(&dac_io_dac1_conf, 2);
    #endif

    dac_write_configuration(&DAC_IO_DAC0, &dac_io_dac0_conf);
    dac_write_configuration(&DAC_IO_DAC1, &dac_io_dac1_conf);

#if 0
    dac_enable(&DAC_IO_DAC0);
    dac_enable(&DAC_IO_DAC1);
#endif
}


static void usb_init(void)
{
	udc_start();
}

void usb_callback_suspend_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Disable hardware component to reduce power consumption
	// Reduce power consumption in suspend mode (max. 2.5mA on VBUS)
	
}

void usb_callback_resume_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Re-enable hardware component
	
}

void usb_callback_remotewakeup_enable(void)
{
	/* USB wake-up remote host feature */

	// Enable application wakeup events (e.g. enable GPIO interrupt)
	
}

void usb_callback_remotewakeup_disable(void)
{
	/* USB wake-up remote host feature */

	// Disable application wakeup events (e.g. disable GPIO interrupt)
	
}

void usb_send_wakeup_event(void)
{
	/* USB wake-up remote host feature */

	udc_remotewakeup();
}

bool usb_callback_cdc_enable(void)
{
	/* USB CDC feature for serial communication */

	my_flag_autorize_cdc_transfert = true;
	return true;
}

void usb_callback_cdc_disable(void)
{
	/* USB CDC feature for serial communication */

	my_flag_autorize_cdc_transfert = false;
}

void usb_callback_config(uint8_t port, usb_cdc_line_coding_t * cfg)
{
		
}

void usb_callback_cdc_set_dtr(uint8_t port, bool b_enable)
{
	
}

void usb_callback_cdc_set_rts(uint8_t port, bool b_enable)
{
	
}

void usb_callback_rx_notify(uint8_t port)
{
	
}

void usb_callback_tx_empty_notify(uint8_t port)
{
	
}



/* RUNNING section */

static void task_dac(void)
{
	static int idx_dacX = 0;
	
	if (dac_channel_is_ready(&DAC_IO_DAC0, DAC_CH0 | DAC_CH1)) {
		dac_set_channel_value(&DAC_IO_DAC0, DAC_IO_DAC0_CHANNEL, dac_io_dac0_buf[idx_dacX]);
		dac_set_channel_value(&DAC_IO_DAC1, DAC_IO_DAC1_CHANNEL, dac_io_dac1_buf[idx_dacX]);
		
		idx_dacX++;
		idx_dacX %= DAC_NR_OF_SAMPLES;
	}
}

static void task_usb(void)
{
	if (my_flag_autorize_cdc_transfert) {
		//if () {
		//	task_usb_cdc();
		//}

		// Waits and gets a value on CDC line
		// int udi_cdc_getc(void);

		// Reads a RAM buffer on CDC line
		// iram_size_t udi_cdc_read_buf(int* buf, iram_size_t size);

		// Puts a byte on CDC line
		// int udi_cdc_putc(int value);

		// Writes a RAM buffer on CDC line
		// iram_size_t udi_cdc_write_buf(const int* buf, iram_size_t size);	}
		
		// Handling ...
	}
}

static void task(void)
{
	/* TASK when woken up */
	task_dac();

	/* Handling the USB connection */
	task_usb();
}

void halt(void)
{
	/* MAIN Loop Shutdown */

	runmode = 0;
}

int main(void)
{
	uint8_t retcode = 0;
	
	/* Init of sub-modules */
	pmic_init();
	sysclk_init();
	evsys_init();
	tc_init();
	adc_init();
	dac_init();
	
	/* All interrupt sources prepared here - IRQ activation */
	irq_initialize_vectors();
	cpu_irq_enable();
	
	board_init();		// Activates all in/out pins - transitions from Z to dedicated states
	sleepmgr_init();	// Unlocks all sleep mode levels
	
	
	/* Start of sub-modules */
	tc_start();			// All clocks and PWM timers start here

	/* Init of USB system */
	usb_init();			// USB device stack start function to enable stack and start USB
	
	
	/* The application code */
	runmode = (uint8_t) 1;
    while (runmode) {
		task();
		sleepmgr_enter_sleep();
    }
	
	cpu_irq_disable();
	sleepmgr_enter_sleep();
	
	return retcode;
}
