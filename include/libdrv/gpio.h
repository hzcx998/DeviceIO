#ifndef _LIBDRV_GPIO_H
#define _LIBDRV_GPIO_H

#include <iokit/gpio.h>

#include <devio.h>

/**
 * Control cmd
 */
#define GPIO_CMD_SET_MODE 1
#define GPIO_CMD_GET_MODE 2
#define GPIO_CMD_SET_VAL 3
#define GPIO_CMD_GET_VAL 4

typedef struct GPIO_Settings {
    unsigned int port;
    /**
     * BIT: 0-15: moded
     * BIT: 16-31: rate
     */
    unsigned int mode;
} GPIO_SettingsType;

typedef struct GPIO_DataIO {
    unsigned int port;
    unsigned int data;
} GPIO_DataIOType;

typedef struct GPIO_Chip {
    char *name;
    
    int base;
    int count;

    void (*setMode)(struct GPIO_Chip * chip, int offset, int mode);
	int  (*getMode)(struct GPIO_Chip * chip, int offset);
    void (*setValue)(struct GPIO_Chip * chip, int offset, int value);
	int  (*getValue)(struct GPIO_Chip * chip, int offset);

    void *extension;
} GPIO_ChipType;

static inline GPIO_ChipType *GPIO_CreatChip(char *name, unsigned long extensionSize)
{
    GPIO_ChipType *chip = IO_Malloc(sizeof(GPIO_ChipType) + extensionSize);
    if (!chip) {
        return chip;
    }
    chip->name = name;
    chip->base = -1;
    chip->count = 0;
    chip->extension = NULL;

    if (extensionSize) {
        chip->extension =  (void *)((chip) + 1);
    }

    chip->setMode = NULL;
    chip->getMode = NULL;
    chip->setValue = NULL;
    chip->getValue = NULL;

    return chip;
}

IO_DeviceType *GPIO_AddChip(GPIO_ChipType *chip, IO_DriverType *driver);

GPIO_ChipType *GPIO_SearchChip(int gpio);

#endif
