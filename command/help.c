/*************************************************************************//**
 * help —— B级新增:列出可用命令(输出为 ASCII,VGA 文本模式无法显示中文)
 *****************************************************************************/
#include "stdio.h"

int main(int argc, char * argv[])
{
	printf("==================== Orange'S Shell Help ====================\n");
	printf(" Built-in commands (handled by the shell itself):\n");
	printf("   help            show this help\n");
	printf("   history         show command history\n");
	printf("   clear / cls     clear the screen\n");
	printf(" External commands (executables on disk):\n");
	printf("   ps              list processes & MLFQ scheduling state\n");
	printf("   echo <text>     echo the text\n");
	printf("   pwd             print working directory\n");
	printf("   cat <file>      print a file's content\n");
	printf("   clear           clear the screen\n");
	printf("   help            show this help\n");
	printf(" Tip: press Alt+F1/F2/F3 to switch virtual consoles.\n");
	printf("============================================================\n");
	return 0;
}
