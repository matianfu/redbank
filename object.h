/*
 * object.h
 *
 *  Created on: Jun 2, 2015
 *      Author: ma
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <stdint.h>
#include <stdbool.h>

typedef int ID;

#define UNDEFINED ((ID)0)

typedef enum {
  OT_Boolean = 0,
  OT_Undefined,
  OT_Null,
  OT_Number,
  OT_String,
  OT_Symbol, //  ECMAScript
  OT_Object,
  OT_Link
} ObjectType;

int object_module_init();
void object_module_deinit();

ObjectType type_of(ID id);

#define MAX_REF (-1)

/*
 * Policy
 *
 * when object are newly created, object module returns it with zero reference.
 * user should increment it's reference manually.
 */
int ref_incr(ID id);
int ref_decr(ID id);

ID new_number_object(int64_t val);
ID new_string_object(char* cstr);
ID new_link_object(int target);
ID new_object_object();

bool is_number_object(ID id);
bool is_link_object(ID id);

/**
 * for performance sake, these functions don't check object type
 * the caller is responsible to assure it.
 */
int64_t number_object_value(ID id);
ID link_object_get_target(ID id);
void link_object_set_target(ID id, ID target);

// void delete_object(ID id);

#endif /* OBJECT_H_ */
