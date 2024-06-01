// HAL lib
#include <stm32f10x_gpio.h>

// DEVIO
#include <devio.h>
#include <libdrv/gpio.h>
#include <iokit/gpio.h>

#include "printf.h"

static IO_DriverType *STM32F10X_GPIO_Driver;

struct STM32F10X_GPIOExt {
    int gpioIrqOff;
    int gpioAddr;
};

#define PIN_BITOFF(off) (1U << (off))

GPIOMode_TypeDef ConvertMode(unsigned int gpiomode)
{
    GPIOMode_TypeDef mode;
    switch (gpiomode)
    {
    case GPIO_MODE_OUTPUT:
        mode = GPIO_Mode_Out_PP;
        break;
    case GPIO_MODE_INPUT:
        mode = GPIO_Mode_IN_FLOATING;
        break;
    case GPIO_MODE_INPUT_PULLUP:
        mode = GPIO_Mode_IPU;
        break;
    case GPIO_MODE_INPUT_PULLDOWN:
        mode = GPIO_Mode_IPD;
        break;
    case GPIO_MODE_OUTPUT_OD:
        mode = GPIO_Mode_Out_OD;
        break;
    case GPIO_MODE_OUTPUT_PP:
        mode = GPIO_Mode_Out_PP;
        break;
    default:
        mode = GPIO_Mode_Out_PP;
        break;
    }
    return mode;
}

GPIOSpeed_TypeDef ConvertSpeed(unsigned int rate)
{
    GPIOSpeed_TypeDef speed;
    switch (rate)
    {
    case GPIO_RATE_SLOW:
        speed = GPIO_Speed_10MHz;
        break;
    case GPIO_RATE_MEDIUM:
        speed = GPIO_Speed_2MHz;
        break;
    case GPIO_RATE_FAST:
        speed = GPIO_Speed_50MHz;
        break;
    default:
        speed = GPIO_Speed_50MHz;
        break;
    }
    return speed;
}

static void STM32_GPIO_SetMode(struct GPIO_Chip * chip, int offset, int mode)
{
    struct STM32F10X_GPIOExt *ext = (struct STM32F10X_GPIOExt *)chip->extension;

    printf("STM32_GPIO_SetMode\n");

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);

    init.GPIO_Pin = PIN_BITOFF(offset);
    init.GPIO_Mode = ConvertMode(mode & 0xffff);
    init.GPIO_Speed = ConvertSpeed(mode >> 16);

    printf("gpio: addr:%d pin:%x, mode:%x, spped:%d\n", ext->gpioAddr, init.GPIO_Pin, init.GPIO_Mode, init.GPIO_Speed);
    if (IS_GET_GPIO_PIN(init.GPIO_Pin)) {
        GPIO_Init(ext->gpioAddr, &init);
    }
}

static int STM32_GPIO_GetMode(struct GPIO_Chip * chip, int offset)
{

}

void STM32_GPIO_SetValue(struct GPIO_Chip * chip, int offset, int value)
{
    struct STM32F10X_GPIOExt *ext = (struct STM32F10X_GPIOExt *)chip->extension;
    GPIO_WriteBit(ext->gpioAddr, PIN_BITOFF(offset), value == GPIO_HIGH ? Bit_SET : Bit_RESET);
}

int STM32_GPIO_GetValue(struct GPIO_Chip * chip, int offset)
{
    struct STM32F10X_GPIOExt *ext = (struct STM32F10X_GPIOExt *)chip->extension;
    uint8_t bit = GPIO_ReadInputDataBit(ext->gpioAddr, PIN_BITOFF(offset));
    return bit == Bit_SET ? GPIO_HIGH : GPIO_LOW;
}

static int ProbeDevice(struct IO_Driver *driver, IO_DeviceNodeType *node)
{
    char *name = IO_GetNodeName(node);
    unsigned long addr = IO_GetNodeAddress(node);
    
    printf("gpio: devname %s, addr:%p\n", name, addr);

    GPIO_ChipType *chip = GPIO_CreatChip(name, sizeof(struct STM32F10X_GPIOExt));
    if (!chip) {
        return NULL;
    }
    chip->base = IO_GetNodeHex(node, "gpio-base", 0);
    chip->count =  IO_GetNodeHex(node, "gpio-count", 0);

    chip->setValue = STM32_GPIO_SetValue;
    chip->getValue = STM32_GPIO_GetValue;
    chip->setMode = STM32_GPIO_SetMode;
    chip->getMode = STM32_GPIO_GetMode;

    IO_DeviceType *dev = GPIO_AddChip(chip, driver);
    if (!dev) {
        return NULL;
    }

    struct STM32F10X_GPIOExt *ext = (struct STM32F10X_GPIOExt *)chip->extension;

    ext->gpioIrqOff = IO_GetNodeHex(node, "interrupt-offset", 0);
    ext->gpioAddr = addr; // ioremap

    printf("gpio: base %d, count:%d, irqoff: %d\n", chip->base, chip->count, ext->gpioIrqOff);

    return dev;
}

static int RemoveDevice(struct IO_Device *device)
{
    return 0;
}

static IO_CompatibleType STM32F10X_GPIOCompatible[] = {
    {"gpio-stm32f10x", NULL},
    {NULL, NULL},
};

/**
 * STM32F10X_GPIO device driver
 */
int STM32F10X_GPIO_Init(void)
{
    /**
     * 读取设备总线信息，并创建总线驱动
     */
    printf("GPIO\n");
    STM32F10X_GPIO_Driver = IO_CreateDriver("STM32F10X_GPIO");

    STM32F10X_GPIO_Driver->compatible = STM32F10X_GPIOCompatible;
    STM32F10X_GPIO_Driver->probeDevice = ProbeDevice;
    STM32F10X_GPIO_Driver->removeDevice = RemoveDevice;

    IO_AttachDriver(STM32F10X_GPIO_Driver);

    return 0;
}

static void STM32F10X_GPIO_Exit(void)
{
}

IO_MODULE(STM32F10X_GPIO, STM32F10X_GPIO_Init, STM32F10X_GPIO_Exit, IO_MODULE_DEV);
