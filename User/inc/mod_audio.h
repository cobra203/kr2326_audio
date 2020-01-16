#ifndef _MOD_AUDIO_H_
#define _MOD_AUDIO_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_sys.h>
#include <cobra_event.h>
#include <cobra_timer.h>

#define SDA_PORT				GPIO_PORT(AUDIO_SDA_PORTx)
#define SDA_PIN					GPIO_PIN(AUDIO_SDA_PINx)
#define SCK_PORT				GPIO_PORT(AUDIO_SCK_PORTx)
#define SCK_PIN					GPIO_PIN(AUDIO_SCK_PINx)
#define BUSY_PORT				GPIO_PORT(AUDIO_BUSY_PORTx)
#define BUSY_PIN				GPIO_PIN(AUDIO_BUSY_PINx)

typedef enum audio_id_e
{
	AUDIO_POWER_ON = 0,
	AUDIO_WIRELESS_INIT = 1,
	AUDIO_PAIR_CLEANING = 2,
	AUDIO_PAIR_CLEANED = 3,
	AUDIO_PAIRING = 4,
	AUDIO_PAIRED = 5,
	AUDIO_CONNECTING = 6,
	AUDIO_CONNECTED = 7,
	AUDIO_POWER_SAVE_MODE = 8,

	AUDIO_POWER_OFF = 10,
	AUDIO_POWER_PLEASE_ON,
	AUDIO_VOLUME = 0xe0,
} AUDIO_ID_E;

typedef struct audio_status_s
{
	uint8_t		last_play;
	uint8_t		volume;
} AUDIO_STATUS_S;

typedef struct mod_audio_s
{
	SYS_STATUS_S		*sys_status;
	AUDIO_STATUS_S		status;

	EVENT_S				play_event;

    void		(*play)   (uint8_t id);
} MOD_AUDIO_S;

extern MOD_AUDIO_S gl_mod_audio;

void mod_audio_init(COBRA_SYS_S *sys);

#ifdef __cplusplus
}
#endif

#endif /* _MOD_AUDIO_H_ */
