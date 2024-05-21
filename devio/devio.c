#include <devio.h>
#include <cJSON.h>
#include <stdlib.h>

#include "tlsf/tlsf.h"

IO_LIST_HEAD(DriverListHead);
IO_LIST_HEAD(DeviceListHead);

static int IO_MemHeapEnabled = 0;

/**
 * 内存管理模块
 */
tlsf_t *tlsfObj = NULL;

void *IO_Malloc(size_t size)
{
    if (IO_MemHeapEnabled != 0) {
        assert(tlsfObj);
        return tlsf_malloc(tlsfObj, size);
    } else {
        return malloc(size);
    }
}

void IO_Free(void *ptr)
{
    if (IO_MemHeapEnabled != 0) {
        assert(tlsfObj);
        tlsf_free(tlsfObj, ptr);
    } else {
        free(ptr);
    }
}

void IO_InitHeap(char *addr, unsigned long len)
{
    printf("IO: init heap: addr:%p-%p, size %p\n", addr, addr + len, len);
    memset(addr, 0, len);
    tlsfObj = tlsf_create_with_pool(addr, len);
    assert(tlsfObj);
    IO_MemHeapEnabled = 1;
}

/**
 * 模块管理模块
 */
IO_ListType ModuleListHead[IO_MODULE_LEVEL_MAX];

void IO_InitModule(void)
{
    int i;
    for (i = IO_MODULE_HW_LOWER; i < IO_MODULE_LEVEL_MAX; i++) {
        IO_InitList(&ModuleListHead[i]);
    }
    IO_InitList(&DriverListHead);
    IO_InitList(&DeviceListHead);
}

/**
 * IO系统模块
 */
void IO_DumpReqStack(IO_StackType *req)
{
    printf("IO Req Stack: func: %d, device:%p\n", req->function, req->device);
}

int IO_DumpDeviceStack(IO_DeviceType *device)
{
    IO_DeviceType *nextDevice;
    nextDevice = device->attachedDevice;

    printf("device %s IO stack list: stack size:%d\n", device->name, device->stackSize);

    /**
     * 已经有挂载的设备了，找到一个没有挂载设备的设备。
     */
    while (nextDevice != NULL) {
        printf("    device stack:%s\n", nextDevice->name);
        /**
         * 如果已经没有挂载设备了，就到达了最外层。
         */
        if (nextDevice->attachedDevice == NULL) {
            break;
        }

        nextDevice = nextDevice->attachedDevice;
    }

    return 0;
}

void IO_CallModule(void)
{
    IO_ModuleType *module;
    int i;

    for (i = IO_MODULE_HW_LOWER; i < IO_MODULE_LEVEL_MAX; i++) {
        IO_ListForEachEntry(module, &ModuleListHead[i], systemList) {
            printf("call mod:%s\n", module->name);
            module->init();
        }
    }
}

int IO_InitSystem(void (*init)(void), char *devtree)
{
    int err = 0;

    IO_InitModule();

    /**
     * 初始化模块
     */
    if (init) {
        init();
    }

    /* 调用模块初始化，按照低到高的顺序初始化设备 */
    IO_CallModule();
    
    /* 解析设备树，并探测设备 */
    IO_InitDeviceTree();
    if (devtree != NULL) {
        err = IO_ParseDeviceTree(devtree);
    }
    return err;
}
