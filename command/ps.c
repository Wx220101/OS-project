/*************************************************************************//**
 * ps —— B级新增:进程状态查看命令
 *
 * 通过 get_procs() 向内核 TASK_SYS 请求进程表快照,打印每个进程的:
 *   PID、名字、状态、所在 MLFQ 层级、基础优先级、剩余时间片、累计 CPU
 *   tick、被调度次数、累计等待 tick。
 *
 * 这是观察"多级反馈队列调度器"运行效果的窗口:CPU 密集型进程(TestA/B/C)
 * 会逐渐沉到较低层级(LVL 变大),交互型进程(shell)与系统任务则停留在
 * 第 0 层。
 *****************************************************************************/
#include "stdio.h"

/* 根据 p_flags 返回可读的状态字符串(与 const.h 中的标志位一致) */
static const char * state_str(int flags)
{
	if (flags == 0)        return "READY";
	if (flags & 0x02)      return "SEND";   /* SENDING   */
	if (flags & 0x04)      return "RECV";   /* RECEIVING */
	if (flags & 0x08)      return "WAIT";   /* WAITING   */
	if (flags & 0x10)      return "HANG";   /* HANGING   */
	return "?";
}

int main(int argc, char * argv[])
{
	struct proc_info procs[48];
	int n = get_procs(procs, 48);
	int i, k;
	char nm[16];

	printf("=== Orange'S process list (MLFQ scheduler) ===\n");
	printf("PID NAME           STATE   LVL PRIO  REM      CPU   DISP     WAIT\n");

	for (i = 0; i < n; i++) {
		struct proc_info * p = &procs[i];

		/* 名字列左对齐,固定 14 字符宽 */
		for (k = 0; k < 14; k++) nm[k] = ' ';
		nm[14] = 0;
		for (k = 0; k < 14 && p->name[k]; k++) nm[k] = p->name[k];

		printf("%3d %s%6s %3d %4d %4d %8d %6d %8d\n",
		       p->pid,
		       nm,
		       state_str(p->p_flags),
		       p->sched_level,
		       p->priority,
		       p->ticks,
		       p->cpu_ticks,
		       p->n_dispatch,
		       p->wait_ticks);
	}

	printf("Total %d procs. Higher LVL = lower priority; CPU/DISP/WAIT are cumulative.\n", n);

	return 0;
}
