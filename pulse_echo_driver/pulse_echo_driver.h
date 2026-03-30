/**
 * @file          pulse_echo_driver.h
 * @brief 			STM32 nonblocking-pulse-echo-measurement-driver
 * @target-board   STM32-NUCLEO-f302r8
 * @see            https://github.com/Abdusselam123/stm32-nonblocking-pulse-echo-measurement-driver
 * @author 			Abdusselam
 * @license 		MIT
 */

#ifndef PULSE_ECHO_DRIVER_PULSE_ECHO_DRIVER_H_
#define PULSE_ECHO_DRIVER_PULSE_ECHO_DRIVER_H_

#include "stm32f3xx.h"

//Pins: ECHO AND TRIG
#define pulse_echo_driver_ECHO_PIN 0   	//TIM2 ch1: PA0
#define pulse_echo_driver_TRIG_PIN 1	//TIM2 ch2: PA1

//Functions
void pulse_echo_driver_init(void);
void pulse_echo_driver_start(void);
void pulse_echo_driver_stop(void);
uint32_t pulse_echo_driver_get_time1(void);
uint32_t pulse_echo_driver_get_time2(void);
uint32_t pulse_echo_driver_get_time(void);
float pulse_echo_driver_get_distance(void);
void pulse_echo_driver_us_settings(uint32_t TIMER_RESOLUTION_us, uint32_t TIMER_CLOCK_F_MHZ_us, uint32_t TRIG_DURATION_us, uint32_t PERIOD_TIME_us);
void pulse_echo_driver_ms_settings(uint32_t TIMER_RESOLUTION_ms, uint32_t TIMER_CLOCK_F_MHZ_ms, uint32_t TRIG_DURATION_ms, uint32_t PERIOD_TIME_ms);

#endif /* PULSE_ECHO_DRIVER_PULSE_ECHO_DRIVER_H_ */
