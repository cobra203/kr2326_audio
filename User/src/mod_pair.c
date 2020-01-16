#include <cobra_define.h>
#include <cobra_console.h>
#include <cobra_timer.h>
#include <mod_pair.h>
#include <mod_power.h>

#if (LOG_PAIR_LEVEL > LOG_LEVEL_NOT)
#define PAIR_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("PAIR    : " fm, ##__VA_ARGS__) \
		console_cmdline_restore(); \
	}
#else
#define PAIR_INFO(fm, ...)
#endif

#if (LOG_PAIR_LEVEL > LOG_LEVEL_INFO)
#define PAIR_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("PAIR    : " fm, ##__VA_ARGS__) \
		console_cmdline_restore(); \
	}
#else
#define PAIR_DEBUG(fm, ...)
#endif

#define PAIR_LOG(level, fm, ...) PAIR_##level(fm, ##__VA_ARGS__)

MOD_PAIR_S gl_mod_pair;

static void _mod_pair_ctrl(CBA_BOOL enable)
{
	if(enable) {
		gl_mod_pair.pair_ctrl.doing(&gl_mod_pair.pair_ctrl, 0, 1);
	}
	else {
		gl_mod_pair.pair_ctrl.doing(&gl_mod_pair.pair_ctrl, 1, 0);
	}
}

static void _mod_pair_clean_post(void *data)
{
	EVENT_S *event = &gl_mod_pair.pair_clean_resp;

	if(CBA_FALSE == event_is_active(event)) {
		_mod_pair_ctrl(CBA_FALSE);
		gl_mod_pair.status.cleaning = CBA_FALSE;
		event_commit(event, EV_PAIR_CLEAN, 3, EV_STATE_RESPONSE, CBA_NULL);
		PAIR_LOG(INFO, "Pair clean ... OK\n");
	}
}

static void _mod_pair_clean_pre(void)
{
	EVENT_S *event = &gl_mod_pair.pair_clean;

	if(CBA_FALSE == event_is_active(event)) {
		if(gl_mod_pair.sys_status->power->enable) {
			_mod_pair_ctrl(CBA_TRUE);
			gl_mod_pair.status.cleaning = CBA_TRUE;
			timer_task_create(&gl_mod_pair.pair_clean_task, TMR_ONCE, 10000, 0, _mod_pair_clean_post);
			event_commit(event, EV_PAIR_CLEAN, 3, EV_STATE_REQUEST, CBA_NULL);
			PAIR_LOG(INFO, "Pair clean ...\n");
		}
		else {
			event_commit(event, EV_PAIR_CLEAN, 3, EV_STATE_NORMAL, CBA_NULL);
		}
	}
	else {
		PAIR_LOG(INFO, "event EV_PAIR_CLEAN is busy\n");
	}
}

static void _mod_pair_key_process(BUTTON_S *button)
{
	if(button->state.active) {
		switch(button->state.effective) {
		case ECT_LOOSE:
		case ECT_UNOCCUPIED:
			break;
		case ECT_PRESSED:
			if(CBA_FALSE == gl_mod_pair.sys_status->power->enable) {
				_mod_pair_clean_pre();
			}
			break;
		case ECT_FOCUSED:
			if(button->focused_count == 10) {
				_mod_pair_clean_pre();
			}
			break;
		}
		button->state.active = 0;
    }
}

static void mod_pair_key_touch(void)
{
	gl_mod_pair.key_pair.detect_count = 10;
}

static void _mod_pair_monitor_handle(void *arg)
{
	BUTTON_S *button = (BUTTON_S *)arg;
	uint16_t gpio_vol;

	/* 1. get gpio_vol */
	if(button->detect_count) {
		gpio_vol = GPIO_ReadInputData(KEY_PAIR_PORT);
		button->state.press = (~gpio_vol) & KEY_PAIR_PIN ? 1 : 0;
		button->detect_count--;
	}
	/* 2. check active effective type */
	button->check_active(button);

	/* 3. logic process */
	_mod_pair_key_process(button);
}

static void mod_pair_cmd(void *cmd)
{
	CMD_S *pcmd = (CMD_S *)cmd;

	PAIR_LOG(INFO, "============================================================\n");
	if(strncmp("clean", pcmd->arg, strlen("clean")) == 0) {
		_mod_pair_clean_pre();
	}
	else {
		PAIR_LOG(INFO, "Invalid Arguments\n");
		PAIR_LOG(INFO, "Usage: pair [clean]\n");
	}
	PAIR_LOG(INFO, "============================================================\n");
}
CMD_CREATE_SIMPLE(pair, mod_pair_cmd);

void mod_pair_init(COBRA_SYS_S *sys)
{
    GPIO_InitTypeDef    gpio_cfg = {0};
	NVIC_InitTypeDef	nvic_cfg = {0};
	EXTI_InitTypeDef	exti_cfg = {0};

	sys->mod_pair			= &gl_mod_pair;
	sys->status.pair		= &gl_mod_pair.status;
	gl_mod_pair.sys_status	= &sys->status;

	gl_mod_pair.key_pair_touch = mod_pair_key_touch;
	gl_mod_pair.key_pair_touch();

	gl_mod_pair.key_pair.check_active 	 		= button_check_active;
	gl_mod_pair.key_pair.interval.dithering		= 3;
	gl_mod_pair.key_pair.interval.long_press	= 100;
	gl_mod_pair.key_pair.interval.continuous	= 100;
	gl_mod_pair.key_pair.interval.unoccupied	= 100;

    gpio_cfg.GPIO_Mode   = GPIO_Mode_IN;
    gpio_cfg.GPIO_PuPd   = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
    gpio_cfg.GPIO_Speed  = GPIO_Speed_2MHz;
    gpio_cfg.GPIO_Pin    = KEY_PAIR_PIN;
    GPIO_Init(KEY_PAIR_PORT, &gpio_cfg);

	SYSCFG_EXTILineConfig(KEY_PAIR_EXTI_PORT, KEY_PAIR_EXIT_SOURCE);
	exti_cfg.EXTI_Line		= KEY_PAIR_EXTI_LINE;
	exti_cfg.EXTI_Mode		= EXTI_Mode_Interrupt;
	exti_cfg.EXTI_Trigger	= EXTI_Trigger_Rising_Falling;
	exti_cfg.EXTI_LineCmd	= ENABLE;
	EXTI_Init(&exti_cfg);

	nvic_cfg.NVIC_IRQChannel = KEY_PAIR_IRQ;
#if defined (STM32F303xE)
	nvic_cfg.NVIC_IRQChannelPreemptionPriority = 0xf;
	nvic_cfg.NVIC_IRQChannelSubPriority = 0xf;
#elif defined (STM32F051)
	nvic_cfg.NVIC_IRQChannelPriority = 3;
#endif
	nvic_cfg.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_cfg);

	led_init(&gl_mod_pair.pair_ctrl, PAIR_CTRL_PORT, PAIR_CTRL_PIN);
	gl_mod_pair.pair_ctrl.doing(&gl_mod_pair.pair_ctrl, 1, 0);

	gl_mod_pair.handle.name = "mod_pair_monitor_handle";
	gl_mod_pair.handle.cb_data = &gl_mod_pair.key_pair;
	timer_task_create(&gl_mod_pair.handle, TMR_CYCLICITY, 10, 10, _mod_pair_monitor_handle);

	cmd_register(&cmd_pair);

	PAIR_LOG(INFO, "%s ... OK\n", __func__);
}


