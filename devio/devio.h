#ifndef _DEVICE_IO_H
#define _DEVICE_IO_H

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <devtree.h>

#ifdef STM32F10X_HD
// stm32 only
#include <printf.h>
#endif

#define IO_API

struct IO_Driver;
struct IO_Device;
struct IO_Req;

typedef struct IO_List {
    struct IO_List *prev;
    struct IO_List *next;
} IO_ListType;

#define IO_LIST_HEAD_INIT(name) { &(name), &(name) }

#define IO_LIST_HEAD(name) \
        IO_ListType name = IO_LIST_HEAD_INIT(name)

static inline void IO_InitList(IO_ListType *list)
{
    list->prev = list;
    list->next = list;
}

static inline void IO_AttachListNode(IO_ListType *list, IO_ListType *prev, IO_ListType *next)
{
    next->prev = list;
    list->next = next;
    list->prev = prev;
    prev->next = list;
}

static inline void IO_AttachList(IO_ListType *list, IO_ListType *head)
{
    IO_AttachListNode(list, head, head->next);
}

static inline void IO_AttachListTail(IO_ListType *list, IO_ListType *head)
{
    IO_AttachListNode(list, head->prev, head);
}

static inline void IO_DeattachList(IO_ListType *node)
{
    IO_ListType *prev, *next;

    prev = node->prev;
    next = node->next;
    /* 将节点前后连接起来，从而使得当前节点被删除 */
    next->prev = prev;
    prev->next = next;

    node->prev = NULL;
    node->next = NULL;
}

static inline int IO_ListEmpty(const IO_ListType *head)
{
    return (head->next == head);
}

#define IO_OFFSET_OF_STRUCT(type, member) ((unsigned long) &((type *)0)->member)

#define IO_PTR_OF_STRUCT(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - IO_OFFSET_OF_STRUCT(type,member) ); \
})

#define IO_ListEntry(ptr, type, member) IO_PTR_OF_STRUCT(ptr, type, member)

#define IO_ListFirstEntry(head, type, member) IO_ListEntry((head)->next, type, member)

#define IO_ListNextEntry(pos, member) \
       IO_ListEntry((pos)->member.next, typeof(*(pos)), member)

#define IO_ListForEachEntry(pos, head, member)                     \
        for (pos = IO_ListFirstEntry(head, typeof(*pos), member);  \
            &pos->member != (head);                             \
            pos = IO_ListNextEntry(pos, member))

extern void *IO_Malloc(size_t size);
extern void IO_Free(void *ptr);

/**
 * 设备的配置信息，从配置文件读取
 */
typedef struct IO_Config {
    char *name;
} IO_ConfigType;

#define IO_DEVICE_NAME_LEN 32

typedef enum IO_DeviceClase {
    IO_DEVICE_CLASS_UNKNOWN = 0,
    IO_DEVICE_CLASS_GPIO,
    IO_DEVICE_CLASS_BLOCK,
    IO_DEVICE_CLASS_MAX,
} IO_DeviceClaseType;

typedef struct IO_Device {
    char name[IO_DEVICE_NAME_LEN];
    int reference;
    IO_ListType driverList;
    IO_ListType systemList;
    IO_DeviceClaseType class;
    struct IO_Driver *driver;
    /**
     * 挂载的设备，用于多层请求转发
     */
    struct IO_Device *attachedDevice;
    /**
     * 设备栈大小。
     */
    char stackSize;
    void *extension;
    /**
     * 正在处理的Req
     */
    struct IO_Req *currentReq;
} IO_DeviceType;

#define IO_DISPATCH_OPEN     0
#define IO_DISPATCH_CLOSE    1
#define IO_DISPATCH_READ     2
#define IO_DISPATCH_WRITE    3
#define IO_DISPATCH_CONTROL  4
#define IO_DISPATCH_MAX_NR   (IO_DISPATCH_CONTROL + 1)

typedef struct IO_Stack {
    /**
     * 请求函数
     */
    int function;

    /**
     * 请求函数参数
     */
    union {
        struct {
            int flags;
        } open;
        struct {
            int dummy;
        } close;
        struct {
            unsigned long off;
            void *buf;
            size_t len;
        } read;
        struct {
            unsigned long off;
            void *buf;
            size_t len;
        } write;
        struct {
            unsigned long cmd;
            void *arg;
        } control;
    } args;
    IO_DeviceType *device;
} IO_StackType;

typedef struct IO_ReqStatus {
    int status;
    unsigned long result;
} IO_ReqStatusType;

#define IO_REQ_NEW          0X01
#define IO_REQ_TRANSING     0X02
#define IO_REQ_FINISHED     0X04

typedef struct IO_Req {
    int flags;
    int size; /* req size */

    /**
     * 当前请求栈位置
     */
    int currentLocation;
    int stackCount;
    IO_StackType *stackBase;
    IO_ReqStatusType status;
} IO_ReqType;

#define IO_StepReqStackLocation(req) {(req)->currentLocation--;}
#define IO_SkipReqStackLocation(req) {(req)->currentLocation++;}

#define IO_GetCurrentReqLocation(req) (req)->currentLocation
#define IO_GetCurrentReqStackLocation(req) ((req)->stackBase + (IO_GetCurrentReqLocation(req)))

#define IO_GetNextReqLocation(req) ((req)->currentLocation - 1)
#define IO_GetNextReqStackLocation(req) ((req)->stackBase + (IO_GetNextReqLocation(req)))

#define IO_DupReqStackToNext(Req) { \
    IO_StackType *stack; \
    IO_StackType *stackNext; \
    stack = IO_GetCurrentReqStackLocation(req); \
    stackNext = IO_GetNextReqStackLocation(req); \
    memcpy(stackNext, stack, sizeof(IO_StackType)); }

#define IO_ReqSize(StackSize) \
    ((unsigned long) (sizeof(IO_ReqType) + ((StackSize) * (sizeof(IO_StackType)))))

#define IO_INIT_REQ(req, reqSize, stackSize) { \
    memset(req, 0, reqSize); \
    (req)->size = reqSize; \
    (req)->stackCount = stackSize; \
    (req)->currentLocation = stackSize + 1; \
    (req)->stackBase = (IO_StackType *)((unsigned char *)(req) + sizeof(IO_ReqType)); }

static inline IO_ReqType *IO_CreateReq(char stackSize)
{
    IO_ReqType *req;
    unsigned long reqSize;
    /* 至少分配1个栈大小 */
    reqSize = IO_ReqSize(stackSize + 1);
    req = IO_Malloc(reqSize);
    if (req) {
        IO_INIT_REQ(req, reqSize, stackSize);
    }
    return req;
}

static inline void IO_DestroyReq(IO_ReqType *req)
{
    if (!req) {
        return;
    }
    IO_Free(req);
}

static inline void IO_FinishReq(IO_ReqType *req)
{
    req->flags |= IO_REQ_FINISHED;
}

extern void IO_DumpReqStack(IO_StackType *req);

/**
 * 通用派发函数类型
 */
typedef int (*IO_DispatchType)(IO_DeviceType *device, IO_ReqType *req);

/**
 * 直接处理函数类型
 */
typedef int (*IO_DirectReadType)(IO_DeviceType *device, unsigned long off, void *buf, size_t len);
typedef int (*IO_DirectWriteType)(IO_DeviceType *device, unsigned long off, void *buf, size_t len);
typedef int (*IO_DirectControlType)(IO_DeviceType *device, unsigned long cmd, void *arg);

/**
 * 直接派发处理函数
 */
typedef struct IO_DirectDispatch {
    IO_DirectReadType directRead;
    IO_DirectWriteType directWrite;
    IO_DirectControlType directControl;
} IO_DirectDispatchType;

typedef struct IO_Driver {
    char name[IO_DEVICE_NAME_LEN];
    IO_ListType deviceHead;
    IO_ListType systemList;
    IO_DirectDispatchType directIO;
    IO_CompatibleType *compatible;
    IO_DispatchType function[IO_DISPATCH_MAX_NR];
    void *extension;
    IO_DeviceType *(*probeDevice)(struct IO_Driver *driver, IO_DeviceNodeType *node);
    int (*removeDevice)(struct IO_Device *device);
} IO_DriverType;

extern IO_ListType DriverListHead;
extern IO_ListType DeviceListHead;

static inline void IO_InitDriver(IO_DriverType *driver, char *name)
{
    strcpy(driver->name, name);
    IO_InitList(&driver->deviceHead);
    IO_InitList(&driver->systemList);
    driver->extension = NULL;
    driver->compatible = NULL;
}

static inline IO_DriverType *IO_CreateDriver(char *name)
{
    IO_DriverType *driver;
    driver = IO_Malloc(sizeof(IO_DriverType));
    if (!driver) {
        return driver;
    }
    IO_InitDriver(driver, name);
    return driver;
}

static inline int IO_AttachDriver(IO_DriverType *driver)
{
    IO_AttachList(&driver->systemList, &DriverListHead);
    return 0;
}

static inline void IO_InitDevice(IO_DeviceType *device, char *name, unsigned long extensionSize, IO_DeviceClaseType class)
{
    strcpy(device->name, name);
    IO_InitList(&device->driverList);
    IO_InitList(&device->systemList);

    device->extension = NULL;
    if (extensionSize) {
        /**
         * 扩展内存在设备结构体后面
         */
        device->extension = (void *)(device + 1);
    }

    /**
     * 默认没有设备栈
     */
    device->stackSize = 0;
    /**
     * 默认没有挂载设备
     */
    device->attachedDevice = NULL;

    device->reference = 0;
    device->class = class;
}

static inline IO_DeviceType *IO_CreateDevice(char *name, unsigned long extensionSize, IO_DeviceClaseType class)
{
    IO_DeviceType *device;
    device = IO_Malloc(sizeof(IO_DeviceType) + extensionSize);
    if (!device) {
        return device;
    }
    IO_InitDevice(device, name, extensionSize, class);
    return device;
}

static inline int IO_DestroyDevice(IO_DeviceType *device)
{
    if (!device) {
        return -1;
    }
    IO_Free(device);
    return 0;
}

static inline int IO_AttachDevice(IO_DeviceType *device, IO_DriverType *driver)
{
    /* 指向驱动，并挂载到驱动上 */
    device->driver = driver;
    IO_AttachList(&device->driverList, &driver->deviceHead);
    /* 挂载到驱动的同时，也挂载的系统中，这里需要做检查，是否设备名重名 */
    IO_AttachList(&device->systemList, &DeviceListHead);
    return 0;
}

/**
 * 将target设备挂载到source设备上，使得sourceDevice可以通过attachedDevice找到挂载的设备
 */
static inline int IO_AttachDeviceToDeviceStack(IO_DeviceType *sourceDevice, IO_DeviceType *targetDevice)
{
    IO_DeviceType *oldDevice;
    oldDevice = sourceDevice->attachedDevice;
    /**
     * 已经有挂载的设备了，找到一个没有挂载设备的设备。
     */
    while (oldDevice != NULL) {
        /**
         * 如果已经没有挂载设备了，就到达了最外层。
         */
        if (oldDevice->attachedDevice == NULL) {
            break;
        }
        oldDevice = oldDevice->attachedDevice;
    }

    if (oldDevice == NULL) {
        /**
         * 没有旧设备，直接挂载到上面。
         */
        oldDevice = sourceDevice;
    }
    /**
     * 更高层的设备
     */
    oldDevice->attachedDevice = targetDevice;
    /**
     * 目标设备的IO栈增大
     */
    targetDevice->stackSize++;
    return 0;
}

extern int IO_DumpDeviceStack(IO_DeviceType *device);

static inline int IO_GetDriverByName(char *name, IO_DriverType **driverAddress)
{
    IO_DriverType *driver;
    int err = -1;
    IO_ListForEachEntry(driver, &DriverListHead, systemList) {
        if (!strcmp(driver->name, name)) {
            err = 0;
            *driverAddress = driver;
            break;
        }
    }
    return err;
}

static inline int IO_GetDriverByCompitable(char *name, IO_DriverType **driverAddress)
{
    IO_DriverType *driver;
    int err = -1;
    IO_CompatibleType *compatible;

    IO_ListForEachEntry(driver, &DriverListHead, systemList) {
        if (driver->compatible != NULL) {
            compatible = driver->compatible;
            while (compatible->name != NULL) {
                /* 匹配比较表 */
                if (!strcmp(compatible->name, name)) {
                    *driverAddress = driver;
                    return 0;
                }
                compatible++;
            }
        }
    }
    return err;
}

static inline int IO_GetDeviceByName(char *name, IO_DeviceType **deviceAddress)
{
    IO_DeviceType *device;
    int err = -1;

    IO_ListForEachEntry(device, &DeviceListHead, systemList) {
        if (!strcmp(device->name, name)) {
            err = 0;
            *deviceAddress = device;
            break;
        }
    }
    return err;
}

#define IO_ForeachDevice(dev) \
    IO_ListForEachEntry(dev, &DeviceListHead, systemList)

#define IO_ForeachDeviceClass(dev, devClass) \
    IO_ForeachDevice(dev) \
        if ((dev)->class == (devClass))

static inline int IO_CallDriver(IO_DeviceType *device, IO_ReqType *req)
{
    IO_StackType *stack;
    IO_DriverType *driver;
    /* 走向下一个 */
    IO_StepReqStackLocation(req);

    assert(IO_GetCurrentReqLocation(req) >= 0);

    stack = IO_GetCurrentReqStackLocation(req);
    stack->device = device;
    driver = device->driver;

    return driver->function[stack->function](device, req);
}

IO_API static inline int IO_OpenDevice(char *name, int flags, IO_DeviceType **deviceOut)
{
    IO_ReqType *req;
    IO_StackType *stack;
    IO_DeviceType *device;
    IO_DriverType *driver;
    int ret;

    ret = IO_GetDeviceByName(name, &device);
    if (ret) {
        return ret;
    }

    driver = device->driver;
    /**
     * 无需调用设备的打开函数
     */
    if (!driver->function[IO_DISPATCH_OPEN]) {
        device->reference++;
        *deviceOut = device;
        return 0;
    }

    /**
     * 第一次打开时才真正调用
     */
    if (device->reference == 0) {

        req = IO_CreateReq(device->stackSize);
        if (!req)
            return -1;

        /**
         * 设置栈信息
         */
        stack = IO_GetNextReqStackLocation(req);
        stack->function = IO_DISPATCH_OPEN;
        stack->args.open.flags = flags;

        ret = IO_CallDriver(device, req);

        /**
         * 没有完成传输，返回错误
         */
        if ((req->flags & IO_REQ_FINISHED) != IO_REQ_FINISHED) {
            ret = -1;
        }

        IO_DestroyReq(req);

        if (ret >= 0) {
            device->reference++;
            *deviceOut = device;
        }
    } else {
        device->reference++;
        *deviceOut = device;
    }

    return ret;
}

IO_API static inline int IO_WriteDevice(IO_DeviceType *device, unsigned long off, char *buf, unsigned long len)
{
    IO_ReqType *req;
    IO_StackType *stack;
    IO_ReqStatusType status;
    IO_DriverType *driver;
    int ret;

    if (!device) {
        return -1;
    }
    
    driver = device->driver;
    /**
     * 无需调用设备的功能
     */
    if (!driver->function[IO_DISPATCH_WRITE]) {
        return -1;
    }

    req = IO_CreateReq(device->stackSize);
    if (!req)
        return -1;

    /**
     * 设置栈信息
     */
    stack = IO_GetNextReqStackLocation(req);
    stack->function = IO_DISPATCH_WRITE;
    stack->args.write.off = off;
    stack->args.write.buf = buf;
    stack->args.write.len = len;

    ret = IO_CallDriver(device, req);

    status = req->status;
    /**
     * 没有完成传输，返回错误
     */
    if ((req->flags & IO_REQ_FINISHED) != IO_REQ_FINISHED) {
        ret = -1;
    }

    IO_DestroyReq(req);

    if (ret < 0) {
        return ret;
    } else {
        return status.result;
    }
}

IO_API static inline int IO_ReadDevice(IO_DeviceType *device, unsigned long off, char *buf, unsigned long len)
{
    IO_ReqType *req;
    IO_StackType *stack;
    IO_ReqStatusType status;
    IO_DriverType *driver;
    int ret;

    if (!device) {
        return -1;
    }
    
    driver = device->driver;
    /**
     * 无需调用设备的功能
     */
    if (!driver->function[IO_DISPATCH_READ]) {
        return -1;
    }

    req = IO_CreateReq(device->stackSize);
    if (!req)
        return -1;

    /**
     * 设置栈信息
     */
    stack = IO_GetNextReqStackLocation(req);
    stack->function = IO_DISPATCH_READ;
    stack->args.read.off = off;
    stack->args.read.buf = buf;
    stack->args.read.len = len;

    ret = IO_CallDriver(device, req);

    status = req->status;
    /**
     * 没有完成传输，返回错误
     */
    if ((req->flags & IO_REQ_FINISHED) != IO_REQ_FINISHED) {
        ret = -1;
    }

    IO_DestroyReq(req);

    if (ret < 0) {
        return ret;
    } else {
        return status.result;
    }
}

IO_API static inline int IO_ControlDevice(IO_DeviceType *device, unsigned long cmd, void *arg)
{
    IO_ReqType *req;
    IO_StackType *stack;
    IO_ReqStatusType status;
    IO_DriverType *driver;
    int ret;

    if (!device) {
        return -1;
    }
    
    driver = device->driver;
    /**
     * 无需调用设备的功能
     */
    if (!driver->function[IO_DISPATCH_CONTROL]) {
        return -1;
    }

    req = IO_CreateReq(device->stackSize);
    if (!req)
        return -1;

    /**
     * 设置栈信息
     */
    stack = IO_GetNextReqStackLocation(req);
    stack->function = IO_DISPATCH_CONTROL;
    stack->args.control.cmd = cmd;
    stack->args.control.arg = arg;
    
    ret = IO_CallDriver(device, req);

    status = req->status;
    /**
     * 没有完成传输，返回错误
     */
    if ((req->flags & IO_REQ_FINISHED) != IO_REQ_FINISHED) {
        ret = -1;
    }

    IO_DestroyReq(req);

    if (ret < 0) {
        return ret;
    } else {
        return status.result;
    }
}

IO_API static inline int IO_CloseDevice(IO_DeviceType *device)
{
    IO_ReqType *req;
    IO_StackType *stack;
    IO_ReqStatusType status;
    IO_DriverType *driver;
    int ret;

    if (!device) {
        return -1;
    }

    driver = device->driver;

    /**
     * 设备没有被打开，不能关闭
     */
    if (device->reference == 0) {
        return -1;
    }

    /**
     * 无需调用设备的功能
     */
    if (!driver->function[IO_DISPATCH_CLOSE]) {
        device->reference--;
        return 0;
    }

    /**
     * 最后一次关闭时才调用关闭
     */
    if (device->reference == 1) {
        req = IO_CreateReq(device->stackSize);
        if (!req)
            return -1;

        /**
         * 设置栈信息
         */
        stack = IO_GetNextReqStackLocation(req);
        stack->function = IO_DISPATCH_CLOSE;

        ret = IO_CallDriver(device, req);

        status = req->status;
        /**
         * 没有完成传输，返回错误
         */
        if ((req->flags & IO_REQ_FINISHED) != IO_REQ_FINISHED) {
            ret = -1;
        }

        IO_DestroyReq(req);

        device->reference--;
    } else {
        device->reference--;
    }

    return ret;
}

void IO_ListDrivers(void);
void IO_ListDevices(void);

#define IO_MODULE_HW_LOWER  0
#define IO_MODULE_HW        1
#define IO_MODULE_HW_UPER   2
#define IO_MODULE_BUS_LOWER 3
#define IO_MODULE_BUS       4
#define IO_MODULE_BUS_UPER  5
/**
 * 设备API接口能操作的也只是DEV层的接口
 */
#define IO_MODULE_DEV_LOWER 6
#define IO_MODULE_DEV       7
#define IO_MODULE_DEV_UPER  8
#define IO_MODULE_LEVEL_MAX (IO_MODULE_DEV_UPER + 1)

typedef struct IO_Module {
    char *name;
    IO_ListType systemList;
    int (*init)(void);
    void (*exit)(void);
    unsigned int level;
} IO_ModuleType;

#define IO_MODULE(_name, _init, _exit, _level) \
    IO_ModuleType ioModule##_name = {.name = #_name, .init = _init, .exit = _exit, .level = _level}

#define IO_MODULE_NAME(_name) ioModule##_name

#define IO_IMPORT_MODULE(_name) extern IO_ModuleType IO_MODULE_NAME(_name)

extern IO_ListType ModuleListHead[];

void IO_InitModule(void);

static inline int IO_AttachModule(IO_ModuleType *module)
{
    IO_AttachList(&module->systemList, &ModuleListHead[module->level]);
    return 0;
}

extern void IO_CallModule(void);

int IO_InitSystem(void (*init)(void), char *devtree);
void IO_InitHeap(char *addr, unsigned long len);

#endif
