/*
 * bytecode.h
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#ifndef BYTECODE_H_
#define BYTECODE_H_

#include "utils.h"

typedef enum {
  OP_CALL = 0,
  OP_CAPTURE,
  OP_DROP,
  OP_FETCH,
  OP_JUMP,
  OP_JUMPC,
  OP_LABEL,
  OP_LITA,
  OP_LITC,
  OP_LITN,
  OP_LITO,
  OP_LITF,
  OP_TEST,
  OP_RET,
  OP_STORE
} OpCode;

extern const char * op_code_name[];

typedef enum {
  OPARG_UNUSED = 0,
  OPARG_STRING,
  OPARG_NUMBER
} op_arg_type;

typedef struct {
  op_arg_type type;
  int64_t num;
  char* string;
} op_arg;

typedef struct {
  OpCode op;
  int argc;
  op_arg args[3];
} ByteCode;

int command_to_bytecode(Command * cmd, ByteCode * code);

#endif /* BYTECODE_H_ */
