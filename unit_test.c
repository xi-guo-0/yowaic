#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void assertStringEqual(char *s, char *t) {
  if (strcmp(s, t))
    error("Expected %s but got %s", s, t);
}

void assertEuqal(size_t s, size_t t) {
  if (s != t)
    error("Expected %u but got %u", s, t);
}

void test_string() {
  ywstr* ys = ywstrCreate("");
  ywstrAppend(ys, 'a');
  assertStringEqual("a", ywstrGet(ys));
  ywstrAppend(ys, 'b');
  assertStringEqual("ab", ywstrGet(ys));

  ywstrAppendFormat(ys, ".");
  assertStringEqual("ab.", ywstrGet(ys));
  ywstrAppendFormat(ys, "%s", "0123456789");
  assertStringEqual("ab.0123456789", ywstrGet(ys));
}

void test_list() {
  ywlist* yl = ywlistCreate();
  ywlistAppend(yl, (void *)1);
  ywlistAppend(yl, (void *)2);
  ywiter* iter = ywlistIter(yl);
  assertEuqal(1, (size_t)ywiterNext(iter));
  assertEuqal(false, (size_t)ywiterEnd(iter));
  assertEuqal(2, (size_t)ywiterNext(iter));
  assertEuqal(true, (size_t)ywiterEnd(iter));
  assertEuqal(0, (size_t)ywiterNext(iter));
  assertEuqal(true, (size_t)ywiterEnd(iter));
}

int main(int argc, char **argv) {
  test_string();
  test_list();
  printf("Passed\n");
  return 0;
}
