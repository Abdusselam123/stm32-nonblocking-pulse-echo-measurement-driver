/*
 * @file 	pulse_echo_driver.c
 * @brief 	STM32 nonblocking-pulse-echo-measurement-driver
 */

#include "pulse_echo_driver.h"

//Variables
static volatile uint32_t time_1 			= 0;
static volatile uint32_t time_2 			= 0;
static volatile uint32_t sensor_time 		= 0;
static volatile float 	 distance 			= 0.0;

static volatile uint32_t timer_resolution 	= 2;
static volatile uint32_t timer_psc 			= 15;
static volatile uint32_t timer_arr 			= 50000;
static volatile uint32_t timer_ccr2 		= 5;


// --- Private Function Prototypes --- //
static void pulse_echo_driver_Pin_Init(void);
static void Timer2_init(void);
static volatile int echo_state=1;



/* ------------------------------------*/
/* --- INIT FUNCTIONS START------------*/
/* ------------------------------------*/
static void pulse_echo_driver_Pin_Init(void){

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;							//GPIOA clock enable

	//ECHO PIN SETTING
	GPIOA->MODER &= ~(3U << pulse_echo_driver_ECHO_PIN*2);		//PA_ECHO_PIN RESET
	GPIOA->MODER |= (2U << pulse_echo_driver_ECHO_PIN*2);		//PA_ECHO_PIN AF MODE
	GPIOA->AFR[0] &= ~(0xFU << pulse_echo_driver_ECHO_PIN*4);	//PA_ECHO_PIN AFR RESET
	GPIOA->AFR[0] |= (1U << pulse_echo_driver_ECHO_PIN*4);		//PA_ECHO_PIN AFR TIM2_CH2: AF1

	//TRIG PIN SETTING
	GPIOA->MODER &= ~(3U << pulse_echo_driver_TRIG_PIN*2);		//PA_ECHO_PIN RESET
	GPIOA->MODER |= (2U << pulse_echo_driver_TRIG_PIN*2);		//PA_ECHO_PIN AF MODE
	GPIOA->AFR[0] &= ~(0xFU << pulse_echo_driver_TRIG_PIN*4);	//PA_ECHO_PIN AFR RESET
	GPIOA->AFR[0] |= (1U << pulse_echo_driver_TRIG_PIN*4);		//PA_ECHO_PIN AFR TIM2_CH2: AF1
}

static void Timer2_init(void){

	//BASIC TIMER SETTINGS
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC 	  = timer_psc;
	TIM2->ARR 	  = timer_arr;

	//CAPTURE-COMPARE SETTINGS
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S;						//RESET CC1S
	TIM2->CCMR1 &= ~TIM_CCMR1_IC1F;						//RESET filter
	TIM2->CCMR1 |=  (1 << 0);							// Map CH1 to TI1
	TIM2->CCMR1 |=  (0xFU << TIM_CCMR1_IC1F_Pos);		// Digital Filter: 16 samples to avoid noise
	TIM2->CCER  &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);
	TIM2->CCER  |=  TIM_CCER_CC1P;						//RM0365, page:616/1086 : if CC1NP=1 and CC1P=1, ..
	TIM2->CCER  |=  TIM_CCER_CC1NP;						//.. both rising and falling edges are enabled for capture.
	TIM2->CCER  |=  TIM_CCER_CC1E;						//c-c enable
	TIM2->DIER  |=  TIM_DIER_CC1IE;						//c-c interrupt enable

	//CAPTURE-COMPARE 2 SETTINGS
	TIM2->CCMR1 &= ~TIM_CCMR1_CC2S;						//RESET CC2S
	TIM2->CCER  |=  TIM_CCER_CC2E;						//c-c enable
	TIM2->DIER  |=  TIM_DIER_CC2IE;						//c-c interrupt enable
	TIM2->CCR2   =  timer_ccr2;							//pulse duration time

	TIM2->EGR |=  TIM_EGR_UG;							//Generate Update to apply settings immediately
	TIM2->SR  &= ~TIM_SR_UIF; 							//Clear Update Flag to prevent premature IRQ
	TIM2->SR  &= ~TIM_SR_CC1IF; 						//Clear Capture Flag
	TIM2->SR  &= ~TIM_SR_CC2IF; 						//Clear Capture Flag

	NVIC_EnableIRQ(TIM2_IRQn);							//Enable TIM2 Global Interrupt
}

/* -----------------------------------*/
/* --- INIT FUNCTIONS END-------------*/
/* -----------------------------------*/



/* -----------------------------------*/
/* --- PUBLIC API FUNCTIONS START --- */
/* -----------------------------------*/
void pulse_echo_driver_init(void){
	pulse_echo_driver_Pin_Init();
	Timer2_init();
}

void pulse_echo_driver_start(void){
	echo_state=1;
	TIM2->CNT = 0;									//RESET CNT value
	TIM2->EGR |=  TIM_EGR_UG;						//Generate Update to apply settings immediately
	TIM2->SR  &= ~TIM_SR_UIF; 						//Clear Update Flag to prevent premature IRQ
	TIM2->SR  &= ~TIM_SR_CC1IF; 					//Clear Capture Flag
	TIM2->SR  &= ~TIM_SR_CC2IF; 					//Clear Capture Flag
	TIM2->CR1 |=  TIM_CR1_CEN;						//Start timer - Counter Enable
}

void pulse_echo_driver_stop(void){
	TIM2->CR1   &= ~TIM_CR1_CEN;					// Counter Disable
	TIM2->CCMR1 |= 	(4U << TIM_CCMR1_OC2M_Pos); 	// Match -> LOW
}

uint32_t pulse_echo_driver_get_time1(void){
	return time_1*timer_resolution;
}
uint32_t pulse_echo_driver_get_time2(void){
	return time_2*timer_resolution;
}

uint32_t pulse_echo_driver_get_time(void){
	return sensor_time*timer_resolution;
}

void pulse_echo_driver_us_settings(uint32_t TIMER_RESOLUTION_us, uint32_t TIMER_CLOCK_F_MHZ_us, uint32_t TRIG_DURATION_us, uint32_t PERIOD_TIME_us){
	TIM2->CR1 	&= ~TIM_CR1_CEN;				// Counter Disable
	TIM2->CCMR1 |= 	(4U << TIM_CCMR1_OC2M_Pos); // Match -> LOW

	timer_psc 		 = TIMER_RESOLUTION_us * (TIMER_CLOCK_F_MHZ_us) -1;
	timer_arr 		 = PERIOD_TIME_us/TIMER_RESOLUTION_us;
	timer_ccr2 		 = TRIG_DURATION_us/TIMER_RESOLUTION_us;
	timer_resolution = TIMER_RESOLUTION_us;

	TIM2->PSC  = (uint16_t)timer_psc;
	TIM2->ARR  = (uint16_t)timer_arr;
	TIM2->CCR2 = (uint16_t)timer_ccr2;

	//Generate Update to apply settings immediately
	TIM2->EGR |= TIM_EGR_UG;
	}

void pulse_echo_driver_ms_settings(uint32_t TIMER_RESOLUTION_ms, uint32_t TIMER_CLOCK_F_MHZ_ms, uint32_t TRIG_DURATION_ms, uint32_t PERIOD_TIME_ms){
	TIM2->CR1 	&= ~TIM_CR1_CEN;				// Counter Disable
	TIM2->CCMR1 |= 	(4U << TIM_CCMR1_OC2M_Pos); // Match -> LOW

	timer_psc 		 = TIMER_RESOLUTION_ms * (TIMER_CLOCK_F_MHZ_ms*1000) -1;
	timer_arr 		 = PERIOD_TIME_ms/TIMER_RESOLUTION_ms;
	timer_ccr2 	 	 = TRIG_DURATION_ms/TIMER_RESOLUTION_ms;
	timer_resolution = TIMER_RESOLUTION_ms;

	TIM2->PSC  = (uint16_t)timer_psc;
	TIM2->ARR  = (uint16_t)timer_arr;
	TIM2->CCR2 = (uint16_t)timer_ccr2;

	//Generate Update to apply settings immediately
	TIM2->EGR |= TIM_EGR_UG;
}

/* -----------------------------------*/
/* --- PUBLIC API FUNCTIONS END------ */
/* -----------------------------------*/



/* ------------------------------------*/
/* --- IRQHandler FUNCTIONS START------*/
/* ------------------------------------*/
void TIM2_IRQHandler(void){
	uint32_t status = TIM2->SR; 						//Read flags status

	// --- 1. TRIG (Rising Edge) ---
	if (status & TIM_SR_UIF) {
			TIM2->SR &= ~TIM_SR_UIF;					//Clear Update Flag

			TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
			TIM2->CCMR1 |= (5U << TIM_CCMR1_OC2M_Pos); 	// Match -> HIGH
	}

	// --- 2. TRIG (Falling Edge) ---
	if (status & TIM_SR_CC2IF) {
	        TIM2->SR &= ~TIM_SR_CC2IF; 					//Clear Compare Flag

	        TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
	      	TIM2->CCMR1 |= 	(4U << TIM_CCMR1_OC2M_Pos); // Match -> LOW
	}


	// --- 2. ECHO (Channel 1) ---
	if (status & TIM_SR_CC1IF) {
	        TIM2->SR &= ~TIM_SR_CC1IF; //Clear Capture Flag

	        if (echo_state == 1) { //Rising edge captured (Echo starts)
	            time_1 = TIM2->CCR1;
	            echo_state = 0;
	        } else { // Falling edge captured (Echo ends)
	            time_2 = TIM2->CCR1;
	            sensor_time = time_2 - time_1;
	            echo_state = 1;
	        }
	}
}
/* ------------------------------------*/
/* --- IRQHandler FUNCTIONS END--------*/
/* ------------------------------------*/
