// Copyright (c) 2018 Verizon Inc. All rights reserved.

#include "ts_driver.h"
#include "ts_platform.h"

#define BUILD				0
#if 0
#include <stm32l4xx_hal.h>
#endif

/* Settings for Sequans Monarch chipset (Nimbelink CAT-M1 board) */
#define MODEM_UART_INST			USART1
#define MODEM_UART_BAUD			921600
#define MODEM_UART_CLK_ENABLE()		__HAL_RCC_USART1_CLK_ENABLE()
#define MODEM_UART_IRQ_PRIORITY		5
#define MODEM_UART_IRQn			USART1_IRQn

#define MODEM_TX_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE();
#define MODEM_TX_AF			GPIO_AF7_USART1
#define MODEM_TX_PORT			GPIOA
#define MODEM_TX_PIN			GPIO_PIN_9
#define MODEM_RX_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE();
#define MODEM_RX_AF			GPIO_AF7_USART1
#define MODEM_RX_PORT			GPIOA
#define MODEM_RX_PIN			GPIO_PIN_10
#define MODEM_RTS_CLK_ENABLE()		__HAL_RCC_GPIOB_CLK_ENABLE();
#define MODEM_RTS_PORT			GPIOB
#define MODEM_RTS_PIN			GPIO_PIN_5
#define MODEM_CTS_CLK_ENABLE()		__HAL_RCC_GPIOB_CLK_ENABLE();
#define MODEM_CTS_PORT			GPIOB
#define MODEM_CTS_PIN			GPIO_PIN_3

#define MODEM_RESET_CLK_ENABLE()	__HAL_RCC_GPIOB_CLK_ENABLE();
#define MODEM_RESET_PORT		GPIOB
#define MODEM_RESET_PIN			GPIO_PIN_4

#define MODEM_PWR_EN_DELAY_MS		1000
#define MODEM_RESET_DELAY_MS		1000
#define MODEM_CTS_TIMEOUT_MS		1500

#define TIMEOUT_MS			5000
/* End of modem hardware configuration. */

#define ts_platform_time_ms()	(ts_platform_time() / TS_TIME_MSEC_TO_USEC)
#define ts_platform_sleep_ms(t)	ts_platform_sleep((t) * TS_TIME_MSEC_TO_USEC)
#define USEC2MSEC(t)			((t) / TS_TIME_MSEC_TO_USEC)

static TsStatus_t ts_create( TsDriverRef_t * );
static TsStatus_t ts_destroy( TsDriverRef_t );
static TsStatus_t ts_tick( TsDriverRef_t, uint32_t );

static TsStatus_t ts_connect( TsDriverRef_t, TsAddress_t );
static TsStatus_t ts_disconnect( TsDriverRef_t );
static TsStatus_t ts_read( TsDriverRef_t, const uint8_t *, size_t *, uint32_t );
static TsStatus_t ts_reader(TsDriverRef_t, void*, TsDriverReader_t);
static TsStatus_t ts_write( TsDriverRef_t, const uint8_t *, size_t *, uint32_t );
static void ts_reset( TsDriverRef_t );

TsDriverVtable_t ts_driver_none_uart = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.reader = ts_reader,
	.write = ts_write,
	.reset = ts_reset
};

typedef struct TsModemDriver * TsModemDriverRef_t;
typedef struct TsModemDriver {
	TsDriver_t _driver;
	bool being_used;
#if 0
	UART_HandleTypeDef modem_uart;
#endif
} TsModemDriver_t;
static TsModemDriver_t nimbelink;

void USART1_IRQHandler(void)
{
#if 0
	volatile USART_TypeDef *uart_instance =
		(volatile USART_TypeDef *)(MODEM_UART_INST);
	volatile uint32_t uart_sr_reg = uart_instance->ISR;

	if (uart_sr_reg & USART_ISR_PE)
		uart_instance->ICR |= USART_ICR_PECF;
	else if (uart_sr_reg & USART_ISR_FE)
		uart_instance->ICR |= USART_ICR_FECF;
	else if (uart_sr_reg & USART_ISR_NE)
		uart_instance->ICR |= USART_ICR_NCF;
	else if (uart_sr_reg & USART_ISR_ORE)
		uart_instance->ICR |= USART_ICR_ORECF;
	else {
		uint8_t data = uart_instance->RDR;
		__SEV();
		nimbelink._driver._reader(&nimbelink._driver, nimbelink._driver._reader_state, &data, 1);
	}
#endif
}

static void uart_pin_init(void)
{
#if 0
	GPIO_InitTypeDef uart_gpio;

	MODEM_TX_CLK_ENABLE();
	uart_gpio.Pin = MODEM_TX_PIN;
	uart_gpio.Mode = GPIO_MODE_AF_PP;
	uart_gpio.Pull = GPIO_PULLUP;
	uart_gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	uart_gpio.Alternate = MODEM_TX_AF;
	HAL_GPIO_Init(MODEM_TX_PORT, &uart_gpio);

	MODEM_RX_CLK_ENABLE();
	uart_gpio.Pin = MODEM_RX_PIN;
	uart_gpio.Alternate = MODEM_RX_AF;
	HAL_GPIO_Init(MODEM_RX_PORT, &uart_gpio);

	MODEM_RTS_CLK_ENABLE();
	uart_gpio.Pin = MODEM_RTS_PIN;
	HAL_GPIO_Init(MODEM_RTS_PORT, &uart_gpio);

	MODEM_CTS_CLK_ENABLE();
	uart_gpio.Pin = MODEM_CTS_PIN;
	HAL_GPIO_Init(MODEM_CTS_PORT, &uart_gpio);

	MODEM_UART_CLK_ENABLE();
#endif
}

static void modem_reset_line_init()
{
#if 0
	GPIO_InitTypeDef reset_gpio;
	/* Minimum delay after power up. */
	ts_platform_sleep_ms(MODEM_PWR_EN_DELAY_MS);

	/* Initialize the reset line. */
	MODEM_RESET_CLK_ENABLE();
	reset_gpio.Pin = MODEM_RESET_PIN;
	reset_gpio.Mode = GPIO_MODE_OUTPUT_PP;
	reset_gpio.Pull = GPIO_NOPULL;
	reset_gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(MODEM_RESET_PORT, &reset_gpio);
	HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_RESET);
#endif
}

static void ts_reset( TsDriverRef_t driver )
{
#if 0
	HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_SET);
	ts_platform_sleep_ms(MODEM_RESET_DELAY_MS);
	HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_RESET);
#endif
}

static bool uart_port_init(TsModemDriverRef_t d)
{
#if 0
	USART_TypeDef *uart_instance = (USART_TypeDef *)(MODEM_UART_INST);
	GPIO_InitTypeDef rts_gpio;
	GPIO_InitTypeDef cts_gpio;

	/* Initialize the emulated RTS / CTS lines */
	MODEM_RTS_CLK_ENABLE();
	rts_gpio.Pin = MODEM_RTS_PIN;
	rts_gpio.Mode = GPIO_MODE_OUTPUT_PP;
	rts_gpio.Pull = GPIO_NOPULL;
	rts_gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(MODEM_RTS_PORT, &rts_gpio);
	HAL_GPIO_WritePin(MODEM_RTS_PORT, MODEM_RTS_PIN, GPIO_PIN_RESET);

	MODEM_CTS_CLK_ENABLE();
	cts_gpio.Pin = MODEM_CTS_PIN;
	cts_gpio.Mode = GPIO_MODE_INPUT;
	cts_gpio.Pull = GPIO_NOPULL;
	cts_gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(MODEM_CTS_PORT, &cts_gpio);

	/* Initialize the serial port. */
	uart_pin_init();
	d->modem_uart.Instance = MODEM_UART_INST;
	d->modem_uart.Init.BaudRate = MODEM_UART_BAUD;
	d->modem_uart.Init.WordLength = UART_WORDLENGTH_8B;
	d->modem_uart.Init.StopBits = UART_STOPBITS_1;
	d->modem_uart.Init.Parity = UART_PARITY_NONE;
	d->modem_uart.Init.Mode = UART_MODE_TX_RX;
	d->modem_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	d->modem_uart.Init.OverSampling = UART_OVERSAMPLING_16;
	d->modem_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

	if (HAL_UART_Init(&d->modem_uart) != HAL_OK)
		return false;

	/* Enable Error Interrupts: (Frame error, noise error, overrun error) */
	SET_BIT(uart_instance->CR3, USART_CR3_EIE);

	/* Enable the UART Parity Error and Data Register not empty Interrupts */
	SET_BIT(uart_instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);

	/* Enable interrupt */
	HAL_NVIC_SetPriority(MODEM_UART_IRQn, MODEM_UART_IRQ_PRIORITY, 0);
	HAL_NVIC_EnableIRQ(MODEM_UART_IRQn);

#endif
	return true;
}

static TsStatus_t ts_create( TsDriverRef_t * driver )
{
	ts_status_trace("ts_driver_create\n");
	*driver = (TsDriverRef_t)&nimbelink;

	TsModemDriverRef_t driver_nimbelink = (TsModemDriverRef_t)(*driver);
	if (driver_nimbelink->being_used) {
		*driver = NULL;
		return TsStatusErrorNoResourceAvailable;
	}

	driver_nimbelink->_driver._address = "";
	driver_nimbelink->_driver._profile = NULL;
	driver_nimbelink->_driver._reader = NULL;
	driver_nimbelink->_driver._reader_state = NULL;
	driver_nimbelink->_driver._spec_budget = 60 * TS_TIME_SEC_TO_USEC;
	driver_nimbelink->_driver._spec_mcu = 2048;
	snprintf((char *)(driver_nimbelink->_driver._spec_id), TS_DRIVER_MAX_ID_SIZE, "%s", "AABBCCDDEEFF");

	modem_reset_line_init();
	uart_pin_init();
	if (!uart_port_init(driver_nimbelink))
		return TsStatusErrorInternalServerError;

	driver_nimbelink->being_used = true;
	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsDriverRef_t driver )
{
	ts_status_trace( "ts_driver_destroy\n" );
	ts_platform_assert( driver != NULL );
	TsModemDriverRef_t driver_nimbelink = (TsModemDriverRef_t)driver;
	driver_nimbelink->being_used = false;
	return TsStatusOk;
}

static TsStatus_t ts_tick( TsDriverRef_t driver, uint32_t budget )
{
	// Not needed.
	ts_status_trace( "ts_driver_tick\n" );
	ts_platform_assert( driver != NULL );
	return TsStatusOk;
}

static TsStatus_t ts_connect( TsDriverRef_t driver, TsAddress_t address )
{
	// Not needed.
	ts_status_trace( "ts_driver_connect\n" );
	ts_platform_assert( driver != NULL );
	return TsStatusOk;
}

static TsStatus_t ts_disconnect( TsDriverRef_t driver )
{
	// Not needed.
	ts_status_trace( "ts_driver_connect\n" );
	ts_platform_assert( driver != NULL );
	return TsStatusOk;
}

static TsStatus_t ts_read( TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget )
{
	// Not needed.
	ts_status_trace( "ts_driver_read\n" );
	ts_platform_assert( driver != NULL );
	return TsStatusOk;
}

static TsStatus_t ts_reader(TsDriverRef_t driver, void *state, TsDriverReader_t reader )
{
	ts_status_trace( "ts_driver_reader\n" );
	ts_platform_assert( driver != NULL );
	driver->_reader = reader;
	driver->_reader_state = state;
	return TsStatusOk;
}

static TsStatus_t ts_write( TsDriverRef_t driver, const uint8_t *buffer, size_t *buffer_size, uint32_t budget )
{
	ts_status_trace( "ts_driver_write\n" );
	ts_platform_assert( driver != NULL );
	ts_platform_assert( buffer != NULL );
	ts_platform_assert( buffer_size != NULL);

	// XXX: Enforce budget more strictly
	budget = USEC2MSEC(budget);
	TsModemDriverRef_t driver_nimbelink = (TsModemDriverRef_t)driver;

#if 0
	/* Mark the controller busy. */
	HAL_GPIO_WritePin(MODEM_RTS_PORT, MODEM_RTS_PIN, GPIO_PIN_SET);

	/* Ensure the modem is ready to receive commands. */
	uint32_t start = ts_platform_time_ms();
	while (HAL_GPIO_ReadPin(MODEM_CTS_PORT, MODEM_CTS_PIN) == GPIO_PIN_SET) {
		uint32_t now = ts_platform_time_ms();
		if (now - start > MODEM_CTS_TIMEOUT_MS) {
			HAL_GPIO_WritePin(MODEM_RTS_PORT, MODEM_RTS_PIN, GPIO_PIN_RESET);
			return TsStatusErrorExceedTimeBudget;
		}
		ts_platform_sleep_ms(10);
	}

	HAL_StatusTypeDef s = HAL_UART_Transmit(&driver_nimbelink->modem_uart, (uint8_t *)data, sz, budget);

	/* Mark the controller free. */
	HAL_GPIO_WritePin(MODEM_RTS_PORT, MODEM_RTS_PIN, GPIO_PIN_RESET);

	if (s != HAL_OK)
		return TsStatusErrorInternalServerError;
#endif
	return TsStatusOk;
}
