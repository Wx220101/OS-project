# Orange'S — B 级操作系统课程设计

在《Orange'S:一个操作系统的实现》随书源码(`chapter11/c` 最终版)基础上,**把简单的单级优先级调度器重写为多级反馈队列(MLFQ)调度器**,并配套实现进程查看命令、增强 Shell、新系统调用与若干系统命令。

> 📖 **完整文档**:[`docs/项目文档.md`](docs/项目文档.md)

## 原创工作一览

| # | 模块 | 内容 |
|---|------|------|
| 1 | **进程调度器(核心)** | 重写为 4 级 MLFQ:时间片用尽降级、主动让出不降级、周期性优先级老化防饿死、每进程调度统计 |
| 2 | **系统调用 + 命令** | 新增 `GET_PROC_INFO` 系统调用与 `ps` 命令,可视化调度状态 |
| 3 | **Shell 增强** | 欢迎横幅、命令历史、`history`/`clear`/`cls`/`exit` 内建命令 |
| 4 | **控制台驱动** | 实现 `\f` 清屏,新增 `clear` 命令 |
| 5 | **命令套件** | `help`、`cat` |
| 6 | **Bug 修复** | 修复文件系统 `do_rdwt()` 读越界缺陷 |

新增/改动代码约 **492 行**;调度子系统新增约 120 行,远超原模块(约 26 行)的一半,满足 B 级要求。

## 环境

- WSL2 + Ubuntu 24.04
- `build-essential gcc-multilib nasm qemu-system-x86 mtools netpbm bc`

详见文档[第 2 节](docs/项目文档.md#2-开发环境搭建)。

## 构建与运行

```bash
# 完整构建(内核 + 命令 + 磁盘镜像)
bash build_all.sh

# 仅重建内核(快速迭代)
bash build_kernel.sh

# 运行
qemu-system-i386 -m 32 -boot a \
  -drive file=a.img,if=floppy,format=raw \
  -drive file=100m.img,format=raw,media=disk
```

> 首次启动会 `mkfs` 并解包命令文件(受 QEMU IDE PIO 时延影响约 1~2 分钟,**仅一次**,之后启动数秒)。
> 进入系统后按 **Alt+F2** 切到 Shell,输入 `help` 看命令、`ps` 看调度。

## 关键源码位置

| 功能 | 文件 |
|------|------|
| MLFQ 调度器 | `kernel/proc.c`(`schedule`/`sched_boost`)、`kernel/clock.c`、`include/sys/proc.h` |
| ps 系统调用 | `kernel/systask.c`、`lib/getprocs.c`、`command/ps.c`、`include/stdio.h` |
| Shell 增强 | `kernel/main.c`(`shabby_shell`) |
| 清屏 | `kernel/console.c`、`command/clear.c` |
| FS 修复 | `fs/read_write.c` |

> 本仓库基于 Forrest Yu《Orange'S:一个操作系统的实现》随书源码二次开发,仅用于课程设计学习目的。
