#ifndef SHELL_H
#define SHELL_H

#include "stdio.h"
#include "memtools.h"

#define COMMAND(op_size, opfun, str)     size = 0;\
    args = tmp;\
    opsize = op_size;\
	while (*args != '\0' && *args != ' ')\
	{\
		size++;\
		args++;\
	}\
	args = tmp;\
	if ((size == opsize) && (memcmp(args, "str", size)) == 0)\
	{\
		opfun(args);\
        return;\
	}\



#endif // SHELL_H
