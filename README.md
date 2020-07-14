# TouchScreenDriver V1.0

Driver provides API for resistive touch screens. Was tested using MCUfriend shield LCD screen
and STM NUCLEO F446RE(STM32F446RE) devboard. 

# Using

Add touch.c and touch.h files to your project. 
Additionaly required HAL and CMSIS drivers according to your STM device. 
It is up to the developer to choose libraries they want to use for LCD but 
could use provided display library for MCUfriend shield. 

# Example

Demo video https://youtu.be/Yb9gq0tuQPY 
```cpp

#include "lcd.h"
#include "touch.h"
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    touch_init();
    LCD_Init();

    //varibal for ADC values
    uint32_t Y_ADC_value = 0;
    uint32_t X_ADC_value = 0;
    //varibals for converted ADC valuse to voltage
    float X_voltage_value = 0;
    float Y_voltage_value = 0;
    //varibals for x and y coordinate values
    uint16_t Y_cord_value = 159;
    uint16_t X_cord_value = 119;
    uint16_t X_last_cord = X_cord_value;
    uint16_t Y_last_cord = Y_cord_value;

    //Draw the initial position of ball
    LCD_FillCircle(X_cord_value, Y_cord_value, 10, WHITE);

    while (1) {
        //detection touching the screen
        if (touch_detect_touch() == TOUCH_DETECTED) {
            //getting ADC values
            X_ADC_value = touch_get_x_adc();
            Y_ADC_value = touch_get_y_adc();
            //getting converted ADC values to voltage
            X_voltage_value = touch_get_x_voltage();
            Y_voltage_value = touch_get_y_voltage();
            //getting the coordinate values
            Y_cord_value = touch_get_y_coord_lcd();
            X_cord_value = touch_get_x_coord_lcd();
            /*Defining the area of the screen that needs to be touched
	     * to start the calibration process */
            if (Y_cord_value > 300 && X_cord_value > 210) {
                touch_calibr(&kX, &kY, &offsetX, &offsetY);
            }
        }
        /* MCU's ports and pins initialization for LCD drawing mode.
	 * It is required in the case if the touch panel and LCD screen are using the
	 * same pins for their functioning.
	 */
        touch_lcd_gpio_init();

        //Printing measured values, just for info
        LCD_SetCursor(0, 0);
        LCD_Printf("ADC x = %-4d\r\n", X_ADC_value);
        LCD_Printf("ADC y = %-4d\r\n", Y_ADC_value);
        LCD_Printf("Vol x = %5.3fV\r\n", X_voltage_value);
        LCD_Printf("Vol y = %5.3fV\r\n", Y_voltage_value);
        LCD_Printf("X_cord = %-4d\r\n", X_cord_value);
        LCD_Printf("Y_cord = %-4d\r\n", Y_cord_value);

        //Drawing the new position of the ball according to the new new X and Y coordinate
        if (X_last_cord != X_cord_value || Y_last_cord != Y_cord_value) {
            LCD_FillCircle(X_last_cord, Y_last_cord, 10, BLACK);
            LCD_FillCircle(X_cord_value, Y_cord_value, 10, WHITE);
            X_last_cord = X_cord_value;
            Y_last_cord = Y_cord_value;
        }
    }
    return 0;
}

```

