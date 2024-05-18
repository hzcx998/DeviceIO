#include <devio.h>

struct SPI_Message
{
    const void *send_buf;           /* 发送缓冲区指针 */
    size_t length;                  /* 发送 / 接收 数据字节数 */
};

/**
 * 应用接口。
 */
IO_DriverType *spiBusDriver;

static int SPI_Open(IO_DeviceType *device, IO_ReqType *req)
{
    /**
     * 打开一个spi设备
     */
    IO_DumpReqStack(IO_GetCurrentReqStackLocation(req));

    IO_FinishReq(req);

    return 0;
}

static int SPI_Write(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    stack = IO_GetCurrentReqStackLocation(req);

    struct SPI_Message *msg = (struct SPI_Message *)stack->args.write.buf;
    unsigned long len = stack->args.write.len;

    printf("spi write: msg:%p, send buf:%p, buf len:%d\n", msg, msg->send_buf, msg->length);

    req->status.result = msg->length;

    IO_FinishReq(req);

    return 0;
}

int SPI_Init(void)
{
    /**
     * 读取设备总线信息，并创建总线驱动
     */
    spiBusDriver = IO_CreateDriver("spi");
    spiBusDriver->function[IO_DISPATCH_OPEN] = SPI_Open;
    spiBusDriver->function[IO_DISPATCH_WRITE] = SPI_Write;

    IO_AttachDriver(spiBusDriver);

    IO_DeviceType *spiDev = IO_CreateDevice("spi0", 0);
    IO_AttachDevice(spiDev, spiBusDriver);

    spiDev = IO_CreateDevice("spi1", 0);
    IO_AttachDevice(spiDev, spiBusDriver);

    return 0;
}

void SPI_Exit(void)
{
}

IO_MODULE(spi, SPI_Init, SPI_Exit, IO_MODULE_BUS);

IO_DriverType *flashDriver;


static int FLASH_Open(IO_DeviceType *device, IO_ReqType *req)
{
    IO_DeviceType *nextDevice;
    /* 打开flash，使能打开flash的中断，使能操作等 */

    nextDevice = device->extension;
    /**
     * 不处理open信息，转发给spi
     */
    IO_SkipReqStackLocation(req);

    IO_CallDriver(nextDevice, req);
    return 0;
}

static int FLASH_Write(IO_DeviceType *device, IO_ReqType *req)
{
    IO_DeviceType *nextDevice;
    IO_StackType *stack;
    IO_StackType *stackNext;
    
    nextDevice = device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.write.buf;
    unsigned long len = stack->args.write.len;

    printf("[flash] %s:%d, cur %d next %d\n", __func__, __LINE__, IO_GetCurrentReqLocation(req), IO_GetNextReqLocation(req));

    /**
     * 不处理write信息，转发给spi
     */
    IO_DupReqStackToNext(req);

    struct SPI_Message msg1;

    msg1.send_buf   = buf;
    msg1.length     = len;

    /* setup next stack */
    stackNext = IO_GetNextReqStackLocation(req);

    stackNext->args.write.buf = &msg1;
    stackNext->args.write.len = sizeof(msg1);
    stackNext->function = IO_DISPATCH_WRITE;

    return IO_CallDriver(nextDevice, req);
}

static int FLASH_Control(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stk;

    stk = IO_GetCurrentReqStackLocation(req);

    printf("flash control: %d, %p\n", stk->args.control.cmd, stk->args.control.arg);

    IO_FinishReq(req);

    return 0;
}

/*
spi0 {
    DriverName: imx spi
    Description: NXP imx spi driver
    DeviceName: spi
    Address: 0x10000
    Size: 0x4000
    Interrupt: SPI 33 0x01
    Status: Enable
}

nand0 {
    DriverName: W25Q64 flash
    DeviceName: nand
    BusDriverName: imx spi
    Address: 0x10000
    Reg: 0
    SpiMaxFrequency: 20000000
    Status: Enable
}

*/

/**
 * NAND FLASH device driver
 */

int FLASH_Init(void)
{
    /**
     * 读取设备总线信息，并创建总线驱动
     */

    flashDriver = IO_CreateDriver("flash drviver");

    flashDriver->function[IO_DISPATCH_OPEN] = FLASH_Open;
    flashDriver->function[IO_DISPATCH_WRITE] = FLASH_Write;
    flashDriver->function[IO_DISPATCH_CONTROL] = FLASH_Control;

    IO_AttachDriver(flashDriver);

    IO_DeviceType *nandDev = IO_CreateDevice("nand0", 0);
    IO_AttachDevice(nandDev, flashDriver);

    IO_DeviceType *spiDev;
    /* 获取设备 */
    if (IO_GetDeviceByName("spi0", &spiDev))
        printf("[BAD] device not found!\n");

    /**
     * 将nand挂载到spi上
     */
    IO_AttachDeviceToDeviceStack(spiDev, nandDev);
    nandDev->extension = spiDev;

    IO_DumpDeviceStack(spiDev);
    IO_DumpDeviceStack(nandDev);


    nandDev = IO_CreateDevice("nand1", 0);
    IO_AttachDevice(nandDev, flashDriver);

    spiDev;
    /* 获取设备 */
    IO_GetDeviceByName("spi1", &spiDev);

    IO_AttachDeviceToDeviceStack(spiDev, nandDev);
    nandDev->extension = spiDev;

    return 0;
}

void FLASH_Exit(void)
{
}

IO_MODULE(flash, FLASH_Init, FLASH_Exit, IO_MODULE_DEV);
