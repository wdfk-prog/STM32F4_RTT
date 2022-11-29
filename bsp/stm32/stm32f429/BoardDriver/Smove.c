/**
  ******************************************************************************
  * @file    Smove.c
  * @brief   �������V3.0
  * @date    2022.08.12
  ******************************************************************************
  * @attention  �ع�����������������
                ��д7S�Ӽ���
                ��ɽǶȶ�λ
                ����ٶȿ���
                ��ɻ�ԭ����
                �����λ���
                
  ����������Ŀ��
  1���ٶȿ���
  2��λ�ÿ���

  �ٶȿ��Ʒ�ʽ.PWM����������߷���ռ�ձ�
  ���Ҳ���ò��7S�Ӽ��ٷ�ʽ�������ٶȱ仯���ȴ���Ҫռ�ϴ��ڴ档��Ҫһ��������
  �������μӼ��ٷ�ʽ������Ҫ�ϴ��ڴ�,����򵥣�����Ҫ�����жϼ�¼����������ơ�
  ��������Ҫ����λ�ã��Ż��7s�Ӽ��ٵ��ٶȿ��ơ�����������μӼ���Ч�����á�
  
  �������巽ʽΪPWM�������ȶ�������Դռ����
  stm32λ�ÿ�����Ҫ��÷��͵���������������4���ֶ�
  1��ÿ����һ�����壬��һ���жϼ���
  2�����ݷ��͵�Ƶ�ʡ����͵�ʱ�䣬����������������ڱ��ٵ����壬�����ۼƻ��ֵķ��������������
  3��һ����ʱ����Ϊ���������壬����һ����ʱ����Ϊ�ӣ��Է��͵��������
  4��ʹ��DMA��ʽ�����繲����1000�����壬��ô����u16 per[1001],ÿ����һ�����壬dma��������и�����һ��ռ�ձ��֣��������һ����Ϊ0����ʾͣ������

  ����4�ַ�������;���ص�
  1�����ڵ���������ȽϺã�����˵���ٷ��������ѡ������10Khz���µģ������ж�ռ��̫���cpu�����ַ���Ҫע�⽫�ж����ȼ���ߣ�����ᶪ������
  2��������ʱ�ļ�ʱ��ȷ�ߣ��������������������ʧ�����
  3�����ӷ�ʽ�������Ķ�ʱ��������������tim1������ tim2���������ķ�ʽ�����۸��ٵ��ټ��ɣ�ͬʱռ��cpu��ͣ�ֻ��Ҫռ�ö�һ����ʱ��
  4��DMA��ʽҲ����һ����ȷ���ķ�ʽ�����ᶪʧ���壬���Ǹ��ٵ�ʱ�򣬻�϶��ռ���ڲ�����ͬʱ��ʹ��һ�������DMA�������������и�ȱ�㣬����ʹ�������Ƚϸ��ӣ�û�дﵽKISSԭ��

  * @author HLY
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "Smove.h"
/* Private includes ----------------------------------------------------------*/
#include <stdlib.h>
#include "user_math.h"

/*ulog include*/
#define LOG_TAG              "Smove"
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define GPIO_TURN(x) (((x) == (GPIO_PIN_RESET))?(GPIO_PIN_SET):(GPIO_PIN_RESET))//��ƽ��ת
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  S�ͼӼ��ٵ��ֹͣ.
  * @param  None.
  * @retval None.
  * @note   ��Ҫֹͣ��ʹ���������
*/
void SMove_Stop(SMove_Control_TypeDef* p)
{
  p->Motor_Stop();
  p->running         = DISABLE;
  p->en              = DISABLE;
}
/**
  * @brief  ���S�������㷨����������.
  * @param  None.
  * @retval None.
  * @note   ����us����ʱ��������.
*/
void SMove_Handler(SMove_Control_TypeDef* p)
{ 
  assert_param(p);
	if(p->en == ENABLE)
	{ 	
    p->Slave_Pulse_Now = __HAL_TIM_GET_COUNTER(p->Slave_TIMx) + 65535 * p->Slave_Pulse_IT;
    int err = p->Slave_Pulse_Now - p->Slave_Pulse_Last;
    if(err != 0)
    {    
      //λ�ü���
      if(p->dir == CW)
      {
        p->CurrentPosition_Pulse+=err;
        if(p->CurrentPosition_Pulse >= p->MaxPosition_Pulse)
        {
          p->CurrentPosition_Pulse=0;
        }
      }
      else //��ʱ��Ϊ���Ƕ�ֵ
      {
        p->CurrentPosition_Pulse-=err;
        if(p->CurrentPosition_Pulse == 0xffffffff)
        {
          p->CurrentPosition_Pulse = p->MaxPosition_Pulse - 1;
        }
      }
      p->Slave_Pulse_Last = p->Slave_Pulse_Now;
      
      p->CurrentPosition = p->CurrentPosition_Pulse / p->reduction_ratio;
      
      //�ٶȿ���ʹ�� �� ����ǰλ�õ���Ŀ��λ�� ���� Ŀ�� + ��ǰ ���� �ܲ�����
      if(p->speedenbale && (p->CurrentIndex == p->TargetIndex || p->TargetIndex + p->CurrentIndex == p->StartTableLength + p->StopTableLength - 1))
      {
        return;
      }
      p->PulsesHaven++; //�����в���
      p->pulsecount++;  //�Ը�Ƶ������������������
      //��תԤ����������ֹͣ��running=0�����Խ�����һ����ת
      if(p->PulsesHaven >= p->PulsesGiven)
      {
        SMove_Stop(p);
        p->en = DISABLE;
        p->running = DISABLE;
        p->CurrentIndex = 0;
      }
      uint32_t stepstop = 0;
      //�ԳƷ�ת
      //		if(p->RevetDot==p->PulsesHaven)
      //		{
      //			p->pulsecount=p->Step_Table[p->CurrentIndex];
      //		}
      if(p->pulsecount >= p->Step_Table[p->CurrentIndex])//��Ҫ������һƵ��
      { 
        //�����ٶȿ��ƣ��˴������ж�p->Ctrl.PulsesHaven>=(p->Ctrl.PulsesGiven>>1)
        //if(p->Ctrl.PulsesGiven-p->Ctrl.PulsesHaven<=p->Ctrl.StopSteps&&p->Ctrl.PulsesHaven>=(p->Ctrl.PulsesGiven>>1))
        if((p->PulsesGiven - p->PulsesHaven <= p->StopSteps && p->speedenbale == ENABLE)||//�ٶ�ģʽ �ж� ʣ�ಽ����������ٽ׶�
           (p->PulsesGiven - p->PulsesHaven <= p->StopSteps && p->PulsesHaven >= (p->PulsesGiven >> 1) && p->speedenbale == DISABLE)) //��ǰ���������ܲ���һ��
        {
          //���ٽ׶�
          if(p->CurrentIndex < p->StartTableLength)
          {
            p->CurrentIndex = p->StartTableLength + p->StopTableLength - p->CurrentIndex;
          }
          p->CurrentIndex++;
          p->pulsecount = 0;
          p->state = DECEL;
          if(p->CurrentIndex >= p->StartTableLength + p->StopTableLength)//����Խ��,��ɲ��˼Ӽ��ٹ��̡����¼�����Ҫ����
          {
//            p->CurrentIndex = p->StartTableLength + p->StopTableLength - 1;
            uint8_t i = 0;
            uint32_t stepstop = 0;
            while(stepstop < p->PulsesGiven - p->PulsesHaven)//ʣ�ಽ��
            {
              i++;
              stepstop += p->Step_Table[i];
            }
            p->CurrentIndex = i;
          }
        }
        else if(p->PulsesHaven <= p->StartSteps)//���ٽ׶�
        {
          //�𲽽׶�
          if(p->CurrentIndex < p->StartTableLength)
          {
            p->CurrentIndex++;
            p->pulsecount = 0;
            if(p->CurrentIndex >= p->StartTableLength)//����ٶ�ģʽ�£����õĵ�ǰλ�ô��ڿ�ʼ���������ڵ�ǰ����
              p->CurrentIndex = p->StartTableLength;
          }
          p->state = ACCEL;
        }
        else
        {
          p->state = CONST;
        }
        p->set_freq = p->Counter_Table[p->CurrentIndex];
        
        if(p->Step_Table[p->CurrentIndex] != 0)
          p->Modify_Freq(p->Master_TIMx,p->Channel,p->set_freq - 1);
      }
    }
	}
}
/**
  * @brief  GetFreAtTime:����S�����߲�����ȡĳ��ʱ�̵�Ƶ��
  * @param  fstart:��ʼƵ��
  * @param  faa   :���μ��ٶ�jerk
  * @param  taa   :�Ӽ��ٶ�ʱ��
  * @param  tua   :�ȼ��ٶ�ʱ��
  * @param  tra   :�����ٶ�ʱ��
  * @param  t     :��ǰʱ��
  * @retval ����Ƶ��
  * @note   https://blog.csdn.net/xueluowutong/article/details/89069363
            https://www.eet-china.com/mp/a45617.html
  tk(k = 0,1,...7) :��ʾ�����׶εĹ��ɵ�ʱ��
  ��k(k = 0,1,...7):�ֲ�ʱ�����꣬��ʾ�Ը����׶ε���ʼ����Ϊʱ������ʱ���ʾ
  ��k = t - t(k-1);(k = 1,...7)
  Tk(k = 1,...7):�����׶εĳ�������ʱ��
  T1 = taa,T2 = tua,T3 = tra
*/
static float GetFreAtTime(float fstart,float faa,float taa,float tua,float tra,float t)
{
    float V1,V2,V3,A_MAX;
    //a_max = J * T1;T1 = t1 - 0;��t = t1 ʱ
      A_MAX = faa * taa;
    //V1 =      Vs + 1/2  ( J  * T1  * T1 )����t = t1ʱ
      V1 = (fstart + 0.5 * faa * taa * taa);
    //V2 = V1 + (A_MAX * T2),��t = t2ʱ
      V2 = V1 + A_MAX * tua;

		//���ݹ�ʽ����ӿ�ʼ������ٹ����У�tʱ�̵�ת��Ƶ��
	  if(t >= 0 && t <= taa)
    {
      /*�Ӽ��ٽ׶� 
             Vs:��ʼƵ�ʣ�   J:���Ӽ��ٶ�
        V  = Vs     + 1/2 * (J * (��1)^2  ),0 �� t �� t1
           = Vs     + 1/2 * (J * (t - 0)^2),0 �� t �� t1*/
			return fstart + 0.5 * faa * t * t;
		}
    else if(taa < t && t <= (taa + tua))
    {
      /*�ȼ��ٽ׶� 
      V = V1 + a_max * ��2,taa �� t �� tua
        =    V1 + a_max * (t - taa),taa �� t �� tua*/
			return V1 + A_MAX * (t - taa);
		}
    else if((taa+tua) < t && t <= (taa+tua+tra))
    {
      /*�����ٽ׶�
      V = V2 + a_max * ��3 - 1/2(J*(��3)^2),tua �� t �� tra
        = V2 + faa * taa * (t - (taa+tua)) - 1/2*(faa*((t - (taa+tua))^2))*/
			//Դ���� �㷨
      //return V2 + 0.5*faa*taa*tra - 0.5*faa*taa*(taa+tua+tra-t)*(taa+tua+tra-t)/(tra);
      return V2 + A_MAX * (t - (taa+tua)) - 0.5 * faa * (t - (taa+tua)) * (t - (taa+tua));
    }		
		return 0;
}
/**
  * @brief  ����S�������㷨��ÿһ����ʱ�����ڼ�ת����
  * @param  None.
  * @retval None.
  * @note   None.
*/
static void CalcMotorPeriStep_CPF(SMove_7s_TypeDef *p)
{
  int  i;
	float fi;
  
  p->STEP_LENGTH = p->STEP_AA + p->STEP_UA + p->STEP_RA;
  
	for(i = 0; i < p->STEP_AA; i++)
	{
		fi=GetFreAtTime(p->fstart,p->faa,p->taa,p->tua,p->tra,p->taa/p->STEP_AA*i);
		p->MotorTimeTable[i]=fi;
		p->MotorStepTable[i]=fi*(p->taa/p->STEP_AA)/p->STEP_PARA;
	}
	for(i=p->STEP_AA;i<p->STEP_AA+p->STEP_UA;i++)
	{
		fi=GetFreAtTime(p->fstart,p->faa,p->taa,p->tua,p->tra,p->taa+(p->tua/p->STEP_UA)*(i-p->STEP_AA));
		p->MotorTimeTable[i]=fi;
		p->MotorStepTable[i]=fi*(p->tua/p->STEP_UA)/p->STEP_PARA;
	}
	for(i=p->STEP_AA+p->STEP_UA;i<p->STEP_AA+p->STEP_UA+p->STEP_RA;i++)
	{
		fi=GetFreAtTime(p->fstart,p->faa,p->taa,p->tua,p->tra,p->taa+p->tua+p->tra/p->STEP_RA*(i-p->STEP_AA-p->STEP_UA));
		p->MotorTimeTable[i]=fi;
		p->MotorStepTable[i]=fi*(p->tra/p->STEP_RA)/p->STEP_PARA;
	}
	fi=GetFreAtTime(p->fstart,p->faa,p->taa,p->tua,p->tra,p->taa+p->tua+p->tra);//�ɴﵽ����ٶ�

	p->MotorTimeTable[p->STEP_AA+p->STEP_UA+p->STEP_RA]=fi;
	p->MotorStepTable[p->STEP_AA+p->STEP_UA+p->STEP_RA]=fi*(p->tra/p->STEP_RA)/p->STEP_PARA;
	
	
	for(i=p->STEP_AA+p->STEP_UA+p->STEP_RA+1;i<2*p->STEP_LENGTH+1;i++)
	{ 
		p->MotorTimeTable[i]=p->MotorTimeTable[2*p->STEP_LENGTH-i];
		p->MotorStepTable[i]=p->MotorStepTable[2*p->STEP_LENGTH-i];
	}
}
/**
  * @brief  MotorRunParaInitial:������в�����ʼ��
  * @param  p:7S�Ӽ��ٽṹ��
  * @retval None.
  * @note   None.
*/
static void SMove_RunParaInitial(SMove_7s_TypeDef* p)
{ 
  CalcMotorPeriStep_CPF(p); 	
}
/**
  * @brief  ���³�ʼ���������ʱ��ز���
  * @param  None.
  * @retval None.
  * @note   None.
*/
void SMove_Reinitial(SMove_Control_TypeDef *pmotor)
{
  assert_param(pmotor);
	pmotor->pulsecount    = 0;
	pmotor->CurrentIndex  = 0;
  pmotor->StartSteps    = 0;                              //�������㣬�������ۼӣ�������ǰһ�εļ���
  pmotor->StopSteps     = 0;                              //ͬ��
	pmotor->speedenbale = DISABLE;

	for(int i = 0; i < pmotor->StartTableLength; i++)
	 pmotor->StartSteps += pmotor->Step_Table[i];
	for(int i = 0;i < pmotor->StopTableLength; i++)
	 pmotor->StopSteps += pmotor->Step_Table[i + pmotor->StartTableLength];
}
/**
  * @brief  Initial_Motor  :��ʼ�����
  * @param  pmotor         :������ƽṹ��
  * @param  HChannel       :ʹ��ͨ��
  * @param  Cycle_pulse_num:����������
  * @param  Reduction_ratio:���ٱ�
  * @param  Master_timx    :ʹ������ʱ��
  * @param  Slave_timx     :ʹ�ôӶ�ʱ��
  * @param  HChannel       :ʹ��ͨ��
  * @param  HChannel       :˳ʱ���ƽ
  * @retval None.
  * @note   None.
*/
void SMove_Initial(SMove_Control_TypeDef *pmotor,SMove_7s_TypeDef *p,uint32_t Cycle_pulse_num,uint16_t Reduction_ratio,TIM_HandleTypeDef* Master_timx,uint32_t HChannel,GPIO_PinState Clockwise
  ,uint32_t *SMove_TimeTable,uint32_t *SMove_StepTable,TIM_HandleTypeDef* Slave_timx)
{
  /* Check the TIM handle allocation */
  assert_param(Master_timx);
  assert_param(Slave_timx);
  assert_param(pmotor);
  assert_param(p);
  
  SMove_RunParaInitial(p);  //��ʼ���Ӽ��ٱ��
  /*���ò���*/
  pmotor->Master_TIMx           = Master_timx;
  pmotor->Channel               = HChannel;
  pmotor->Slave_TIMx            = Slave_timx;
  pmotor->clockwise             = Clockwise;//����˳ʱ���Ӧֵ
  pmotor->reduction_ratio       = Reduction_ratio;
  pmotor->cycle_pulse_num       = Cycle_pulse_num;
  pmotor->MaxPosition           = Cycle_pulse_num;
  pmotor->MaxPosition_Pulse     = pmotor->MaxPosition * pmotor->reduction_ratio;
  /*��ֵ*/
  p->STEP_LENGTH                = p->STEP_AA + p->STEP_UA + p->STEP_RA;
  pmotor->StartTableLength      = p->STEP_LENGTH;
  pmotor->StopTableLength       = p->STEP_LENGTH; 
  pmotor->Counter_Table         = SMove_TimeTable;//ָ������ʱ��ʱ�����������
  pmotor->Step_Table            = SMove_StepTable;//ָ������ʱ��ÿ��Ƶ�����������
  pmotor->speedenbale           = DISABLE;
  /*����*/
  pmotor->CurrentPosition       = 0;
  pmotor->CurrentPosition_Pulse = 0;
  pmotor->CurrentIndex          = 0;
  pmotor->StartSteps            = 0;//�������㣬�������ۼӣ�������ǰһ�εļ���
  pmotor->StopSteps             = 0;//ͬ��
  
  pmotor->Modify_Freq(pmotor->Master_TIMx,pmotor->Channel,pmotor->Counter_Table[0] - 1);
  
  pmotor->PulsesGiven = -1;
  SMove_Reinitial(pmotor);
  pmotor->Motor_Enable();
  pmotor->Motor_init(pmotor->Master_TIMx); //��ʼ�����
}
/*****************************���Ƶ������ָ���ǶȺ���****************************************************/
/**
  * @brief  Start_SMove_S:�����������S�����߲�������
  * @param  p:���Ʋ���
  * @param  Dir:��ת����
  * @param  DestPosition:Ŀ��λ��
  * @retval None.
  * @note   ���ܷ���while��
            ����Ҫ�ж��Ƿ���
*/
static void SMove_Start(SMove_Control_TypeDef *p,Directionstate Dir,uint32_t DestPosition)
{
  assert_param(p);
  uint32_t conte_temp  = 0,stop_temp = 0,start_step = 0;
  GPIO_PinState level  = GPIO_PIN_RESET;
  p->MaxPosition  = p->cycle_pulse_num;                      //������������
  p->MaxPosition_Pulse = p->MaxPosition * p->reduction_ratio;//������������
  if(DestPosition==0)return;  
  
  /*����true,ֹͣ������˳����ƺ���
  ����FALSE,�������п��ƺ���
  ��֤״̬�쳣ʱ���޷��������õ��
  */
  if(p->Motor_Stop_Priority())
  {
    SMove_Stop(p);
    p->running         = DISABLE;
    p->en              = DISABLE;
    return;
  }
 
  p->Motor_Direction(Dir);
  
  if(p->running == DISABLE)//�Ƕȸ������
  {
    p->PulsesHaven = 0;
    p->PulsesGiven = DestPosition;
    p->CurrentIndex = 0;
    
    p->en           = ENABLE;
    p->speedenbale  = DISABLE;
    
    p->set_freq = p->Counter_Table[0];
    p->Modify_Freq(p->Master_TIMx,p->Channel,p->set_freq - 1);
    p->Motor_Start();
    p->running = ENABLE;
  }
  else//�ǶȻ��ڸ��� 
  {   
    p->speedenbale  = DISABLE;
    switch(p->state)
    {
      case ACCEL:
        if(DestPosition < p->StopSteps)//����ֵ�ܲ�����١�����ֲ����� -���� ��ʼ����ֵ������ֵ�ߡ���ʼ����ֵ������ֵ�͡�
        {
          p->PulsesHaven = p->StartSteps;//��ǰ�����ڼ��ٽ׶ο�ʼ
          p->PulsesGiven = DestPosition + p->PulsesHaven;//Ŀ�경������
          for(int i = 0; i < p->StopTableLength; i ++)
          {
            stop_temp   += p->Step_Table[p->StartTableLength + p->StopTableLength - i - 1];//���Խ�������
            if(stop_temp > DestPosition)
            {
              conte_temp = i;
              p->CurrentIndex= p->StartTableLength + p->StopTableLength - conte_temp;//ֱ�ӿ��Ե�0�ļ�������
              p->pulsecount=p->Step_Table[p->CurrentIndex];
              break;
            }
          }
          /*ֱ���ܼ���*/
        }
        else//����ֵ�ܵ�����١�����ֲ����� -���� ��ʼ����ֵ������ֵ�ߡ�
        {
          p->PulsesHaven = p->StartSteps;//��ǰ�����ڼ��ٽ׶ο�ʼ
          p->PulsesGiven = DestPosition + p->PulsesHaven;//Ŀ�경������
          p->CurrentIndex = p->StartTableLength + 1;//���ٲ���Ҫ�ı䵱ǰ�ٶ�
          p->pulsecount=p->Step_Table[p->CurrentIndex];
        }
        break;
      case CONST:
        if(DestPosition < p->StopSteps)//����ֵ�ܲ������
        {
          p->PulsesHaven = p->PulsesHaven;//��ǰ���������ٽ׶� -�� ���ٽ׶�
          p->PulsesGiven = DestPosition + p->PulsesHaven;//Ŀ�경������
          for(int i = 0; i < p->StopTableLength; i ++)
          {
            stop_temp   += p->Step_Table[p->StartTableLength + p->StopTableLength - i - 1];//���Խ�������
            if(stop_temp > DestPosition)
            {
              conte_temp = i;
              p->CurrentIndex= p->StartTableLength + p->StopTableLength - conte_temp;//ֱ�ӿ��Ե�0�ļ�������
              p->pulsecount=p->Step_Table[p->CurrentIndex];
              break;
            }
          }
          /*ֱ���ܼ���*/
        }
        else//����ֵ�ܵ������
        {
          p->PulsesHaven = p->StartSteps;//��ǰ�����ڼ��ٽ׶� -�� ���ٽ׶�
          p->PulsesGiven = DestPosition + p->PulsesHaven;//Ŀ�경������
          p->CurrentIndex = p->CurrentIndex;//���ٲ���Ҫ�ı䵱ǰ�ٶ�
          p->pulsecount=p->Step_Table[p->CurrentIndex];
        }
        break;
      case DECEL:
       if(DestPosition < p->StopSteps)//����ֵ�ܲ�����١�����ֲ����� -���� ��ʼ����ֵ������ֵ�ߡ���ʼ����ֵ������ֵ�͡�
        {
          p->PulsesHaven = p->StartSteps;//��ǰ�����ڼ��ٽ׶ο�ʼ
          p->PulsesGiven = DestPosition + p->PulsesHaven;//Ŀ�경������
          for(int i = 0; i < p->StopTableLength; i ++)
          {
            stop_temp   += p->Step_Table[p->StartTableLength + p->StopTableLength - i - 1];//���Խ�������
            if(stop_temp > DestPosition)
            {
              conte_temp = i;
              p->CurrentIndex= p->StartTableLength + p->StopTableLength - conte_temp;//ֱ�ӿ��Ե�0�ļ�������
              p->pulsecount=p->Step_Table[p->CurrentIndex];
              break;
            }
          }
          /*ֱ���ܼ���*/
        }
        else//����ֵ�ܵ�����١�����ֲ����� -���� ��ʼ����ֵ������ֵ�ߡ�
        {
          p->PulsesHaven = p->StartSteps;//��ǰ�����ڼ��ٽ׶� -�� ���ٽ׶�
          p->PulsesGiven = DestPosition + p->PulsesHaven;//Ŀ�경������
          p->CurrentIndex = p->StartTableLength + 1;//���ٲ���Ҫ�ı䵱ǰ�ٶ�
          p->pulsecount=p->Step_Table[p->CurrentIndex];
        }
        break;
    }
  }
}
/**
  * @brief  �жϵ���Ƕȷ�Χ
  * @param  SMove_AngleTypeDef ����ǶȽṹ��
  * @param  input ����Ҫ�ж��ĽǶ�
  * @retval None
  * @note   �ж��Ƿ񳬳�����������Ϊ�ٽ�ֵ
            ��û�����ýǶȷ�Χ�����жϷ�Χ���ƣ���������ֵ
*/
static float Angle_Range_Judgment(SMove_AngleTypeDef *p,float angle)
{
  assert_param(p);
  float input = angle;
  if(p->min_angle == 0 && p->max_angle == 0)
    return input;
  if(p->min_angle <= input && input <= p->max_angle)
  {
    p->over_range = DISABLE;//����û�г����Ƕ�
  }
  else if(input < p->min_angle)
  {
    input = p->min_angle;
    p->over_range = ENABLE;//���볬���Ƕ�
    LOG_W("Input beyond minimum Angle");
  }
  else if(input > p->max_angle)
  {
    input = p->max_angle;
    p->over_range = ENABLE;//���볬���Ƕ�
    LOG_W("Input beyond maximum Angle");
  }
  return input;
}
/**
  * @brief  ���ص���Ƕȣ�Ԥ��ֵ��[���л���]
  * @param  SMove_Control_TypeDef ����������
  * @retval �Ƕ�
  * @note   �����������Ԥ����ǰ����Ƕȡ�
*/
float SMove_Get_Angle(SMove_Control_TypeDef* p)
{
  assert_param(p);
  return Angle_Conversion(((float)p->CurrentPosition_Pulse / p->cycle_pulse_num / p->reduction_ratio) * 360);
}
/**
  * @brief  ������ԽǶ��������
  * @param  SMove_TypeDef ����������
  * @retval None
  * @note   �Ƕ��޻���
*/
FunctionalState SMove_SetAngle_Relative(SMove_TypeDef *p,float relative_angle)
{
  assert_param(p);
  
  p->Rel.set_angle = Angle_Conversion(relative_angle);
  
  if(p->Rel.set_angle == 0)
    return p->Ctrl.running;
  else if(p->Rel.set_angle > 0)
  {
    p->Ctrl.dir = CW;
  }
  else  if(p->Rel.set_angle < 0)
  {
    p->Ctrl.dir = CCW;
  }
  
  p->Rel.set_angle = Angle_Range_Judgment(&p->Rel,p->Rel.set_angle);

  p->Rel.dest_position = (fabs(p->Rel.set_angle) / 360) * p->Ctrl.MaxPosition_Pulse;

  SMove_Start(&p->Ctrl,p->Ctrl.dir,p->Rel.dest_position);
  
  return p->Ctrl.running;
}
/**
  * @brief  �����ԽǶ��������
  * @param  SMove_TypeDef ����������
  * @retval None
  * @note   �ǶȻ��� ��Χ��-180~180��
*/
FunctionalState SMove_SetAngle_Absolute(SMove_TypeDef *p,float absolute_angle)
{
  assert_param(p);
  absolute_angle = Angle_Conversion(absolute_angle);
  p->Abs.set_angle = absolute_angle;
  
  /*�Ƕȸ����ж�*/
  p->Abs.err  = (float)(absolute_angle - p->Abs.last);//�жϽǶ����ޱ仯�����С�Ƕȡ�
  p->Abs.last = absolute_angle;                       //������һ�νǶ��뵱ǰ�Ƕ�����
  
  
  /*�����Ƕȷ����仯�Ž������*/
  if(fabs(p->Abs.err) != 0)
  {
    absolute_angle = Angle_Range_Judgment(&p->Abs,absolute_angle);
    p->Abs.get_angle = SMove_Get_Angle(&p->Ctrl);
    return SMove_SetAngle_Relative(p,absolute_angle - p->Abs.get_angle);
  }

  return p->Ctrl.running;
}
/**
  * @brief  ���õ���Ƕȷ�Χ
  * @param  SMove_AngleTypeDef ����ǶȽṹ��
  * @retval None
  * @note   ���ýǶȷ�Χ
*/
void SMove_SetAngle_Range(SMove_AngleTypeDef *p,float max,float min)
{
  assert_param(p);
  p->max_angle = max;
  p->min_angle = min;
}
/**
  * @brief  ����ǶȻ���
  * @param  SMove_AngleTypeDef ����ǶȽṹ��
  * @retval None
  * @note   ���ԽǶȶ�λ��Ҫ��¼��һ�νǶȣ����������������������˶�
*/
void SMove_SetAngle_Cache(SMove_AngleTypeDef *p)
{
  assert_param(p);
  p->last = 720;//�Ƕȷ�Χ����180��ỻ�㣬�������õ�720�ȡ�
}
/**
  * @brief  ��������еλ����Ϣ
  * @param  SMove_Control_TypeDef ���ƽṹ��.
  * @retval None.
  * @note   None.
*/
void SMove_SetAxis_Reset(SMove_Control_TypeDef *p)
{
  assert_param(p);
  p->CurrentPosition_Pulse = 0;
}
/**
  * @brief  ��ѯ�Ƕ��Ƿ񳬳�����
  * @param  SMove_AngleTypeDef ����ǶȽṹ��
  * @retval None
  * @note   None
*/
FunctionalState SMove_GetAngle_Over(SMove_AngleTypeDef *p)
{
  assert_param(p);
  return p->over_range;
}
/*****************************���Ƶ������ָ���ٶȺ���****************************************************/
/**
  * @brief  ���õ�������ٶȣ���RPM��λ����
  * @param  p:���Ʋ���:
  * @param  rpm:ת��/ÿ���� 
  * @param  Stop:ֹͣ�����־λ
  * @retval ֹͣ�Ƿ���� 0��δ��� 1����� -1���жϵ���ֹͣͣ -2:�����������
  * @note   �����ǻ���
            ���Ӽ���
            ����ʱֻ�跢��һ�Ρ�
            ����ʱ��Ҫѭ����ѯ�жϡ�
*/
MOVE_state SMove_SetRPM(SMove_TypeDef * p,int16_t rpm,FunctionalState Stop)
{
  if(rpm == 0 && Stop ==DISABLE)//�ڲ��Ǽ���ģʽʱ�������ٶ�Ϊ0ֹͣ
  {
    SMove_Stop(&p->Ctrl);
    return MOVE_STOP;
  }
  
  uint32_t freq = abs(rpm) * p->Ctrl.cycle_pulse_num * p->Ctrl.reduction_ratio / 60;//ת/S * һת����������
  int16_t num = Binary_Search(freq,p->Ctrl.Counter_Table,p->Ctrl.StartTableLength-1);
  
  if(num == 0)//Ϊ0���뺯���޷��жϷ���
    num = 1;
  
  if(rpm < 0)
    num = -num;

  return SMove_SetSpeed(p,num,Stop);
}
/**
  * @brief  ���õ�������ٶ�
  * @param  p:���Ʋ���:
  * @param  speedindex:�ٶȵȼ�����Χ��0~STEP_LENGTH��
  * @param  Stop:ֹͣ�����־λ
  * @retval ֹͣ�Ƿ���� 0��δ��� 1����� -1���жϵ���ֹͣͣ -2:�����������
  * @note   �����ǻ���
            ���Ӽ���
            ����ʱֻ�跢��һ�Ρ�
            ����ʱ��Ҫѭ����ѯ�жϡ�
*/
MOVE_state SMove_SetSpeed(SMove_TypeDef *p,int8_t SpeedIndex,FunctionalState Stop)
{
	int currentindex = 0,i = 0;
	unsigned int stepstostop  = 0;
  uint8_t speedindex;
  
  assert_param(p);
  p->Ctrl.MaxPosition = p->Ctrl.cycle_pulse_num;                            //������������
  p->Ctrl.MaxPosition_Pulse = p->Ctrl.MaxPosition * p->Ctrl.reduction_ratio;//������������
  if(p->Ctrl.Motor_Stop_Priority())
    return MOVE_STOP;
  
  if(Stop == 1)//���ٲ�ֹͣ���
  {
    //ֹͣ���,p->Ctrl.CurrentIndex=currentindex-1;ֱ������һ����
    p->Ctrl.speedenbale = DISABLE;
    currentindex = p->Ctrl.CurrentIndex; 
    
    if(p->Ctrl.CurrentIndex >= p->Ctrl.StartTableLength )
    {
        currentindex = p->Ctrl.StartTableLength + p->Ctrl.StopTableLength - p->Ctrl.CurrentIndex - 1;
    }
    
    for(i=0;i<currentindex;i++)
    {
       stepstostop += p->Ctrl.Step_Table[i];
    }
    //�������index
    if(p->Ctrl.CurrentIndex < p->Ctrl.StartTableLength )
    {
        currentindex = p->Ctrl.StartTableLength + p->Ctrl.StopTableLength - p->Ctrl.CurrentIndex - 1;  
    }
    p->Ctrl.CurrentIndex = currentindex;
    p->Ctrl.pulsecount   = p->Ctrl.Step_Table[p->Ctrl.CurrentIndex];
    p->Ctrl.PulsesGiven  = p->Ctrl.PulsesHaven + stepstostop;
    p->Ctrl.PulsesHaven  = p->Ctrl.PulsesGiven >> 1;
    if(stepstostop == 0)
    {
      SMove_Stop(&p->Ctrl);
      p->Ctrl.running         = DISABLE;
      p->Ctrl.en              = DISABLE;
      return MOVE_DONE;//ֹͣ
    }
    else
      return MOVE_UNDONE;
  }
  else if(Stop == 0)
  {
    if(SpeedIndex > 0)
    {
      p->Ctrl.dir = CW;
    }
    else if(SpeedIndex < 0)
    {
      p->Ctrl.dir = CCW;
    }
    
    p->Ctrl.Motor_Direction(p->Ctrl.dir);
    
    speedindex = abs(SpeedIndex);
    
    if(speedindex > p->V.max_speed)
      speedindex = p->V.max_speed;
    else if(speedindex < p->V.min_speed)
      speedindex = p->V.min_speed;
     
    //ֱ������һ�ٶ�
    currentindex = p->Ctrl.CurrentIndex;//��ǰ���ڼ��ٽ׶Σ���ǰλ�ò����仯

    p->Ctrl.PulsesHaven = 0;//������в���
    if(p->Ctrl.CurrentIndex >= p->Ctrl.StartTableLength)//��ǰ�ٶ��ڲ��ڼ���״̬
    {
        currentindex = p->Ctrl.StartTableLength + p->Ctrl.StopTableLength - p->Ctrl.CurrentIndex - 1;
    }
    
    if(currentindex >= speedindex)//��ǰ�ٶȴ��ڵ���Ŀ���ٶ�
    {
        //��Ҫ����
        p->Ctrl.PulsesGiven = p->Ctrl.PulsesHaven + p->Ctrl.StopSteps - 2;//�ܲ�������ֹͣ����
    }
    else//��ǰ�ٶ�С��Ŀ���ٶ�
    {
      //��Ҫ����
      p->Ctrl.PulsesGiven = 0xffffffff;
    }
    p->Ctrl.CurrentIndex = currentindex;
    p->Ctrl.pulsecount   = p->Ctrl.Step_Table[p->Ctrl.CurrentIndex];
    p->Ctrl.TargetIndex  = speedindex;
    p->Ctrl.speedenbale  = ENABLE;
    p->Ctrl.running      = ENABLE;
    p->Ctrl.en           = ENABLE;
    p->Ctrl.Motor_Start();
    return MOVE_UNDONE;
  }
   return -1;
}
/**
  * @brief  ���õ���ٶȷ�Χ
  * @param  SMove_SpeedTypeDef ����ٶȽṹ��
  * @retval None
  * @note   �����ٶȷ�Χ
*/
void SMove_SetSpeed_Range(SMove_TypeDef *p,uint32_t max,uint32_t min)
{
  p->V.max_speed = Binary_Search(max * p->Ctrl.cycle_pulse_num * p->Ctrl.reduction_ratio / 60,p->Ctrl.Counter_Table,p->Ctrl.StartTableLength-1);
  p->V.min_speed = Binary_Search(min * p->Ctrl.cycle_pulse_num * p->Ctrl.reduction_ratio / 60,p->Ctrl.Counter_Table,p->Ctrl.StartTableLength-1);
}
/**
  * @brief  ����ת�����ٶ�Ƶ��
  * @param  p:���Ʋ���
  * @retval Ƶ��ֵ
  * @note   ��������Ƶ�ʹ��ƣ���ʵ�������޹ء�
            ���ڶ�ʱ���� 10msһ�Ρ�
*/
int32_t SMove_Get_Speed(SMove_Control_TypeDef* p)
{
  assert_param(p);
  static uint32_t num = 0,last_num = 0;
  last_num = num;
  num = p->CurrentPosition_Pulse;
  return num - last_num;
}
/*****************************�������������������****************************************************/
/**
  * @brief  �����λ
  * @param  zero_max:��ԭ�����ٶȵȼ�����S�ͼӼ��١�
  * @param  zero_min:��ԭ����
  * @param  zero_state:���״̬
  * @param  limit_state:��λ״̬
  * @retval ��ԭ����״̬
  * @note   None.
*/
ZERO_state SMove_DoReset(SMove_TypeDef *p,int32_t zero_max,int32_t zero_min,SENSOR_state zero_state, SENSOR_state limit_state)
{
  assert_param(p);
  uint8_t speed_stop = 0;//�ٶ�ģʽֹͣ�����־ 0:�����٣�1����ʼ���٣�2���������

  p->Do.speed_high   = zero_max;
  p->Do.speed_low    = zero_min;
  
  if(limit_state == SENSOR_TOUCH)
  {
    SMove_Stop(&p->Ctrl);
    p->Do.flag = ZERO_ABNORMAL;
    return p->Do.flag;
  }
  
  if(zero_state == SENSOR_LEAVE)//û����
  {
    if(p->Do.flag != LOW_LEAVE)//û��������
    {
      p->V.set_speed = p->Do.speed_high;
      speed_stop = 0;
      p->Do.flag = QUICK_TOUCH;
    }
    if(p->Do.flag == LOW_LEAVE)//����
    {
      p->Ctrl.running = DISABLE;
      speed_stop =1;
      p->V.set_speed = 0;
    }
  }
  else if(zero_state == SENSOR_TOUCH)//����
  {  
    p->V.set_speed = p->Do.speed_low;
    speed_stop = 0;
    p->Do.flag = LOW_LEAVE;//��������
  }
  else
  {
    SMove_Stop(&p->Ctrl);
    p->Do.flag = ZERO_ABNORMAL;
    return p->Do.flag;
  }
  MOVE_state ret = SMove_SetRPM(p,p->V.set_speed,speed_stop);
  if(ret == MOVE_DONE)//�������
  {
    p->Do.flag = ZERO_OK;//��λ���
  }
  else if(ret == MOVE_STOP)
  {
    p->Do.flag = ZERO_ABNORMAL;
  }
  
  return p->Do.flag;
}
/**
  * @brief  ������Ƽ�غ���
  * @param  mode:1���ж���λ��������2�����㴫������3��һ���ж�.4��������λ��0���ж�
  * @param  SMove_TypeDef* p ����������
  * @param  zero_state����λ������״̬
  * @param  limit_state ��λ������״̬
  * @retval SMove_MONITOR_state.
  * @note   None.
*/
LIMIT_MONITOR_STATE SMove_Limit_Detection(SMove_TypeDef* p,uint8_t mode,SENSOR_state zero_state, SENSOR_state limit_state)
{
  if(limit_state == SENSOR_TOUCH && zero_state == SENSOR_TOUCH)
  {
      SMove_Stop(&p->Ctrl);
      return MONITOR_ALL;//�쳣״̬��������ͬʱ��������������
  }
  
  switch(mode)
  {
    case 0://�����ж�
      break;
    case 1:
      if(limit_state == SENSOR_TOUCH || limit_state == SENSOR_TIMEOUT)
      {
         SMove_Stop(&p->Ctrl);
         return MONITOR_LIMIT;
      }
      else if(limit_state == SENSOR_LEAVE || limit_state == SENSOR_INIT)
      {
         return MONITOR_NORMAL;
      }
      break;
    case 2:
      if(zero_state == SENSOR_TOUCH || zero_state == SENSOR_TIMEOUT)
      {
         SMove_Stop(&p->Ctrl);
         return MONITOR_ZERO;
       }
      else if(zero_state == SENSOR_LEAVE || zero_state == SENSOR_INIT)
      {
         return MONITOR_NORMAL;
      }
      break;
    case 3:
      if(zero_state == SENSOR_TOUCH   ||  zero_state == SENSOR_TIMEOUT)
      {
         SMove_Stop(&p->Ctrl);
         return MONITOR_ZERO;
      }
      else if(limit_state == SENSOR_TIMEOUT || limit_state == SENSOR_TOUCH)//����
      {
         SMove_Stop(&p->Ctrl);
         return MONITOR_LIMIT;
      }
      else if(zero_state == SENSOR_LEAVE && limit_state == SENSOR_LEAVE)
      {
         return MONITOR_NORMAL;
      }
      else
      {
          SMove_Stop(&p->Ctrl);
          return MONITOR_ABNORMAL;//�쳣״̬
      }
      break;
    case 4:
      if(p->Ctrl.dir == CCW)
      {
        if(zero_state == SENSOR_TOUCH || zero_state == SENSOR_TIMEOUT)
        {
           SMove_Stop(&p->Ctrl);
           return MONITOR_ZERO;
        }
        else if(zero_state == SENSOR_LEAVE || zero_state == SENSOR_INIT)
        {
           return MONITOR_NORMAL;
        }
      }
      else if(p->Ctrl.dir == CW)
      {
        if(limit_state == SENSOR_TOUCH || limit_state == SENSOR_TIMEOUT)
        {
           SMove_Stop(&p->Ctrl);
           return MONITOR_LIMIT;
        }
        else if(limit_state == SENSOR_LEAVE || limit_state == SENSOR_INIT)
        {
           return MONITOR_NORMAL;
        }
      }
      else
      {
        if(zero_state == SENSOR_TOUCH   ||  zero_state == SENSOR_TIMEOUT)
        {
           SMove_Stop(&p->Ctrl);
           return MONITOR_ZERO;
        }
        else if(limit_state == SENSOR_TIMEOUT || limit_state == SENSOR_TOUCH)//����
        {
           SMove_Stop(&p->Ctrl);
           return MONITOR_LIMIT;
        }
        else if(zero_state == SENSOR_LEAVE && limit_state == SENSOR_LEAVE)
        {
           return MONITOR_NORMAL;
        }
        else
        {
            SMove_Stop(&p->Ctrl);
            return MONITOR_ABNORMAL;//�쳣״̬
        }
      }
      break;
    default://�������ģʽ
      SMove_Stop(&p->Ctrl);
      return MONITOR_ABNORMAL;//�쳣״̬
      break;
  }
  return MONITOR_INIT;
}