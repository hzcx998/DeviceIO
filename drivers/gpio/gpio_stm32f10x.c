#include <stm32f10x_gpio.h>
#include <devio.h>

static IO_DriverType *STM32F10X_GPIO_Driver;

struct STM32F10X_GPIOExt {
    int gpioPortCount;
    int gpioVirtPort;
    int gpioBase;
    int gpioIrqOff;
    int gpioAddr;
};

static int STM32F10X_GPIO_Open(IO_DeviceType *device, IO_ReqType *req)
{
    /* 打开的时候初始化 */

    IO_FinishReq(req);
    return 0;
}

static int STM32F10X_GPIO_Close(IO_DeviceType *device, IO_ReqType *req)
{
    /* 关闭的时候反初始化 */

    IO_FinishReq(req);
    return 0;
}

static int STM32F10X_GPIO_Write(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    struct STM32F10X_GPIOExt *ext = device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.write.buf;
    unsigned long len = stack->args.write.len;
    unsigned long off = stack->args.write.off;

    /* io read/write */

    IO_FinishReq(req);

    req->status.result = len;

    return 0;
}

static int STM32F10X_GPIO_Read(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    struct STM32F10X_GPIOExt *ext = device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.read.buf;
    unsigned long len = stack->args.read.len;
    unsigned long off = stack->args.read.off;

    /* io read/write */

    IO_FinishReq(req);

    req->status.result = len;

    return 0;
}

#define GPIO_CMD_SET_MODE 1
#define GPIO_CMD_GET_MODE 2
#define GPIO_CMD_SET_VAL 3
#define GPIO_CMD_GET_VAL 4

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

static void ConvertInitStruct(GPIO_SettingsType *set, GPIO_InitTypeDef *init)
{
    GPIO_StructInit(init);

    init->GPIO_Pin = set->port;
    init->GPIO_Mode = ConvertMode(set->mode & 0xffff);
    init->GPIO_Speed = ConvertSpeed(set->mode >> 16);
}

static int STM32F10X_GPIO_Control(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;
    struct STM32F10X_GPIOExt *ext = (struct STM32F10X_GPIOExt *)device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    unsigned long cmd = stack->args.control.cmd;
    void *arg = stack->args.control.arg;

    /* 根据 cmd 进行gpio配置 */
    printf("gpio control: cmd %d, arg %p\n", cmd, arg);

    switch (cmd)
    {
    case GPIO_CMD_SET_MODE:
        {        
            GPIO_InitTypeDef init;
            GPIO_SettingsType *set = (GPIO_SettingsType *)arg;
            
            printf("gpio: set:%p pin:%d, mode:%x, speed%x\n", set, set->port, set->mode & 0xffff, set->mode >> 16);
            ConvertInitStruct(set, &init);
                
            printf("gpio: addr:%d pin:%d, mode:%x, spped:%d\n", ext->gpioAddr, init.GPIO_Pin, init.GPIO_Mode, init.GPIO_Speed);
            if (IS_GPIO_PIN_SOURCE(init.GPIO_Pin)) {
                GPIO_Init(ext->gpioAddr, &init);
            } else {
                // bad pin source
            }
        }
        break;
    case GPIO_CMD_GET_MODE:
        /* code */
        break;
    case GPIO_CMD_SET_VAL:
        {
            GPIO_DataIOType *data = (GPIO_DataIOType *)arg;
            GPIO_WriteBit(ext->gpioAddr, data->port, data->data == GPIO_LOW ? Bit_RESET : Bit_SET);
        }
        break;
    case GPIO_CMD_GET_VAL:
        {
            GPIO_DataIOType *data = (GPIO_DataIOType *)arg;
            uint8_t bit = GPIO_ReadInputDataBit(ext->gpioAddr, data->port);
            data->data = bit == Bit_SET ? GPIO_HIGH : GPIO_LOW;
        }
        break;
    default:
        break;
    }

    req->status.status = 0;
    IO_FinishReq(req);

    return 0;
}

static int ProbeDevice(struct IO_Driver *driver, IO_DeviceNodeType *node)
{
    char *name = IO_GetNodeName(node);
    unsigned long addr = IO_GetNodeAddress(node);
    printf("gpio: devname %s, addr:%p\n", name, addr);

    IO_DeviceType *ramDev = IO_CreateDevice(name, sizeof(struct STM32F10X_GPIOExt));
    IO_AttachDevice(ramDev, driver);

    struct STM32F10X_GPIOExt *ext = (struct STM32F10X_GPIOExt *)ramDev->extension;

    ext->gpioBase = IO_GetNodeHex(node, "gpio-base", 0);
    ext->gpioPortCount = IO_GetNodeHex(node, "gpio-count", 0);
    ext->gpioIrqOff = IO_GetNodeHex(node, "interrupt-offset", 0);
    ext->gpioAddr = addr;

    printf("gpio: base %d, count:%d, irqoff: %d\n", ext->gpioBase, ext->gpioPortCount, ext->gpioIrqOff);

    return 0;
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

    STM32F10X_GPIO_Driver->function[IO_DISPATCH_OPEN] = STM32F10X_GPIO_Open;
    STM32F10X_GPIO_Driver->function[IO_DISPATCH_CLOSE] = STM32F10X_GPIO_Close;
    STM32F10X_GPIO_Driver->function[IO_DISPATCH_WRITE] = STM32F10X_GPIO_Write;
    STM32F10X_GPIO_Driver->function[IO_DISPATCH_READ] = STM32F10X_GPIO_Read;
    STM32F10X_GPIO_Driver->function[IO_DISPATCH_CONTROL] = STM32F10X_GPIO_Control;

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
