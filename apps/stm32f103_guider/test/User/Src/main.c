#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_rtc.h"
#include "bsp_spi_flash.h"
#include "printf.h"

#include <devio.h>

IO_IMPORT_MODULE(Ramdisk);

extern char __HEAP_START[], __HEAP_MAX[];

void AttachAllModule(void)
{
  /**
   * 初始化堆内存
   */
  IO_InitHeap(__HEAP_START, __HEAP_MAX - __HEAP_START);
  /**
   * 挂载模块
   */
  IO_AttachModule(&IO_MODULE_NAME(Ramdisk));
}

char *GetLinkerDeviceTree(void)
{
    extern char __io_devtree_start[];
    extern char __io_devtree_end[];

    printf("json: start=%p, end=%p, size=%d\n", __io_devtree_start, __io_devtree_end,
            __io_devtree_end - __io_devtree_start);
    if (__io_devtree_end == __io_devtree_start) {
		printf("no json device tree inside !\n");
		return NULL;
	}
    printf("json devtree:\n%s\n", __io_devtree_start);
    return __io_devtree_start;
}

int test_devio(void)
{
  printf("hello, DeviceIO!\n");
 
  char *linkerDevtree = GetLinkerDeviceTree();
  /* 初始化IO系统 */
  IO_InitSystem(AttachAllModule, linkerDevtree);

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

      ret = IO_ReadDevice(dev, 0, buf, 32);
      printf("[OK] IO_ReadDevice %d\n", ret);

      ret = IO_WriteDevice(dev, 0, buf, 32);
      printf("[OK] IO_WriteDevice %d\n", ret);

      IO_CloseDevice(dev);
  }

  IO_DeviceType *dev1 = NULL, *dev2 = NULL;
  if (IO_OpenDevice("ramdisk0", 0, &dev1)) {
      printf("[BAD] IO_OpenDevice\n");
      return -1;
  } else {
      printf("[OK] IO_OpenDevice\n");
  }
  if (IO_OpenDevice("ramdisk0", 0, &dev2)) {
      printf("[BAD] IO_OpenDevice\n");
      return -1;
  } else {
      printf("[OK] IO_OpenDevice\n");
  }
  if (dev2) {
      ret = IO_CloseDevice(dev2);
      printf("IO_CloseDevice: %d\n", ret);
  }

  if (dev1) {
      ret = IO_CloseDevice(dev1);
      printf("IO_CloseDevice: %d\n", ret);

      ret = IO_CloseDevice(dev1);
      printf("IO_CloseDevice: %d\n", ret);

  }

}

/**
  * @brief  ������
  * @param  ��  
  * @retval ��
  */
int main(void)
{	
  LED_GPIO_Config();	
	LED_BLUE;
  USART_Config(); 
  
  Usart_SendString( DEBUG_USARTx,"hello, world!\r\n");

  printf("hello, world! %d %s\n", 123, "abc");

  test_devio();

  while (1)
  {

  }

}

/*********************************************END OF FILE**********************/
