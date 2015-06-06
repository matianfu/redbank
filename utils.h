/*
 * utils.h
 *
 *  Created on: Jun 3, 2015
 *      Author: ma
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

#define FAIL_EXIT()   do { printf("Failed at file %s line %d\n", __FILE__, __LINE__); exit(EXIT_FAILURE); } while (0)
// #define ASSERT_TRUE(expr)         do { if (!(expr)) { FAIL_EXIT(); }} while (0)
#define ASSERT_FALSE(expr)        do { if (expr) { FAIL_EXIT(); }} while (0)

#define FAIL_IF(expr)     do { if (expr) { FAIL_EXIT(); }} while (0)


#define FAIL_RETURN             printf("Function %s failed at file %s line %d\n", __func__, __FILE__, __LINE__); return -1;
#define ASSERT_TRUE(expr)       do { if (!(expr)) { FAIL_RETURN }} while (0)
#define ASSERT_SUCCESS(expr)    do { if (0 != (expr)) { FAIL_RETURN } } while (0)

/**
 * as client
 */
#define   REMOTE_SERVER_ADDR    "127.0.0.1"
#define   REMOTE_SERVER_PORT    6969

/**
 * as server
 */
#define   LOCAL_SERVER_ADDR     "127.0.0.1"
#define   LOCAL_SERVER_PORT     7979

/**
 * as client
 */
int connect_to_server();

/**
 * as server
 */

// return socket_fd or -1 on error
int start_server();
int accept_new_client();
void stop_server();
bool isSocketUp(int socket_fd);

ssize_t read_line(int fd, void *buffer, size_t n);

#define COMMAND_MAXARG  4

typedef struct {
  int argc;
  char * args[COMMAND_MAXARG];
} Command;
int line_to_command(char * line, Command * cmd);

int string_to_number(char * string, int64_t * number);

#endif /* UTILS_H_ */
