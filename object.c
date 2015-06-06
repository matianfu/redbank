/*
 * objects.c
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "hash.h"
#include "object.h"

static void create_undefined_object();
static void create_null_object();

#define PREDEFINED_UNDEFINED  0
#define PREDEFINED_NULL       1
#define OBJECTS_START         2

/*
 * macros
 */

// how many bits (of hash) will be used as opa address
// the address table will be 2 ** bitsize
#define OPA_BITSIZE     (16)
#define OPA_TABLE_SIZE  ( 1 << OPA_BITSIZE )
#define OPA_BITMASK     (( (uint32_t)0xFFFFFFFF >> (32 - OPA_BITSIZE) ) << (32 - OPA_BITSIZE))
#define OPA_INDEX(hash) ((hash & OPA_BITMASK) >> (32 - OPA_BITSIZE))

/**
 * how many bits (of hash) will be used as internalized string table address
 * the address table size will be 2 ** bitsize
 */
#define STRINTERN_BITSIZE     (16)
#define STRINTERN_TABLE_SIZE  ( 1 << STRINTERN_BITSIZE )
#define STRINTERN_BITMASK     (( (uint32_t)0xFFFFFFFF >> (32 - STRINTERN_BITSIZE) ) << (32 - STRINTERN_BITSIZE))
#define STRINTERN_INDEX(hash) ((hash & STRINTERN_BITMASK) >> (32 - STRINTERN_BITSIZE))

#define OBJECT_NUMBER   64

/*
 * structs and types
 */

/**
 * hash should not be included since we do not use all 32bits of hash value,
 * some OPA with different hash value may be stored in the same slot.
 */
typedef struct opa_node {
  ID parent;
  ID property;
  ID child;
  struct opa_node * next;
} OPANode;

typedef struct {
  uint16_t type;
  uint16_t ref;
} SharedObject;

typedef struct {
  SharedObject S;
} UndefinedObject;

typedef struct {
  SharedObject S;
  int64_t val;
} NumberObject;

#define STRING_OBJECT_SIZE(len)  (sizeof(SharedObject) + sizeof(uint32_t) + len + 1)
typedef struct {
  SharedObject S;
  uint32_t hash;
  uint8_t buf[1];
} StringObject;

typedef struct {
  SharedObject S;
  ID target;
} LinkObject;

/** type for string node **/
typedef struct StringNode {
  ID id;
  struct StringNode * next;
} StringNode;

typedef struct {
  SharedObject S;
  StringNode * next;
} ObjectObject;

typedef union {
  SharedObject * obj; // when some object is stored here
  ID next;           // when this slot is free slot
} ObjectTableEntry;

/**
 * globals
 */

OPANode * opa_table[OPA_TABLE_SIZE];
/** string hash table **/
StringNode * string_table[STRINTERN_TABLE_SIZE];

ObjectTableEntry Objects[OBJECT_NUMBER];

int free_head;  // index of first free slot
int free_tail;  // index of last free slot

/**
 * functions
 */

uint32_t RB_Hash_Incremental(uint32_t hash, ID parent) {
  return RBHash32(hash, sizeof(ID), (uint8_t*) (&parent));
}

static void init_object_pool() {
  int i;
  // build linked list
  free_head = OBJECTS_START;    // first slot,

  for (i = OBJECTS_START; i < OBJECT_NUMBER; i++) {
    Objects[i].next = (i + 1); // point to next slot
  }

  free_tail = OBJECT_NUMBER - 1;
  Objects[free_tail].next = 0;
}

static void init_string_table() {

  int i;
  for (i = 0; i < STRINTERN_TABLE_SIZE; i++) {
    string_table[i] = 0;
  }
}

static void init_opa_table() {

  int i;
  for (i = 0; i < OPA_TABLE_SIZE; i++) {
    opa_table[i] = 0;
  }
}

int object_module_init() {

  init_opa_table();
  init_string_table();
  init_object_pool();
  create_undefined_object();
  create_null_object();
  return 0;
}

void object_module_deinit() {

}

static ID allocate_id() {
  ID id;
  if (free_head == 0)
    return 0;

  // the last one
  if (free_head == free_tail) {
    id = free_head;
    free_head = free_tail = 0;
    return id;
  }

  id = free_head;
  free_head = Objects[free_head].next;
  return id;
}

static void deallocate_id(ID id) {

  if (free_head == 0) {
    free_head = free_tail = id;
    return;
  }

  Objects[free_tail].next = id;
  free_tail = id;
  Objects[free_tail].next = 0;
}

static void opa_create(ID parent, ID property, ID child) {

  StringObject* prop = (StringObject*) Objects[property].obj;
  uint32_t hash = RB_Hash_Incremental(prop->hash, parent);
  int index = OPA_INDEX(hash);

  OPANode * node = (OPANode*) MALLOC(sizeof(OPANode));
  memset(node, 0, sizeof(OPANode));

  node->parent = parent;
  node->property = property;
  node->child = child;  // TODO reference for child
  node->next = opa_table[index];
  opa_table[index] = node;
}

ID opa_lookup(ID parent, ID property) {

  StringObject * prop = (StringObject*) Objects[property].obj;
  uint32_t hash = RB_Hash_Incremental(prop->hash, parent);
  int index = OPA_INDEX(hash);

  OPANode * node = opa_table[index];

  while (node) {
    if (node->parent == parent && node->property == property) {
      return node->child;
    }
    node = node->next;
  }
  return 0;
}

static void opa_delete(ID parent, ID property) {

  StringObject* prop = (StringObject*) Objects[property].obj;
  uint32_t hash = RB_Hash_Incremental(prop->hash, parent);
  int index = OPA_INDEX(hash);

  OPANode * node = opa_table[index];
  if (node->parent == parent && node->property == property) {
    opa_table[index] = node->next;
    FREE(node); // TODO reference for child
  }
  else {
    while (node->next) {
      OPANode * tmp = node->next;
      if (tmp->parent == parent && tmp->property == property) {
        node->next = tmp->next;
        FREE(tmp); // TODO reference for child
      }
    }
  }
}

ObjectType type_of(ID id) {
  return Objects[id].obj->type;
}

ID new_number_object(int64_t val) {

  NumberObject * number = (NumberObject*) MALLOC(sizeof(NumberObject));
  memset(number, 0, sizeof(NumberObject));
  number->S.type = OT_Number;

  ID id = allocate_id();
  Objects[id].obj = (SharedObject*) number;
  number->val = val;
  return id;
}

bool is_number_object(ID id) {
  if (Objects[id].obj->type == OT_Number)
    return true;
  return false;
}


int64_t number_object_value(ID id) {

  NumberObject * number;

  // TODO verify id in range

  number = (NumberObject*)Objects[id].obj;
  return number->val;
}

bool is_link_object(ID id) {

  // TODO verify id in range

  if (Objects[id].obj->type == OT_Link)
    return true;

  return false;
}

ID link_object_get_target(ID id) {

  // TODO verify

  LinkObject* link = (LinkObject*)Objects[id].obj;
  return link->target;
}

void link_object_set_target(ID id, ID target) {

  LinkObject* link = (LinkObject*)Objects[id].obj;
  ref_decr(link->target);
  link->target = target;
  ref_incr(link->target);
}

static int string_table_lookup(int index, uint32_t hash, const char* cstr) {

  StringNode * node = string_table[index];
  while (node) {
    // TODO assert?
    StringObject * istr = (StringObject *) Objects[node->id].obj;
    if (istr->hash == hash && strcmp((char*) istr->buf, cstr) == 0) {
      return node->id;
    }
  }

  return 0;
}

ID new_string_object(char* cstr) {

  ID id;

  // get string length
  int len = strlen(cstr);

  // calculate hash
  uint32_t hash = RBHash32(HASH_SEED, len, (uint8_t *) cstr);

  // get table entry index
  int index = STRINTERN_INDEX(hash);

  // find if exist?
  id = string_table_lookup(index, hash, cstr);
  if (id)
    return id;

  // alloc string_data TODO wrong size !!!
  StringObject * so = (StringObject*) MALLOC(STRING_OBJECT_SIZE(len));
  memset(so, 0, STRING_OBJECT_SIZE(len));
  so->S.type = OT_String;
  so->hash = hash;
  memcpy(so->buf, cstr, len);

  id = allocate_id();
  Objects[id].obj = (SharedObject*) so;

  StringNode* node = (StringNode *) MALLOC(sizeof(*node));
  memset(node, 0, sizeof(*node));
  node->id = id;

  // insert into tables
  node->next = string_table[index];
  string_table[index] = node;

  return id;
}

void delete_string_object(int id) {

  StringObject* istr = (StringObject*) Objects[id].obj;
  uint32_t hash = istr->hash;
  int index = STRINTERN_INDEX(hash);

  // remove string node out of table and free memory
  StringNode * node = string_table[index];
  if (node->id == id) {
    string_table[index] = node->next;
    FREE(node);
  }
  else {
    while (node->next) {
      if (node->next->id == id) {
        StringNode * tmp = node->next;
        node->next = tmp->next;
        FREE(tmp);
        break;
      }
    }
  }

  FREE(Objects[id].obj);
  deallocate_id(id);
}

ID new_link_object(ID target) {

  LinkObject * link = (LinkObject*) MALLOC(sizeof(LinkObject));
  memset(link, 0, sizeof(LinkObject));
  link->S.type = OT_Link;

  ID id = allocate_id();
  Objects[id].obj = (SharedObject*) link;
  return id;
}

// TODO no prototype yet
ID new_object_object() {

  ObjectObject * oo = (ObjectObject*) MALLOC(sizeof(ObjectObject));
  memset(oo, 0, sizeof(ObjectObject));
  oo->S.type = OT_Object;

  ID id = allocate_id();
  Objects[id].obj = (SharedObject*) oo;
  return id;
}

void create_undefined_object() {

  SharedObject* so = (SharedObject*) MALLOC(sizeof(SharedObject));
  memset(so, 0, sizeof(SharedObject));

  so->type = OT_Undefined;
  so->ref = MAX_REF;

  Objects[PREDEFINED_UNDEFINED].obj = so;
}

static void create_null_object() {

  SharedObject* so = (SharedObject*) MALLOC(sizeof(SharedObject));
  memset(so, 0, sizeof(SharedObject));

  so->type = OT_Null;
  so->ref = MAX_REF;

  Objects[PREDEFINED_NULL].obj = so;
}

void create_boolean_true() {

}

void create_boolean_false() {

}

int ref_incr(ID id) {

  SharedObject* so = Objects[id].obj;
  if (so->ref == MAX_REF) {
    return MAX_REF;
  }

  so->ref++;
  return so->ref;
}

int ref_decr(ID id) {

  SharedObject* so = Objects[id].obj;
  if (so->ref == MAX_REF) {
    return MAX_REF;
  }

  so->ref--;
  return so->ref;
}
