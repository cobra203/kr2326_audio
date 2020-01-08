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
} SYS_EVENT_ID_E;

typedef struct sys_status_s
{
	uint8_t	work;
	struct power_status_s	*power;
	struct pair_status_s	*pair;
} SYS_STATUS_S;

typedef struct cobra_sys_s
{
	SYS_STATUS_S			status;
	struct timer_task_s		handle;
	struct mod_power_s		*mod_power;
	struct mod_pair_s		*mod_pair;

	struct console_event_s	cmd_status_resp;
	struct timer_task_s		cmd_status_resp_task;
	struct timer_task_s		cmd_status_resp_timeout;

	int			(*event_process)	(EVENT_S *event);
	void		(*sys_handle)		(void *args);

} COBRA_SYS_S;

extern COBRA_SYS_S gl_sys;
void cobra_sys_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_SYS_H_ */

