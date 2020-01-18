/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/stm32f0xx_it.c
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    05-December-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include <cobra_common.h>
#include <cobra_console.h>
#include <mod_power.h>
#include <mod_pair.h>
#include <mod_wireless.h>

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	timer_itc(0);
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void USART_IRQHandler(void)
{
	char byte;

    if(USART_GetITStatus(CONSOLE_UART_COM, USART_IT_RXNE)) {
		byte = USART_ReceiveData(CONSOLE_UART_COM);

		if(byte == '\r') {
			console_send_byte('\n');
			console_puts(CONSOLE_TAG);
			gl_console.cmdline[gl_console.cmdline_size] = 0;
			gl_console.cmdline_touch();
			return;
		}
		if(gl_console.cmdline_size < _CMDLINE_MAX_SIZE_ - 1) {
			if('\b' == byte) {
				if(gl_console.cmdline_size > 0) {
					console_puts("\b \b");
					gl_console.cmdline_size--;
				}
			}
			else {
				gl_console.cmdline[gl_console.cmdline_size++] = byte;
				console_send_byte(byte);
			}
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			TIM_SetCounter(TIM3, 0);
			TIM_Cmd(TIM3, ENABLE);
		}
    }
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		TIM_Cmd(TIM3, DISABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
  	}
}

#ifdef STM32F303xE
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(KEY_POWER_EXTI_LINE) != RESET) {
		EXTI_ClearITPendingBit(KEY_POWER_EXTI_LINE);
		if(gl_sys.mod_power) {
			gl_mod_power.key_power_touch();
		}
	}
}
#endif

#ifdef STM32F051
void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(KEY_POWER_EXTI_LINE) != RESET) {
		EXTI_ClearITPendingBit(KEY_POWER_EXTI_LINE);
		if(gl_sys.mod_power) {
			gl_mod_power.key_power_touch();
		}
	}
	if(EXTI_GetITStatus(KEY_PAIR_EXTI_LINE) != RESET) {
		EXTI_ClearITPendingBit(KEY_PAIR_EXTI_LINE);
		if(gl_sys.mod_pair) {
			gl_mod_pair.key_pair_touch();
		}
	}
	if(EXTI_GetITStatus(LED_STAT_EXTI_LINE) != RESET) {
		EXTI_ClearITPendingBit(LED_STAT_EXTI_LINE);
		if(gl_sys.mod_wireless) {
			gl_mod_wireless.led_stat_touch();
		}
	}
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
