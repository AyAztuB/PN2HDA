#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command_line.h"
#include "logger.h"
#include "vector.h"

struct argument {
    char* name;
    char short_name;
    char* description;
    bool is_flag;
    union {
        bool is_set;
        char* value;
    } user;
};

static Vector(struct argument) arguments = NULL;

void add_argument(const char* name, char short_name, const char* description, bool is_flag, arg_default_value default_value) {
    if (!arguments) arguments = vector_new(sizeof(struct argument), 0);
    if (!description || !name) return;
    struct argument arg = {
        .description = strdup(description),
        .name = strdup(name),
        .short_name = short_name,
        .is_flag = is_flag,
    };
    if (is_flag)
        arg.user.is_set = default_value.is_set;
    else if (!default_value.value)
        arg.user.value = NULL;
    else
        arg.user.value = strdup(default_value.value);
    vector_push(arguments, &arg);
}

static void free_one_arg(void* arg, __attribute__((unused))void* unsed) {
    struct argument *a = arg;
    free(a->description);
    free(a->name);
    if (!a->is_flag) free(a->user.value);
}

void free_argument_parser(void) {
    vector_forall(arguments, free_one_arg, NULL);
    vector_destroy(arguments);
    arguments = NULL;
}

bool parse_command_line(int argc, char **args) {
    for (int i = 1; i < argc; i++) {
        if (args[i][0] == '-') {
            bool is_found = false;
            struct argument *a = vector_to_array(arguments);
            for (size_t j = 0; j < vector_length(arguments); j++) {
                if ((args[i][1] == '-' && !strcmp(args[i] + 2, a[j].name))
                    || (args[i][1] && args[i][1] == a[j].short_name && !args[i][2])) {
                    if (a[j].is_flag)
                        a[j].user.is_set = true;
                    else if (i < argc - 1) {
                        if (a[j].user.value)
                            free(a[j].user.value);
                        a[j].user.value = strdup(args[++i]);
                    }
                    else {
                        LOG(ERROR, "Unable to parse option %s: missing value", args[i]);
                        return false;
                    }
                    is_found = true;
                    break;
                }
            }
            if (!is_found) {
                LOG(ERROR, "Unknown argument %s", args[i]);
                return false;
            }
        }
    }
    return true;
}

static void print_one_arg(void* arg, __attribute__((unused))void* unused) {
    struct argument* a = arg;
    printf("\t--%s", a->name);
    if (a->short_name)
        printf(", -%c", a->short_name);
    printf("\t:%s %s\n", a->is_flag ? "(flag)" : "", a->description);
}

void display_help(const char* header) {
    printf("%s\n", header);
    vector_forall(arguments, print_one_arg, NULL);
}

static bool is_equal(void* a, void* b) {
    struct argument* arg = a;
    const char* name = b;
    return strcmp(arg->name, name) == 0;
}

bool is_flag_set(const char* name) {
    size_t idx = vector_find(arguments, is_equal, (void*) name);
    if (idx >= vector_length(arguments))
        return false;
    struct argument* a = vector_to_array(arguments);
    if (!a[idx].is_flag)
        return false;
    return a[idx].user.is_set;
}

const char* get_argument_value(const char* name) {
    size_t idx = vector_find(arguments, is_equal, (void*) name);
    if (idx >= vector_length(arguments))
        return NULL;
    struct argument* a = vector_to_array(arguments);
    if (a[idx].is_flag)
        return NULL;
    return a[idx].user.value;
}
