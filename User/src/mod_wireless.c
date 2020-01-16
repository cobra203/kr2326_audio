#include <cobra_define.h>
#include <cobra_console.h>
#include <cobra_sys.h>
#include <cobra_timer.h>
#include <cobra_button.h>
#include <cobra_led.h>
#include <mod_wireless.h>

#if (LOG_WIRELESS_LEVEL > LOG_LEVEL_NOT)
#define WIRELESS_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("WIRELESS: " fm, ##__VA_ARGS__) \
		console_cmdline_restore(); \
	}
#else
#define WIRELESS_INFO(fm, ...)
#endif

#if (LOG_WIRELESS_LEVEL > LOG_LEVEL_INFO)
#define WIRELESS_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("WIRELESS: " fm, ##__VA_ARGS__) \
		console_cmdline_restore(); \
	}
#else
#define WIRELESS_DEBUG(fm, ...)
#endif

#define WIRELESS_LOG(level, fm, ...) WIRELESS_##level(fm, ##__VA_ARGS__)

typedef enum {
	LED_STATUS_DARK,
	LED_STATUS_BRIGHT,
	LED_STATUS_POWER_SAVE,
} LED_STATUS_E;

MOD_WIRELESS_S gl_mod_wireless;

static void _mod_wireless_led_ctrl(uint8_t status)
{
#ifdef FUNC_LED_EN
	switch(status) {
	case LED_STATUS_POWER_SAVE:
		gl_mod_wireless.led.doing(&gl_mod_wireless.led, 3800, 200);
		break;
	case LED_STATUS_BRIGHT:
		gl_mod_wireless.led.doing(&gl_mod_wireless.led, 0, 1);
		break;
	case LED_STATUS_DARK:
	default:
		gl_mod_wireless.led.doing(&gl_mod_wireless.led, 1, 0);
		break;
	}
#endif
}

static void _mod_wireless_event_commint(EVENT_S *event, uint8_t event_id, uint8_t status_id)
{
	if(CBA_FALSE == event_is_active(event)) {
		gl_mod_wireless.status.state = status_id;
		WIRELESS_LOG(INFO, "%s %d\n", EV_WIRELESS_STATE == event_id ? "state" : "notice", status_id);
		event_commit(event, event_id, 3, EV_STATE_NORMAL, CBA_NULL);
	}
	else {
		WIRELESS_LOG(INFO, "event %s is busy\n", EV_WIRELESS_STATE == event_id ? "EV_WIRELESS_STAT" : "EV_WIRELESS_NOTICE");
	}
}

static void _mod_wireless_stat_blinking_process(BUTTON_S *button)
{
#if 1
	static uint8_t blinking_slowly_times = 0;
	static uint8_t blinking_fast_times = 0;

	if((ECT_UNOCCUPIED == button->state.effective || ECT_FOCUSED == button->state.effective) ||
	    (ECT_LOOSE == button->state.effective && button->focused_count) ||
	    (ECT_PRESSED == button->state.effective && button->loose_count)) {
	    blinking_fast_times = 0;
	}
	if(ECT_PRESSED == button->state.effective && !button->loose_count) {
		if(blinking_fast_times < 5) {
			blinking_fast_times++;
		}
		if(4 == blinking_fast_times) {
			_mod_wireless_event_commint(&gl_mod_wireless.stat_event, EV_WIRELESS_STATE, STAT_CONNECTING);
		}
	}
	switch(button->state.effective) {
	case ECT_LOOSE:
		/* blinking_slowly */
		if(1 != button->focused_count) {
			blinking_slowly_times = 0;
		}
		break;
	case ECT_UNOCCUPIED:
		if(gl_mod_wireless.status.active) {
			/* blinking_slowly */
			if(1 == button->loose_count && 1 == button->focused_count) {
				if(blinking_slowly_times < 3) {
					blinking_slowly_times++;
				}
				if(2 == blinking_slowly_times) {
					_mod_wireless_event_commint(&gl_mod_wireless.stat_event, EV_WIRELESS_STATE, STAT_INIT);
				}
			}
			else {
				blinking_slowly_times = 0;
			}
		}
		break;
	case ECT_PRESSED:
		/* blinking_slowly */
		if(1 != button->loose_count) {
			blinking_slowly_times = 0;
		}
		break;
	case ECT_FOCUSED:
		if(gl_mod_wireless.status.active) {
			/* blinking_slowly */
			if(1 != button->loose_count || 1 != button->focused_count) {
				blinking_slowly_times = 0;
			}
		}
		break;
	}

#endif
}

static void _mod_wireless_stat_process(BUTTON_S *button)
{
	if(button->state.active) {
		switch(button->state.effective) {
		case ECT_LOOSE:
			WIRELESS_LOG(DEBUG, "led stat ECT_LOOSE\n");
			WIRELESS_LOG(DEBUG, "loose_count[%d], focused_count[%d]\n", button->loose_count, button->focused_count);
			_mod_wireless_led_ctrl(LED_STATUS_DARK);
			break;
		case ECT_UNOCCUPIED:
			if(gl_mod_wireless.status.active) {
				WIRELESS_LOG(DEBUG, "led stat ECT_UNOCCUPIED\n");
				WIRELESS_LOG(DEBUG, "loose_count[%d], focused_count[%d]\n", button->loose_count, button->focused_count);
				if(button->loose_count == 6) {
					_mod_wireless_event_commint(&gl_mod_wireless.stat_event, EV_WIRELESS_STATE, STAT_POWER_SAVE);
					_mod_wireless_led_ctrl(LED_STATUS_POWER_SAVE);
					WIRELESS_LOG(INFO, "wireless into power save mode\n");
				}
			}
			break;
		case ECT_PRESSED:
			WIRELESS_LOG(DEBUG, "led stat ECT_PRESSED\n");
			WIRELESS_LOG(DEBUG, "loose_count[%d], focused_count[%d]\n", button->loose_count, button->focused_count);
			_mod_wireless_led_ctrl(LED_STATUS_BRIGHT);
			break;
		case ECT_FOCUSED:
			if(gl_mod_wireless.status.active) {
				WIRELESS_LOG(DEBUG, "led stat ECT_FOCUSED\n");
				WIRELESS_LOG(DEBUG, "loose_count[%d], focused_count[%d]\n", button->loose_count, button->focused_count);
				if(button->focused_count == 6) {
					_mod_wireless_event_commint(&gl_mod_wireless.stat_event, EV_WIRELESS_STATE, STAT_CONNECTED);
					WIRELESS_LOG(INFO, "wireless connect successful\n");
				}
			}
			break;
		}
		_mod_wireless_stat_blinking_process(button);
		button->state.active = 0;
    }
}

static void mod_wireless_stat_touch(void)
{
	gl_mod_wireless.led_stat.detect_count = 10;
}

static void _mod_wireless_monitor_handle(void *arg)
{
	BUTTON_S *button = (BUTTON_S *)arg;
	uint16_t gpio_vol;

	/* 1. get gpio_vol */
	if(gl_mod_wireless.status.active) {
		if(button->detect_count) {
			gpio_vol = GPIO_ReadInputData(LED_STAT_PORT);
			button->state.press = (~gpio_vol) & LED_STAT_PIN ? CBA_ENABLE : CBA_DISABLE;
			button->detect_count--;
		}
	}
	else {
		button->state.press = CBA_DISABLE;
	}
	/* 2. check active effective type */
	button->check_active(button);

	/* 3. logic process */
	_mod_wireless_stat_process(button);
}

static void _mod_wireless_monitor_handle_start(void)
{
	memset(&gl_mod_wireless.led_stat.state, 0, sizeof(BTN_STATE_S));
	gl_mod_wireless.led_stat.loose_count = 0;
	gl_mod_wireless.led_stat.focused_count = 0;
	timer_task_create(&gl_mod_wireless.handle, TMR_CYCLICITY, 0, 10, _mod_wireless_monitor_handle);
	mod_wireless_stat_touch();
}

static void _mod_wireless_monitor_handle_release(void)
{
	timer_task_release(&gl_mod_wireless.handle);
	_mod_wireless_led_ctrl(LED_STATUS_DARK);
}

static void mod_wireless_notice(void *cmd)
{
	CMD_S *pcmd = (CMD_S *)cmd;
	uint32_t notice_id = 0;

	WIRELESS_LOG(INFO, "============================================================\n");
	if(strlen(pcmd->arg) && (1 == sscanf(&pcmd->arg[0], "%d", &notice_id))) {
		_mod_wireless_event_commint(&gl_mod_wireless.notice_event, EV_WIRELESS_NOTICE, notice_id);
	}
	else {
		WIRELESS_LOG(INFO, "Invalid Arguments\n");
		WIRELESS_LOG(INFO, "Usage: wireless [notice_id]\n");
		WIRELESS_LOG(INFO, "The [notice_id] use decimal format\n");
	}
	WIRELESS_LOG(INFO, "============================================================\n");
}
CMD_CREATE_SIMPLE(wireless, mod_wireless_notice);

void mod_wireless_init(COBRA_SYS_S *sys)
{
	GPIO_InitTypeDef	gpio_cfg = {0};
	NVIC_InitTypeDef	nvic_cfg = {0};
	EXTI_InitTypeDef	exti_cfg = {0};

	sys->mod_wireless			= &gl_mod_wireless;
	sys->status.wireless		= &gl_mod_wireless.status;
	gl_mod_wireless.sys_status	= &sys->status;

	gl_mod_wireless.led_stat_touch = mod_wireless_stat_touch;

	gl_mod_wireless.led_stat.check_active 	 		= button_check_active;
	gl_mod_wireless.led_stat.interval.dithering		= 1;
	gl_mod_wireless.led_stat.interval.long_press	= 50;
	gl_mod_wireless.led_stat.interval.continuous	= 50;
	gl_mod_wireless.led_stat.interval.unoccupied	= 50;

    gpio_cfg.GPIO_Speed  = GPIO_Speed_2MHz;
	gpio_cfg.GPIO_Mode   = GPIO_Mode_IN;
    gpio_cfg.GPIO_PuPd   = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
    gpio_cfg.GPIO_Pin    = LED_STAT_PIN;
    GPIO_Init(LED_STAT_PORT, &gpio_cfg);

	SYSCFG_EXTILineConfig(LED_STAT_EXTI_PORT, LED_STAT_EXIT_SOURCE);
	exti_cfg.EXTI_Line		= LED_STAT_EXTI_LINE;
	exti_cfg.EXTI_Mode		= EXTI_Mode_Interrupt;
	exti_cfg.EXTI_Trigger	= EXTI_Trigger_Rising_Falling;
	exti_cfg.EXTI_LineCmd	= ENABLE;
	EXTI_Init(&exti_cfg);

	nvic_cfg.NVIC_IRQChannel = LED_STAT_IRQ;
#if defined (STM32F303xE)
	nvic_cfg.NVIC_IRQChannelPreemptionPriority = 0xf;
	nvic_cfg.NVIC_IRQChannelSubPriority = 0xf;
#elif defined (STM32F051)
	nvic_cfg.NVIC_IRQChannelPriority = 3;
#endif
	nvic_cfg.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_cfg);

#ifdef FUNC_LED_EN
	led_init(&gl_mod_wireless.led, LED_PORT, LED_PIN);

    gpio_cfg.GPIO_Speed  = GPIO_Speed_2MHz;
    gpio_cfg.GPIO_Mode   = GPIO_Mode_OUT;
    gpio_cfg.GPIO_PuPd   = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
    gpio_cfg.GPIO_OType  = GPIO_OType_PP;//GPIO_OType_OD;
    gpio_cfg.GPIO_Pin    = LED_PIN;
    GPIO_Init(LED_PORT, &gpio_cfg);

	_mod_wireless_led_ctrl(LED_STATUS_DARK);
#endif

	gl_mod_wireless.handle.name = "mod_wireless_monitor_handle";
	gl_mod_wireless.handle.cb_data = &gl_mod_wireless.led_stat;
	gl_mod_wireless.handle_start = _mod_wireless_monitor_handle_start;
	gl_mod_wireless.handle_release = _mod_wireless_monitor_handle_release;

	cmd_register(&cmd_wireless);

	WIRELESS_LOG(INFO, "%s ... OK\n", __func__);
}

