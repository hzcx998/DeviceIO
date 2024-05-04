
#include <devio.h>

#include <stdio.h>

IO_IMPORT_MODULE(spi);
IO_IMPORT_MODULE(flash);
IO_IMPORT_MODULE(Ramdisk);

void AttachAllModule(void)
{
    IO_AttachModule(&IO_MODULE_NAME(spi));
    IO_AttachModule(&IO_MODULE_NAME(flash));
    IO_AttachModule(&IO_MODULE_NAME(Ramdisk));
}

int main(int argc, char *argv[])
{
    printf("hello, DeviceIO!\n");

    /* 初始化IO系统 */
    IO_InitSystem(AttachAllModule);

    /**
     * 操作设备
     */
    IO_DeviceType *dev = NULL;
    char buf[32];
    int ret;

    if (IO_OpenDevice("ramdisk0", 0, &dev)) {
        printf("[BAD] IO_OpenDevice\n");
    } else {
        printf("[OK] IO_OpenDevice\n");
    }

    ret = IO_ReadDevice(dev, 0, buf, 32);
    printf("[OK] IO_ReadDevice %d\n", ret);

    ret = IO_WriteDevice(dev, 0, buf, 32);
    printf("[OK] IO_WriteDevice %d\n", ret);

    IO_CloseDevice(dev);

    IO_DeviceType *dev1, *dev2;
    if (IO_OpenDevice("ramdisk0", 0, &dev1)) {
        printf("[BAD] IO_OpenDevice\n");
    } else {
        printf("[OK] IO_OpenDevice\n");
    }
    if (IO_OpenDevice("ramdisk0", 0, &dev2)) {
        printf("[BAD] IO_OpenDevice\n");
    } else {
        printf("[OK] IO_OpenDevice\n");
    }
    ret = IO_CloseDevice(dev2);
    printf("IO_CloseDevice: %d\n", ret);
    
    ret = IO_CloseDevice(dev1);
    printf("IO_CloseDevice: %d\n", ret);

    ret = IO_CloseDevice(dev1);
    printf("IO_CloseDevice: %d\n", ret);

#if 0
    if (IO_OpenDevice("nand0", 0, &dev)) {
        printf("[BAD] IO_OpenDevice\n");
    } else {
        printf("[OK] IO_OpenDevice\n");
    }

    ret = IO_WriteDevice(dev, 0, buf, 32);
    printf("[OK] IO_WriteDevice %d\n", ret);

    ret = IO_ReadDevice(dev, 0, buf, 32);
    printf("[OK] IO_ReadDevice %d\n", ret);

    ret = IO_ControlDevice(dev, 1, buf);
    printf("[OK] IO_ControlDevice %d\n", ret);

    IO_CloseDevice(dev);
#endif

    return 0;
}
