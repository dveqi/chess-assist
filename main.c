#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "uci.h"
#include "msg.h"
#include "lib/mongoose.h"

struct uci_engine info;

void add_cors_headers(struct mg_connection *c)
{
    static const char *headers = 
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Origin, Accept, Content-Type, X-Requested-With, X-CSRF-Token\r\n";
    mg_printf(c, headers);
}

void handle_options_request(struct mg_connection *c)
{
    msg(INFO, "Received OPTIONS request\n");
    mg_http_reply(c, 204, 
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, PATCH, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Access-Control-Max-Age: 86400\r\n",
        "");
}

void process_get_request(struct mg_connection *c, struct mg_http_message *hm)
{
    if (mg_vcmp(&hm->uri, "/uci/position") == 0) {
        struct mg_str fen2 = mg_http_var(hm->query, mg_str("fen"));
        char* cleanfen = (char*) malloc(fen2.len + 1);
        mg_url_decode(fen2.ptr, fen2.len, cleanfen, fen2.len, 0);
        cleanfen[fen2.len] = '\0';
        uci_set_fen(&info, cleanfen);
        free(cleanfen);
        mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\n", "{%m:%m,%m:%m}\n",
            MG_ESC("uri"), mg_print_esc, hm->uri.len, hm->uri.ptr,
            MG_ESC("sergio"), mg_print_esc, hm->body.len, hm->body.ptr);
    } else if (mg_vcmp(&hm->uri, "/uci/bestmove") == 0) {
        uci_bestmove(&info, "10");
        mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\n", "{%m:%m}\n",
            MG_ESC("bestMove"), mg_print_esc, strlen(info.last_bestmove), info.last_bestmove);
    } else if (mg_vcmp(&hm->uri, "/uci/is_ready") == 0) {
        uci_isready(&info);
        mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\n", "{%m:%m}\n",
            MG_ESC("isReady"), mg_print_esc, strlen("true"), "true");
    }
}

static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if (mg_vcmp(&hm->method, "OPTIONS") == 0) {
            handle_options_request(c);
        } else if (mg_vcmp(&hm->method, "GET") == 0) {
            process_get_request(c, hm);
        }
    }
    (void) fn_data;
}

int main(int argc, char *argv[])
{
    msg(INFO, "On\n");

    int opt;
    char *path = NULL;

    while ((opt = getopt(argc, argv, "VP:")) != -1) {
        switch (opt) {
        case 'V':
            msg_verbose_state = 1;
            break;
        case 'P':
            path = optarg;
            break;
        default:
            msg(ERROR, "Usage: %s [-V] [-P path]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    struct spawn_engine_params p;

    if (path) {
        strcpy(p.path, path);
    }

    uci_spawn_engine(&p, &info);

    msg(INFO, "Engine name: (%s)\n", info.name);
    msg(INFO, "Engine author: (%s)\n", info.author);
    msg(INFO, "Engine options count: %d\n", info.options_size);

    uci_set_fen(&info, "startpos");
    uci_isready(&info);

    struct mg_mgr mgr;
    struct mg_connection *c;

    mg_mgr_init(&mgr);

    if ((c = mg_http_listen(&mgr, "http://0.0.0.0:8080", cb, &mgr)) == NULL) {
        msg(ERROR, "Cannot listen on %s. Use http://ADDR:PORT or :PORT", "http://0.0.0.0:8080");
        exit(EXIT_FAILURE);
    }

    while (true) mg_mgr_poll(&mgr, 1);

    return 0;
}
