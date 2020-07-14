
/*Copyright (c) 2020 Oleksandr Ivanov.
  *
  * This software component is licensed under MIT license.
  * You may not use this file except in compliance retain the
  * above copyright notice.
  */

#ifndef TOUCHSCREEN
#define TOUCHSCREEN

//need to be changed according to device used
#include "stm32f4xx_hal.h"
#include "lcd.h"

//Define ports and pins for Y coordinate power supply
#define Y_MINUS_PIN GPIO_PIN_1
#define Y_PLUS_PORT GPIOA
#define Y_PLUS_PIN GPIO_PIN_8
#define Y_MINUS_PORT GPIOA

//Define ports and pins for X coordinate power supply
#define X_PLUS_PIN GPIO_PIN_10
#define X_PLUS_PORT GPIOB
#define X_MINUS_PIN GPIO_PIN_4
#define X_MINUS_PORT GPIOA

//Define ADC measuring ports and pins for Y coordinate
#define Y_MEASURE_PORT GPIOA
#define Y_MEASURE_PIN GPIO_PIN_4
#define X_MEASURE_PORT GPIOA
#define X_MEASURE_PIN GPIO_PIN_1

//Define ADC measuring ports and pins for X coordinate
#define Y_MES_ANALOG_PORT GPIOB
#define Y_MES_ANALOG_PIN GPIO_PIN_10
#define X_MES_ANALOG_PORT GPIOA
#define X_MES_ANALOG_PIN GPIO_PIN_8

//Define ports and pins for touch detection
#define DET_TOUC_PLUS1_PIN GPIO_PIN_1
#define DET_TOUC_PLUS2_PIN GPIO_PIN_8
#define DET_TOUC_IN_PIN GPIO_PIN_4
#define DET_TOUC_P_IN_PORT GPIOA
#define DET_TOUC_AN_PIN GPIO_PIN_10
#define DET_TOUC_AN_PORT GPIOB

#define TOUCH_DETECTED 1
#define TOUCH_NOT_DETECTED 0

//Define max reference voltage
#define ADC_REF_VOL_P 3.3
//Define ADC max size
#define ADC_SIZE 4095UL

#define DEBOUNCE 75

//Empirically measured values for max and min ADC values for x and y refer to display size
#define X_MIN_ADC_REF_LCD 430
#define X_MAX_ADC_REF_LCD 3670
#define Y_MIN_ADC_REF_LCD 370
#define Y_MAX_ADC_REF_LCD 3777

//LCD screen pixel size
#define X_PIX_MAX 239
#define Y_PIX_MAX 319

#define OFFSET_POINT 20          // Offset in pixels of the coordinate of the calibration point
#define POINT1_X OFFSET_POINT    // Left top point (Point 1)
#define POINT1_Y OFFSET_POINT
#define POINT2_X (X_PIX_MAX - OFFSET_POINT)    // Right top point (Point 2)
#define POINT2_Y OFFSET_POINT
#define POINT3_X (X_PIX_MAX - OFFSET_POINT)    // Right down point (Point 3)
#define POINT3_Y (Y_PIX_MAX - OFFSET_POINT)
#define POINT4_X OFFSET_POINT                  // Left down point (Point 4)
#define POINT4_Y (Y_PIX_MAX - OFFSET_POINT)

//Screen points for touch calibration
#define LEFT_TOP_1 0
#define RIGHT_TOP_2 1
#define RIGHT_DOWN_3 2
#define LEFT_DOWN_4 3

extern float kX;    // proportional coefficient for X axis(horizontal)
extern float kY;    // proportional coefficient for Y axis(horizontal)
extern uint8_t offsetX;    // X origin offset
extern uint8_t offsetY;    // Y origin offset


//pointers to function releted to LCD screen operation
void (*p_lcd_printf)(const char *fmt, ...);
void (*p_lcd_setcursor)(uint16_t x, uint16_t y);
void (*p_lcd_fill_screen)(uint16_t color);
void (*p_lcd_fill_circle)(int16_t x0, int16_t y0, int16_t r, uint16_t color);
//pointers to delay function
void (*p_delay)(uint32_t delay);

void touch_init();                    //touch screen initialization
uint8_t touch_detect_touch();         //touch detection
uint16_t touch_get_x_adc();           //return ADC value for x coordinate
uint16_t touch_get_y_adc();           //return ADC value for y coordinate
float touch_get_x_voltage();          //return voltage value for x coordinate
float touch_get_y_voltage();          //return voltage value for y coordinate
uint16_t touch_get_x_coord_lcd();     //return x coordinate
uint16_t touch_get_y_coord_lcd();     //return y coordinate
void touch_lcd_gpio_init();           //initialize PINs for LCD drawing mode
void touch_error_handler(char *error);//touch error handler
void touch_calibr(float *pkX, float *pkY, uint8_t *p_offsetX,
		          uint8_t *p_offsetY); //touch screen calibration


#endif /* TOUCHSCREEN_TOUCH */
