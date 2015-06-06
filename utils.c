/*
 * utils.c
 *
 *  Created on: Jun 3, 2015
 *      Author: ma
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "utils.h"

int local_server_up = 0;

int socket_fd, client_socket_fd;

int connect_to_server() {

  int socket_fd;
  struct sockaddr_in server;

  //Create socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Could not create socket\n");
  }

  server.sin_addr.s_addr = inet_addr(REMOTE_SERVER_ADDR);
  server.sin_family = AF_INET;
  server.sin_port = htons(REMOTE_SERVER_PORT);

  //Connect to remote server
  if (connect(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
    printf("connect error\n");
    return -1;
  }

  return socket_fd;
}

ssize_t read_line(int fd, void *buffer, size_t n) {
  ssize_t numRead; /* # of bytes fetched by last read() */
  size_t totRead; /* Total bytes read so far */
  char *buf;
  char ch;

  if (n <= 0 || buffer == NULL) {
    errno = EINVAL;
    return -1;
  }

  buf = buffer; /* No pointer arithmetic on "void *" */

  totRead = 0;
  for (;;) {
    numRead = read(fd, &ch, 1);

    if (numRead == -1) {
      if (errno == EINTR) /* Interrupted --> restart read() */
        continue;
      else
        return -1; /* Some other error */

    }
    else if (numRead == 0) { /* EOF */
      if (totRead == 0) /* No bytes read; return 0 */
        return 0;
      else
        /* Some bytes read; add '\0' */
        break;

    }
    else { /* 'numRead' must be 1 if we get here */
      if (totRead < n - 1) { /* Discard > (n - 1) bytes */
        totRead++;
        *buf++ = ch;
      }

      if (ch == '\n')
        break;
    }
  }

  *buf = '\0';

  return totRead;
}

/**
 * return 0 for success
 */
int line_to_command(char * line, Command * cmd) {

  char * pch;

  if (!line || !cmd)
    return -1;
  if (strlen(line) > 4096)
    return -1;

  cmd->argc = 0;
  do {
    pch = strtok((cmd->argc == 0) ? line : NULL, " \r\n");
    cmd->args[cmd->argc++] = pch;
  } while (pch);
  cmd->argc--;

  return 0;
}

int string_to_number(char * string, int64_t * number) {

  char * endp;
  int64_t val;

  if (!string || !number)
    return -1;

  val = strtol(string, &endp, 10);
  if (string == endp)
    return -1;

  *number = val;
  return 0;
}

int start_server() {
  struct sockaddr_in server_addr;

  // create tcp socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    printf("ERROR opening socket\n");
    return -1;
  }

  // prepare server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(LOCAL_SERVER_PORT);

  // bind
  if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    printf("ERROR on binding\n");
    return -1;
  }

  local_server_up = 1;

  printf("Local server started...\n");
  return 0;
}

int accept_new_client() {

  socklen_t client_addr_len;
  struct sockaddr_in client_addr;

  printf("waiting for new client connection...\n");
  listen(socket_fd, 5); // originally 5 but we only accept one.

  // accept
  client_addr_len = sizeof(client_addr);
  client_socket_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
  if (client_socket_fd < 0) {
    printf("ERROR on accept\n");
    return -1;
  }

  printf("new client connected...\n");
  return client_socket_fd;
}

void stop_server() {

  if (local_server_up) {
    close(client_socket_fd);
    close(socket_fd);
    local_server_up = 0;

    printf("Local server stopped...\n");
  }
}

bool isSocketUp(int socket_fd) {
  int ret, error = 0;
  socklen_t len = sizeof(error);
  ret = getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);

  if (ret == 0)
    return true;
  return false;
}

