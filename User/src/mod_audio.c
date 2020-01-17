#include <cobra_define.h>
#include <cobra_console.h>
#include <cobra_sys.h>
#include <cobra_timer.h>
#include <mod_audio.h>

#if (LOG_AUDIO_LEVEL > LOG_LEVEL_NOT)
#define AUDIO_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("AUDIO   : " fm, ##__VA_ARGS__) \
		console_cmdline_restore(); \
	}
#else
#define AUDIO_INFO(fm, ...)
#endif

#if (LOG_AUDIO_LEVEL > LOG_LEVEL_INFO)
#define AUDIO_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("AUDIO   : " fm, ##__VA_ARGS__) \
		console_cmdline_restore(); \
	}
#else
#define AUDIO_DEBUG(fm, ...)
#endif

#define AUDIO_LOG(level, fm, ...) AUDIO_##level(fm, ##__VA_ARGS__)

MOD_AUDIO_S gl_mod_audio;

static inline void _mod_audio_sda_ctrl(uint8_t val)
{
	if(val) {
		GPIO_SetBits(SDA_PORT, SDA_PIN);
	}
	else {
		GPIO_ResetBits(SDA_PORT, SDA_PIN);
	}
}

static inline void _mod_audio_sck_ctrl(uint8_t val)
{
	if(val) {
		GPIO_SetBits(SCK_PORT, SCK_PIN);
	}
	else {
		GPIO_ResetBits(SCK_PORT, SCK_PIN);
	}
}

static void _mod_audio_play_single(uint8_t id)
{
	uint8_t size = 8;

	_mod_audio_sck_ctrl(0);
	delay_ms(5);

	while(size--) {
		_mod_audio_sda_ctrl((id & 0x1) ? 1 : 0);
		delay_us(150);
		_mod_audio_sck_ctrl(1);
		delay_us(300);
		_mod_audio_sck_ctrl(0);
		delay_us(150);

		id = id >> 1;
	}
	_mod_audio_sda_ctrl(1);
	delay_us(150);

	_mod_audio_sck_ctrl(1);
}

static void _mod_audio_play(uint8_t id)
{
	if(id < 0xe0) {
		_mod_audio_play_single(id);
		gl_mod_audio.status.last_play = id;
		AUDIO_LOG(INFO, "play %d\r\n", id);
	}
	else if (0xe0 <= id && id <= 0xe7) {
		gl_mod_audio.status.volume = id - 0xe0;
		AUDIO_LOG(INFO, "volume set to %d\r\n", id - 0xe0);
	}
}

static void _mod_audio_play_pre(uint8_t id)
{
	static uint8_t play_id = 0;
	EVENT_S *event = &gl_mod_audio.play_event;

	if(CBA_FALSE == event_is_active(event)) {
		_mod_audio_play(id);
		play_id = id;
		event_commit(event, EV_AUDIO_PLAY, 3, EV_STATE_NORMAL, &play_id);
	}
	else {
		AUDIO_LOG(INFO, "event EV_AUDIO_PLAY is busy\r\n");
	}
}

static void mod_audio_cmd(void *cmd)
{
	CMD_S *pcmd = (CMD_S *)cmd;
	uint32_t id = 0;

	AUDIO_LOG(INFO, "============================================================\r\n");
	if(strlen(pcmd->arg) && (1 == sscanf(&pcmd->arg[0], "%d", &id))) {
		_mod_audio_play_pre(id);
	}
	else {
		AUDIO_LOG(INFO, "Invalid Arguments\r\n");
		AUDIO_LOG(INFO, "Usage: audio [id]\r\n");
		AUDIO_LOG(INFO, "The [id] use decimal format\r\n");
	}
	AUDIO_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(audio, mod_audio_cmd);

void mod_audio_init(COBRA_SYS_S *sys)
{
	GPIO_InitTypeDef		gpio_cfg;

	sys->mod_audio			= &gl_mod_audio;
	sys->status.audio		= &gl_mod_audio.status;
	gl_mod_audio.sys_status	= &sys->status;
	gl_mod_audio.play = _mod_audio_play;

    gpio_cfg.GPIO_Speed  = GPIO_Speed_2MHz;
    gpio_cfg.GPIO_Mode   = GPIO_Mode_OUT;
    gpio_cfg.GPIO_PuPd   = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
    gpio_cfg.GPIO_OType  = GPIO_OType_PP;//GPIO_OType_OD;
    gpio_cfg.GPIO_Pin    = SDA_PIN;
    GPIO_Init(SDA_PORT, &gpio_cfg);

	gpio_cfg.GPIO_Speed  = GPIO_Speed_2MHz;
    gpio_cfg.GPIO_Mode   = GPIO_Mode_OUT;
    gpio_cfg.GPIO_PuPd   = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
    gpio_cfg.GPIO_OType  = GPIO_OType_PP;//GPIO_OType_OD;
    gpio_cfg.GPIO_Pin    = SCK_PIN;
    GPIO_Init(SCK_PORT, &gpio_cfg);

	gpio_cfg.GPIO_Speed  = GPIO_Speed_2MHz;
	gpio_cfg.GPIO_Mode   = GPIO_Mode_IN;
    gpio_cfg.GPIO_PuPd   = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
    gpio_cfg.GPIO_Pin    = BUSY_PIN;
    GPIO_Init(BUSY_PORT, &gpio_cfg);

	GPIO_SetBits(SDA_PORT, SDA_PIN);
	GPIO_SetBits(SCK_PORT, SCK_PIN);
	delay_ms(100);
	_mod_audio_play(AUDIO_VOLUME + 7);

	cmd_register(&cmd_audio);

	AUDIO_LOG(INFO, "%s ... OK\r\n", __func__);
}
