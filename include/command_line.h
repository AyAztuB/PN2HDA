#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdbool.h>

typedef union { bool is_set; const char* value; } arg_default_value;

void add_argument(const char* name, char short_name, const char* description, bool is_flag, arg_default_value default_value);
void free_argument_parser(void);
bool parse_command_line(int argc, char **args);
void display_help(const char* header);
bool is_flag_set(const char* name);
const char* get_argument_value(const char* name);

#endif // COMMAND_LINE_H
