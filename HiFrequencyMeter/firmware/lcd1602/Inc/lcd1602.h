#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gpio.h"
#include "main.h"

#ifndef INC_LCD1602_H_
#define INC_LCD1602_H_

_BEGIN_STD_C

void lcd_init (void);   // initialize lcd

void init_lcd_pins(); // init pin use for lcd.

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);

_END_STD_C
#endif /* INC_LCD1602_H_ */
