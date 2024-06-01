#include <libdrv/gpio.h>

GPIO_ChipType *GPIO_SearchChip(int gpio)
{
    IO_DeviceType *dev;
    GPIO_ChipType *chip;
    IO_ForeachDeviceClass(dev, IO_DEVICE_CLASS_GPIO) {
        chip = (GPIO_ChipType *)dev->extension;
        if (chip->base <= gpio && gpio < chip->base + chip->count) {
            return chip;
        }
    }
    return NULL;
}

static int GPIO_ChipOpen(IO_DeviceType *device, IO_ReqType *req)
{
    /* 打开的时候初始化 */

    IO_FinishReq(req);
    return 0;
}

static int GPIO_ChipClose(IO_DeviceType *device, IO_ReqType *req)
{
    /* 关闭的时候反初始化 */

    IO_FinishReq(req);
    return 0;
}

static int GPIO_ChipWrite(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;
    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.write.buf;
    unsigned long len = stack->args.write.len;
    unsigned long off = stack->args.write.off;

    /* io read/write */

    IO_FinishReq(req);

    req->status.result = len;

    return 0;
}

static int GPIO_ChipRead(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.read.buf;
    unsigned long len = stack->args.read.len;
    unsigned long off = stack->args.read.off;

    /* io read/write */

    IO_FinishReq(req);

    req->status.result = len;

    return 0;
}

static int GPIO_ChipControl(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;
    GPIO_ChipType *chip = (GPIO_ChipType *)device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    unsigned long cmd = stack->args.control.cmd;
    void *arg = stack->args.control.arg;

    /* 根据 cmd 进行gpio配置 */
    printf("gpio control: cmd %d, arg %p\n", cmd, arg);

    switch (cmd)
    {
    case GPIO_CMD_SET_MODE:
        {        
            GPIO_SettingsType *set = (GPIO_SettingsType *)arg;
            if (chip->setMode) {
                chip->setMode(chip, set->port, set->mode);
            }
        }
        break;
    case GPIO_CMD_GET_MODE:
        /* code */
        break;
    case GPIO_CMD_SET_VAL:
        {
            GPIO_DataIOType *data = (GPIO_DataIOType *)arg;
            if (chip->setValue) {
                chip->setValue(chip, data->port, data->data);
            }
        }
        break;
    case GPIO_CMD_GET_VAL:
        {
            GPIO_DataIOType *data = (GPIO_DataIOType *)arg;
            if (chip->getValue) {
                data->data = chip->getValue(chip, data->port);
            }
        }
        break;
    default:
        break;
    }

    req->status.status = 0;
    IO_FinishReq(req);

    return 0;
}

IO_DeviceType *GPIO_AddChip(GPIO_ChipType *chip, IO_DriverType *driver)
{
    if (!chip || !chip->name) {
        return NULL;
    }

    if (chip->base < 0 || chip->count <= 0) {
        return NULL;
    }

    /**
     * 创建设备
     */
    IO_DeviceType *dev = IO_CreateDevice(chip->name, 0, IO_DEVICE_CLASS_GPIO);
    
    dev->extension = chip;

    if (IO_AttachDevice(dev, driver)) {
        IO_DestroyDevice(dev);
        return NULL;
    }

    /**
     * 设置派遣函数
     */
    driver->function[IO_DISPATCH_OPEN] = GPIO_ChipOpen;
    driver->function[IO_DISPATCH_CLOSE] = GPIO_ChipClose;
    driver->function[IO_DISPATCH_WRITE] = GPIO_ChipWrite;
    driver->function[IO_DISPATCH_READ] = GPIO_ChipRead;
    driver->function[IO_DISPATCH_CONTROL] = GPIO_ChipControl;

    return dev;
}
