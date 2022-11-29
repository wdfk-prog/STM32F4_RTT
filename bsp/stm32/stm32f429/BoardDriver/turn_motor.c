/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : 
  * @brief          : 
  * @date           :
  ******************************************************************************
  * @attention
  ����Ӧʱ��Ϊ1sת��180�ȣ���RPM = 120��Ϊ2ת/S
  S�ͼӼ�����ɼӼ������貽��Ϊ159363
  һȦ��Ҫ������=���ӳ�����20*10000ϸ�֣�����Ҫ�����ٶ�Ƶ��Ϊ200k*2=400k
  
  �Ӽ�������Ч���ã��������Ӧʱ�䡣������Ӧʱ�䣬�Ӽ���Ч���
  ****************************�������*********************************************
  �����߳�   <-��д-> |         |
  MB�������� <-ֻ��-> |���Ʋ��� | <-��д-> us��ʱ���ж�  
  ��ԭ�߳�   <-��д-> |         | <-ֻ��-> 10ms�����ʱ�� 
                      
                      |         |
  MB�������� <-ֻ��-> |��ʾ���� | <-ֻд-> 10ms�����ʱ��
                      |         |
  ****************************�������߳�*****************************************
            ��������߳�             ���ȼ����ڿ����߳�
  �����ʼ��------------>����̼߳��------------------>ֹͣ���
                                      �жϵ��쳣
  ****************************��ԭ����*********************************************
             ��ԭ����          ִ�����              
  MB�����߳�--------------->��ԭ�߳�--------->�����ԭ�߳�
             �ָ���ԭ�߳�
  * @author
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "turn_motor.h"
/* Private includes ----------------------------------------------------------*/
#include "mb_key.h"
#include "mb_handler.h"
#include "user_math.h"
/*ulog include*/
#define LOG_TAG              "turn"
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private typedef -----------------------------------------------------------*/
struct stm32_hwtimer
{
    rt_hwtimer_t time_device;
    TIM_HandleTypeDef    tim_handle;
    IRQn_Type tim_irqn;
    char *name;
};
/* Private define ------------------------------------------------------------*/
/*�˿ڶ���*/
#define TURN      	       (turn)
 /* ��ʱ������ */
#define HWTIMER_DEV_NAME   "timer9"    
#define IRQ_PRIORITY  15

#define MASTER_TIM          htim2
#define CHANNELX            TIM_CHANNEL_2   
#define SLAVE_TIM           htim9
#define Cycle_Pulse_Num     Turn_Cycle_Pulse_Num//һ������Ҫ������
#define REDUCTION_RATIO     Turn_REDUCTION_RATIO//���ٱ�
#define CLOCKWISE           Turn_CLOCKWISE      //ת���������ƽֵ

#define ANGLE_MAX           Turn_ANGLE_MAX
#define ANGLE_MIN           Turn_ANGLE_MIN

#define SPEED_MAX           Turn_SPEED_MAX
#define SPEED_MIN           Turn_SPEED_MIN

#define BACK_HIGH_SPEED     Turn_BACK_HIGH_SPEED
#define BACK_LOW_SPEED      Turn_BACK_LOW_SPEED
#define SANGLE_INIT         Turn_SANGLE_INIT

#define BREAK_ENABLE        0//��ɲ������

#define SET_ANGLE_ADDR      1
#define GET_ANGLE_ADDR      201
#define SET_ANGLE_MAX_ADDR  504
#define SET_ANGLE_MIN_ADDR  505
/* Private macro -------------------------------------------------------------*/
/*MOTOR 7S�Ӽ��ٲ��� structures definition*/
#define TURN_STEP_PARA		  50      			 //����ʱ��ת�������������ӡ�Ӱ��ʱ����ϸ�ֶȣ�ԽСʱ��Խ����Խϸ�֡�
#define TURN_STEP_AA			  31       		   //�Ӽ��ٽ׶Σ���ɢ��������Խ��Խϸ�֣���Ӱ��ʱ�䡿
#define TURN_STEP_UA			  31			  	   //�ȼ��ٽ׶Σ���ɢ������
#define TURN_STEP_RA			  31					   //�����ٽ׶Σ���ɢ������
#define TURN_STEP_LENGTH (TURN_STEP_AA + TURN_STEP_UA + TURN_STEP_RA)//�ܲ���
  
#define M_FRE_START	        1600;           //���������Ƶ�ʡ�����Ƶ�ʵͣ���Ƶ�ʲ���Ϊ0��
#define M_FRE_AA					  10000;          //���Ƶ�ʵļӼ��ٶȡ���������ٶ����ޡ�[Ӱ���ܲ���]
#define M_T_AA					    1;              //���Ƶ�ʵļӼ���ʱ��
#define M_T_UA					    3;              //���Ƶ�ʵ��ȼ���ʱ��
#define M_T_RA					    1;              //���Ƶ�ʵļ�����ʱ��
/* Private variables ---------------------------------------------------------*/
TURN_MOTOR_TypeDef turn;
/*S�����߲������ɵı�� �������*/
static uint32_t SMove_TimeTable [2 * TURN_STEP_LENGTH + 1];
static uint32_t SMove_StepTable [2 * TURN_STEP_LENGTH + 1];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  ����Ƕȶ�λ�Ƕ�����
  * @param  None.
  * @retval None.
  * @note   ��֤�ٴν���Ƕȶ�λ�������̽��ж�λ��
*/
static void Turn_Motor_Reentrant_SetAngle(void)
{
  TURN.motor.Abs.get_angle = SMove_Get_Angle(&TURN.motor.Ctrl);
  TURN.get_radian          = TURN.motor.Abs.get_angle * PI / 180;
  usRegHoldingBuf[GET_ANGLE_ADDR]     = (_short)(TURN.get_radian*1000);
  usRegHoldingBuf[SET_ANGLE_ADDR]       = usRegHoldingBuf[GET_ANGLE_ADDR];
  TURN.set_radian          = TURN.get_radian ;
  TURN.motor.Abs.last      = TURN.motor.Abs.get_angle;
}
/************************�̼߳��жϺ���******************************************/
/**
  * @brief  ת������غ���
  * @param  ��ص��״̬
  * @retval None.
  * @note   
*/
void Turn_Motor_Detection(void)
{
  static LIMIT_MONITOR_STATE last = 0XFF;
  static stop_type last_code = NO_STOP;
  
  LIMIT_MONITOR_STATE ret = SMove_Limit_Detection(&TURN.motor,TURN.limit_mode,TURN.motor.Do.zero.state,TURN.motor.Do.limit.state);
  if(ret != last)
  {
    switch(ret)
    {
      case MONITOR_ABNORMAL:
        USER_SET_BIT(TURN.Stop_state,Detection_STOP);
        LOG_W("Abnormal motor monitoring.");
        break;
      case MONITOR_ALL:
        TURN_LIMIT_ALARM_SET;
        TURN_ZERO_ALARM_SET;
        USER_SET_BIT(TURN.Stop_state,Detection_STOP);
        LOG_W("Abnormal motor sensor,All touch.");
        break;
      case MONITOR_INIT:
        break;
      case MONITOR_LIMIT:
        TURN_LIMIT_ALARM_SET;
        USER_SET_BIT(TURN.Stop_state,Detection_STOP);
        LOG_W("The limit sensor is abnormally touched.");
        break;
      case MONITOR_NORMAL:
        USER_CLEAR_BIT(TURN.Stop_state,Detection_STOP);
        break;
      case MONITOR_ZERO:
        TURN_ZERO_ALARM_SET;
        USER_SET_BIT(TURN.Stop_state,Detection_STOP);
        LOG_W("The zero sensor is abnormally touched.");
        break;
    }
    last = ret;
  }
  if(TURN.Stop_state != last_code)
  {
    if(TURN.Stop_state != NO_STOP)
     LOG_W("Motor stop, stop code is 0X%04X",TURN.Stop_state);
    else
     LOG_I("Motor return to normal,code is 0X%04X",TURN.Stop_state);
    last_code = TURN.Stop_state;
  }
}
/**
  * @brief  ת������ԭ����
  * @param  ��Ҫ���ת������������
  * @retval None.
  * @note   ��ɺ��˳�ѭ������ɾ���߳�
*/
void Turn_SMove_DoReset(void * p)
{
  while(1)
  {
    rt_thread_mdelay(1);
    switch(TURN.motor.Do.init_state)
    {
      case 1:
        //�������߰���
        WALK_BREAK_SET;
        //����ת�򰴼�
        TURN_LOCATE_RESET;
        TURN_UP_RESET;
        TURN_DOWN_RESET;
        TURN_ZERO_SET;
        MBKey_Shield_Operate(KEY_TURN_UP,DISABLE);    //������ɨ�谴��
        MBKey_Shield_Operate(KEY_TURN_DOWN,DISABLE);  //������ɨ�谴��
        MBKey_Shield_Operate(KEY_TURN_LOCATE,DISABLE);//������ɨ�谴��
        //�޵���λ
        TURN.limit_mode = 1;
        TURN.jog_flag   = 4;
        //��ԭ����
        ZERO_state ret = SMove_DoReset(&TURN.motor,TURN.motor.Do.speed_high,TURN.motor.Do.speed_low,TURN.motor.Do.zero.state,TURN.motor.Do.limit.state);
        if(ret == ZERO_OK)
        {
          TURN.motor.Ctrl.CurrentPosition_Pulse = TURN.motor.Ctrl.MaxPosition_Pulse * (1 - TURN.motor.Do.init_freq * 180 / PI / 360);//��λ��ǰΪ��Ҫ��λ�Ƕ�;
          SMove_SetAngle_Cache(&TURN.motor.Abs);
          TURN.motor.Do.init_state = 2;//����ڶ�����ʼ��
          LOG_I("Coming back to the positive");
        }
        else if(ret == ZERO_ABNORMAL)
        {
          //�ָ����߰���
          WALK_BREAK_RESET;
          //�ָ�ת�򰴼�
          TURN_ZERO_RESET;
          MBKey_Shield_Operate(KEY_TURN_UP,ENABLE);    //����ɨ�谴��
          MBKey_Shield_Operate(KEY_TURN_DOWN,ENABLE);  //����ɨ�谴��
          MBKey_Shield_Operate(KEY_TURN_LOCATE,ENABLE);//����ɨ�谴��
          MBKey_Shield_Operate(KEY_TURN_ZERO,ENABLE);  //����ɨ�谴��
          TURN.motor.Do.init_state = 0XFF;
          LOG_I("The motor is abnormal, and the thread is quit");
          return;
        }
        break;
      case 2:
        //������λ
        TURN.limit_mode = 4;
        TURN.jog_flag   = 4;
        //�ص���Ƕ�λ��
        TURN.motor.Abs.set_angle = 0;
        if(SMove_SetAngle_Absolute(&TURN.motor,TURN.motor.Abs.set_angle) == DISABLE)
        {
          usRegHoldingBuf[SET_ANGLE_MAX_ADDR] = (uint16_t)ANGLE_MAX;
          usRegHoldingBuf[SET_ANGLE_MIN_ADDR] = (uint16_t)ANGLE_MIN;
          SMove_SetAngle_Cache(&TURN.motor.Abs);
          //�ָ����߰���
          WALK_BREAK_RESET;
          //�ָ�ת�򰴼�
          TURN_ZERO_RESET;
          TURN_LOCATE_SET;
          MBKey_Shield_Operate(KEY_TURN_UP,ENABLE);    //����ɨ�谴��
          MBKey_Shield_Operate(KEY_TURN_DOWN,ENABLE);  //����ɨ�谴��
          MBKey_Shield_Operate(KEY_TURN_LOCATE,ENABLE);//����ɨ�谴��
          MBKey_Shield_Operate(KEY_TURN_ZERO,ENABLE);  //����ɨ�谴��
          TURN.motor.Do.init_state = 3;//�����ʼ������
          LOG_I("Back to the original finish");
        }
        break;
      case 3: //��ԭ��ʼ������
        LOG_I("Return to the origin and the initialization is complete");
        return;
        break;
      default:
        break;
    }
  }
}
/**
  * @brief  ת������ʱ��ѯ����
  * @param  None.
  * @retval None.
  * @note   None.
            ���붨ʱ��10ms�ж�
*/
static void Turn_Motor_Get_Value_10MS(void *parameter)
{
  if(TURN.motor.Ctrl.running == 1)
    TURN.set_freq  = TURN.motor.Ctrl.Counter_Table[TURN.motor.Ctrl.CurrentIndex];
  else
    TURN.set_freq = 0;
  
  TURN.get_freq  = SMove_Get_Speed(&TURN.motor.Ctrl)  * 100;
  TURN.motor.V.get_speed = 60 * TURN.get_freq / (int32_t)(TURN.motor.Ctrl.cycle_pulse_num * TURN.motor.Ctrl.reduction_ratio);
  TURN.motor.Abs.get_angle = SMove_Get_Angle(&TURN.motor.Ctrl);
  TURN.get_radian    = TURN.motor.Abs.get_angle * PI / 180;
}
/**
  * @brief  ��ʱ����ʱ�ص�����
  * @param  None
  * @retval None
  * @note   None
*/
static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
    TURN.motor.Ctrl.Slave_Pulse_IT++;
    return 0;
}
/**
  * @brief  ��ӡS�ͼӼ����б�
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void SMove_list(void)
{
  for(uint8_t i = 0;i < TURN_STEP_LENGTH+1;i++)
  {
    if(i % 3 == 0)
    {
      rt_kprintf("\n");
    }
    rt_kprintf("num%3d|Hz:%7d|Step:%4d#",i,SMove_TimeTable[i],SMove_StepTable[i]);
  }
  rt_kprintf("\r\n");
  rt_kprintf("The total step:%d\r\n",TURN.motor.Ctrl.StartSteps+TURN.motor.Ctrl.StopSteps);
}
//MSH_CMD_EXPORT_ALIAS(SMove_list,turn_SMove_list,TURN motor hz / step list);
/************************�û��ҹ���д����******************************************/
/**
  * @brief  �����ͣ���ȼ�.
  * @param  None.
  * @retval None.
  * @note   
��ͣ���»��ߵ��δʹ��
*/
static uint8_t Motor_Stop_Priority(void)
{
  if(TURN.Stop_state != NO_STOP)
  {
    Turn_Motor_Stop();
    Turn_Motor_Reentrant_SetAngle();
    TURN.motor.Do.init_state = 0xFF;//��������״̬����ֹ��ͣ����������ʼ��״̬��
    return 1;
  }
  else
    return 0;
}
/**
  * @brief  ʹ�ܵ��
  * @param  None.
  * @retval None.
  * @note   �͵�ƽʹ��
*/
static void Motor_Enable(void)
{
  USER_CLEAR_BIT(TURN.Stop_state,ENABLE_STOP);
  TURN.PIN.EN.level = GPIO_PIN_RESET;
  HAL_GPIO_WritePin(TURN.PIN.EN.GPIOx,TURN.PIN.EN.GPIO_Pin,TURN.PIN.EN.level);    //ʹ�ܵ��
}
/**
  * @brief  ���õ��
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void Motor_Disenable(void)
{
  USER_SET_BIT(TURN.Stop_state,ENABLE_STOP);
  TURN.PIN.EN.level = GPIO_PIN_SET;
  HAL_GPIO_WritePin(TURN.PIN.EN.GPIOx,TURN.PIN.EN.GPIO_Pin,TURN.PIN.EN.level);    //���õ��
}
/**
  * @brief  �����ʼ����
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void Motor_Start(void)
{
  TURN.state.PWM   = Motor_PWM_Start(TURN.motor.Ctrl.Master_TIMx,TURN.motor.Ctrl.Channel);//��������ʱ��
  TURN.state.MOTOR = Motor_BUSY;                                     //����������
#if(BREAK_ENABLE == 1)
  TURN.PIN.Brake.level = GPIO_PIN_RESET;
  HAL_GPIO_WritePin(TURN.PIN.Brake.GPIOx,TURN.PIN.Brake.GPIO_Pin,TURN.PIN.Brake.level);    //�Ϳ�ɲ��
#endif
}
/**
  * @brief  ���ֹͣ����
  * @param  None.
  * @retval None.
  * @note   None.
*/
void Turn_Motor_Stop(void)
{ 
  TURN.state.PWM   = Motor_PWM_Stop(TURN.motor.Ctrl.Master_TIMx,TURN.motor.Ctrl.Channel);//�ر�����ʱ��
  TURN.state.MOTOR = Motor_STOP;                                                //���ֹͣ���
  Turn_Motor_Reentrant_SetAngle();
#if(BREAK_ENABLE == 1)
  TURN.PIN.Brake.level = GPIO_PIN_SET;
  HAL_GPIO_WritePin(TURN.PIN.Brake.GPIOx,TURN.PIN.Brake.GPIO_Pin,TURN.PIN.Brake.level);    //ɲ��
#endif
}
/**
  * @brief  ���õ�������ٶ�
  * @param  dir:���뷽��
  * @retval None.
  * @note   None.
*/
static void Motor_Direction(Directionstate dir)
{
  assert_param(TURN.PIN.Dir.GPIOx);
  assert_param(TURN.PIN.Dir.GPIO_Pin);
  if(dir == CW)
  {
    TURN.PIN.Dir.level = TURN.motor.Ctrl.clockwise;
  }
  else if(dir == CCW)
  {
    TURN.PIN.Dir.level = GPIO_TURN(TURN.motor.Ctrl.clockwise);
  }
  HAL_GPIO_WritePin(TURN.PIN.Dir.GPIOx,TURN.PIN.Dir.GPIO_Pin,TURN.PIN.Dir.level);
}
/************************�û���ʼ������******************************************/
/**
  * @brief  PWM��ʼ������.
  * @param  None.
  * @retval None.
  * @note   None
*/
static int PWM_Init(void)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* ��ʱ����ʱֵ */
    rt_device_t hw_dev = RT_NULL;   /* ��ʱ���豸��� */
    rt_hwtimer_mode_t mode;         /* ��ʱ��ģʽ */

    /* ���Ҷ�ʱ���豸 */
    hw_dev = rt_device_find(HWTIMER_DEV_NAME);
    if (hw_dev == RT_NULL)
    {
        LOG_E("hwtimer sample run failed! can't find %s device!", HWTIMER_DEV_NAME);
        return RT_ERROR;
    }

    /* �Զ�д��ʽ���豸 */
    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("open %s device failedn", HWTIMER_DEV_NAME);
        return ret;
    }

    /* ���ó�ʱ�ص����� */
    rt_device_set_rx_indicate(hw_dev, timeout_cb);

    /* ����ģʽΪ�����Զ�ʱ������δ���ã�Ĭ����HWTIMER_MODE_ONESHOT��*/
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        LOG_E("set mode failed! ret is :%d", ret);
        return ret;
    }
    
    MX_TIM2_Init();//�ȳ�ʼ������ʱ��
    MX_TIM9_Init();//�ٳ�ʼ���Ӷ�ʱ�� ���ɴ���˳��
    HAL_TIM_Base_Start_IT(&htim9);  /* ����TIM9*/
    
#ifdef IRQ_PRIORITY
    struct stm32_hwtimer *tim_device = RT_NULL;
    tim_device = rt_container_of(hw_dev, struct stm32_hwtimer, time_device);
    
    HAL_NVIC_SetPriority(tim_device->tim_irqn,IRQ_PRIORITY, 0);
#endif
    return ret;
}
/**
  * @brief  RTT�����ʱ����ʼ��
  * @param  None
  * @retval None
  * @note   None
*/
static void RTT_Timer_Init(void)
{
  rt_err_t ret = RT_EOK;
  /* ��ʱ���Ŀ��ƿ� */
  static rt_timer_t timer;
  /* ������ʱ�� 1  ���ڶ�ʱ�� */
  timer = rt_timer_create("Turn10ms", Turn_Motor_Get_Value_10MS,
                             RT_NULL, rt_tick_from_millisecond(10),
                             RT_TIMER_FLAG_PERIODIC);

  /* ������ʱ�� 1 */
  if (timer != RT_NULL) rt_timer_start(timer);
}
/**
  * @brief  ת������ʼ��
  * @param  None
  * @retval None
  * @note   None
*/
void Turn_Motor_Init(void)
{ 
  PWM_Init();
  RTT_Timer_Init();
  
  TURN.PIN.EN.GPIOx             = Turn_ENA_GPIO_Port;
  TURN.PIN.EN.GPIO_Pin          = Turn_ENA_Pin;
  
  TURN.PIN.Dir.GPIOx            = Turn_DIR_GPIO_Port;
  TURN.PIN.Dir.GPIO_Pin         = Turn_DIR_Pin;
#if(BREAK_ENABLE == 1)
  TURN.motor.PIN.Brake.GPIOx       = Turn_Brake_GPIO_Port;
  TURN.motor.PIN.Brake.GPIO_Pin    = Turn_Brake_Pin;
#endif
  TURN.ALM.IO.GPIOx             = Turn_ALM_GPIO_Port; 
  TURN.ALM.IO.GPIO_Pin          = Turn_ALM_Pin;
  
  HAL_GPIO_EXTI_Callback(TURN.ALM.IO.GPIO_Pin);//��ʼ����ȡһ�ε�ƽ
  
  TURN.motor.Ctrl.Motor_Enable        = Motor_Enable;
  TURN.motor.Ctrl.Motor_Disenable     = Motor_Disenable;
  TURN.motor.Ctrl.Motor_Start         = Motor_Start;
  TURN.motor.Ctrl.Motor_Stop          = Turn_Motor_Stop;
  TURN.motor.Ctrl.Motor_Direction     = Motor_Direction;
  TURN.motor.Ctrl.Modify_Freq         = Modify_TIM_Freq;
  TURN.motor.Ctrl.Motor_init          = Motor_PWM_init;
  TURN.motor.Ctrl.Motor_Stop_Priority = Motor_Stop_Priority;
  
  TURN.motor.CFG.fstart               = M_FRE_START;
  TURN.motor.CFG.faa                  = M_FRE_AA;
  TURN.motor.CFG.taa                  = M_T_AA;
  TURN.motor.CFG.tua                  = M_T_UA;
  TURN.motor.CFG.tra                  = M_T_RA;
  
  TURN.motor.CFG.STEP_PARA					  = TURN_STEP_PARA;
  TURN.motor.CFG.STEP_AA					    = TURN_STEP_AA;
  TURN.motor.CFG.STEP_UA					    = TURN_STEP_UA;
  TURN.motor.CFG.STEP_RA					    = TURN_STEP_RA;

  TURN.motor.CFG.MotorTimeTable				= SMove_TimeTable;
  TURN.motor.CFG.MotorStepTable				= SMove_StepTable;
  
  TURN.motor.Do.zero.IO.GPIOx         = Turn_Zero_GPIO_Port;
  TURN.motor.Do.zero.IO.GPIO_Pin      = Turn_Zero_Pin;
  TURN.motor.Do.limit.IO.GPIOx        = Turn_Limit_GPIO_Port;
  TURN.motor.Do.limit.IO.GPIO_Pin     = Turn_Limit_Pin;
  
  TURN.motor.Do.speed_high            = BACK_HIGH_SPEED;
  TURN.motor.Do.speed_low             = BACK_LOW_SPEED;
  TURN.motor.Do.init_freq             = SANGLE_INIT;
  TURN.motor.Do.init_state            = 0xFF;
  
  TURN.limit_mode = 4;
  
  HAL_GPIO_EXTI_Callback(TURN.motor.Do.zero.IO.GPIO_Pin);//��ʼ����ȡһ�ε�ƽ
  HAL_GPIO_EXTI_Callback(TURN.motor.Do.limit.IO.GPIO_Pin);//��ʼ����ȡһ�ε�ƽ
  
  SMove_Initial(&TURN.motor.Ctrl,             //������ƽṹ��
                &TURN.motor.CFG,              //����Ӽ��ٲ����ṹ��
                Cycle_Pulse_Num,              //һ������Ҫ������
                REDUCTION_RATIO,              //���ٱ�
                &MASTER_TIM,                  //ʹ�ö�ʱ��
                CHANNELX,                     //ʹ��ͨ��
                CLOCKWISE,                    //˳ʱ���ƽ
                TURN.motor.CFG.MotorTimeTable,//�Ӽ���Ƶ���б�
                TURN.motor.CFG.MotorStepTable,//�Ӽ��ٲ����б�
                &SLAVE_TIM
                );
                
  SMove_SetSpeed_Range(&TURN.motor,SPEED_MAX,SPEED_MIN);    //���ýǶȷ�Χ
  SMove_SetAngle_Range(&TURN.motor.Abs,ANGLE_MAX,ANGLE_MIN);//���ýǶȷ�Χ
}