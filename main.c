#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include "object.h"
#include "bytecode.h"
#include "utils.h"
#include "vm.h"

int vm_main() {

  int ret;

  ret = start_server();
  if (ret < 0)
    return 0;

  for (;;) {
    int socket_fd = accept_new_client();
    if (socket_fd < 0)
      return -1;
    vm_run(socket_fd);
  }

  stop_server();
  return 0;
}

int main() {

  return vm_main();

}

