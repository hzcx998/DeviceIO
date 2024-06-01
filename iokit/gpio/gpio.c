#include <iokit/gpio.h>
#include <libdrv/gpio.h>
#include <devio.h>

void GPIO_SetMode(unsigned int gpio, int mode)
{
    GPIO_ChipType *chip = GPIO_SearchChip(gpio);
    if (chip && chip->setMode) {
        chip->setMode(chip, gpio - chip->base, mode);
    }
}

void GPIO_SetValue(unsigned int gpio, int value)
{
    GPIO_ChipType *chip = GPIO_SearchChip(gpio);
    if (chip && chip->setValue) {
        chip->setValue(chip, gpio - chip->base, value);
    }
}

int GPIO_GetMode(unsigned int gpio)
{
    GPIO_ChipType *chip = GPIO_SearchChip(gpio);
    if (chip && chip->getMode) {
        return chip->getMode(chip, gpio - chip->base);
    }
    return 0;
}

int GPIO_GetValue(unsigned int gpio)
{
    GPIO_ChipType *chip = GPIO_SearchChip(gpio);
    if (chip && chip->getValue) {
        return chip->getValue(chip, gpio - chip->base);
    }
    return 0;
}
