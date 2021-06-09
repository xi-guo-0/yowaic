// util.c
// commonly used utility functions
// Copyright (C) 2018: see LICENSE
#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void errorf(char *file, int line, char *fmt, ...) {
  fprintf(stderr, "%s:%d: ", file, line);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

void warningf(char *file, int line, char *fmt, ...) {
  fprintf(stderr, "%s:%d: ", file, line);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}

char *ywstrCopy(char *s) {
  char *p = malloc(sizeof(char) * (strlen(s) + 1));
  strcpy(p, s);
  return p;
}

static void ywstrRealloc(ywstr *ys) {
  ys->size += ys->size >> 1;
  char *new_stack = malloc(sizeof(char) * ys->size);
  strncpy(new_stack, ys->stack, ys->size);
  free(ys->stack);
  ys->stack = new_stack;
}

ywstr *ywstrCreate(char *s) {
  ywstr *ys = malloc(sizeof(ywstr));
  ys->size = 16;
  ys->length = strlen(s);
  while (ys->length >= ys->size) {
    ys->size += ys->size >> 1;
  }
  ys->stack = malloc(sizeof(char) * ys->size);
  strncpy(ys->stack, s, ys->length);
  return ys;
}
char *ywstrGet(ywstr *ys) {
  return ywstrCopy(ys->stack);
}
void ywstrAppend(ywstr *ys, char c) {
  if (ys->length + 1 == ys->size)
    ywstrRealloc(ys);
  ys->stack[ys->length] = c;
  ys->stack[ys->length + 1] = '\0';
  ys->length += 1;
}

void ywstrAppendFormat(ywstr *ys, char *fmt, ...) {
  char s2[256];
  va_list ap;
  va_start(ap, fmt);
  if (0 > vsprintf(s2, fmt, ap))
    error("Format string too long");
  va_end(ap);
  int s2_length = strlen(s2);
  while (ys->length + s2_length + 1 >= ys->size) {
    ywstrRealloc(ys);
  }
  strncpy(ys->stack + ys->length, s2, s2_length);
  ys->length += s2_length;  
}

ywlist* ywlistCreate() {
  ywlist* ret = malloc(sizeof(ywlist));
  ret->length = 0;
  ret->head = NULL;
  ret->tail = NULL;
  return ret;
}

void ywlistAppend(ywlist* yl, void* element) {
  ywlist_node* node = malloc(sizeof(ywlist_node));
  node->element = element;
  node->next = NULL;
  if (!yl->head)
    yl->head = node;
  else
    yl->tail->next = node;
  yl->tail = node;
  yl->length++;
}
size_t ywlistLen(ywlist* yl) {
  return yl->length;
}
ywiter* ywlistIter(ywlist* yl) {
  ywiter* ret = malloc(sizeof(ywiter));
  ret->ptr = yl->head;
  return ret;
}
void* ywiterNext(ywiter* iter) {
  if (!iter->ptr)
    return NULL;
  void* ret = iter->ptr->element;
  iter->ptr = iter->ptr->next;
  return ret;
}
bool ywiterEnd(ywiter* iter) {
  return !iter->ptr;
}

