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

static char *devtree_buf = "{\n\
    \"model\": \"Sipeed Longan Pi 3H\",\n\
    \"compatible\": [\n\
        \"sipeed,longan-pi-3h\", \"sipeed,longan-module-3h\", \"allwinner,sun50i-h618\"\n\
    ],\n\
    \n\
    \"chosen\": \n\
    {\n\
        \"stdout-path\": \"serial0:115200n8\"\n\
    },\n\
    \"uart0@5000000\":\n\
    {\n\
        \"compatible\": \"snps,dw-apb-uart\",\n\
        \"reg\" : [\n\
            \"0x05000000\",\n\
            \"0x400\"\n\
        ],\n\
        \"status\": \"disabled\"\n\
    },\n\
    \"uart1@5000400\":\n\
    {\n\
        \"compatible\": \"snps,dw-apb-uart\",\n\
        \"reg\" : [\n\
            \"0x05000400\",\n\
            \"0x400\"\n\
        ],\n\
        \"status\": \"disabled\"\n\
    },\n\
    \"ramdisk@0\":\n\
    {\n\
        \"compatible\": \"virt,ramdisk\",\n\
        \"reg\" : [\n\
            \"0x2000\",\n\
            \"0x200\"\n\
        ],\n\
        \"status\": \"enabled\",\n\
        \"EnableFaseIO\" : true\n\
    }\n\
}";

int test_devio(void)
{
  printf("hello, DeviceIO!\n");

  /* 初始化IO系统 */
  IO_InitSystem(AttachAllModule, devtree_buf);

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
