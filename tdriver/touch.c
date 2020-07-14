/*Copyright (c) 2020 Oleksandr Ivanov.
  *
  * This software component is licensed under MIT license.
  * You may not use this file except in compliance retain the
  * above copyright notice.
  */
#include "touch.h"
#include <stdlib.h>
// The number of cycles to read the coordinate
#define JITTER 20
// The number of maximum and minimum discarded values (JITTER_CUT must be less than JITTER / 2)
#define JITTER_CUT 3
#define X 0
#define Y 1
//Initial values of the calibration (selected empirically)
float kX = 13.427135f;    // kX - proportional coefficient X axis
float kY = 10.286438f;    // kY - proportional coefficient Y axis
uint8_t offsetX = 31;     // X origin offset
uint8_t offsetY = 31;     // Y origin offset

//GPIO Init structure definition for Touch detection
GPIO_InitTypeDef GPIO_TouchDetectOut = { 0 };
GPIO_InitTypeDef GPIO_TouchDetectIn_Potr_A = { 0 };
GPIO_InitTypeDef GPIO_TouchDetectIn_Potr_B = { 0 };

//GPIO Init structure definition for Y coordinate measuring
GPIO_InitTypeDef GPIO_Y_measuri_power = { 0 };
GPIO_InitTypeDef GPIO_Y_measuri_analog = { 0 };
GPIO_InitTypeDef GPIO_Y_measuri_ADC = { 0 };

//GPIO Init structure definition for X coordinate measuring
GPIO_InitTypeDef GPIO_X_measuri_powerP = { 0 };
GPIO_InitTypeDef GPIO_X_measuri_powerGND = { 0 };
GPIO_InitTypeDef GPIO_X_measuri_analog = { 0 };
GPIO_InitTypeDef GPIO_X_measuri_ADC = { 0 };

//ADC1 Handle structure definition for XY coordinate measuring
ADC_HandleTypeDef hadc1_XYmeasuring;

//Channel config structure definition for Y coordinate measuring
ADC_ChannelConfTypeDef sConfigYmeasuring = { 0 };

//Channel config structure definition for X coordinate measuring
ADC_ChannelConfTypeDef sConfigXmeasuring = { 0 };

void y_messuring_init();
void x_messuring_init();
uint16_t adc_messuring(ADC_HandleTypeDef *hadc);
uint16_t messuring_cord(uint8_t cord);
uint16_t average_array(uint16_t array[], size_t arra_length);
void calibr_start();
void calibr_stop();
void set_point(uint16_t point_x, uint16_t point_y);
void fun_pointer_diff();


/**
 *  Initialize GPIOs init structures, Handel Structure for ADC and configuration structure for ADC channels.
 *  @context: Should be called before main's while(1) cycle.
 */
void touch_init()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();    //RCC clock enabling for GPIO
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();    // RCC clock enabling for ADC1

    /* GPIO init structures initialization for touch detection step */

    GPIO_TouchDetectOut.Pin = DET_TOUC_PLUS1_PIN | DET_TOUC_PLUS2_PIN;
    GPIO_TouchDetectOut.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_TouchDetectOut.Pull = GPIO_NOPULL;
    GPIO_TouchDetectOut.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_TouchDetectIn_Potr_A.Pin = DET_TOUC_IN_PIN;
    GPIO_TouchDetectIn_Potr_A.Mode = GPIO_MODE_INPUT;
    GPIO_TouchDetectIn_Potr_A.Pull = GPIO_PULLDOWN;
    GPIO_TouchDetectIn_Potr_B.Pin = DET_TOUC_AN_PIN;
    GPIO_TouchDetectIn_Potr_B.Mode = GPIO_MODE_ANALOG;
    GPIO_TouchDetectIn_Potr_B.Pull = GPIO_NOPULL;

    /* GPIO init structure initialization for Y coordinate measuring */

    //Initialization for X_plus and X_minus
    GPIO_Y_measuri_power.Pin = Y_PLUS_PIN | Y_MINUS_PIN;
    GPIO_Y_measuri_power.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Y_measuri_power.Pull = GPIO_NOPULL;
    GPIO_Y_measuri_power.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //Initialization  for  GPIO pin to high impedance stay
    GPIO_Y_measuri_analog.Pin = Y_MES_ANALOG_PIN;
    GPIO_Y_measuri_analog.Mode = GPIO_MODE_ANALOG;
    GPIO_Y_measuri_analog.Pull = GPIO_NOPULL;

    //ADC1 GPIO Initialization
    GPIO_Y_measuri_ADC.Pin = Y_MEASURE_PIN;
    GPIO_Y_measuri_ADC.Mode = GPIO_MODE_ANALOG;
    GPIO_Y_measuri_ADC.Pull = GPIO_NOPULL;

    /* GPIO init structure initialization for X coordinate measuring */

    //Initialization for X_plus for continuously output 1(3.3 V)
    GPIO_X_measuri_powerP.Pin = X_PLUS_PIN;
    GPIO_X_measuri_powerP.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_X_measuri_powerP.Pull = GPIO_NOPULL;
    GPIO_X_measuri_powerP.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //Initialization for X_plus for continuously output 0(GND)
    GPIO_X_measuri_powerGND.Pin = X_MINUS_PIN;
    GPIO_X_measuri_powerGND.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_X_measuri_powerGND.Pull = GPIO_NOPULL;
    GPIO_X_measuri_powerGND.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //Initialization  for  GPIO pin to high impedance stay
    GPIO_X_measuri_analog.Pin = X_MES_ANALOG_PIN;
    GPIO_X_measuri_analog.Mode = GPIO_MODE_ANALOG;
    GPIO_X_measuri_analog.Pull = GPIO_NOPULL;

    //ADC GPIO Initialization
    GPIO_X_measuri_ADC.Pin = X_MEASURE_PIN;
    GPIO_X_measuri_ADC.Mode = GPIO_MODE_ANALOG;
    GPIO_X_measuri_ADC.Pull = GPIO_NOPULL;

    /*Handle structure initialization for the global features of the ADC (Clock, Resolution,
                                              * Data Alignment and number of conversion)*/
    hadc1_XYmeasuring.Instance = ADC1;
    hadc1_XYmeasuring.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1_XYmeasuring.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1_XYmeasuring.Init.ScanConvMode = DISABLE;
    hadc1_XYmeasuring.Init.ContinuousConvMode = DISABLE;
    hadc1_XYmeasuring.Init.DiscontinuousConvMode = DISABLE;
    hadc1_XYmeasuring.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1_XYmeasuring.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1_XYmeasuring.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1_XYmeasuring.Init.NbrOfConversion = 1;
    hadc1_XYmeasuring.Init.DMAContinuousRequests = DISABLE;
    hadc1_XYmeasuring.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    //Configure ADC1
    if (HAL_ADC_Init(&hadc1_XYmeasuring) != 0) {
    	touch_error_handler("Error!ADC_Inti.\r\n");
    }

    /*Y measuring Init structure initialization for the selected ADC regular channel its corresponding
                                              *rank in the sequencer and its sample time.*/
    sConfigYmeasuring.Channel = ADC_CHANNEL_4;
    sConfigYmeasuring.Rank = 1;
    sConfigYmeasuring.SamplingTime = ADC_SAMPLETIME_480CYCLES;

    /*X measuring Init structure initialization for the selected ADC regular channel its corresponding
	                                              *rank in the sequencer and its sample time.*/
    sConfigXmeasuring.Channel = ADC_CHANNEL_1;
    sConfigXmeasuring.Rank = 1;
    sConfigXmeasuring.SamplingTime = ADC_SAMPLETIME_480CYCLES;

    fun_pointer_diff();
}

/**
 *  Detects if touch panel was pressed
 *  @return: 1 if touch detected and 0 if not
 */
uint8_t touch_detect_touch()
{
    static uint32_t last_tick;
    if (HAL_GetTick() - last_tick >= DEBOUNCE) {
        //Configure GPIO pins for continuously output 1(3.3 V)
        HAL_GPIO_Init(DET_TOUC_P_IN_PORT, &GPIO_TouchDetectOut);
        HAL_GPIO_WritePin(DET_TOUC_P_IN_PORT, Y_PLUS_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(DET_TOUC_P_IN_PORT, Y_MINUS_PIN, GPIO_PIN_SET);

        //Configure GPIO pin for input
        HAL_GPIO_Init(DET_TOUC_P_IN_PORT, &GPIO_TouchDetectIn_Potr_A);

        //Configure GPIO pin for high impedance stay
        HAL_GPIO_Init(DET_TOUC_AN_PORT, &GPIO_TouchDetectIn_Potr_B);
        last_tick = HAL_GetTick();
        return HAL_GPIO_ReadPin(DET_TOUC_P_IN_PORT, DET_TOUC_IN_PIN);
    }
    return 0;
}

/**
 *  Provides ADC value for Y coordinate
 *  @return: uint16_t with max value no more 4095 (12 bit ADC)
 *  @context: should be used if touch_detect_touch() return 1
 */
uint16_t touch_get_y_adc()
{
    y_messuring_init();
    return adc_messuring(&hadc1_XYmeasuring);
}

/**
 *  Provides ADC value for X coordinate
 *  @return: uint16_t with max value no more 4095 (12 bit ADC)
 *  @context: should be used if touch_detect_touch() return 1
 */
uint16_t touch_get_x_adc()
{
    x_messuring_init();
    return adc_messuring(&hadc1_XYmeasuring);
}

/**
 *  Initialize MCU to the state for LCD drawing mode
 */

void touch_lcd_gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    //GPIO Ports Clock Enable
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();

    //Configure GPIO control pins
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //Configure GPIO data pins
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
/**
 *  Provides voltage value from ADC pin for X coordinate
 *  @return: float max value no more then 3.3 V
 *  @context: Should be used if touch_detect_touch() return 1
 */
float touch_get_x_voltage()
{
    return (touch_get_x_adc() * ADC_REF_VOL_P) / ADC_SIZE;
}

/**
 *  Provides voltage value from ADC pin for Y coordinate
 *  @return: float max value no more then 3.3 V
 *  @context: Should be used after touch_detect_touch() return 1
 */
float touch_get_y_voltage()
{
    return (touch_get_y_adc() * ADC_REF_VOL_P) / ADC_SIZE;
}

/**
 *  Provides calibrated X coordinate of LCD
 *  @return: uint16_t value from 0 to X_PIX_MAX
 *  @context: should be used after touch_detect_touch() return 1
 */
uint16_t touch_get_x_coord_lcd()
{
    x_messuring_init();
    return (messuring_cord(X) / kX) - offsetX;
}
/**
 *  Provides calibrated Y coordinate of LCD
 *  @return: uint16_t value from 0 to Y_PIX_MAX
 */
uint16_t touch_get_y_coord_lcd()
{
    y_messuring_init();
    return (messuring_cord(Y) / kY) - offsetY;
}

/**
 *  Initialize GPIO and ADC for X messuring
 */
void x_messuring_init()
{
    //Configure GPIO pins : PA4 (GND) and PB10(3.3 V) for power supply
    HAL_GPIO_Init(X_PLUS_PORT, &GPIO_X_measuri_powerP);
    HAL_GPIO_Init(X_MINUS_PORT, &GPIO_X_measuri_powerGND);

    HAL_GPIO_WritePin(X_PLUS_PORT, X_PLUS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(X_MINUS_PORT, X_MINUS_PIN, GPIO_PIN_RESET);

    //Configure GPIO pin : PA8 for high impedance stay
    HAL_GPIO_Init(X_MES_ANALOG_PORT, &GPIO_X_measuri_analog);

    //ADC1 GPIO configuration PA1 ------> ADC1_IN1
    HAL_GPIO_Init(X_MEASURE_PORT, &GPIO_X_measuri_ADC);

    //Configure channel ADC1
    if (HAL_ADC_ConfigChannel(&hadc1_XYmeasuring, &sConfigXmeasuring) != 0) {
    	touch_error_handler("Error!ADC_Config.\r\n");
    }
}
/**
 *  Initialize GPIO and ADC for Y messuring
 */
void y_messuring_init()
{
    //Configure GPIO pins : PA1 (GND) and PA8(3.3 V) for power supply
    HAL_GPIO_Init(Y_MINUS_PORT, &GPIO_Y_measuri_power);
    HAL_GPIO_WritePin(Y_PLUS_PORT, Y_PLUS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Y_MINUS_PORT, Y_MINUS_PIN, GPIO_PIN_RESET);

    //Configure GPIO pin : PB10 for high impedance stay
    HAL_GPIO_Init(Y_MES_ANALOG_PORT, &GPIO_Y_measuri_analog);

    //ADC1 GPIO configuration PA4 ------> ADC1_IN4
    HAL_GPIO_Init(Y_MEASURE_PORT, &GPIO_Y_measuri_ADC);

    //Configure channel ADC1
    if (HAL_ADC_ConfigChannel(&hadc1_XYmeasuring, &sConfigYmeasuring) != 0) {
    	touch_error_handler("Error!ADC_Config.\r\n");
    }
}
/**
 *  Provides ADC value
 *  @hadc: ADC handler structure
 *  @return: uint16_t max value no more 4095 (12 bit ADC)
 */
uint16_t adc_messuring(ADC_HandleTypeDef *hadc)
{
    uint16_t ADC_value = 0;
    HAL_ADC_Start(hadc);
    if (HAL_ADC_PollForConversion(hadc, 10) == 0) {
        ADC_value = HAL_ADC_GetValue(hadc);
    }
    return ADC_value;
}

/**
 *  Provides X or Y coordinate
 *  @cord: if 0 mesures X coordinate if 1 measures Y coordinate
 *  @return: Not calibrated coordinate value
 */
uint16_t messuring_cord(uint8_t cord)
{
    uint16_t array[JITTER] = { 0 };
    for (uint8_t i = 0; i < JITTER; ++i) {
        if (cord == X) {
            array[i] = touch_get_x_adc();
        } else if (cord == Y) {
            array[i] = touch_get_y_adc();
        }
    }
    size_t array_length = sizeof(array) / sizeof(array[0]);
    return average_array(array, array_length);
}

/**
 *  Comparison function for qsort()
 *  @a: first argument
 *  @b: second argument
 *  @return:  negative integer value if the first argument is less than the second
 *            a positive integer value if the first argument is greater than the second
 *            and zero if the arguments are equa
 */

int compare(const void* a, const void* b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;

}

/**
 *  Calculate average value of the array
 *  @array: array of integer
 *  @return: average integer value
 */

uint16_t average_array(uint16_t array[], size_t arra_length)
{
    qsort(array, arra_length, sizeof(array[0]), compare);
    uint32_t temp = 0;
    //Excluding extreme elements of array
    for (uint8_t i = JITTER_CUT; i < JITTER - JITTER_CUT; ++i) {
        temp += array[i];
    }
    temp /= JITTER - JITTER_CUT * 2;
    return temp;
}

/**
 *  Calibrates touch screen
 *  @pkX: proportional coefficient X axis
 *  @pkY: proportional coefficient Y axis
 *  @p_offsetX: X origin offset
 *  @p_offsetX: Y origin offset
 */
void touch_calibr(float *pkX, float *pkY, uint8_t *p_offsetX, uint8_t *p_offsetX)
{
    touch_lcd_gpio_init();
    p_delay(100);
    p_lcd_setcursor(0, Y_PIX_MAX / 2);
    p_lcd_fill_screen(BLACK);
    p_lcd_printf("\t\tCALIBRATION LOADING....\r\n");
    p_delay(2000);

    uint8_t MeasureFlags = 0;
    uint16_t adcX;
    uint16_t adcY;
    uint16_t pointX[4] = { 0 };
    uint16_t pointY[4] = { 0 };

    calibr_start();

    while (MeasureFlags != 0x0F) {
        while (touch_detect_touch() == TOUCH_NOT_DETECTED);
        adcX = messuring_cord(X);
        adcY = messuring_cord(Y);

        if ((adcX < ADC_SIZE / 2) && (adcY < ADC_SIZE / 2)) {    // Left top dot (DOT 1)
            pointX[LEFT_TOP_1] = adcX;
            pointY[LEFT_TOP_1] = adcY;
            MeasureFlags |= (1 << LEFT_TOP_1);
            set_point(POINT1_X, POINT1_Y);
        } else if ((adcX > ADC_SIZE / 2) && (adcY < ADC_SIZE / 2)) {    // Right top dot (DOT 2)
            pointX[RIGHT_TOP_2] = adcX;
            pointY[RIGHT_TOP_2] = adcY;
            MeasureFlags |= (1 << RIGHT_TOP_2);
            set_point(POINT2_X, POINT2_Y);
        } else if ((adcX > ADC_SIZE / 2) && (adcY > ADC_SIZE / 2)) {    // Right down dot (DOT 3)
            pointX[RIGHT_DOWN_3] = adcX;
            pointY[RIGHT_DOWN_3] = adcY;
            MeasureFlags |= (1 << RIGHT_DOWN_3);
            set_point(POINT3_X, POINT3_Y);
        } else if ((adcX < ADC_SIZE / 2) && (adcY > ADC_SIZE / 2)) {    //Left down dot (DOT 4)
            pointX[LEFT_DOWN_4] = adcX;
            pointY[LEFT_DOWN_4] = adcY;
            MeasureFlags |= (1 << LEFT_DOWN_4);
            set_point(POINT4_X, POINT4_Y);
        }

        while (touch_detect_touch() == TOUCH_DETECTED);
        p_delay(1000);
    }
    //Averaging the value of the X and Y coordinate at points
    pointX[RIGHT_DOWN_3] = (pointX[RIGHT_DOWN_3] + pointX[RIGHT_TOP_2]) / 2;
    pointX[LEFT_TOP_1] = (pointX[LEFT_TOP_1] + pointX[LEFT_DOWN_4]) / 2;
    pointY[RIGHT_DOWN_3] = (pointY[RIGHT_DOWN_3] + pointY[LEFT_DOWN_4]) / 2;
    pointY[LEFT_TOP_1] = (pointY[LEFT_TOP_1] + pointY[RIGHT_TOP_2]) / 2;
    //X and Y axis proportional factor calculation
    kX = (float)(pointX[RIGHT_DOWN_3] - pointX[LEFT_TOP_1]) / (POINT3_X - POINT1_X);
    kY = (float)(pointY[RIGHT_DOWN_3] - pointY[LEFT_TOP_1]) / (POINT3_Y - POINT1_Y);
    // Calculation of the displacement coefficient along the X and Y axis
    offsetX = pointX[LEFT_TOP_1] / kX - POINT1_X;
    offsetY = pointY[LEFT_TOP_1] / kY - POINT1_Y;

    *pkX = kX;
    *pkY = kY;
    *p_offsetX = offsetX;
    *p_offsetX = offsetY;

    calibr_stop();
}

/**
 *  Displays the message of the calibration beginning
 */
void calibr_start()
{
    void touch_lcd_gpio_init();
    p_lcd_fill_screen(BLACK);
    p_lcd_setcursor(0, Y_PIX_MAX / 2);
    p_lcd_printf("\t\tSCREEN CALIBRATION\n");
    p_lcd_printf("\tTouch points on the screen\n");
    // Display on the screen 4 calibration points
    p_lcd_fill_circle(POINT1_X, POINT1_Y, 5, GREEN);
    p_lcd_fill_circle(POINT2_X, POINT2_Y, 5, GREEN);
    p_lcd_fill_circle(POINT3_X, POINT3_Y, 5, GREEN);
    p_lcd_fill_circle(POINT4_X, POINT4_Y, 5, GREEN);
}

/**
 *  Displays that curent point calibration compleated
 *  @point_x:  X coordinate of calibrated point
 *  @point_y:  Y coordinate of calibrated point
 */
void set_point(uint16_t point_x, uint16_t point_y)
{
    touch_lcd_gpio_init();
    p_lcd_fill_circle(point_x, point_y, 5, RED);    // Change color of calibration point
}

/**
 *  Functions pointer definition related to LCD screen operation
 *  @context: Definition could be changed to another LCD's library accordingly.
 */
void fun_pointer_diff()
{
	p_lcd_setcursor = LCD_SetCursor;
	p_lcd_printf = LCD_Printf;
	p_lcd_fill_screen = LCD_FillScreen;
	p_delay = HAL_Delay;
	p_lcd_fill_circle = LCD_FillCircle;
}

/**
 *  Displays the massage about calibration finish
 */
void calibr_stop()
{
    touch_lcd_gpio_init();
    p_lcd_fill_screen(BLACK);
    p_lcd_setcursor(1, 1);
    p_lcd_printf("SCREEN CALIBRATION FINISHED\n");
    p_lcd_printf("kX = %4f\n", kX);
    p_lcd_printf("kY = %4f\n", kY);
    p_lcd_printf("offsetX = %4d\n", offsetX);
    p_lcd_printf("offsetY = %4d\n", offsetY);
    p_delay(5000);
    p_lcd_fill_screen(BLACK);
}

/**
 *  Displays the error on the screen and blocks execution
 *  @error:  Error code or description.
 */
void touch_error_handler(char *error)
{
	touch_lcd_gpio_init();
	p_lcd_fill_screen(BLACK);
	p_lcd_printf("%sTry to reset the device.\r\n", error);
	while(1);
}
