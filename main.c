
#include <devio.h>

#include <stdio.h>

IO_IMPORT_MODULE(spi);
IO_IMPORT_MODULE(flash);

void AttachAllModule(void)
{
    IO_AttachModule(&IO_MODULE_NAME(spi));
    IO_AttachModule(&IO_MODULE_NAME(flash));
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
    if (IO_OpenDevice("nand0", 0, &dev)) {
        printf("[BAD] IO_OpenDevice\n");
    } else {
        printf("[OK] IO_OpenDevice\n");
    }

    char buf[32];
    int ret = IO_WriteDevice(dev, 0, buf, 32);
    printf("[OK] IO_WriteDevice %d\n", ret);

    ret = IO_ReadDevice(dev, 0, buf, 32);
    printf("[OK] IO_ReadDevice %d\n", ret);

    ret = IO_ControlDevice(dev, 1, buf);
    printf("[OK] IO_ControlDevice %d\n", ret);

    IO_CloseDevice(dev);

    return 0;
}
