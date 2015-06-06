/*
 * vm.h
 *
 *  Created on: Jun 3, 2015
 *      Author: ma
 */

#ifndef VM_H_
#define VM_H_

typedef enum {
  VT_Object,  // Link is merged here
  VT_Freevar,
  VT_Local,
  VT_Argument,
  VT_Property // actually a string
} JSVarType;

typedef struct {
  JSVarType type;
  int id;
} JSVar;

int vm_run(int fd);

#endif /* VM_H_ */
