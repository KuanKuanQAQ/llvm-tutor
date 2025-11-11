#include <stdio.h>
#include <stdbool.h>

int foo(int);
int bar(int, int);

int apple(int a) {
  return foo(a * 2);
}

int beer(int a, int b) {
  return (a + bar(b, b*2) * 2);
}
