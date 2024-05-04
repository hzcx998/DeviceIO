# DeviceIO

DeviceIO是一个驱动框架，用于描述设备信息。

## 编译

本程序使用xmake进行编译，也可以使用其它方式，毕竟只有少数文件。

```bash
xmake && xmake run
```

## API

```c
int IO_OpenDevice(char *name, int flags, IO_DeviceType **deviceOut);
int IO_WriteDevice(IO_DeviceType *device, unsigned long off, char *buf, unsigned long len);
int IO_ReadDevice(IO_DeviceType *device, unsigned long off, char *buf, unsigned long len);
int IO_ControlDevice(IO_DeviceType *device, unsigned long cmd, void *arg);
int IO_CloseDevice(IO_DeviceType *device);
```

## TODO
* 添加directIO接口支持
* 添加设备描述配置信息(基于json)
* 添加真正的驱动设备案例
