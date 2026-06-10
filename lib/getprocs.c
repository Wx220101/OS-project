/*************************************************************************//**
 *****************************************************************************
 * @file   getprocs.c
 * @brief  B级新增:get_procs() —— ps 命令使用的用户态库函数。
 *
 * 通过消息机制向 TASK_SYS 请求 GET_PROC_INFO,内核把进程表填入用户提供
 * 的缓冲区,返回实际进程条目数。
 * @author OS 课程设计
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
#include "proto.h"


/*****************************************************************************
 *                                get_procs
 *****************************************************************************/
/**
 * 读取系统进程表。
 *
 * @param buf  用户缓冲区,用于接收 proc_info 数组。
 * @param max  缓冲区最多可容纳的 proc_info 条目数。
 *
 * @return 实际写入 buf 的进程条目数。
 *****************************************************************************/
PUBLIC int get_procs(struct proc_info * buf, int max)
{
	MESSAGE msg;
	msg.type    = GET_PROC_INFO;
	msg.BUF     = (void*)buf;
	msg.BUF_LEN = max * sizeof(struct proc_info);

	send_recv(BOTH, TASK_SYS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}
