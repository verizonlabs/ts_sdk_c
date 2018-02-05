// Copyright (c) 2018 Verizon, Inc. All rights reserved.

#include "ts_platform.h"
#include <stdlib.h>

#define BUILD			0

#if BUILD
#include <stm32l4xx_hal.h>
#if defined(FREE_RTOS)
#include <cmsis_os.h>
#endif
#endif

/* Pin / port configurations for the debug port on the STM32L4. */
#define DBG_UART_INST		UART5
#define DBG_UART_BAUD		115200
#define DBG_UART_CLK_ENABLE()	__HAL_RCC_UART5_CLK_ENABLE()

#define DBG_TX_CLK_ENABLE()	__HAL_RCC_GPIOC_CLK_ENABLE()
#define DBG_TX_PORT		GPIOC
#define DBG_TX_PIN		GPIO_PIN_12
#define DBG_TX_AF		GPIO_AF8_UART5

#define LED_CLK_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE()
#define LED_PORT		GPIOA
#define LED_PIN			GPIO_PIN_5

#define TIMEOUT_MS		5000
/* End of pin / port configuration for the debug port. */

static void ts_initialize(void);
static void ts_printf(const char *, ...);
static void ts_vprintf(const char *, va_list);
static uint64_t ts_time(void);
static void ts_sleep(uint32_t);
static void ts_random(uint32_t*);
static void * ts_malloc(size_t);
static void ts_free(void *, size_t);
static void ts_assertion(const char *, const char *, int);

TsPlatformVtable_t ts_platform_none = {
	.initialize = ts_initialize,
	.printf = ts_printf,
	.vprintf = ts_vprintf,
	.time = ts_time,
	.sleep = ts_sleep,
	.random = ts_random,
	.malloc = ts_malloc,
	.free = ts_free,
	.assertion = ts_assertion,
};

#if BUILD
static struct {
	uint32_t base_tick;
	uint64_t accu_tick;
	RNG_HandleTypeDef hwrng_handle;
	UART_HandleTypeDef debug_uart;
} platform_state;
#endif

/*
 * The standard C library function printf calls _write to transmit individual
 * characters over the debug UART port.
 */
__attribute__((used)) ssize_t _write(int fd, const void *buf, size_t count)
{
	if (buf == NULL || count == 0)
		return 0;
#if BUILD
	HAL_StatusTypeDef s = HAL_UART_Transmit(&platform_state.debug_uart, (uint8_t *)buf, count, TIMEOUT_MS);
	return (s == HAL_OK) ? count : -1;
#else
	return 0;
#endif
}

#if BUILD
static void uart_pin_init(void)
{
	GPIO_InitTypeDef uart_gpio;
	DBG_TX_CLK_ENABLE();
	DBG_UART_CLK_ENABLE();
	uart_gpio.Pin = DBG_TX_PIN;
	uart_gpio.Mode = GPIO_MODE_AF_PP;
	uart_gpio.Pull = GPIO_PULLUP;
	uart_gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	uart_gpio.Alternate = DBG_TX_AF;
	HAL_GPIO_Init(DBG_TX_PORT, &uart_gpio);
}

static bool debug_port_init(void)
{
	uart_pin_init();
	platform_state.debug_uart.Instance = DBG_UART_INST;
	platform_state.debug_uart.Init.BaudRate = DBG_UART_BAUD;
	platform_state.debug_uart.Init.WordLength = UART_WORDLENGTH_8B;
	platform_state.debug_uart.Init.StopBits = UART_STOPBITS_1;
	platform_state.debug_uart.Init.Parity = UART_PARITY_NONE;
	platform_state.debug_uart.Init.Mode = UART_MODE_TX;
	platform_state.debug_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	platform_state.debug_uart.Init.OverSampling = UART_OVERSAMPLING_16;
	platform_state.debug_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

	return HAL_UART_Init(&platform_state.debug_uart) == HAL_OK;
}

static void raise_err(void)
{
	GPIO_InitTypeDef gpio;
	LED_CLK_ENABLE();
	gpio.Pin = LED_PIN;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(LED_PORT, &gpio);
	while(1) {
		HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
		sys_delay(1000);
		HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
		sys_delay(1000);
	}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* MSI is enabled after System reset, activate PLL with MSI as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 40;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLP = 7;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		/* Initialization Error */
		raise_err();

	/*
	 * Select PLL as system clock source and configure the HCLK, PCLK1 and
	 * PCLK2 clocks dividers.
	 */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK |
			RCC_CLOCKTYPE_HCLK |
			RCC_CLOCKTYPE_PCLK1 |
			RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
		/* Initialization Error */
		raise_err();
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
	__HAL_RCC_RNG_CLK_ENABLE();
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
	PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
	__HAL_RCC_RNG_FORCE_RESET();
	__HAL_RCC_RNG_RELEASE_RESET();
}

static bool hwrng_module_init(void)
{
	platform_state.hwrng_handle.Instance = RNG;	/* Point handle at the RNG registers */

	if (HAL_RNG_DeInit(&platform_state.hwrng_handle) != HAL_OK)
		return false;

	if (HAL_RNG_Init(&platform_state.hwrng_handle) != HAL_OK)
		return false;

	return true;
}
#endif

static void ts_initialize(void)
{
#if BUILD
	HAL_Init();
	SystemClock_Config();
	debug_port_init();
	hwrng_module_init();
#endif
}

static void ts_printf(const char * format, ...)
{
	va_list argp;
	va_start(argp, format);
	vprintf(format, argp);
	va_end(argp);
}

static void ts_vprintf(const char * format, va_list argp)
{
	vprintf(format, argp);
}

static uint64_t ts_time(void)
{
	uint32_t cur_ts = 0;
#if BUILD
#if defined(FREE_RTOS)
	cur_ts = osKernelSysTick();
#else
	cur_ts = HAL_GetTick();
#endif
	uint32_t diff_ms = 0;
	if (cur_ts >= platform_state.base_tick)
		diff_ms = cur_ts - platform_state.base_tick;
	else
		diff_ms = 0xffffffff - platform_state.base_tick + cur_ts;

	platform_state.accu_tick += diff_ms;
	platform_state.base_tick = cur_ts;
	return platform_state.accu_tick * TS_TIME_MSEC_TO_USEC;
#else
	return 0;
#endif
}

static void ts_sleep(uint32_t microseconds)
{
#if BUILD
#if defined(FREE_RTOS)
	osDelay(microseconds / TS_TIME_MSEC_TO_USEC);
#else
	HAL_Delay(microseconds / TS_TIME_MSEC_TO_USEC);
#endif
#endif
}

// XXX: Bring back FIPS 140-2 check for non-equal adjacent random numbers?
static void ts_random(uint32_t * number)
{
#if BUILD
	HAL_RNG_GenerateRandomNumber(&hwrng_handle, number)
#endif
}

static void * ts_malloc(size_t size)
{
	return malloc(size);
}

static void ts_free(void *pointer, size_t size)
{
	free(pointer);
}

static void ts_assertion(const char *msg, const char *file, int line)
{
	printf("assertion failed, '%s' at %s:%d\n", msg, file, line);
#if BUILD
	raise_err();
#else
	while (1)
		;
#endif
}

/* Increments the SysTick value. */
void SysTick_Handler(void)
{
#if BUILD
#if defined(FREE_RTOS)
	osSystickHandler();
#else
	HAL_IncTick();
#endif
#endif
}

/*
 * The following are interrupt handlers that trap illegal memory access and
 * illegal program behavior. These should be defined to prevent runaway code.
 */
void HardFault_Handler(void)
{
#if BUILD
	raise_err();
#endif
}

void MemManage_Handler(void)
{
#if BUILD
	raise_err();
#endif
}

void BusFault_Handler(void)
{
#if BUILD
	raise_err();
#endif
}

void UsageFault_Handler(void)
{
#if BUILD
	raise_err();
#endif
}
