# stm32-nonblocking-pulse-echo-measurement-driver
WHAT IS A PULSE-ECHO DRIVER? 
This driver is designed with a non-blocking architecture, utilizing only two pins (Trigger/Pulse 
and Echo) and a single hardware timer.

The primary objective of this driver is to support devices that are triggered by a pulse signal 
at specific periods and respond with an echo signal. A prominent example of such a device 
is the HC-SR04 ultrasonic distance sensor. In these types of sensors, the critical data 
required is the duration of the echo signal. This driver provides specialized functions to 
capture and return this duration without stalling the CPU.

It is assumed that the echo signal will be terminated before the period defined by the device 
expires. Therefore, no timeout protection is implemented within this driver.

HARDWARE CONFIGURATION 
To ensure the driver operates correctly, the hardware must be configured as follows. This 
driver has been developed on the specified target board: 
● Target Board: STM32Nucleo-F302R8 
● Hardware Timer: TIM2 
● Pin Assignments: 
  ○ PA0: ECHO PIN (Input Capture) 
  ○ PA1: PULSE/TRIGGER PIN (Output) 

For more detailed information, technical constraints, and API usage, please refer to the UM-STM32-PulseEcho-Driver-V1.0.pdf.
