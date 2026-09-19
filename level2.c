#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

//改一下unsigned char的名字，减少代码量。
typedef unsigned char lchar;

// 定义一个类型用于存放一个字符
typedef struct
{
	// 所占屏幕宽度
	lchar Scr_Width;
	// 类型
	lchar bytes[4];
}CharInfo;

// 获取具体数据
int check_type(lchar c)
{
	if (c < 0x80) return 1;
    	if (c < 0xE0) return 2;
	if (c < 0xF0) return 3;
	return 4;
}

int get_width(lchar c)
{
	if (c < 0x80) return 1;
	return 2;
}

// 扩大接收数组
lchar* expand_arr(lchar *arr,int freq)
{
	lchar *new_arr = (lchar*)malloc(freq * 1024 * sizeof(lchar));
	if (new_arr == NULL)
	{
		return NULL;
	}
	memcpy(new_arr,arr,(freq - 1) * 1024);
	free(arr);
	return new_arr;
}

// 封装输出功能
// 需要：存储数组及其长度，屏幕大小参数,始输出坐标
// 为了实现光标不能移动到⽂本范围之外，还需要定义两个数组来存储每一行每一列的长度。
// 不对，首先只需要一个，一个y对应一个x。其次还不知道y轴总长，需要定义动态数组，所以在主函数先定义一个int ylinkx[1].
// 推翻，如果这样可能会导致实际行数不准确，所以实际记录时应该是先扩容再赋值，这样才能保证所有情况下行数都是对的。所以main函数里的ylink应该是个空指针
int show_txt(lchar *arr,int len,int x_max,int y_max,int start_index,int **ylinkx)
{
	// 擦除上一次的结果
	erase();

	// 基本量定义

	int row_len = 0;
	int column_len = 0;// 当前输出的x/y轴长度
	CharInfo string[1024]; // 预备输出数组
	int str_index = 0; // 预备输出数组当前下标
	int yx_len = 0; // ylinkx数组当前长度，需要返回

	// 遍历存储数组并输出内容
	for (int i = start_index;i <len;)
	{
		if (arr[i] == '\n') // 先判断换行符
		{
			// 输出预备数组已有内容
			for(int m = 0;m < str_index;m++)
			{
					char str[check_type(string[m].bytes[0]) + 1];
					str[check_type(string[m].bytes[0])] = '\0';
					for (int j = 0;j < check_type(string[m].bytes[0]);j++) // 输出数组每个元素的有效部分
					{
						// 输出元素
						str[j] = string[m].bytes[j];
					}
					printw("%s",str);
			}
			refresh();

			// 更新光标及其他基本量

			// 扩容ylinkx
			yx_len++;
			int *tmp = realloc(*ylinkx,yx_len * sizeof(int));
			if(tmp == NULL)
			{
					free(*ylinkx);
					perror("realloc failed");
					return -1;// 如果接收到-1则说明扩容失败
			}
			*ylinkx = tmp;

			// 更新ylinkx
			(*ylinkx)[column_len] = row_len;

			// 基本量更新
			row_len = 0;
			column_len++;
			str_index = 0;

			// 提前判断y轴有没有触底
			if(column_len >= y_max) break;

			// 防止最后一个字符为'\n'
			// 需要把判断是否触底提前，不然这种情况下yx_len会超过屏幕能显示的行数，因为在没有判断之前多加了一次。
			if ((i + 1) == len)
			{
				// 扩容ylinkx
				yx_len++;
				int *tmp = realloc(*ylinkx,yx_len * sizeof(int));
				if(tmp == NULL)
				{
					free(*ylinkx);
					perror("realloc failed");
					return -1;// 如果接收到-1则说明扩容失败
				}
				*ylinkx = tmp;

				// 更新ylinkx
				(*ylinkx)[column_len] = row_len;
			}
			i++;

			// 移动光标
			move(column_len,row_len);
			refresh();

			// 判断y轴有没有触底
			if(column_len >= y_max) break;

			continue; // 进入下一次循环
		}
		else
		{

			//判断当前字符类型并输入string
			string[str_index].Scr_Width = get_width(arr[i]);

			// 判断会不会触边
			row_len += string[str_index].Scr_Width; // 更新当前长度
			if (row_len >= x_max)
			{
				// 输出预备输出数组已有内容
				for(int m = 0;m < str_index;m++)
				{
					char str[check_type(string[m].bytes[0]) + 1];
					str[check_type(string[m].bytes[0])] = '\0';
					for (int j = 0;j < check_type(string[m].bytes[0]);j++) // 输出数组每个元素的有效部分
					{
						// 输出元素
						str[j] = string[m].bytes[j];
					}
					printw("%s",str);
				}
				refresh();

				// 更新光标及其他基本量
				
				// 扩容ylinkx
				yx_len++;
				int *tmp = realloc(*ylinkx,yx_len * sizeof(int));
				if(tmp == NULL)
				{
					free(*ylinkx);
					perror("realloc failed");
					return -1;
				}
				*ylinkx = tmp;

				 // 更新ylinkx
				(*ylinkx)[column_len] = row_len -= string[str_index].Scr_Width;

				// 更新其他基本量
				str_index = 0;
				row_len = 0;
				column_len++;

				// 移动光标
				move(column_len,row_len);

				// 判断y轴有没有触底
				if(column_len >= y_max) break;

				continue; // 进入下一次循环
			}

			// 存放字符数据
			int location = i; // 存放起始位置
			for(int j = 0;j < check_type(arr[i]);j++)
			{
				string[str_index].bytes[j] = arr[location];
				location++;
			}
			i += check_type(arr[i]);
			str_index++;
		}
	}

	// 由于如果最后一行可能没有换行符或者无法填满一行，会导致最后一行无法输出，故还需要额外判断
	if (str_index != 0)
	{
		// 输出剩余元素
		for(int m = 0;m < str_index;m++)
		{
			char str[check_type(string[m].bytes[0]) + 1];
			str[check_type(string[m].bytes[0])] = '\0';
			for (int j = 0;j < check_type(string[m].bytes[0]);j++) // 输出数组每个元素的有效部分
			{
				// 输出元素
				str[j] = string[m].bytes[j];
			}
			printw("%s",str);
		}
		refresh();


		// 扩容ylinkx
		yx_len++;
		int *tmp = realloc(*ylinkx,yx_len * sizeof(int));
		if(tmp == NULL)
		{
			free(*ylinkx);
			perror("realloc failed");
			return -1;
		}
		*ylinkx = tmp;

		// 更新ylinkx
		(*ylinkx)[column_len] = row_len;

		// 更新基本量
		str_index = 0;
		row_len = 0;
		column_len++;

		// 应该不用判断有没有触底了，如果后面发现bug来这看看
	}
	if (len == 0)
	{
		// 扩容ylinkx
		yx_len++;
		int *tmp = realloc(*ylinkx,yx_len * sizeof(int));
		if(tmp == NULL)
		{
			free(*ylinkx);
			perror("realloc failed");
			return -1;// 如果接收到-1则说明扩容失败
		}
		*ylinkx = tmp;
		(*ylinkx)[0]= 0;
	}
	return yx_len; // 文本的y轴长度
}

// 阅读文档，得到在当前窗口下的实际行数
// 返回实际行数，row_counter用来记录每一行有多少个字符
// 所以主体内容应该和show大差不差
// 改为存每一行首字节的下标
int read_txt(lchar *arr,int len,int x_max,int **row_counter)
{
	// 基本量定义
    int row_len = 0;
    int column_len = 0;// 当前输出的x/y轴长度
	int row_index = 1; 

	// 遍历存储数组并输出内容
	for (int i = 0;i <len;)
	{
		if (arr[i] == '\n') // 先判断换行符
		{
			// 扩容row_counter
			row_index++;
			int *tmp = realloc(*row_counter,row_index * sizeof(int));
			if(tmp == NULL)
			{
					free(*row_counter);
					perror("realloc failed");
					return -1;// 如果接收到-1则说明扩容失败
			}
			*row_counter = tmp;

			// 基本量更新
			row_len = 0;
			column_len++;
			i++;

			// 更新row_counter
			(*row_counter)[column_len] = i;

			continue; // 进入下一次循环
		}
		else
		{
			// 判断会不会触边
			row_len += get_width(arr[i]); // 更新当前长度
			if (row_len >= x_max)
			{
				// 扩容row_counter
				row_index++;
				int *tmp = realloc(*row_counter,row_index * sizeof(int));
				if(tmp == NULL)
				{
					free(*row_counter);
					perror("realloc failed");
					return -1;// 如果接收到-1则说明扩容失败
				}
				*row_counter = tmp;

				// 基本量更新
				row_len = 0;
				column_len++;
		        
				// 更新row_counter
				(*row_counter)[column_len] = i;
				continue; // 进入下一次循环
			}

			// 统计字符个数
			i += check_type(arr[i]);
			
		}
	}

	// 由于如果最后一行可能没有换行符或者无法填满一行，会导致最后一行无法输出，故还需要额外判断
	// 由于是统计每一行的起始位置，那这个就不用管了。
	return row_index; // 文本的y轴长度
}

int main(int argc,char *argv[])
{
	//初始化
	setlocale(LC_ALL,"");
	initscr();
	raw();
	keypad(stdscr,TRUE);
	set_escdelay(25);
	noecho();

	//// 基本量定义
	int x_max,y_max;
	getmaxyx(stdscr,y_max,x_max);
	int new_x_max,new_y_max;
	int *ylinkx = NULL; // 文本范围
	int *row_counter = (int*)malloc(1 * sizeof(int));
	row_counter[0] = 0;
	// 申请了几次数组
	int freq = 1;

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

		// 阅读文件
		// 1.level1.4的基本功能
		// 2.用户调整终端大小时能够实时调整显示范围
			// 思路：用动态分配内存的方式(链表不太合适，用数组，链表速度太慢）先将整个文本读取，并将调整前屏幕（0，0）处的文本记录，调整后重新根据屏幕大小从记录的（0，0）处开始输出文本。
			// 难点：1.如何判断是否x触边；2.如何处理中文和其他字符字节大小不同，导致的重新输出问题。
			// 注意：做的是软换行，所以只需要考虑y方向上的文本输出变换
		// 3.根据光标位置移动文本显示范围

		// 先初始化一个数组
		lchar *arr = (lchar*)malloc(1024 * sizeof(lchar));
		if (arr == NULL)
		{
			endwin();
			perror("malloc failed");
			return 1;
		}
		int n;
		// 数组当前有效长度
		int len = 0;
		// 开始填充数组的位置
		int Start = 0;
		while ((n = fread(&arr[Start],1,1024,file)) !=0)
		{
			if (n == 1024)
			{
				//扩大数组
				freq++;
				Start += 1024;
				len += 1024;
				lchar *tmp = expand_arr(arr,freq);

				// 判断是否扩容成功
				if (tmp == NULL)
				{
					endwin();
                        		perror("realloc failed");
                        		return 1;
				}
				arr = tmp;
			}
			else
			{
				// 读取文件完毕
				len += n;
			}
		}
		// 功能区域
		int ch;
		int x = 0;
		int y = 0;

		// 定义起始坐标量
		int Where_Start = 0;

		// 光标的实际行数
		int Cur_True_Location = 0;

		// 在执行while前得先输出一次，先把没初始化好的基本量初始化
		int yx_len = show_txt(arr,len,x_max,y_max,Where_Start,&ylinkx); // yx_len是最大行数
		int row_index = read_txt(arr,len,x_max,&row_counter);

		while ((ch = getch()) != 17)
		{
			// 监测用户是否改变窗口大小
			getmaxyx(stdscr,new_y_max,new_x_max);
			// 执行调整显示范围功能
			if((new_x_max != x_max) || (new_y_max != y_max))// 如果窗口大小改变
			{
				// 更新屏幕参数
				x_max = new_x_max;
				y_max = new_y_max;
				row_index = read_txt(arr,len,x_max,&row_counter);

				// 重新输出文本内容
				yx_len = show_txt(arr,len,x_max,y_max,Where_Start,&ylinkx);
			}
//			else if() // 存储文件功能键的实现
//			{
//
//			}
			else // 控制光标移动，重新输出文本内容
			{
				// 控制光标移动，记得改判断条件
				switch(ch) // 当光标超过边界时，需要重新输出文本内容
				{
				case KEY_UP: // 这样会不会直接插入中文内部啊？后面调试有bugger来这修
					if (y > 0)
					{
						y--;
						Cur_True_Location--;

						// 需要考虑不能超出文本范围，所以要判断x坐标要不要移动
						if (x > ylinkx[y]) x = ylinkx[y];
					}
					else // 继续往上，需要计算起始坐标量
					{
						// 计算,需要注意，还要判断是否在文本开头位置
						// 那还要加一个变量判断当前光标对应文本的行数
						// 还少了一个量用来记录有多少个字符，我真服了。
						// 否定，应该是要存字节
						if (Cur_True_Location == 0) break;
						Where_Start = row_counter[Cur_True_Location - 1];
						Cur_True_Location--;
						yx_len = show_txt(arr,len,x_max,y_max,Where_Start,&ylinkx);
						if (x > ylinkx[y]) x = ylinkx[y];
					}
					break;
				case KEY_DOWN: // 需要注意，如果已到文档底部，那用户再按下键应该没有任何操作.
					if (y < yx_len - 1)
					{
						y++;
						Cur_True_Location++;
					}// 所以刚刚定义的那个实际行数其实有问题。
					// 或许可以试试输出完后停止输出然后继续阅读？
					// 否认，这样每次调用函数都需要重新阅读，程序会冗杂。
					// 定义一个阅读函数。
					// 会存在一个如果最后一行为幻影行则光标会在屏幕丢失的问题，需要回到特判幻影行的位置解决。
					else if ((y == yx_len - 1) && (Cur_True_Location == (row_index - 1))) break;
					else if	((y == yx_len - 1) && (Cur_True_Location < (row_index - 1)))
					{
						Where_Start = row_counter[Cur_True_Location - yx_len + 2];
						Cur_True_Location++;
						yx_len = show_txt(arr,len,x_max,y_max,Where_Start,&ylinkx);
					}
					if (x > ylinkx[y]) x = ylinkx[y];
					break;
				case KEY_LEFT: // 可以实现通过左右键上下移动的功能，后面有空再写
					if (x > 0)x--;
					break;
				case KEY_RIGHT:
					if (x < ylinkx[y])x++;
					break;
				}
				move(y,x);
				refresh();

				// 移动显示区域
			}
		}
		free(arr);
		fclose(file);
	}
	else
	{
		// 提示用户没有传文件

		// 退出程序
		int ch;
		while ((ch = getch()) != 17)
		{

		}
	}
	free(ylinkx);
	free(row_counter);
	endwin();
	return 0;
}

