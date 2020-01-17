#ifndef _COBRA_SYS_H_
#define _COBRA_SYS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_common.h>
#include <cobra_timer.h>
#include <cobra_console.h>

typedef enum sys_event_id_e
{
	EV_CON_CMD,
	EV_POWER_SWITCH,
	EV_PAIR_CLEAN,
	EV_AUDIO_PLAY,
	EV_WIRELESS_STATE,
	EV_WIRELESS_NOTICE,
} SYS_EVENT_ID_E;

typedef struct sys_status_s
{
	uint32_t	flag;
	struct power_status_s		*power;
	struct pair_status_s		*pair;
	struct audio_status_s		*audio;
	struct wireless_status_s	*wireless;
} SYS_STATUS_S;

typedef struct cobra_sys_s
{
	SYS_STATUS_S			status;
	struct timer_task_s		handle;
	struct mod_power_s		*mod_power;
	struct mod_pair_s		*mod_pair;
	struct mod_audio_s		*mod_audio;
	struct mod_wireless_s	*mod_wireless;

	struct console_event_s	cmd_test_resp;
	struct timer_task_s		cmd_test_resp_task;
	struct timer_task_s		cmd_test_resp_timeout;

	int			(*event_process)	(EVENT_S *event);
	void		(*sys_handle)		(void *args);

} COBRA_SYS_S;

extern COBRA_SYS_S gl_sys;
void cobra_sys_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_SYS_H_ */

