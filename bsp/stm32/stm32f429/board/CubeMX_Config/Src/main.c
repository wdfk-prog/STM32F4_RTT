/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 
  * @date           : 
  ******************************************************************************
  * @attention
  RTT Kernel��scheduler.c��rt_schedule���Ż���
����Ϊ��cpuprot.c�е�__rt_ffs�������Ż�������__attribute__((optnone))ȡ���Ż��ȿ�

  **********************NIVC���ȼ�����*****************************************
  �ж�                  ��ռ���ȼ�   �����ȼ�
  TIM14                 1             0
  USART1                2             0
  USART2                2             0
  USART3                3             0
  TIM5                  4             0
  **********************RTT����***********************************************
           pri  status      sp     stack size max used left tick  error
  -------- ---  ------- ---------- ----------  ------  ---------- ---
  tshell    20  running 0x00000258 0x00001000    20%   0x00000009 000
  mfbd       8  suspend 0x000000a0 0x00000400    68%   0x00000006 000
  mb key    11  suspend 0x00000128 0x00000400    30%   0x0000000a 000
  turnsafe   7  suspend 0x0000009c 0x00000400    15%   0x0000000a 000
  syswatch   0  suspend 0x000000a0 0x00000400    61%   0x00000013 000
  uart2      9  suspend 0x000000c8 0x00000400    24%   0x0000000a 000
  uart1      9  suspend 0x0000014c 0x00000400    49%   0x00000009 000
  ulog_asy  30  suspend 0x000000ac 0x00000400    29%   0x00000003 000
  tidle0    31  ready   0x00000084 0x00000400    12%   0x00000004 000
  main      10  suspend 0x00000134 0x00000800    35%   0x0000000c 000
********************** GPIO*************************************************
    WKUP                    PA0
    ��ص�ѹ                PA3
    Lift_Reflection         PA4 //��������䴫����
    ALM(���ߵ������)       PA6 //����Ϊ�½��ش���
    SV(���ߵ���ٶ�)        PA7 //CTL2+
    USART1_TX               PA9 //�����ߡ������ߡ����ء�
    USART1_RX               PA10//�����ߡ������ߡ����ȡ�
    SYS_JTMS-SWDIO          PA13
    SYS_JTCK-SWCLK          PA14
    Radar_switch2           PA15//Ӳ������
    BOOT1                   PB2 //
    Turn_PUL                PB3 //ת�����ٶ����
    Encoder-A               PB6 //���ߵ��������
    Encoder_B               PB7 //���ߵ��������
    CAN1_RX                 PB8
    CAN1_TX                 PB9
    STOP_BUTTON             PB10 //��ͣ��ť �ߵ�ƽ���� �͵�ƽ�ɿ�
    Walk_BK                 PB12 //����ɲ��
    PC0   ------> FMC_SDNWE
    Before_Radar_2          PC1
    After_Radar_2           PC2
    Before_Radar_3          PC4 
    After_Radar_1           PC5
    Walk_SV                 PC7 //�����ٶ����
    USART3_TX               PC10
    USART3_RX               PC11
    LED                     PC13//�������ƣ���һ���
    RCC_OSC32_IN            PC14//�ⲿ���پ���
    RCC_OSC32_OUT           PC15//�ⲿ���پ���
    PD0    ------> FMC_D2
    PD1    ------> FMC_D3
    Photoelectric_Sensor2   PD3 //ת������λ������
    USART2_TX               PD5
    USART2_RX               PD6
    Turn_ENA                PD7 //ת����ʹ��
    Jack_Relay              PD11
    LED1                    PD12
    Turn_DIR                PD13
    PD8    ------> FMC_D13
    PD9    ------> FMC_D14
    PD10   ------> FMC_D15
    LED1                    PD12//���İ��Դ�LED �������ƣ���һ���
    Turn_DIR                PD13//ת��������
    PD14   ------> FMC_D0
    PD15   ------> FMC_D1
    PE0    ------> FMC_NBL0
    PE1    ------> FMC_NBL1
    PE7    ------> FMC_D4
    PE8    ------> FMC_D5
    PE9    ------> FMC_D6
    PE10   ------> FMC_D7
    PE11   ------> FMC_D8
    PE12   ------> FMC_D9
    PE13   ------> FMC_D10
    PE14   ------> FMC_D11
    PE15   ------> FMC_D12
    PF0    ------> FMC_A0
    PF1    ------> FMC_A1
    PF2    ------> FMC_A2
    PF3    ------> FMC_A3
    PF4    ------> FMC_A4
    PF5    ------> FMC_A5
    PF6    ------> SPI5_NSS
    PF11   ------> FMC_SDNRAS
    PF12   ------> FMC_A6
    PF13   ------> FMC_A7
    PF14   ------> FMC_A8
    PF15   ------> FMC_A9
    PG0    ------> FMC_A10
    PG1    ------> FMC_A11
    Photoelectric_Sensor1   PG2 //ת�������λ
    PG4    ------> FMC_BA0
    PG5    ------> FMC_BA1
    Beep                    PG7 //������
    PG8    ------> FMC_SDCLK
    PG15   ------> FMC_SDNCAS
    Lower_Limit             PG13//�������λ������
    Start_Button            PG14//������ť
    RCC_OSC_IN              PH0 //�ⲿ���پ���
    RCC_OSC_OUT             PH1 //�ⲿ���پ���
    Walk_FR                 PH4 //���ߵ������ CTL1+
    Walk_EN                 PH5 //���ߵ��ʹ��
    Radar_switch1           PH8 
    Article_Crash           PH9 //��ײ��������Ϊ0���ɿ�Ϊ1
    Drawstring_A            PH10
    Drawstring_B            PH11
    ת��������            PH12//ת������������˸��ơ�δʹ����˸�̵ơ�
    Lift_SQP                PH14//��۽ӽ�����
    PH6   ------> FMC_SDNE1
    PH7   ------> FMC_SDCKE1
    Before_Radar_1          PI1
    Fall_Relay              PI3
    Walk_ALM                PI6 //���ߵ������
    After_Radar_3           PI10
    **********************ʹ������*****************************************
    System Core************************************************************
    DMA1:USART2_RX,USART2_TX
    DMA2:USART1_RX,USART1_TX,ADC1
    IWDG:
    RCC:�ⲿ����
    SYS
    
    ADC1:��ص�ѹ���
    
    TIM2:���PWM(ת����)           https://www.jianshu.com/p/eae5386aa8ee
    TIM3:���PWM(���ߵ��)
    TIM9:TIM2�Ӷ�ʱ����ȡ�������
    TIM14:us����ʱ��
    
    USART1:����MODBUS����
    USART2:����MODBUS����
    USART3:���������е���
  * @author
  ******************************************************************************
  */
  
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <rtconfig.h>
#include "monitor.h"
/*ulog include*/
#define LOG_TAG              "main" 
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*�����ж����ȼ�����*/
#define IRQ_PRIORITY  4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static int Version(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  ����RTCʱ��
  * @param  None
  * @retval ulogʱ���׼ȷ
  */
static int Set_RTC_Time(void)
{
#include <time.h>
  rt_err_t ret = RT_EOK;
  time_t now;

  /* �������� */
  ret = set_date(YEAR, MONTH + 1, DAY);
  if (ret != RT_EOK)
  {
    LOG_E("set RTC date failed");
  }
  /* ����ʱ�� */
  ret = set_time(HOUR, MINUTE, SEC + BURN_TIME);
  if (ret != RT_EOK)
  {
    LOG_E("set RTC time failed");
  }
  return ret;
}
INIT_COMPONENT_EXPORT(Set_RTC_Time);
#ifdef IRQ_PRIORITY
/**
  * @brief  ����FINSH�����ж����ȼ�
  * @param  None
  * @retval None
  */
static int Set_FINSH_IRQ(void)
{
  rt_err_t ret = RT_EOK;
  /* �����豸��� */
  rt_device_t serial;
  /* ���Ҵ����豸 */
  serial = rt_device_find(RT_CONSOLE_DEVICE_NAME);
  if (!serial)
  {
      LOG_E("find %s failed!", RT_CONSOLE_DEVICE_NAME);
      ret = -RT_ERROR;
  }
  struct stm32_uart *uart;

  uart = rt_container_of(serial, struct stm32_uart, serial);
  /* parameter check */
  RT_ASSERT(uart != RT_NULL);
  HAL_NVIC_SetPriority(uart->config->irq_type,IRQ_PRIORITY, 0);
  return ret;
}
INIT_COMPONENT_EXPORT(Set_FINSH_IRQ);
#endif
/**
 * This function will initial STM32 board.
  * @note   ����GPIO��ʼ��
 */
void rt_hw_board_init(void)
{
#ifdef BSP_SCB_ENABLE_I_CACHE
    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();
#endif

#ifdef BSP_SCB_ENABLE_D_CACHE
    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();
#endif

    /* HAL_Init() function is called at the beginning of the program */
    HAL_Init();

    /* System clock initialization */
    SystemClock_Config();

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif
    MX_GPIO_Init();
    /* Pin driver initialization is open by default */
#ifdef RT_USING_PIN
    rt_hw_pin_init();
#endif

    /* USART driver initialization is open by default */
#ifdef RT_USING_SERIAL
    rt_hw_usart_init();
#endif

    /* Set the shell console output device */
#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
/**
  * @brief  The application entry point.
  * @retval int
  * @note   �ж�����logsĿ¼������ulog�ļ���˳�ʼ��
  */
int main(void)
{
/* Ӳ�� BSP ��ʼ��*/
/***********************����ģʽ�½��ö������Ź�IWDG**********************************/
  __HAL_DBGMCU_FREEZE_IWDG();	  //����ģʽ�£����ῴ�Ź�������ʱ��
  __HAL_DBGMCU_FREEZE_WWDG();
  __HAL_DBGMCU_FREEZE_CAN1();
  __HAL_DBGMCU_FREEZE_CAN2();
/*********************����ģʽ��ʹ�ܶ������Ź�IWDG**********************************/
//__HAL_DBGMCU_UNFREEZE_IWDG();	  //����ģʽ�£�ʹ�ܿ��Ź�������ʱ��
  Version();
#if(OUT_FILE_ENABLE == 1 && MV_MONITOR_ENABLE == 1)
  mv_log_timer_init();
  mv_log_timer_start();
#endif
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#ifdef CUBU_ERROR
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
#endif
/**
  * @brief  ��ȡ����汾
  * @param  None
  * @retval None
  */
static int Version(void)
{
  /* ��ȡ����汾*/
  LOG_I(" \\ | /");
  LOG_W("- HLY -    Version FULL V%s",VERSION);
  LOG_E(" / | \\     build %s %s",__DATE__,__TIME__);
  return RT_EOK;
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  LOG_E("Wrong parameters value: file %s on line %d\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
