/* Automatically-generated code, do not modify! */
#ifndef __CFGPARSER_H__
#define __CFGPARSER_H__
#include <stdbool.h>
#include <glib.h>
#include <stdio.h>
#include <stdint.h>
extern int yyparse();
extern int yylex_destroy();
extern void yyerror(const char *fmt, ...);
extern FILE *yyin;
extern int yylineno;
typedef struct cfg_options_t {
    const char * username;
    const char * groupname;
    uint32_t log_level;
    const char * error_log;
    uint32_t buffered_frames;
} cfg_options_t;
bool cfg_options_callback(cfg_options_t *section);
typedef struct cfg_socket_t {
    const char * listen_on;
    const char * port;
    bool ipv6;
    bool sctp;
    uint32_t sctp_streams;
} cfg_socket_t;
bool cfg_socket_callback(cfg_socket_t *section);
typedef struct cfg_vhost_t {
    GList * aliases;
    const char * access_log;
    const char * twin;
    const char * document_root;
    const char * virtuals_root;
    uint32_t max_connections;
    GList * dynamic_resource_paths;
    uint32_t connection_count;
    FILE *access_log_file;
} cfg_vhost_t;
bool cfg_vhost_callback(cfg_vhost_t *section);
#endif
