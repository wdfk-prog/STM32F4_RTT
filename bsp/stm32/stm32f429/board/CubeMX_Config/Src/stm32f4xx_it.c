/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "walk_motor.h"
#include "turn_motor.h"
#include "mb_handler.h"

/*ulog include*/
#define LOG_TAG              "" 
#define LOG_LVL              DBG_INFO
#include <ulog.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(WK_UP_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */
  rt_interrupt_leave();
  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(Turn_Zero_Pin);
  /* USER CODE BEGIN EXTI2_IRQn 1 */
  rt_interrupt_leave();
  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line3 interrupt.
  */
void EXTI3_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI3_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END EXTI3_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(Turn_Limit_Pin);
  /* USER CODE BEGIN EXTI3_IRQn 1 */
  rt_interrupt_leave();
  /* USER CODE END EXTI3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END EXTI4_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(Lift_Reflection_Pin);
  /* USER CODE BEGIN EXTI4_IRQn 1 */
  rt_interrupt_leave();
  /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(Walk_ALM_Pin);
  HAL_GPIO_EXTI_IRQHandler(Article_Crash_Pin);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */
  rt_interrupt_leave();
  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(Stop_Button_Pin);
  HAL_GPIO_EXTI_IRQHandler(Turn_ALM_Pin);
  HAL_GPIO_EXTI_IRQHandler(Lower_Limit_Pin);
  HAL_GPIO_EXTI_IRQHandler(Lift_SQP_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */
  rt_interrupt_leave();
  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */
  rt_interrupt_enter();
  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */
  rt_interrupt_enter();
  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None.
  * @note   None
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if(GPIO_Pin==WK_UP_Pin)//PA0
   {
//      if(HAL_GPIO_ReadPin(Turn_Limit_GPIO_Port, Turn_Limit_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
//      {    

//      }
//      else
//      {

//      }//�ⲿ�ж�0
   }
   else if(GPIO_Pin == Turn_Limit_Pin)     //�ж��Ƿ�Ϊ��λ������
   {
      if(HAL_GPIO_ReadPin(Turn_Limit_GPIO_Port, Turn_Limit_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
      {    
          turn.motor.Do.limit.state = SENSOR_LEAVE;   //���ñ�־
          SENSOR_LIMIT_RESET;
      }
      else
      {
          turn.motor.Do.limit.state = SENSOR_TOUCH;   //���ñ�־
          SENSOR_LIMIT_SET;
      }//�ⲿ�ж�2
   }
   else if(GPIO_Pin == Turn_Zero_Pin)        //�ж��Ƿ�Ϊ��λ������
   {
      if(HAL_GPIO_ReadPin(Turn_Zero_GPIO_Port, Turn_Zero_Pin) == RESET)    //�ж��Ƿ�Ϊ��
      {    
          turn.motor.Do.zero.state = SENSOR_LEAVE;   //���ñ�־
          SENSOR_ZERO_RESET;
      }
      else
      {
          turn.motor.Do.zero.state = SENSOR_TOUCH;   //���ñ�־
          SENSOR_ZERO_SET;
      }//�ⲿ�ж�3
   }   
   else if(GPIO_Pin == Lift_Reflection_Pin)        //��������䴫����
   { 
      if(HAL_GPIO_ReadPin(Lift_Reflection_GPIO_Port,Lift_Reflection_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
      {
        LIFT_REFLECTION_SET;
      }
      else
      {
        LIFT_REFLECTION_RESET;
      }//�ⲿ�ж�4
   }
   else if(GPIO_Pin == Walk_ALM_Pin)        //�ж��Ƿ�Ϊ���߱���
   { 
      if(HAL_GPIO_ReadPin(Walk_ALM_GPIO_Port, Walk_ALM_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
      {    
          walk.ALM.state = SENSOR_TOUCH;   //���ñ�־
          Walk_ALARM_SET;
          USER_SET_BIT(walk.Stop_state,ALM_STOP);
      }
      else
      {
         walk.ALM.state = SENSOR_LEAVE;   //���ñ�־
      }//�ⲿ�ж�6
   }  
   else if(GPIO_Pin == Article_Crash_Pin)        //�ж��Ƿ�Ϊ��ײ��
   { 
      if(HAL_GPIO_ReadPin(Article_Crash_GPIO_Port, Article_Crash_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
      {    
        CRASH_SET;
        CRASH_ALARM_SET;
        USER_SET_BIT(turn.Stop_state,CRASH_STOP);
        USER_SET_BIT(walk.Stop_state,CRASH_STOP);
      }
      else
      {
        CRASH_RESET;//����������״̬
      }//�ⲿ�ж�9
   }
   else if(GPIO_Pin == Stop_Button_Pin)                  //�ж��Ƿ�Ϊ��ͣ��ť
   {
      if(HAL_GPIO_ReadPin(Stop_Button_GPIO_Port, Stop_Button_Pin) == SET)    //�ж��Ƿ�Ϊ��
      {
          ReadEMG_SET;
          SOFT_STOP_SET;//��ͣ��ť����ʱһ��������ͣ
          USER_SET_BIT(turn.Stop_state,HARD_STOP);
          USER_SET_BIT(walk.Stop_state,HARD_STOP);
      }
      else//�ж��Ƿ�Ϊ��
      {    
          ReadEMG_RESET;
          SOFT_STOP_RESET;//��ͣ��ť�ɿ�ʱһ���ɿ���ͣ
          USER_CLEAR_BIT(turn.Stop_state,HARD_STOP);
          USER_CLEAR_BIT(walk.Stop_state,HARD_STOP);
      }//�ⲿ�ж�10
   }
   else if(GPIO_Pin == Turn_ALM_Pin)        //�ж��Ƿ�Ϊת��������
   { 
      if(HAL_GPIO_ReadPin(Turn_ALM_GPIO_Port,Turn_ALM_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
      {    
          turn.ALM.state = SENSOR_LEAVE;   //���ñ�־
      }
      else
      {
          TURN_ALARM_SET;
          turn.ALM.state = SENSOR_TOUCH;   //���ñ�־
          USER_SET_BIT(turn.Stop_state,ALM_STOP);
      }//�ⲿ�ж�12
   }
   else if(GPIO_Pin == Lift_SQP_Pin)        //��۽ӽ�����
   { 
      if(HAL_GPIO_ReadPin(Lift_SQP_GPIO_Port,Lift_SQP_Pin) == RESET)    //�жϰ����Ƿ�Ϊ��
      {    
          LIFT_SQP_SET;
      }
      else
      {
          LIFT_SQP_RESET;
      }//�ⲿ�ж�14
   }
}
/* USER CODE END 1 */
