#ifndef STRUCT_FUNC_INFO_H
#define STRUCT_FUNC_INFO_H

#define ARR_SIZE 10

struct func_info {
	
	char func_name [ARR_SIZE];
	char type_of_args [ARR_SIZE];
	char type_of_ret_value [ARR_SIZE];
	int num_of_args;	
} __attribute__((packed));

#endif