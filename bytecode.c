/*
 * bytecode.c
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "bytecode.h"

const char * op_code_name[] = { "CALL", "CAPTURE", "DROP", "FETCH", "JUMP", "JUMPC", "LABEL",
    "LITA", "LITC", "LITN", "LITO", "LITF", "TEST", "RET", "STORE" };

/**
 * return 0 for success, -1 for error
 */
int token2op(char * tok, OpCode * opc) {

  int i, size;

  if (!tok || !opc)
    return -1;

  size = sizeof(op_code_name) / sizeof(op_code_name[0]);
  for (i = 0; i < size; i++) {
    if (strcmp(op_code_name[i], tok) == 0) {
      *opc = i;
      return 0;
    }
  }
  return -1;
}

/**
 * convert token string to number
 */
int token2number(char * tok, op_arg * arg) {

  char * endp;
  int64_t val;

  if (!tok || !arg)
    return -1;

  arg->type = OPARG_UNUSED;
  val = strtol(tok, &endp, 10);
  if (tok == endp)
    return -1;

  arg->type = OPARG_NUMBER;
  arg->num = val;
  return 0;
}

int token2string(char * tok, op_arg * arg) {

  int size;

  if (!tok || !arg)
    return -1;

  size = strlen(tok) + 1;
  arg->type = OPARG_UNUSED;
  arg->string = (char*) malloc(size);
  if (arg->string == 0) {
    return -1;
  }

  memset(arg->string, 0, size);
  strcpy(arg->string, tok);

  arg->type = OPARG_STRING;
  return 0;
}

int token2value(char * tok, op_arg * arg) {

  if (token2number(tok, arg) == 0)
    return 0;

  return token2string(tok, arg);
}

int command_to_bytecode(Command * cmd, ByteCode * code) {

  int ret, i;

  if (!cmd || !code || cmd->argc == 0 || cmd->argc > 4)
    return -1;

  ret = token2op(cmd->args[0], &code->op);
  if (ret < 0)
    return -1;

  code->argc = cmd->argc - 1;

  for (i = 0; i < 3; i++)
    code->args[i].type = OPARG_UNUSED;

  switch (code->op) {
  case OP_CALL:
    ASSERT_TRUE(cmd->argc == 1);
    break;
  case OP_CAPTURE:
    break;
  case OP_DROP:
    ASSERT_TRUE(cmd->argc == 1);
    break;
  case OP_FETCH:
    ASSERT_TRUE(cmd->argc == 1);
    break;
  case OP_JUMP:
    break;
  case OP_JUMPC:
    break;
  case OP_LABEL:
    ASSERT_TRUE(cmd->argc == 2);
    ASSERT_TRUE(token2number(cmd->args[1], &code->args[0]) == 0);
    break;
  case OP_LITA:
    ASSERT_TRUE(cmd->argc == 3);
    ASSERT_TRUE(token2string(cmd->args[1], &code->args[0]) == 0);
    ASSERT_TRUE(token2number(cmd->args[2], &code->args[1]) == 0);
    break;
  case OP_LITC:
    ASSERT_TRUE(cmd->argc == 2);
    ASSERT_TRUE(token2value(cmd->args[1], &code->args[0]) == 0);
    break;
  case OP_LITN:
    ASSERT_TRUE(cmd->argc == 2);
    ASSERT_TRUE(token2number(cmd->args[1], &code->args[0]) == 0);
    break;
  case OP_LITO:
    break;
  case OP_LITF:
    break;
  case OP_TEST:
    ASSERT_TRUE(cmd->argc == 2);
    ASSERT_TRUE(token2string(cmd->args[1], &code->args[0]) == 0);
    break;
  case OP_RET:
    ASSERT_TRUE(cmd->argc == 1);
    break;
  case OP_STORE:
    ASSERT_TRUE(cmd->argc == 1);
    break;
  default:
    FAIL_IF(true);
    break;
  }

  return 0;
}

