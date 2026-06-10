/*************************************************************************//**
 *****************************************************************************
 * @file   systask.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2007
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE int read_register(char reg_addr);
PRIVATE u32 get_rtc_time(struct time *t);
PRIVATE int get_proc_info(int dest_pid, void* buf, int buf_len);	/* B级新增 */

/*****************************************************************************
 *                                task_sys
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK SYS.
 * 
 *****************************************************************************/
PUBLIC void task_sys()
{
	MESSAGE msg;
	struct time t;

	while (1) {
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;

		switch (msg.type) {
		case GET_TICKS:
			msg.RETVAL = ticks;
			send_recv(SEND, src, &msg);
			break;
		case GET_PID:
			msg.type = SYSCALL_RET;
			msg.PID = src;
			send_recv(SEND, src, &msg);
			break;
		case GET_RTC_TIME:
			msg.type = SYSCALL_RET;
			get_rtc_time(&t);
			phys_copy(va2la(src, msg.BUF),
				  va2la(TASK_SYS, &t),
				  sizeof(t));
			send_recv(SEND, src, &msg);
			break;
		case GET_PROC_INFO:	/* B级新增:供 ps 命令读取进程表 */
			msg.RETVAL = get_proc_info(src, msg.BUF, msg.BUF_LEN);
			msg.type = SYSCALL_RET;
			send_recv(SEND, src, &msg);
			break;
		default:
			panic("unknown msg type");
			break;
		}
	}
}


/*****************************************************************************
 *                                get_rtc_time
 *****************************************************************************/
/**
 * Get RTC time from the CMOS
 * 
 * @return Zero.
 *****************************************************************************/
PRIVATE u32 get_rtc_time(struct time *t)
{
	t->year = read_register(YEAR);
	t->month = read_register(MONTH);
	t->day = read_register(DAY);
	t->hour = read_register(HOUR);
	t->minute = read_register(MINUTE);
	t->second = read_register(SECOND);

	if ((read_register(CLK_STATUS) & 0x04) == 0) {
		/* Convert BCD to binary (default RTC mode) */
		t->year = BCD_TO_DEC(t->year);
		t->month = BCD_TO_DEC(t->month);
		t->day = BCD_TO_DEC(t->day);
		t->hour = BCD_TO_DEC(t->hour);
		t->minute = BCD_TO_DEC(t->minute);
		t->second = BCD_TO_DEC(t->second);
	}

	t->year += 2000;

	return 0;
}

/*****************************************************************************
 *                                read_register
 *****************************************************************************/
/**
 * Read register from CMOS.
 * 
 * @param reg_addr 
 * 
 * @return 
 *****************************************************************************/
PRIVATE int read_register(char reg_addr)
{
	out_byte(CLK_ELE, reg_addr);
	return in_byte(CLK_IO);
}

/*****************************************************************************
 *                                get_proc_info
 *****************************************************************************/
/**
 * <Ring 1> B级新增:把当前进程表(只含已分配的槽位)填充为 proc_info
 *          数组,拷贝到调用者(ps 命令)提供的缓冲区。
 *
 * @param dest_pid  调用者 PID(用于把内核数据拷到其地址空间)。
 * @param buf       调用者缓冲区(用户虚拟地址)。
 * @param buf_len   缓冲区字节数。
 *
 * @return 实际返回的进程条目数。
 *****************************************************************************/
PRIVATE int get_proc_info(int dest_pid, void* buf, int buf_len)
{
	struct proc_info info[NR_TASKS + NR_PROCS];
	int cnt = 0;
	struct proc* p;

	for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
		if (p->p_flags & FREE_SLOT)	/* 跳过空闲槽位 */
			continue;

		struct proc_info* pi = &info[cnt];
		pi->pid         = (int)(p - proc_table);
		memcpy(pi->name, p->name, 16);
		pi->name[15]    = 0;
		pi->p_flags     = p->p_flags;
		pi->sched_level = p->sched_level;
		pi->priority    = p->priority;
		pi->ticks       = p->ticks;
		pi->cpu_ticks   = p->cpu_ticks;
		pi->n_dispatch  = p->n_dispatch;
		pi->wait_ticks  = p->wait_ticks;
		cnt++;
	}

	int bytes = cnt * sizeof(struct proc_info);
	if (bytes > buf_len) {		/* 缓冲区不够则按容量截断 */
		cnt   = buf_len / sizeof(struct proc_info);
		bytes = cnt * sizeof(struct proc_info);
	}

	phys_copy(va2la(dest_pid, buf),
		  va2la(TASK_SYS, info),
		  bytes);

	return cnt;
}

