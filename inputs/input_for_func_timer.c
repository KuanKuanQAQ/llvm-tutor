#include <stdio.h>
#include <stdbool.h>

void log_time(const char *func, bool is_exit)
{
    printf("func: %s\n", func);
    printf("is_exit: %d\n", is_exit);
}

int foo(int a) {
  return a * 2;
}

int bar(int a, int b) {
  return (a + foo(b) * 2);
}

int fez(int a, int b, int c) {
  return (a + bar(a, b) * 2 + c * 3);
}

int main(int argc, char *argv[]) {
  int a = 123;
  int ret = 0;

  ret += foo(a);
  ret += bar(a, ret);
  ret += fez(a, ret, 123);

  return ret;
}
