#ifndef _IOKIT_GPIO_H
#define _IOKIT_GPIO_H

#define GPIO_LOW                 0x00
#define GPIO_HIGH                0x01

#define GPIO_MODE_OUTPUT         0x00
#define GPIO_MODE_INPUT          0x01
#define GPIO_MODE_INPUT_PULLUP   0x02
#define GPIO_MODE_INPUT_PULLDOWN 0x03
#define GPIO_MODE_OUTPUT_OD      0x04
#define GPIO_MODE_OUTPUT_PP      0x05

#define GPIO_RATE_SLOW           0x00
#define GPIO_RATE_MEDIUM         0x01
#define GPIO_RATE_FAST           0x02

void GPIO_SetMode(unsigned int gpio, int mode);
void GPIO_SetValue(unsigned int gpio, int value);
int GPIO_GetMode(unsigned int gpio);
int GPIO_GetValue(unsigned int gpio);

#endif
