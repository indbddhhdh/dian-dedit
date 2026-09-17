#include <ncurses.h>
#include <stdio.h>
#include <locale.h>

int main(int argc,char *argv[])
{
	// 初始化
	setlocale(LC_ALL,"");
	initscr();
	raw();
	keypad(stdscr,TRUE);
	set_escdelay(25);
	noecho();

	// 基本量定义
	int x_max,y_max;
        getmaxyx(stdscr,y_max,x_max);

	// 打开传入文件功能实现
	if (argc > 1)   // 当前仅需要打开一个文件
	{
		FILE *file = fopen(argv[1],"r");

		if (file == NULL)
		{
			// 文件不存在时 有专门的函数啊，不用printw
			endwin();
			perror("该文件不存在！");
			return 1;
		}
		// 还需要判断文件能不能读取：是否超过边界
		// 或许可以采取手动换行的操作来判断是否触底,需要把上一个版本的读取屏幕最大值提前。
		// 有点困难，先处理1.4基本任务


	// 阅读文件
		char arr[1025];
		// 储存一行的字符串
                char string[1025];
		int n;
		int Row_Location,Column_Location;
		while((n = fread(arr,1,1024,file)) != 0)
		{
			// 记录停下的位置
			arr[n] = '\0';
			int len = 0;
			for(int i = 0;i < n;i++)
			{
			// 不能用printf，要用printw,且printw需要refresh才能将内容刷新到屏幕
				if ( arr[i] == '\n')
				{
					// 一行结尾
					string[len] = '\0';
					// 此时需要移动光标到下一行，但需要知道此时行数。
					// printw能自动移动光标，那肯定有一个预先定义的光标坐标在实时变化，应该有一个函数能够得到
					// 有了，还需要一个变量来存储行数。
					printw("%s",string);
					len = 0;
					refresh();
					// 移动光标
					getyx(stdscr,Row_Location,Column_Location);
					Row_Location++;
					Column_Location = 0;
					move(Row_Location,Column_Location);
				}
				else
				{
					// 先储存进预备数组
					string[len] = arr[i];
					len++;
				}
			}
		}


		fclose(file);
	}
	// 关闭程序

	int ch;
	while(1)
	{
		if ((ch = getch()) == 17)
		{
			endwin();
			return 0;
		}
	}
}
