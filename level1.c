#include <ncurses.h>

int main()
{
	// 初始化
	initscr();
	raw();
	keypad(stdscr,TRUE);
	set_escdelay(25);
	noecho();

	// 功能实现
	int x,y;
	x = 0;
	y = 0;
	int ch;
	move(x,y);
	int x_max,y_max;
	getmaxyx(stdscr,y_max,x_max);
	// 移动光标
	// 尚未区分长短按
	while((ch = getch()) != 27)
	{
		switch(ch)
		{
			case KEY_UP:
				if (y > 0)y--;
				break;
			case KEY_DOWN:
				if (y < y_max)y++;
				break;
			case KEY_LEFT:
				if (x > 0)x--;
				break;
			case KEY_RIGHT:
				if (x < x_max)x++;
				break;
			default:
				if(y < y_max)
				{
					if(x < x_max)
					{
						mvaddch(y,x,ch);
						x++;
					}
					else{
						mvaddch(y,x,ch);
						x = 0;
						y++;
					}
				}
				break;
		}
		move(y,x);
		refresh();
	}

	// 退出
	endwin();
	return 0;
}
