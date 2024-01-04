#include "uci.h"
#include "msg.h"
#include "proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#define UNIMPLEMENTED(_str) \
    do { \
        msg(ERROR, "Unimplemented feature (%s)\n", _str); \
    } while (0);

#define remove_newline(_str) \
    do { \
        const size_t len = strlen(_str); \
        _str[len - 1] = '\0'; \
    } while (0);

void parse_engine_id(struct uci_engine* info) 
{
    msg(DEBUG, "Started Engine id parsing\n");

    char* type = strtok(NULL, " ");
    char id_value[255] = { 0 };

    char* tmp = strtok(NULL, " ");

    while (tmp != NULL) {
        if (id_value[0]) strcat(id_value, " ");
        strcat(id_value, tmp);
        tmp = strtok(NULL, " ");
    }

    msg(DEBUG, "Engine id parsed: (%s)\n", id_value);

    if (strcmp(type, "name") == 0) {
        strcpy(info->name, id_value);
    } else if (strcmp(type, "author") == 0) {
        strcpy(info->author, id_value);
    } else {
        msg(ERROR, "Unknown engine id type: (%s)\n", type);
    }
}

void parse_engine_option(struct uci_engine* engine)
{
    msg(DEBUG, "Started Engine option parsing\n");

    char opt_name[256] = { 0 };
    char opt_type_str[256];
    char default_value[256];
    char min[256];
    char max[256];

    char* tmp = strtok(NULL, " ");
    while (tmp != NULL && strcmp(tmp, "type") != 0) {
        if (opt_name[0]) strcat(opt_name, " ");
        strcat(opt_name, tmp);
        tmp = strtok(NULL, " ");
    }

    if (tmp == NULL) {
        msg(ERROR, "Option type not found for %s\n", opt_name);
        return;
    }

    tmp = strtok(NULL, " ");
    if (tmp == NULL) {
        msg(ERROR, "Option type missing after 'type' for %s\n", opt_name);
        return;
    }
    strcpy(opt_type_str, tmp);

    enum uci_option_type opt_type;
    if (strcmp(opt_type_str, "check") == 0) opt_type = CHECK;
    else if (strcmp(opt_type_str, "spin") == 0) opt_type = SPIN;
    else if (strcmp(opt_type_str, "combo") == 0) opt_type = COMBO;
    else if (strcmp(opt_type_str, "button") == 0) opt_type = BUTTON;
    else if (strcmp(opt_type_str, "string") == 0) opt_type = STRING;
    else {
        msg(ERROR, "Unknown option type for %s\n", opt_name);
        return;
    }

    tmp = strtok(NULL, " ");

    if (tmp == NULL) {
        msg(DEBUG, "Default value missing for %s\n", opt_name);
        tmp = "\0";
    }

    strcpy(default_value, tmp);

    if (opt_type == SPIN) {
        tmp = strtok(NULL, " ");
        if (tmp == NULL) {
            msg(ERROR, "Min value missing for %s\n", opt_name);
            return;
        }
        strcpy(min, tmp);

        tmp = strtok(NULL, " ");
        if (tmp == NULL) {
            msg(ERROR, "Max value missing for %s\n", opt_name);
            return;
        }
        strcpy(max, tmp);
    }

    struct uci_option* new_option = malloc(sizeof(struct uci_option));
    if (new_option == NULL) {
        msg(ERROR, "Memory allocation failed for option %s\n", opt_name);
        return;
    }

    strcpy(new_option->name, opt_name);
    new_option->type = opt_type;
    strcpy(new_option->default_value, default_value);
    if (opt_type == SPIN) {
        strcpy(new_option->min, min);
        strcpy(new_option->max, max);
    }

    struct uci_option* new_options = malloc((engine->options_size + 1) * sizeof(struct uci_option));
    if (new_options == NULL) {
        msg(ERROR, "Memory allocation failed for new options array\n");
        free(new_option);
        return;
    }

    if (engine->options_size > 0 && engine->options != NULL) {
        memcpy(new_options, engine->options, engine->options_size * sizeof(struct uci_option));
        free(engine->options);
    }

    new_options[engine->options_size] = *new_option;
    free(new_option);

    engine->options = new_options;
    engine->options_size += 1;

    msg(DEBUG, "Option added: %s\n", opt_name);
    msg(DEBUG, "Option parsed: %s\n", opt_name);
}

typedef void (uci_message_handler_t)(struct uci_engine* engine, char* raw_message);

void uci_command(
    struct uci_engine* engine,
    char* cmd,
    char* expect,
    uci_message_handler_t handler, 
    bool expect_exact_match,
    bool handler_on_expected)
{
    msg(DEBUG, "Starting uci_command (%s)\n", cmd);

    fprintf(engine->_stdin, "%s", cmd);

    for (;;) {
        char* line = NULL;
        size_t size = 0;
        ssize_t bytes_read = getline(&line, &size, engine->_stdout);

        if (bytes_read == -1) {
            msg(ERROR, "EOF Reached\n");
            exit(-1);
        }

        int cmp = expect_exact_match ? strcmp(expect, line) == 0 :
            strncmp(expect, line, strlen(expect)) == 0;

        if (cmp) {
            msg(DEBUG, "Expected message reached, exiting loop...\n");
            if (handler_on_expected && handler) {
                handler(engine, line);
            }
            goto exit_loop;
        }

        if (handler)
            handler(engine, line);
    }

exit_loop:
    msg(DEBUG, "uci_command Loop exited\n");
}

void uci_spawn_engine_handler(struct uci_engine* engine, char* raw_message)
{
    remove_newline(raw_message);
    msg(DEBUG, "Raw message: (%s)\n", raw_message);

    char* token = strtok(raw_message, " ");

    if (!token) {
        msg(DEBUG, "Could not strtok (%s)\n", raw_message);
        return;
    }

    if (strcmp(token, "id") == 0) {
        parse_engine_id(engine);
    } else if (strcmp(token, "option") == 0) {
        parse_engine_option(engine);
    } else {
        msg(DEBUG, "unknown uci response (%s)\n", raw_message);
    }
}

int uci_spawn_engine(struct spawn_engine_params *params, struct uci_engine* engine)
{
    if (!params || params->path[0] == '\0') {
        msg(ERROR, "Invalid params: missing path\n");
        exit(-1);
        return -1;
    }

    char* line = NULL;
    size_t size = 0;

    popen_with_fd(params->path, &(engine->_stdin), &(engine->_stdout));

    engine->options_size = 0;
    uci_command(engine, "uci\n", "uciok\n", uci_spawn_engine_handler, true, false);

    return 0;
}

int uci_set_fen(struct uci_engine* engine, char* fen)
{
    msg(INFO, "Setting FEN position (%s)\n", fen);

    if (strcmp(fen, "startpos") == 0) {
        msg(DEBUG, "dbg (%s)\n", fen);
        fprintf(engine->_stdin, "position %s\n", fen);
    }
    else
        fprintf(engine->_stdin, "position fen %s\n", fen);

    return 0;
}

void uci_bestmove_handler(struct uci_engine* engine, char* raw_message)
{
    remove_newline(raw_message);
    msg(DEBUG, "Starting bestmove handler\n");
    msg(DEBUG, "Raw message: (%s)\n", raw_message);

    char* tmp = strtok(raw_message, " ");

    if (strncmp(tmp, "bestmove", 9) == 0) {
        msg(DEBUG, "Bestmove found!\n");
        tmp = strtok(NULL, " ");
        strcpy(engine->last_bestmove, tmp);
    }
}

int uci_bestmove(struct uci_engine *engine, char *time_ms)
{
    msg(DEBUG, "Starting go command for %s ms\n", time_ms);

    char tmp[256] = { 0 };
    snprintf(tmp, 256, "go movetime %s\n", time_ms);

    uci_command(engine, tmp, "bestmove", uci_bestmove_handler, false, true);
    return 0;
}

int uci_isready(struct uci_engine* engine)
{
    msg(DEBUG, "Starting isready command\n");

    uci_command(engine, "isready\n", "readyok\n", NULL, true, false);

    return 0;
}
