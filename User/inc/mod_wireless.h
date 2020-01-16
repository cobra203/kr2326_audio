#ifndef _MOD_WIRELESS_H_
#define _MOD_WIRELESS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_sys.h>
#include <cobra_event.h>
#include <cobra_timer.h>

#define FUNC_LED_EN

#define LED_STAT_PORT			GPIO_PORT(WIRELESS_STAT_PORTx)
#define LED_STAT_PIN			GPIO_PIN(WIRELESS_STAT_PINx)
#define LED_STAT_EXTI_PORT		EXTI_PORT(WIRELESS_STAT_PORTx)
#define LED_STAT_EXIT_SOURCE	EXIT_PIN(WIRELESS_STAT_PINx)
#define LED_STAT_EXTI_LINE		EXTI_LINE(WIRELESS_STAT_PINx)
#define LED_STAT_IRQ			EXTI_IRQ(WIRELESS_STAT_IRQ_RANGE)

#define LED_PORT				GPIO_PORT(WIRELESS_LED_PORTx)
#define LED_PIN					GPIO_PIN(WIRELESS_LED_PINx)

typedef enum wireless_stat_e
{
	STAT_NULL,
	STAT_INIT,
	STAT_PAIRING,
	STAT_PAIRED,
	STAT_CONNECTING,
	STAT_CONNECTED,
	STAT_POWER_SAVE,
} WIRELESS_STAT_E;

typedef struct wireless_status_s
{
	uint8_t		state	:5;
	uint8_t				:2;
	uint8_t		active	:1;
} WIRELESS_STATUS_S;

typedef struct mod_wireless_s
{
	SYS_STATUS_S		*sys_status;
	WIRELESS_STATUS_S	status;

	BUTTON_S        	led_stat;
	LED_S				led;
	EVENT_S				stat_event;		/* form the led_stat */
	EVENT_S				notice_event;	/* form the cmdline */
	TIMER_TASK_S		handle;

	void		(*led_stat_touch)   (void);
	void		(*handle_start)		(void);
	void		(*handle_release)	(void);
} MOD_WIRELESS_S;

extern MOD_WIRELESS_S gl_mod_wireless;

void mod_wireless_init(COBRA_SYS_S *sys);

#ifdef __cplusplus
}
#endif

#endif /* _MOD_WIRELESS_H_ */
