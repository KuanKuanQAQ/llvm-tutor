#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

int apple(int);
int beer(int, int);

void x(char *funcName, bool isExit) {
  if (!isExit) {
    printf("entry %s\n", funcName);
  } else {
    printf("exit %s\n", funcName);
  }
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
  srand((unsigned)time(NULL));

  int a = 123;
  int ret = 0;

  int (*random_func)(int) = NULL;
  if (rand() % 2 == 0) {
    random_func = foo;
  } else {
    random_func = (int (*)(int))bar; // 强转不管第二个参数了
  }

  ret += random_func(a);
  ret += fez(a, ret, 123);
  ret += apple(a);
  ret += beer(a, ret);

  return ret;
}