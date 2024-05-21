#include <devio.h>

#define RAMDISK_SIZE (1 * 1024 * 1024)

IO_DriverType *RamdiskDriver;

struct RamdiskExt {
    unsigned char *ram;
    unsigned long ramSize;
    unsigned long blockSize;
};

static int Ramdisk_Open(IO_DeviceType *device, IO_ReqType *req)
{
    printf("ramdisk open\n");
    /* 打开Ramdisk，使能打开Ramdisk的中断，使能操作等 */
    IO_FinishReq(req);
    return 0;
}

static int Ramdisk_Close(IO_DeviceType *device, IO_ReqType *req)
{
    printf("ramdisk close\n");
    /* 打开Ramdisk，使能打开Ramdisk的中断，使能操作等 */
    IO_FinishReq(req);
    return 0;
}

static int Ramdisk_Write(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    struct RamdiskExt *ext = device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.write.buf;
    unsigned long len = stack->args.write.len;
    unsigned long off = stack->args.write.off;

    if (off + len > ext->ramSize) {
        IO_FinishReq(req);
        return -1;
    }

    memcpy(&ext->ram[off], buf, len);

    IO_FinishReq(req);

    req->status.result = len;

    return 0;
}

static int Ramdisk_Read(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    struct RamdiskExt *ext = device->extension;

    stack = IO_GetCurrentReqStackLocation(req);

    char *buf = stack->args.read.buf;
    unsigned long len = stack->args.read.len;
    unsigned long off = stack->args.read.off;

    if (off + len > ext->ramSize) {
        IO_FinishReq(req);
        return -1;
    }

    memcpy(buf, &ext->ram[off], len);

    IO_FinishReq(req);

    req->status.result = len;

    return 0;
}

static int Ramdisk_Control(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;

    stack = IO_GetCurrentReqStackLocation(req);

    unsigned long cmd = stack->args.control.cmd;
    void *arg = stack->args.control.arg;

    printf("ramdisk control: cmd %d, arg %p\n", cmd, arg);

    IO_FinishReq(req);

    return 0;
}

static int ProbeDevice(struct IO_Driver *driver, IO_DeviceNodeType *node)
{
    printf(">>>>>>>>\nramdisk probe: %s, id: %d\n", IO_GetNodeName(node), IO_GetNodeID(node));
    printf("get bool: %d\n", IO_GetNodeBool(node, "EnableFaseIO", 0));
    printf("get string: %s\n", IO_GetNodeString(node, "status", "enabled"));
    printf("get reg: %d\n", IO_GetNodeArrayLength(node, "reg"));

    printf("get reg[0]: %s\n", IO_GetNodeArraString(node, "reg", 0, "0x00"));
    printf("get reg[1]: %s\n", IO_GetNodeArraString(node, "reg", 1, "0x00"));

    printf("get reg[0]: %p\n", IO_GetNodeArraHex(node, "reg", 0, 0x00));
    printf("get reg[1]: %p\n", IO_GetNodeArraHex(node, "reg", 1, 0x00));

    printf("get reg[0]: %p\n", IO_GetNodeArrayU8(node, "reg2", 0, 0x00));
    printf("get reg[1]: %p\n", IO_GetNodeArrayU8(node, "reg2", 1, 0x00));

    printf("get reg[0]: %p\n", IO_GetNodeArrayU16(node, "reg2", 0, 0x00));
    printf("get reg[1]: %p\n", IO_GetNodeArrayU16(node, "reg2", 1, 0x00));

    printf("get reg[0]: %p\n", IO_GetNodeArrayU32(node, "reg2", 0, 0x00));
    printf("get reg[1]: %p\n", IO_GetNodeArrayU32(node, "reg2", 1, 0x00));

    printf("get reg[0]: %p\n", IO_GetNodeArrayU64(node, "reg2", 0, 0x00));
    printf("get reg[1]: %p\n", IO_GetNodeArrayU64(node, "reg2", 1, 0x00));

    IO_DeviceType *ramDev = IO_CreateDevice("ramdisk0", sizeof(struct RamdiskExt));
    IO_AttachDevice(ramDev, driver);

    struct RamdiskExt *ext = (struct RamdiskExt *)ramDev->extension;

    unsigned long diskSize = IO_GetNodeArraHex(node, "reg", 0, RAMDISK_SIZE);
    printf("ramdisk: size=%p\n", diskSize);

    ext->ram = IO_Malloc(diskSize);
    if (ext->ram) {
        memset(ext->ram, 0, diskSize);
        ext->ramSize = diskSize;
        ext->blockSize = 512;
    }

    printf("create ramdisk0 device at %p size %d\n", ext->ram, ext->ramSize);

    return 0;
}

static int RemoveDevice(struct IO_Device *device)
{
    return 0;
}

static IO_CompatibleType RamdiskCompatible[] = {
    {"virt,ramdisk", NULL},
    {NULL, NULL},
};

/**
 * Ramdisk device driver
 */
int Ramdisk_Init(void)
{
    /**
     * 读取设备总线信息，并创建总线驱动
     */
    printf("ramdisk\n");
    RamdiskDriver = IO_CreateDriver("Ramdisk drviver");

    RamdiskDriver->function[IO_DISPATCH_OPEN] = Ramdisk_Open;
    RamdiskDriver->function[IO_DISPATCH_CLOSE] = Ramdisk_Close;
    RamdiskDriver->function[IO_DISPATCH_WRITE] = Ramdisk_Write;
    RamdiskDriver->function[IO_DISPATCH_READ] = Ramdisk_Read;
    RamdiskDriver->function[IO_DISPATCH_CONTROL] = Ramdisk_Control;

    RamdiskDriver->compatible = RamdiskCompatible;
    RamdiskDriver->probeDevice = ProbeDevice;
    RamdiskDriver->removeDevice = RemoveDevice;

    IO_AttachDriver(RamdiskDriver);

    return 0;
}

static void Ramdisk_Exit(void)
{
}

IO_MODULE(Ramdisk, Ramdisk_Init, Ramdisk_Exit, IO_MODULE_DEV);
