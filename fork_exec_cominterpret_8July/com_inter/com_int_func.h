#ifndef COM_INT_FUNC_H
#define COM_INT_FUNC_H

void comm_main(void);

char* read_comm_str();

char** parsing_comm_str(char* command_str);

void start_comm(char** args);

#endif