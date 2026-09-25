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
	if (c < 0xF5) return 4;
	else return -1;
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



// level2.2代码块内容
// 要求：1.实现一些基本的交互功能；2.保留方向键（应该是保留2.1的功能）
// 或许可拓展：既然有修改那就有保存吧，ctrl + s两键保存更改。
// 思路：这些交互本质上是修改内容。而修改了内容，要么是在show_txt阶段，要么就是直接修改了arr，显然后者更好实现。那么肯定有部分需要重新输出。但全部重新输出显然更简单。
// 首先否认修改arr为链表，输入输出时间成本过高，那依旧只能是数组。
// 交互可分为功能键和普通输入。后者需要定义一个变量来接收用户输入，并翻译成CharInfo数组。
// 绕不开中文，让用户忍一忍吧。。。

// 接收用户输入的字符，返回字符长度
// 还需要定义一个数组来接收
// int Get_Input(lchar **User_Input,int ch,int *Input_len)
// {
// 	Input_len++;

// 	// 扩容数组
// 	lchar *tmp = realloc(*User_Input,Input_len * sizeof(lchar));
// 	if(tmp == NULL)
// 	{
// 		free(*User_Input);
// 		perror("realloc failed");
// 		return -1;// 如果接收到-1则说明扩容失败
// 	}
// 	*User_Input = tmp;
// 	(*User_Input)[Input_len - 1] = (lchar)ch;
// } // 不对，好像不用这玩意

// 在光标位置插入字符
// 后移逻辑，所以起始输出位置不需要变，只需要挪arr,所以需要传入arr、有效长度、实际长度(用于判断是否需要扩容)
int InsertString(lchar **arr,int *len,int *freq,int Cur_True_Location,int **row_counter,int *row_index,int *x,int *y,int ch,int x_max,int y_max,int *Where_Start)
{
	// 先判断是否需要扩容
	while(*len + 1 > (*freq) * 1024)
	{
		(*freq)++;
		lchar *tmp = expand_arr(*arr,*freq); // 后续可优化成一个一个字符地扩大。

		// 判断是否扩容成功
		if (tmp == NULL)
		{
			endwin();
			perror("realloc failed");
			return -1;
		}
		*arr = tmp;
	}
	(*len)++;

	// 接下来需要挪动arr

	// 难点：如何知道插入位置右侧字符对应arr的首字节？

	// 方案A：扩展row_counter数组。新定义一个类，存储每个字符首字节对应arr的下标、在每一行的x轴位置、字节数，以及一个指向下个元素的指针。这样能够构成一个类似哈希表的结构，即数组+元素为链表
	// 否定，不用修改，因为row_counter已经提供每一行的定位作为锚点
	// 方案B:光标移动时先移动，再往右判断该字节是否有身份标签，若没有则继续往右找
	// 否定，首先这需要重新优化check_type函数，而且row_counter完全可以配合check_type,get_width来定位该字符的下标,这个方法更好。
	// 方案C：因为光标x轴坐标是用字符屏幕长度来定位的，所以可以以首元素首字节为基准，用get_width获得长度并累加，同时用chec_type来推进下标移动，当当前长度等于光标坐标时判正。
	// 好处：1.实现元素定位 2.光标移动能兼容中文。

	// 需要当前行数，光标x坐标，还有记录数组。更新参数列表,注意数组时需要更新的，还有文件最大行、光标坐标也需要更新

	// 记录光标右侧字符的首字节下标
	int Right_index = (*row_counter)[Cur_True_Location];
	int Right_Location = 0; // x轴坐标
	while (Right_Location != *x) // 记得更新光标移动逻辑，兼容中文
	{
		Right_Location += get_width((*arr)[Right_index]);
		Right_index += check_type((*arr)[Right_index]);
	}

	int move_index = *len - 1;

	// 移动数组
	while(move_index > Right_index) // 需要额外定义一个量作为当前移动下标
	{
		(*arr)[move_index] = (*arr)[move_index - 1];
		move_index--;
	}

	// 插入字符
	// 判断是否为换行符，等下先输出看看他的ascii值
	if (ch == 10)
	{
		(*arr)[Right_index] = '\n';
		*x = 0;
		if ((*y) < y_max - 1)
		{
			(*y)++;
			return 1;
		}
		else
		{
			return -2; // whereStart需要动,还有Cur_True_Location
		}
	}
	else (*arr)[Right_index] = (lchar)ch;

	// 更新x，y坐标，这个等下再实现
	// 由于输入可能会有中文，所以这部分放到外面。
	// 后续直接重建表+重新输出即可

	// 回来了，移动光标
	int length = check_type(ch);

	if (length == -1)return 0;// 非字符首字节
	else
	{
		if ((*x + get_width(ch) > x_max) && (*y == y_max - 1))// 还需要行长度和屏幕行最大值
		{
			*x = 0;
			return -2; // whereStart需要动
		}
		else if ((*x + get_width(ch) > x_max) && (*y < y_max))
		{
			*x = 0;
			(*y)++;
			return 1;
		}
		else
		{
			*x += get_width(ch);
			return -3;
		} 
	}
}

// 删除光标前字符
// 如果删除起始输出位置还需要更新Where_Start
// 还需要释放多余空间
// 测得ascii = 263
// 这里出现的bug最多，需要重新梳理一边逻辑
int Backspace(int *Where_Start,lchar **arr,int *freq,int *len,int Cur_True_Location,int **row_counter,int *x,int y)
{
	// 防止在文章开头
	if (((*x) == 0) && (Cur_True_Location == 0)) return -2;

	// 记录光标左侧字符的首字节下标
	int Left_index = (*row_counter)[Cur_True_Location];
	int Left_Location = 0; // x轴坐标
	while (1) // 记得更新光标移动逻辑，兼容中文
	{
		if((Left_Location + get_width((*arr)[Left_index])) < *x)
		{
			Left_Location += get_width((*arr)[Left_index]);
			Left_index += check_type((*arr)[Left_index]);
		}
		else break;
	}

	// 防止在一行开头
	if (((*x) == 0) && (Cur_True_Location > 0))
	{
		Left_index = (*row_counter)[Cur_True_Location - 1];
		Left_Location = 0;
	// 从上一行开始数

		while ((Left_index + check_type((*arr)[Left_index])) < (*row_counter)[Cur_True_Location]) // 要找到光标左侧的第一个字节的下标，需要注意是不是换行符 // 不对，就算是换行符也能正常运算
		{
				Left_Location += get_width((*arr)[Left_index]);
				Left_index += check_type((*arr)[Left_index]);
		}
	}

	// 记录被删去的字符
	int bk_width = get_width((*arr)[Left_index]);
	int bk_type = check_type((*arr)[Left_index]);

	int move_index = Left_index;
	while((move_index + bk_type) < (*len - 1)) // 只能是小于，要考虑删掉的是最后一个字符
	{
		// 因为arr在变化，所以不能简单用check_type(arr[Left_index]），要用别的变量记录下来
		(*arr)[move_index] = (*arr)[move_index + bk_type];
		move_index++;
	}

	// 这个释放需要放到移动之后，因为要确定要删掉多少字节，而且要防止内容丢失。
	// 判断是否需要释放,为避免计算混乱，一次释放1024个空间
	if(*len - bk_type < (*freq - 1) * 1024)
	{
		(*freq)--;
		lchar *tmp = realloc(*arr,(*freq) * 1024);
		if (tmp == NULL)
		{
			perror("malloc fail");
			return -1; // 报错信息
		}
		*arr = tmp;
	}
	*len -= bk_type;

	// 考虑到可能存在值冲突，正数部分全部由bk_width

	// 同样需要移动光标，后面再实现。
	if (((*x) == 0) && (Cur_True_Location > 0) && (y > 0))
	{
		*x = Left_Location;
		return -3;
	}
	else if (((*x) == 0) && (Cur_True_Location > 0) && (y == 0))
	{
		*Where_Start = (*row_counter)[Cur_True_Location - 1];
		*x = Left_Location;
		// y不用动
		return -4;
	}
	return bk_width; // 否则则按照字符长度变化
	// 以及是否需要修改Where_Start
}


// 删除光标后字符
// 测得ascii = 330
int Del(int *Where_Start,lchar **arr,int *freq,int *len,int Cur_True_Location,int **row_counter,int *x)
{
	// 记录光标右侧字符的首字节下标
	int Right_index = (*row_counter)[Cur_True_Location];
	int Right_Location = 0; // x轴坐标
	while (Right_Location != *x) // 记得更新光标移动逻辑，兼容中文
	{
		Right_Location += get_width((*arr)[Right_index]);
		Right_index += check_type((*arr)[Right_index]);
	}

	// 判断是否需要移动，防止光标后没字符
	if(Right_index == *len) return 1;

	// 记录被删去的字符
	int del_width = get_width((*arr)[Right_index]);
	int del_type = check_type((*arr)[Right_index]);


	// 移动逻辑和删除光标前相同

	int move_index = Right_index;
	while((move_index + del_type) < (*len - 1)) 
	{
		(*arr)[move_index] = (*arr)[move_index + del_type];
		move_index++;
	}

	if(*len - del_type < (*freq - 1) * 1024)
	{
		(*freq)--;
		lchar *tmp = realloc(*arr,(*freq) * 1024);
		if (tmp == NULL)
		{
			perror("malloc fail");
			return 1;
		}
		*arr = tmp;
	}

	*len -= del_type;
	return 0;

	// 同样需要移动光标，后面再实现。
	// 以及是否需要修改Where_Start

}

// 换行,本质是在指定位置加一个换行符
// 突然发现和插入是同一个函数

// 为了重写光标移动逻辑，需要封装一个判断x轴移哪的函数
void Where_x_Going_to(int *x,int Go_Which_Row,int current_row,int *row_counter,lchar *arr,int ch,int *ylinkx)
{
	int coord = 0;
	int assit_index = row_counter[Go_Which_Row];
	if ((Go_Which_Row != current_row) && ((ch == KEY_DOWN) || (ch == KEY_UP)))
	{
		while (coord < *x)
		{
			if (coord >= ylinkx[Go_Which_Row])
			{ 
				*x = ylinkx[Go_Which_Row];
				return;
			}
			coord += get_width(arr[assit_index]);
			assit_index += check_type(arr[assit_index]); 
		}
		*x = coord;
	}
	else if ((Go_Which_Row == current_row) && (ch == KEY_RIGHT))
	{
		while (coord <= *x)
		{
			coord += get_width(arr[assit_index]);
			assit_index += check_type(arr[assit_index]); 
		}
		*x = coord;
	}
	else
	{
		while (coord < *x)
		{
			if(coord + get_width(arr[assit_index]) == *x)
			{
				*x = coord;
				break;
			}
			coord += get_width(arr[assit_index]);
			assit_index += check_type(arr[assit_index]); 
		}
	}
}


// level2.3代码内容
// 需求：状态栏，包含当前编辑器状态，当前文件名，文件是否存在为保存修改，光标坐标
// 分析：当前编辑器状态是按下功能键进入，这个好说。而是否保存会涉及到每次修改都会进入未修改状态
// 思路：对switch环节的交互部分外面再套一层if，如果ch == 19（即ctrl + s）则为已保存，若为其他则未保存
// 思路其二：由于该状态栏不需要交互，故可以直接在屏幕最下方单独划一片区域直接打印输出。还需要注意当前窗口大小能不能支持打印输出。
// 所以似乎不需要额外的函数来实现？还是写一个吧。
int win_state(int y_max,int x_max,int x,int y,int ch,char** argv,int name_len,int is_change,int is_mode_change,int *choose_which)
{	
	// 判断需要显示的宽度
	// 用一个动态数组存储需要显示的内容，由于文件名在程序运行时已定，故只需要得到一次。所以这个数组定义在main函数内，先进行一次文件名获取。
	
	// if判断要进哪个模式
	// 等等，完全可以先规定各个内容在哪一块实现，这样状态栏长度可以固定（如果不考虑显示光标坐标）,那么就不需要动态数组存储需要显示的内容了。
	// 已知编辑器状态有NORMAL/SEARCH/RELPACE/UNSAVED这几种，则此处最多7个屏幕长度。而最后一种只需要在退出没保存时显示;默认NORMA。判断直接写后面那里
	char *mode[4] = {"NORMAL ","SEARCH ","REPLACE","UNSAVED "};
	// 通过下标·定义一个变量用来选择哪一个，默认为0，要写在函数外面
	if (is_mode_change == 1)
	{
		switch (ch)
		{
			case 6:
			{
				*choose_which = 1;
				break;
			}
			case 18:
			{
				*choose_which = 2;
				break;
			}
			case 17:
			{
				*choose_which = 3;
				break;
			}
			case 27:
			{
				*choose_which = 0;
				break;
			}
		}
	}

	if(is_change == 0)// 由于只有屏幕参数改变时才需要判断能不能输出，故用if分流
	{
		// 先判断当前屏幕大小能不能支持状态栏，所以需要传y_max & x_max
		// 得到x，y一共有几位	
		int cnt = 0;
		if (x == 0)cnt += 1;
		if (y == 0)cnt += 1;
		for(int num = x;num > 0;num /= 10) cnt++;
		for(int num = y;num > 0;num /= 10) cnt++;
		if ((y_max > 1) && (x_max >= (name_len + 1 + 7 + 1 + cnt + 1))) // 由于输出宽度不定，所以得先判断有多宽,1是“|”，“：”等符号
		{
			move(y_max-1, 0); 
			clrtoeol();
			// 还要维持上一次输出不变，引入一个新变量吧。
			mvprintw(y_max - 1,0,"%s|%s|%d:%d",argv[1],mode[*choose_which],x,y);
			return 0;
		}
		else
		{
			return -1;// 无法显示，什么都不做
		}
	}
	else if(is_change == 1)
	{
		move(y_max-1, 0); 
		clrtoeol();
		mvprintw(y_max - 1,0,"%s|%s|%d:%d",argv[1],mode[*choose_which],x,y);
		return 0;
	}
}

// 2.4代码内容
// 需求：按下 Ctrl-S 保存⽂件；
// 当⽂件被修改且⽤⼾未保存时，按下 Ctrl-Q 提⽰⽂件未保存；
// 再次按下强制退出不保存。

// 提示文件未保存等
// 思路：文件未保存时，用户需要按两次ctrl + q才能够强制退出，而如果保存了，则按一次足以。所以可以用一个外置的变量exit_time来记录这个“次数”
// 实现路径：当用户进行了交互时，exit_time置1，如果按下ctrl + q则exit_time+1;如果用户按下了ctrl + s，则手动将exit_time置2
// 所以while循环判断条件需要更改，只有当按下ctrl + q且exit_time为2时才退出。exit_time+1默认为2，因为刚进入时不需要保存。
// 也不行，这样按任意键会直接退出，还是把循环结束条件写进while里

// 保存文件实现
// 分析：首先提示文件是否保存，这个只有在退出文件时才可能需要提示。然后如果用户继续交互则退出到上一状态。（或许要把2.3的优化一下？按ESC退出到NORMAL）

// 等等，状态栏的模式代码优先级应该要大于一般交互代码，而退出代码的优先级应该也要大于状态栏模式代码。而且交互后要返回NORMAL。所以main函数里还需要多一层判断.



// 3.1代码内容
// 需求：输⼊ Ctrl-F 开启搜索功能，在搜索模式下：⽤⼾可以在状态栏上⾯⼀栏输⼊⽂字，作为搜索词汇；状态栏显⽰为搜索状态。按下 Enter 将搜索词汇匹配，⾼亮显⽰。
// 本质上是字母串匹配，可以试试数据结构课上的KMP算法
// 需要定义两个函数，一个获取模式串，一个匹配
// 如果想让开启搜索模式后能单独在倒数第二行输入，还需要有一个判断来强制把用户的输入拿走

// 获得用户输入
// 考虑到屏幕大小限制，需要限制用户输入的内容长度,预先定义一个数组，然后限制条件为数组下标长度和屏幕宽度
// 模式串要和arr匹配，所以要是lchar类型的，而且需要获得模式串长度
void get_user_input(int ch,lchar *pattern_string,int pattern_general_len,int *pattern_str_len,int *pattern_str_width,int x_max,int y_max)
{
	// 判断是不是首字节
	if (check_type(ch) != -1)
	{
		// 判断能否装下
		if ((*pattern_str_width + get_width(ch)) > x_max) // 超出屏幕宽度
		{
			return;
		}
		else
		{
			if ((*pattern_str_len + check_type(ch)) > (pattern_general_len)) // 超出数组宽度
			{
				return;
			}
			else
			{
				*pattern_str_len += 1;
				pattern_string[*pattern_str_len - 1] = (lchar)ch;
				*pattern_str_width += get_width(ch);
			}
		}
	}
	else // 直接输入
	{
		*pattern_str_len += 1;
		pattern_string[*pattern_str_len - 1] = ch;
	}
	

	// 将当前已有的输出到倒数第二行
	if (*pattern_str_len > 0) // 判断有没有东西
	{
		move(y_max - 2,0);
		clrtoeol();
		int start_index = 0;
		while ((start_index + check_type(pattern_string[start_index]) <= *pattern_str_len))
		{
			char output_string[check_type(pattern_string[start_index]) + 1];
			output_string[check_type(pattern_string[start_index])] = '\0';
			for (int i = 0;i < check_type(pattern_string[start_index]);i++) // 把要输出的字符先存到output_string
			{
				output_string[i] = pattern_string[start_index + i];
			}
			printw("%s",output_string);
			start_index += check_type(pattern_string[start_index]);
		}
		refresh();
	}
}

// 生成next
void compute_next(char *pattern_string,int pattern_str_len, int *next) 
{
    next[0] = 0;
    int k = 0;
    for (int i = 1;i < pattern_str_len;i++) 
	{
        while (k > 0 && pattern_string[k] != pattern_string[i]) k = next[k - 1];
        if (pattern_string[k] == pattern_string[i]) k++;
        next[i] = k;
    }
}


// kmp匹配
int kmp_search(char *arr,int len,char *pattern_string,int pattern_str_len,int *Where_Start_Search) 
{
    int next[pattern_str_len];
    compute_next(pattern_string, pattern_str_len, next);

    int j = 0;
    for (int i = *Where_Start_Search; i < len; i++) 
	{
        while (j > 0 && arr[i] != pattern_string[j]) j = next[j - 1];
        if (arr[i] == pattern_string[j]) j++;
        if (j == pattern_str_len) return i - pattern_str_len + 1; // 匹配位置
    }
    return -1; // 未找到
}

// 借鉴的代码真好用（

// 实现基本功能：
// 打开搜索功能后光标要移动到倒数第二行，并且倒数第二行要清空。
// 再次按下enter后要切换到下一个候选，本质是从匹配位置末尾继续找？从匹配下标的下一个开始吧。
void find_matching(int *Where_Start_Search,lchar *pattern_string,int pattern_str_len,lchar *arr,int len,int *row_counter,int row_index,int x_max,int y_max,int *Where_Start,int **ylinkx,int txt_y,int *yx_len)
{
	// 按下回车找下一个，但是第一次回车是进入搜索，第二次之后才是找下一个，所以还需要一个量来记录。哦，不需要。但是需要有一个量来决定每次开始找的位置，并且这个位置不能超出文本
	int find_location = kmp_search(arr,len,pattern_string,pattern_str_len,Where_Start_Search);
	if (find_location == -1) return;// 什么都不做

	// 修改下一次开始找的位置
	if ((find_location + check_type(arr[find_location])) > (len - 1)){} // 超过数组长度，什么都不做
	else *Where_Start_Search = find_location + check_type(arr[find_location]);

	// 找到这个下标在哪,将画面切到这里.先找到从第几行开始,所以还需要每行下标的代码
	int row = 0;
	while ((row_counter[row] <= find_location) && ((row + 1) <= (row_index - 1)))
	{
		if (row_counter[row + 1] > find_location) break;
		row++;
	}

	// 确定如何重新显示，首先需要屏幕高度不小于三，这个得趁早判断.
	// 如果进入search后屏幕大小改变导致相关界面无法显示，那内置mode是回到normal还是保留呢？回到normal吧。
	// 还要注意是否触底，优先直接从该行开始显示。
	// 光标跳转到所在行，如果要重新搜索则先退出再重新进入search

	// 判断是否触底
	if ((row + txt_y - 1) < row_index)
	{
		*Where_Start = row_counter[row];
		*yx_len = show_txt(arr,len,x_max,txt_y,*Where_Start,ylinkx);
		move(0,0);
	}
	else
	{
		if ((row_index - txt_y) > 0)
		{
			*Where_Start = row_counter[row_index - txt_y];
			*yx_len = show_txt(arr,len,x_max,txt_y,*Where_Start,ylinkx);
			move(txt_y - row_index - row,0);
	
		}
		else
		{
			*Where_Start = 0;
			*yx_len = show_txt(arr,len,x_max,txt_y,*Where_Start,ylinkx);
			move(row,0);
		}
	}
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
			// 由于是软换行，只有在涉及y方向时才需要考虑文本的变换输出
			// 由于中文占两个屏幕长度单位，所以光标移动还需要考虑中文。不过之后再说吧，万一能自动跳呢？

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
				// 补丁：判断文件是否读完
				if (feof(file))
				{
					len += 1024;
					break;
				} //不能直接break，这会导致有1kb的长度没记录。

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
		fclose(file);

		// 功能区域
		int ch;
		int x = 0;
		int y = 0;

		// 定义起始坐标量
		int Where_Start = 0;

		// 光标的实际行数
		int Cur_True_Location = 0;

		// 功能键统一回收
		int inter;

		// // 用户输入量
		// lchar *User_Input = NULL;
		// int	Input_len = 0;

		// 状态栏相关量，获取文件名
		int name_len = strlen(argv[1]); // 先不考虑中文的情况
		int choose_which = 0;

		int txt_y = y_max;

		// 2.4相关变量
		int exit_time = 2;

		// 3.1相关变量
		lchar pattern_string[256];
		int pattern_general_len = 256;
		int pattern_str_len = 0;
		int pattern_str_width = 0;
		int force_search = 0; // 控制强制锁定在search模式的变量
		int Where_Start_Search = 0;

		// 在执行while前得先输出一次，先把没初始化好的基本量初始化
		inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
		if (inter == 0)
		{
			txt_y = y_max - 1;
		}
		int yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx); // yx_len是最大行数
		if (yx_len == -1)
		{
			endwin();
			perror("realloc");
			return 1;
		}
		int row_index = read_txt(arr,len,x_max,&row_counter);
		if (row_index == -1)
		{
			endwin();
			perror("realloc");
			return 1;
		}
		inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);

		move(0,0);

		while (1)
		{
			// 获得用户输入
			ch =getch();
			// 监测用户是否改变窗口大小
			getmaxyx(stdscr,new_y_max,new_x_max);
			// 执行调整显示范围功能
			if((new_x_max != x_max) || (new_y_max != y_max))// 如果窗口大小改变
			{
				// 更新屏幕参数
				x_max = new_x_max;
				y_max = new_y_max;
				inter = win_state(y_max,x_max,x,y,ch,argv,name_len,1,0,&choose_which);
				if (inter == 0)
				{
					txt_y = y_max - 1;

					if (y_max < 3)
					{
						win_state(y_max,x_max,x,y,27,argv,name_len,0,1,&choose_which);
						force_search = 0;
						pattern_str_len = 0;
						pattern_str_width = 0;
					}
				}
				else
				{
					txt_y = y_max;

					// 搜索相关量需要重置
					win_state(y_max,x_max,x,y,27,argv,name_len,0,1,&choose_which);
					force_search = 0;
					pattern_str_len = 0;
					pattern_str_width = 0;
				}
				row_index = read_txt(arr,len,x_max,&row_counter);
				if (row_index == -1)
				{
					endwin();
					perror("realloc");
					return 1;
				}

				// 重新输出文本内容
				yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
				if (yx_len == -1)
				{
					endwin();
					perror("realloc");
					return 1;
				}

				// 补丁：光标显示坐标
				if (y >txt_y - 1)
				{
					y = txt_y - 1;
					if (x > ylinkx[y])x = ylinkx[y];
				}
				if (x > ylinkx[y])x = ylinkx[y];

				inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
			}
			else if ((force_search == 1) && (ch != 27))
			{
				if (ch != 10) // 判断有没有按下回车，如果回车则开始匹配并返回
				{
					get_user_input(ch,pattern_string,pattern_general_len,&pattern_str_len,&pattern_str_width,x_max,y_max);
				}
				else
				{
					find_matching(&Where_Start_Search,pattern_string,pattern_str_len,arr,len,row_counter,row_index,x_max,y_max,&Where_Start,&ylinkx,txt_y,&yx_len);
				}
			}
			else // 状态栏代码
			{	
				if ((ch == 17) && (exit_time ==2)) break;
				switch(ch)
				{
					case 6:
						if(y_max < 3)break;
						txt_y -= 1;
						yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
						move(y_max - 2,0);
						win_state(y_max,x_max,x,y,ch,argv,name_len,0,1,&choose_which);
						force_search = 1;
						break;
					case 18:
						win_state(y_max,x_max,x,y,ch,argv,name_len,0,1,&choose_which);
						break;
					case 17: // 退出
						win_state(y_max,x_max,x,y,ch,argv,name_len,0,1,&choose_which);
						exit_time = 2; // 再按下则强制退出
						break;
					case 19: // 保存文件
						exit_time = 2;
						// 用于保存的临时副本文件
						FILE *fp = fopen("copies", "w");
						if (fp == NULL)
						{
							endwin();
							perror("copies_create");
							return -1;
						}
						size_t written = fwrite(arr,1,len,fp);
						if (written != (size_t)len)
						{
							endwin();
							perror("copies_written");
							return -1;
						}
						if (fclose(fp) == EOF)
						{
							endwin();
							perror("copies_close_fail");
							return -1;
						}
						rename("copies",argv[1]);

						break;
					case 27:
						if (force_search == 1)// 如果不加的话txt_y会一直增长
						{
							// 让文件回到开头
							force_search = 0;
							txt_y += 1;
							pattern_str_len = 0;
							pattern_str_width = 0;
							Where_Start_Search = 0;
							Where_Start = 0;
							yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
						}
						win_state(y_max,x_max,x,y,ch,argv,name_len,0,1,&choose_which);
						break;
					default: // 控制光标移动，重新输出文本内容
					{
						// 回到NORMAL状态
						win_state(y_max,x_max,x,y,27,argv,name_len,0,1,&choose_which);
						// 光标移动不会修改exit_time

						// 控制光标移动，记得改判断条件
						switch(ch) // 当光标超过边界时，需要重新输出文本内容
						{
							case KEY_UP: // 这样会不会直接插入中文内部啊？后面调试有bug来这修——看来是会的。
								if (y > 0)
								{
									y--;
									Cur_True_Location--;

									// 需要考虑不能超出文本范围，所以要判断x坐标要不要移动
									if (x > ylinkx[y]) x = ylinkx[y];
									else Where_x_Going_to(&x,Cur_True_Location,Cur_True_Location + 1,row_counter,arr,ch,ylinkx);
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
									yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
									if (yx_len == -1)
									{
										endwin();
										perror("realloc");
										return 1;
									}
									if (x > ylinkx[y]) x = ylinkx[y];
									else Where_x_Going_to(&x,Cur_True_Location,Cur_True_Location + 1,row_counter,arr,ch,ylinkx);
								}
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
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
									yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
									if (yx_len == -1)
									{
										endwin();
										perror("realloc");
										return 1;
									}
								}
								if (x > ylinkx[y]) x = ylinkx[y];
								else Where_x_Going_to(&x,Cur_True_Location,Cur_True_Location - 1,row_counter,arr,ch,ylinkx);
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
								break;
							case KEY_LEFT: // 还需要写水平移动的功能
								if (x > 0)Where_x_Going_to(&x,Cur_True_Location,Cur_True_Location,row_counter,arr,ch,ylinkx);
								else// 需要先判断是否顶格
								{
									if (y > 0)
									{
									x = ylinkx[y - 1];
									y--;
									Cur_True_Location--;
									}
									else
									{
										if (Cur_True_Location == 0) break;
										Where_Start = row_counter[Cur_True_Location - 1];
										Cur_True_Location--;
										yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
										if (yx_len == -1)
										{
											endwin();
											perror("realloc");
											return 1;
										}
										x = ylinkx[y];
									}
								}
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
								break;
							case KEY_RIGHT:
								if (x < ylinkx[y])Where_x_Going_to(&x,Cur_True_Location,Cur_True_Location,row_counter,arr,ch,ylinkx);
								else
								{
									if (y < (yx_len - 1))
									{
										y++;
										Cur_True_Location++;
									}
									else if ((y == yx_len - 1) && (Cur_True_Location == (row_index - 1))) break;
									else if	((y == yx_len - 1) && (Cur_True_Location < (row_index - 1)))
									{
										Where_Start = row_counter[Cur_True_Location - yx_len + 2];
										Cur_True_Location++;
										yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
										if (yx_len == -1)
										{
											endwin();
											perror("realloc");
											return 1;
										}
									}
									x = 0;
								}
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
								break;
							case 263:
								inter = Backspace(&Where_Start,&arr,&freq,&len,Cur_True_Location,&row_counter,&x,y);
								if (inter == -1)
								{
									endwin();
									perror("Backspace");
									return 1;
								}
								if (inter == -2)break;
								
								// 更新文本参数
								row_index = read_txt(arr,len,x_max,&row_counter);

								// 光标移动
								if (inter == -3)// 不需要改变wherestart
								{
									y--;
									Cur_True_Location--;
									yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
									if (yx_len == -1)
									{
										endwin();
										perror("realloc");
										return 1;
									}
								}
								else if(inter == -4)
								{
									Cur_True_Location--;
									yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
									if (yx_len == -1)
									{
										endwin();
										perror("realloc");
										return 1;
									}
								}
								else
								{
									x -= inter;
									yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
									if (yx_len == -1)
									{
										endwin();
										perror("realloc");
										return 1;
									}
								}
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
								exit_time = 1;
								break;
							case 330:
								// 删后面的应该不用移动光标
								Del(&Where_Start,&arr,&freq,&len,Cur_True_Location,&row_counter,&x);
								row_index = read_txt(arr,len,x_max,&row_counter);
								yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
								if (yx_len == -1)
									{
										endwin();
										perror("realloc");
										return 1;
									}
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
								exit_time = 1;
								break;
							default:
								inter = InsertString(&arr,&len,&freq,Cur_True_Location,&row_counter,&row_index,&x,&y,ch,x_max,txt_y,&Where_Start);

								if (inter == -1)
								{
									endwin();
									perror("InsertString");
									return 1;
								}
								row_index = read_txt(arr,len,x_max,&row_counter);

								// 光标移动
								// 如果输入长文本咋整？要先整一个预输入数组吗？——学到一个新知识，只有首字节有身份标签,果然还是得修改check_type的逻辑
								if (inter == -2)
								{
									Where_Start = row_counter[Cur_True_Location - yx_len + 2];
									Cur_True_Location++;
									y++;
								}

								yx_len = show_txt(arr,len,x_max,txt_y,Where_Start,&ylinkx);
								if (yx_len == -1)
								{
									endwin();
									perror("realloc");
									return 1;
								}
								inter = win_state(y_max,x_max,x,y,ch,argv,name_len,0,0,&choose_which);
								exit_time = 1;
								break;
						}
						// 移动显示区域
					}
				}	
			}
			// 注意要先调用win_state再移动光标，否则光标会锁死在状态栏末尾
			if(force_search == 0)move(y,x);
			refresh();
		}
		free(arr);
	}
	else
	{
		// 提示用户没有传文件
		printw("当前没有文件传入");

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

