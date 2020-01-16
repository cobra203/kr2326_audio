#ifndef _MOD_PAIR_H_
#define _MOD_PAIR_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_sys.h>
#include <cobra_button.h>
#include <cobra_event.h>
#include <cobra_timer.h>
#include <cobra_led.h>

#define KEY_PAIR_PORT			GPIO_PORT(KEY_PAIR_PORTx)
#define KEY_PAIR_PIN			GPIO_PIN(KEY_PAIR_PINx)
#define KEY_PAIR_EXTI_PORT		EXTI_PORT(KEY_PAIR_PORTx)
#define KEY_PAIR_EXIT_SOURCE	EXIT_PIN(KEY_PAIR_PINx)
#define KEY_PAIR_EXTI_LINE		EXTI_LINE(KEY_PAIR_PINx)
#define KEY_PAIR_IRQ			EXTI_IRQ(KEY_PAIR_IRQ_RANGE)

#define PAIR_CTRL_PORT			GPIO_PORT(PAIR_CTRL_PORTx)
#define PAIR_CTRL_PIN			GPIO_PIN(PAIR_CTRL_PINx)

typedef struct pair_status_s
{
	uint8_t		cleaning	:1;
	uint8_t					:7;
} PAIR_STATUS_S;

typedef struct mod_pair_s
{
	SYS_STATUS_S		*sys_status;
	PAIR_STATUS_S		status;
	LED_S				pair_ctrl;
    BUTTON_S        	key_pair;
	EVENT_S				pair_clean;
	EVENT_S				pair_clean_resp;
	TIMER_TASK_S		pair_clean_task;
	TIMER_TASK_S		handle;

    void		(*key_pair_touch)   (void);
} MOD_PAIR_S;

extern MOD_PAIR_S gl_mod_pair;

void mod_pair_init(COBRA_SYS_S *sys);


#ifdef __cplusplus
}
#endif

#endif /* _MOD_PAIR_H_ */

