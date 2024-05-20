#include "shell.h"

extern task_struct cur_task;

uint8_t other_end = 0;

// 函数注册，添加COMMAND即可
// 格式：触发词长度，调用函数，触发词

void rvhelp(char *args)
{
	args++;
	printf("my is help\n");
	printf("create: create a new process\n");
	printf("   test: create a test process\n");
	printf("bg: create a new process in background\n");
	printf("   pab: create a process print a and b\n");
	printf("   hanoi: create a process hanoi\n");
	printf("   life: create a process game of life\n");
	printf("   tian: create a process donut\n");

}

void rvcreate(char *args)
{
	extern int progress1(int argc, char **argv);
	args++;
	// 从字符串中读取要创建的进程名，然后调用创建的函数
	if (memcmp(args, "test", 4) == 0)
	{
		progress1(0, NULL);	
	}

	if (memcmp(args, "pab", 3) == 0)
	{
		printaandb();	
	}

	if (memcmp(args, "hanoi", 5) == 0)
	{
		hanoi();	
	}

	if (memcmp(args, "life", 4) == 0)
	{
		game_of_life();	
	}	

	if (memcmp(args, "tian", 4) == 0)
	{
		donut();	
	}		
}

void bgcreate(char *args)
{
	extern int progress0(int argc, char **argv);
	extern int progress1(int argc, char **argv);
	int task_create(void* start_routin, uint32_t counter, uint32_t priority, uint32_t tty, uint32_t father, int argc, char **argv);
	args++;
	// 从字符串中读取要创建的进程名，然后调用创建的函数
	if (memcmp(args, "pro", 3) == 0)
	{
		// 创建进程
		//progress1(0, NULL);	
		task_create(progress1, 5, 0, 0, cur_task.pid, 0, NULL);
	}

	if (memcmp(args, "pab", 3) == 0)
	{
		task_create(printaandb, 5, 0, 0, cur_task.pid, 0, NULL);
	}

	if (memcmp(args, "hanoi", 5) == 0)
	{
		task_create(hanoi, 5, 0, 0, cur_task.pid, 0, NULL);
	}

	if (memcmp(args, "life", 4) == 0)
	{
		task_create(game_of_life, 5, 0, 0, cur_task.pid, 0, NULL);	
	}	

	if (memcmp(args, "tian", 4) == 0)
	{
		task_create(donut, 5, 0, 0, cur_task.pid, 0, NULL);	
	}		
}

void quit(char *args)
{
	args++;
	// 从字符串中读取要创建的进程名，然后调用创建的函数
	other_end = 1;	
}

void set_option(char *args)
{
	char *tmp = args;
	int opsize;
	int size;

	// rvhelp
	size = 0;
	args = tmp;
	opsize = 4;
	while (*args != '\0' && *args != ' ')
	{
		size++;
		args++;
	}
	args = tmp;
	if ((size == opsize) && (memcmp(args, "help", size) == 0))
	{
		args = args + size;
		rvhelp(args);
		return;
	}

	// rvcreate
	size = 0;
	args = tmp;
	opsize = 6;
	while (*args != '\0' && *args != ' ')
	{
		size++;
		args++;
	}
	args = tmp;
	if ((size == opsize) && (memcmp(args, "create", size) == 0))
	{
		args = args + size;
		rvcreate(args);
		return;
	}

	// bgcreate 后台创建应用
	size = 0;
	args = tmp;
	opsize = 2;
	while (*args != '\0' && *args != ' ')
	{
		size++;
		args++;
	}
	args = tmp;
	if ((size == opsize) && (memcmp(args, "bg", size) == 0))
	{
		args = args + size;
		bgcreate(args);
		return;
	}

	// quit 退出所有其他应用
	size = 0;
	args = tmp;
	opsize = 4;
	while (*args != '\0' && *args != ' ')
	{
		size++;
		args++;
	}
	args = tmp;
	if ((size == opsize) && (memcmp(args, "quit", size) == 0))
	{
		args = args + size;
		quit(args);
		return;
	}
	

	printf("command not found\n");
}

int shell_loop(int argc, char **argv)
{
	while (1)
	{
		printf("zbc@rvos: ");
		char s[500];
		uint32_t sptr = 0;
		while (1)
		{
			char c = getc();
			printf("%c", c);
			if (c != '\r')
			{
				s[sptr++] = c;
			}
			else
			{
				s[sptr] = '\0';
				printf("\n");
				break;
			}
			// printf("string:%s\n", s);
		}
		set_option(s);
	}

	return 1;
}

int shell_test(int argc, char **argv)
{
	// printf("shell test\n");
	// while (1){}
	
	// return 0;
	


	return 1;
}