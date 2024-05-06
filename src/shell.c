#include "shell.h"



// 函数注册，添加COMMAND即可
// 格式：触发词长度，调用函数，触发词

void rvhelp(char *args)
{
	args++;
	printf("my is help\n");
}

void rvcreate(char *args)
{
	extern int progress1(int argc, char **argv);
	args++;
	// 从字符串中读取要创建的进程名，然后调用创建的函数
	if (memcmp(args, "pro", 3) == 0)
	{
		// 创建进程
		progress1(0, NULL);	
	}
}

void bgcreate(char *args)
{
	extern int progress1(int argc, char **argv);
	int task_create(void* start_routin, uint32_t counter, uint32_t priority, uint32_t tty, uint32_t father, int argc, char **argv);
	args++;
	// 从字符串中读取要创建的进程名，然后调用创建的函数
	if (memcmp(args, "pro", 3) == 0)
	{
		// 创建进程
		progress1(0, NULL);	
	}
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
		//set_option(s);
	}

	return 1;
}