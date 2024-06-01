#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_rtc.h"
#include "bsp_spi_flash.h"
#include "printf.h"

#include <devio.h>

#include <iokit/gpio.h>

IO_IMPORT_MODULE(Ramdisk);
IO_IMPORT_MODULE(STM32F10X_GPIO);

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
  IO_AttachModule(&IO_MODULE_NAME(STM32F10X_GPIO));
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

#define GPIO_CMD_SET_MODE 1
#define GPIO_CMD_GET_MODE 2
#define GPIO_CMD_SET_VAL 3
#define GPIO_CMD_GET_VAL 4

#define GPIO_LOW                 0x00
#define GPIO_HIGH                0x01

#define GPIO_MODE_OUTPUT         0x00
#define GPIO_MODE_INPUT          0x01
#define GPIO_MODE_INPUT_PULLUP   0x02
#define GPIO_MODE_INPUT_PULLDOWN 0x03
#define GPIO_MODE_OUTPUT_OD      0x04
#define GPIO_MODE_OUTPUT_PP      0x05

#define GPIO_RATE_SLOW           0x00
#define GPIO_RATE_MEDIUM         0x01
#define GPIO_RATE_FAST           0x02

typedef struct GPIO_Settings {
    unsigned int port;
    /**
     * BIT: 0-15: moded
     * BIT: 16-31: rate
     */
    unsigned int mode;
} GPIO_SettingsType;

typedef struct GPIO_DataIO {
    unsigned int port;
    unsigned int data;
} GPIO_DataIOType;

int test_devio(void)
{
  printf("hello, DeviceIO!\n");
 
  char *linkerDevtree = GetLinkerDeviceTree();
  /* 初始化IO系统 */
  IO_InitSystem(AttachAllModule, linkerDevtree);

  IO_ListDrivers();
  IO_ListDevices();

  SysTick_Init();
  /**
   * 操作设备
   */
  IO_DeviceType *dev = NULL;
  char buf[32];
  int ret;

  printf("[INFO] TEST DEV\n");

  /* enable clock */
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

#if 0
  /* open gpio device */
  if (IO_OpenDevice("gpio-b", 0, &dev)) {
      printf("[BAD] open gpio-b\n");
  } else {
      printf("[OK] open gpio-b\n");

      GPIO_SettingsType set = {.port = 5, .mode = GPIO_MODE_OUTPUT_PP | (GPIO_RATE_FAST << 16)}; // pgiob5
      ret = IO_ControlDevice(dev, GPIO_CMD_SET_MODE, &set);
      printf("[OK] GPIO_SettingsType %d\n", ret);

      while(1) {
        /* default high to disable light */
        GPIO_DataIOType dataio = {.port = 5, .data = GPIO_HIGH};
        ret = IO_ControlDevice(dev, GPIO_CMD_SET_VAL, &dataio);
        printf("[OK] GPIO_DataIOType %d\n", ret);

        SysTick_Delay_Ms(1000);

        /* set to low to enable light */
        dataio.data = GPIO_LOW;
        ret = IO_ControlDevice(dev, GPIO_CMD_SET_VAL, &dataio);
        printf("[OK] GPIO_DataIOType %d\n", ret);
        SysTick_Delay_Ms(1000);
      }
      
      IO_CloseDevice(dev);
  }
#else

#define RED_LED_GPIO (16 + 5)
#define GREEN_LED_GPIO (16 + 0)
#define BLUE_LED_GPIO (16 + 1)

    GPIO_SetMode(RED_LED_GPIO, GPIO_MODE_OUTPUT_PP | (GPIO_RATE_SLOW << 16));
    GPIO_SetMode(GREEN_LED_GPIO, GPIO_MODE_OUTPUT_PP | (GPIO_RATE_FAST << 16));
    GPIO_SetMode(BLUE_LED_GPIO, GPIO_MODE_OUTPUT_PP | (GPIO_RATE_FAST << 16));

    while(1) {
        SysTick_Delay_Ms(1000);

        GPIO_SetValue(RED_LED_GPIO, GPIO_LOW);
        GPIO_SetValue(GREEN_LED_GPIO, GPIO_HIGH);
        GPIO_SetValue(BLUE_LED_GPIO, GPIO_HIGH);

        SysTick_Delay_Ms(1000);

        GPIO_SetValue(RED_LED_GPIO, GPIO_HIGH);
        GPIO_SetValue(GREEN_LED_GPIO, GPIO_LOW);
        GPIO_SetValue(BLUE_LED_GPIO, GPIO_HIGH);

        SysTick_Delay_Ms(1000);

        GPIO_SetValue(RED_LED_GPIO, GPIO_HIGH);
        GPIO_SetValue(GREEN_LED_GPIO, GPIO_HIGH);
        GPIO_SetValue(BLUE_LED_GPIO, GPIO_LOW);
    }
      
#endif

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
  // LED_GPIO_Config();	
	// LED_BLUE;
  USART_Config(); 
  
  Usart_SendString( DEBUG_USARTx,"hello, world!\r\n");

  printf("hello, world! %d %s\n", 123, "abc");

  test_devio();

  while (1)
  {

  }

}

/*********************************************END OF FILE**********************/
