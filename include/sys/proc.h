
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


struct stackframe {	/* proc_ptr points here				↑ Low			*/
	u32	gs;		/* ┓						│			*/
	u32	fs;		/* ┃						│			*/
	u32	es;		/* ┃						│			*/
	u32	ds;		/* ┃						│			*/
	u32	edi;		/* ┃						│			*/
	u32	esi;		/* ┣ pushed by save()				│			*/
	u32	ebp;		/* ┃						│			*/
	u32	kernel_esp;	/* <- 'popad' will ignore it			│			*/
	u32	ebx;		/* ┃						↑栈从高地址往低地址增长*/		
	u32	edx;		/* ┃						│			*/
	u32	ecx;		/* ┃						│			*/
	u32	eax;		/* ┛						│			*/
	u32	retaddr;	/* return address for assembly code save()	│			*/
	u32	eip;		/*  ┓						│			*/
	u32	cs;		/*  ┃						│			*/
	u32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	u32	esp;		/*  ┃						│			*/
	u32	ss;		/*  ┛						┷High			*/
};


struct proc {
	struct stackframe regs;    /* process registers saved in stack frame */

	u16 ldt_sel;               /* gdt selector giving ldt base and limit */
	struct descriptor ldts[LDT_SIZE]; /* local descs for code and data */

        int ticks;                 /* remained ticks */
        int priority;

	/* u32 pid;                   /\* process id passed in from MM *\/ */
	char name[16];		   /* name of the process */

	int  p_flags;              /**
				    * process flags.
				    * A proc is runnable iff p_flags==0
				    */

	MESSAGE * p_msg;
	int p_recvfrom;
	int p_sendto;

	int has_int_msg;           /**
				    * nonzero if an INTERRUPT occurred when
				    * the task is not ready to deal with it.
				    */

	struct proc * q_sending;   /**
				    * queue of procs sending messages to
				    * this proc
				    */
	struct proc * next_sending;/**
				    * next proc in the sending
				    * queue (q_sending)
				    */

	int p_parent; /**< pid of parent process */

	int exit_status; /**< for parent */

	struct file_desc * filp[NR_FILES];

	/*=====================================================================*
	 * B 级新增字段:多级反馈队列(MLFQ)调度 + 调度统计                  *
	 *---------------------------------------------------------------------*
	 * 注意:这些字段必须追加在 struct proc 的【末尾】。                   *
	 * 因为 sconst.inc / kernel.asm 对 regs、ldt_sel、ldts 的偏移做了硬    *
	 * 编码(P_STACKBASE=0, P_LDT_SEL=76, P_LDT=80),在它们之前插入字段  *
	 * 会破坏汇编保存/恢复现场的逻辑;追加到末尾则完全安全。              *
	 *=====================================================================*/
	int  sched_level;	/**< 当前所在 MLFQ 层级,0 为最高优先级 */
	int  is_user;		/**< 1=用户进程(参与时间片降级),0=系统任务 */
	u32  cpu_ticks;		/**< 累计占用 CPU 的 tick 数(统计) */
	u32  n_dispatch;	/**< 被调度器选中运行的累计次数(统计) */
	u32  wait_ticks;	/**< 处于就绪态但未获得 CPU 的累计 tick(统计) */
};

struct task {
	task_f	initial_eip;
	int	stacksize;
	char	name[32];
};

#define proc2pid(x) (x - proc_table)

/* Number of tasks & processes */
#define NR_TASKS		5
#define NR_PROCS		32
#define NR_NATIVE_PROCS		4
#define FIRST_PROC		proc_table[0]
#define LAST_PROC		proc_table[NR_TASKS + NR_PROCS - 1]

/*===========================================================================*
 *          B 级新增:多级反馈队列(MLFQ)调度器配置参数                    *
 *===========================================================================*
 * 设计要点:
 *   - 共 NR_SCHED_LEVELS 个优先级队列,0 层最高、数字越大优先级越低。
 *   - 新进程从第 0 层开始;一旦用完整个时间片(说明它是 CPU 密集型)就被
 *     降到下一层(SCHED_QUANTUM 随层级翻倍,低层进程一次跑更久但更少被选)。
 *   - 主动放弃 CPU(阻塞在收发消息/读键盘)的进程不会被降级,因此交互型
 *     进程(shell)始终停留在高层,保证响应速度。
 *   - 每隔 SCHED_BOOST_TICKS 个 tick 把所有进程提回第 0 层(优先级老化),
 *     防止低层的 CPU 密集型进程被饿死(MLFQ 规则 5)。
 *===========================================================================*/
#define NR_SCHED_LEVELS		4	/* MLFQ 层级数:0(最高) ~ 3(最低) */
#define SCHED_BASE_QUANTUM	2	/* 第 0 层的时间片(单位:tick) */
/* 第 lvl 层的时间片:2、4、8、16 个 tick(越低优先级时间片越长) */
#define SCHED_QUANTUM(lvl)	(SCHED_BASE_QUANTUM << (lvl))
#define SCHED_BOOST_TICKS	300	/* 每隔多少 tick 做一次全局优先级提升 */

/**
 * All forked proc will use memory above PROCS_BASE.
 *
 * @attention make sure PROCS_BASE is higher than any buffers, such as
 *            fsbuf, mmbuf, etc
 * @see global.c
 * @see global.h
 */
#define	PROCS_BASE		0xA00000 /* 10 MB */
#define	PROC_IMAGE_SIZE_DEFAULT	0x100000 /*  1 MB */
#define	PROC_ORIGIN_STACK	0x400    /*  1 KB */

/* stacks of tasks */
#define	STACK_SIZE_DEFAULT	0x4000 /* 16 KB */
#define STACK_SIZE_TTY		STACK_SIZE_DEFAULT
#define STACK_SIZE_SYS		STACK_SIZE_DEFAULT
#define STACK_SIZE_HD		STACK_SIZE_DEFAULT
#define STACK_SIZE_FS		STACK_SIZE_DEFAULT
#define STACK_SIZE_MM		STACK_SIZE_DEFAULT
#define STACK_SIZE_INIT		STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTA	STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTB	STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTC	STACK_SIZE_DEFAULT

#define STACK_SIZE_TOTAL	(STACK_SIZE_TTY + \
				STACK_SIZE_SYS + \
				STACK_SIZE_HD + \
				STACK_SIZE_FS + \
				STACK_SIZE_MM + \
				STACK_SIZE_INIT + \
				STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC)

