#include <devio.h>

#include <stdio.h>

/* 必须手动初始化 */
IO_ListType ModuleListHead[IO_MODULE_LEVEL_MAX];

IO_LIST_HEAD(DriverListHead);
IO_LIST_HEAD(DeviceListHead);

void IO_InitModule(void)
{
    int i;
    for (i = IO_MODULE_HW_LOWER; i < IO_MODULE_LEVEL_MAX; i++) {
        IO_InitList(&ModuleListHead[i]);
    }    
}

void IO_InitSystem(void (*initModule)(void))
{
    IO_InitModule();
    
    if (initModule) {
        initModule();
    }

    /* 调用模块初始化，按照低到高的顺序初始化设备 */
    IO_CallModule();
}
