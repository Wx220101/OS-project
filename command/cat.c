/*************************************************************************//**
 * cat —— B级新增:显示文件内容
 *
 * 打开 argv[1] 指定的文件,循环读取并原样写到标准输出。
 *****************************************************************************/
#include "stdio.h"

int main(int argc, char * argv[])
{
	char buf[1024];
	int fd, n;

	if (argc < 2) {
		printf("Usage: cat <file>\n");
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("cat: cannot open '%s'\n", argv[1]);
		return 1;
	}

	while ((n = read(fd, buf, sizeof(buf))) > 0)
		write(1, buf, n);	/* 原样输出,二进制文件也安全 */

	close(fd);
	printf("\n");
	return 0;
}
