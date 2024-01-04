#ifndef _LIBUCI_UCI_H
#define _LIBUCI_UCI_H

#include <stdio.h>

enum uci_option_type {
    CHECK, SPIN, COMBO, BUTTON, STRING
};

struct uci_option {
    char name[256];
    enum uci_option_type type;
    char default_value[256];
    char min[256];
    char max[256];
};

struct uci_engine {
    FILE *_stdin;
    FILE *_stdout;
    char name[255];
    char author[255];
    struct uci_option *options;
    size_t options_size;
    char last_bestmove[256];
};

struct spawn_engine_params {
    char path[255];
};

int uci_spawn_engine(struct spawn_engine_params *params, struct uci_engine* engine);
int uci_set_fen(struct uci_engine* engine, char* fen);
void uci_bestmove_handler(struct uci_engine* engine, char* raw_message);
int uci_bestmove(struct uci_engine *engine, char *time_ms);
int uci_isready(struct uci_engine* engine);

#endif
