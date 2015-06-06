/*
 * vm.c
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>

#include "object.h"
#include "bytecode.h"
#include "utils.h"
#include "vm.h"

/**
 * definitions
 */
#define FORMAT_HLINE  "=================================================="

#define BYPASS_TEST           (0)
#define LINEBUFFER_SIZE       (4096)
#define BYTECODE_SIZE         (4096)
#define STACK_SIZE            (4096)

#define TOS                   Stack[StackSize - 1]
#define NOS                   Stack[StackSize - 2]
#define ThirdOS               Stack[StackSize - 3]

#define LOCAL(n)              Stack[FP + n]

/**
 * globals
 */
int FD;
int client_aborted;

char linebuf[LINEBUFFER_SIZE];
Command command;

ByteCode bytecodes[BYTECODE_SIZE];
uint32_t bytecodes_length = 0;

JSVar Stack[STACK_SIZE];
uint32_t StackSize;

uint32_t PC;    // program counter
uint32_t FP;    // frame pointer, also locals[0]
uint32_t FV;    // freevars[0]
uint32_t this;  // this object id
uint32_t ARG;   // args[0]
uint32_t ARGC;

/**
 * statics
 */
static void print_stack();
static void emit_json_command(const char * cmd, const char * arg);

void push(JSVar var) {

  if (var.type == VT_Object) {
    ref_incr(var.id);
  }

  Stack[StackSize] = var;
  StackSize++;
}

void pop() {

  if (TOS.type == VT_Object) {
    ref_decr(TOS.id);
  }
  StackSize--;
}

#define TEST_SUCCESS(expr)  do { if (0 > (expr)) { goto fail;} } while (0)
#define TEST_TRUE(expr)     do { if (!(expr)) { goto fail; } } while (0)


int test(const char * testname) {

  int i, ret;


  static char testcode[16][256];
  static Command testcmd[16];
  static int test_count = 0;

  emit_json_command("TEST", testname);

  test_count = 0;
  for (i = 0; i < 16; i++) {

    ret = read_line(FD, testcode[i], 256);
    if (ret <= 0) {
      emit_json_command("TESTFAIL", "Readline Error");
      return -1;
    }

    ret = line_to_command(testcode[i], &testcmd[i]);
    if (ret < 0) {
      emit_json_command("TESTFAIL", "Parse line error");
      return -1;
    }

    if (testcmd[i].argc == 0)
      break;

    if (0 == strcmp(testcmd[i].args[0], "ABORT") || 0 == strcmp(testcmd[i].args[0], "Abort")) {
      client_aborted = 1;
      printf("clients abort\n");
      break;
    }

    test_count++;
  }

  for (i = 0; i < test_count; i++) {

    int64_t number, val;

    if (0 == strcmp(testcmd[i].args[0], "StackSize")) {
      TEST_TRUE(testcmd[i].argc == 2);
      TEST_SUCCESS(string_to_number(testcmd[i].args[1], &number));
      TEST_TRUE(StackSize == number);
    }
    else if (0 == strcmp(testcmd[i].args[0], "StackSlotUndefined")) {
      TEST_TRUE(testcmd[i].argc == 2);
      TEST_SUCCESS(string_to_number(testcmd[i].args[1], &number));
      TEST_TRUE(Stack[number].type == VT_Object);
      TEST_TRUE(Stack[number].id == UNDEFINED);
    }
    else if (0 == strcmp(testcmd[i].args[0], "StackSlotNumberValue")) {
      TEST_TRUE(testcmd[i].argc == 3);
      TEST_SUCCESS(string_to_number(testcmd[i].args[1], &number));
      TEST_SUCCESS(string_to_number(testcmd[i].args[2], &val));
      TEST_TRUE(Stack[number].type == VT_Object);
      TEST_TRUE(is_number_object(Stack[number].id));
      TEST_TRUE(val == number_object_value(Stack[number].id));
    }
    else {
      goto fail;
    }
  }

  printf(">>> TEST: %s passed\n", testname);
  return 0;

  fail:

  emit_json_command("TESTFAIL", testcmd[i].args[0]);
  return -1;
}

int lita(ByteCode bytecode) {

  StackSize++;
  TOS.id = bytecode.args[1].num;
  if (0 == strcmp(bytecode.args[0].string, "LOCAL")) {
    TOS.type = VT_Local;
  }
  else if (0 == strcmp(bytecode.args[0].string, "PARAM")) {
    TOS.type = VT_Argument;
  }
  else if (0 == strcmp(bytecode.args[0].string, "FRVAR")) {
    TOS.type = VT_Freevar;
  }
  else if (0 == strcmp(bytecode.args[0].string, "PROP")) {
    TOS.type = VT_Property;
  }
  else {
    return -1;
  }

  return 0;
}

int store_to_local() {

  ASSERT_TRUE(TOS.type == VT_Object);
  ASSERT_TRUE(NOS.type == VT_Local);

  if (is_link_object(LOCAL(NOS.id).id)) {
    link_object_set_target(LOCAL(NOS.id).id, TOS.id);
  }
  else {
    ref_decr(LOCAL(NOS.id).id);
    LOCAL(NOS.id).id = TOS.id;
    ref_incr(LOCAL(NOS.id).id);
  }
  pop();
  pop();
  return 0;
}

int store() {

  int ret = 0;

  if (NOS.type == VT_Local) {
    ret = store_to_local();
  }
  else if (NOS.type == VT_Argument) {

  }
  else if (NOS.type == VT_Freevar) {

  }
  else {

  }

  return ret;
}

int step() {

  int ret = 0;
  JSVar var;
  int i;

  ByteCode bytecode = bytecodes[PC++];

  switch (bytecode.op) {
  case OP_CALL:
    break;
  case OP_CAPTURE:
    break;
  case OP_DROP:
    pop();
    break;
  case OP_FETCH:
    break;
  case OP_JUMP:
    break;
  case OP_JUMPC:
    break;
  case OP_LABEL:
    break;

  case OP_LITA:
    ret = lita(bytecode);
    break;

  case OP_LITC:
    var.type = VT_Object;
    if (bytecode.args[0].type == OPARG_NUMBER) {
      var.id = new_number_object(bytecode.args[0].num);
      push(var);
    }
    else if (bytecode.args[0].type == OPARG_STRING) {
      var.id = new_string_object(bytecode.args[0].string);
      push(var);
    }
    else {
      FAIL_IF(true);
    }
    break;

  case OP_LITN:
    var.type = VT_Object;
    var.id = UNDEFINED;
    for (i = 0; i < bytecode.args[0].num; i++) {
      push(var);
    }
    break;

  case OP_LITO:
    break;
  case OP_LITF:
    break;
  case OP_TEST:
    if (!BYPASS_TEST && !client_aborted) {
      ASSERT_SUCCESS(test(bytecode.args[0].string));
    }
    break;
  case OP_RET:
    break;

  case OP_STORE:
    ret = store();
    break;

  default:
    printf("Unrecognized opcode\n");
    return -1;
    break;
  }

  return ret;
}

static void print_bytecode(ByteCode code) {

  int i;

  printf("OPCODE: %s", op_code_name[code.op]);

  for (i = 0; i < code.argc; i++) {
    op_arg arg = code.args[i];
    if (arg.type == OPARG_NUMBER) {
      printf(" %lld", (long long) arg.num);
    }
    else if (arg.type == OPARG_STRING) {
      printf(" %s", arg.string);
    }
  }

  printf("\n");
}

int run() {

  int ret = 0;

  PC = 0;
  FP = 0;
  StackSize = 0;

  while (PC < bytecodes_length) {

    print_stack();
    printf(FORMAT_HLINE "\n");
    printf("PC: %d, FP: %d\n", PC, FP);
    print_bytecode(bytecodes[PC]);

    ret = step();

    if (ret < 0) {
      break;
    }
  }

  return ret;
}

static void print_stack() {

  int i;

  if (StackSize == 0) {
    printf("Stack Empty\n");
    return;
  }
  else {
    printf("Stack Size: %d\n", StackSize);

    for (i = StackSize - 1; i >= 0; i--) {
      JSVar v = Stack[i];

      switch (v.type) {
      case VT_Object:
        if (type_of(v.id) == OT_Undefined) {
          printf("%d : UNDEFINED\n", i);
        }
        else if (type_of(v.id) == OT_Number) {
        printf("%d : ID %d, Num %lld\n", i, Stack[i].id, (long long)number_object_value(Stack[i].id));
      }

      break;
    case VT_Local:
      printf("%d : [addr] local %d\n", i, v.id);
      break;
    case VT_Argument:
      printf("%d : [addr] args %d\n", i, v.id);
      break;
    case VT_Freevar:
      printf("%d : [addr] freevar %d\n", i, v.id);
      break;
    case VT_Property:
      printf("%d : [addr] property %d\n", i, v.id);
      break;
      }
    }
  }
}

/**
 * Receiving and parsing bytecode
 */
int receive_bytecode() {

  int ret;
  ByteCode code;

  for (;;) {
    ret = read_line(FD, linebuf, 4096);
    if (ret <= 0) {
      printf("error read_line\n");
      return -1;
    }

    ret = line_to_command(linebuf, &command);
    if (ret < 0) {
      printf("error line_to_command\n");
      return -1;
    }

    if (command.argc == 0)
      break;

    ret = command_to_bytecode(&command, &code);
    if (ret < 0) {
      printf("error command_to_bytecode\n");
      return -1;
    }

    bytecodes[bytecodes_length++] = code;
  }

  printf("bytecode parsing finished. total code size: %d\n", bytecodes_length);

  return 0;
}

// TODO initialize globals
void vm_init(int fd) {

  FD = fd;
  client_aborted = 0;

  memset(linebuf, 0, LINEBUFFER_SIZE);
  memset(&command, 0, sizeof(Command));
  memset(bytecodes, 0, sizeof(ByteCode) * BYTECODE_SIZE);
  bytecodes_length = 0;
  memset(Stack, 0, sizeof(JSVar) * STACK_SIZE);
  StackSize = 0;
  PC = 0;
  FP = 0;
  FV = 0;
  this = 0;
  ARG = 0;
  ARGC = 0;
}

void emit_json_command(const char * cmd, const char * arg) {

  char buf[256];
  sprintf(buf, "{\"command\":\"%s\",\"argument\":\"%s\"}\n", cmd, arg);
  write(FD, buf, strlen(buf));
}

int vm_run(int fd) {

  int ret;

  printf("Red Bank vm start...\n");
  vm_init(fd);

  emit_json_command("READY", "");

  ret = receive_bytecode();
  if (ret < 0) {
    return -1;
  }

  ret = object_module_init();
  if (ret < 0) {
    return -1;
  }

  ret = run();
  if (ret < 0) {
    return -1;
  }

  object_module_deinit();

  emit_json_command("FINISH", "SUCCESS");
  printf("Red Bank vm exits %s\n", (ret == 0) ? "successfully" : "with failure");
  return ret;
}
