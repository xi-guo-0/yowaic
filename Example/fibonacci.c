int fibonacci(int n) {
  int a = 0;
  int b = 1;
  int c = 1;
  for (int i = 0; i < n; i = i + 1) {
    printf("%d\n", a);
    c = a + b;
    a = b;
    b = c;
  }
  return 0;
}

int test_array() {
  int a[]={20,30,40};
  int *b=a+1;
  printf("*b = %d\n", *b);
  char str[] = "this is a single string";
  char* p = str;
  for(; *p; p = p + 1) {
    printf("%c ", *p);
  }
  printf("\n");
  for(p = str; *p; p = p + 2) {
    printf("%c ", *p);
  }
  printf("\n");
  return 0;
}

int main() {
  fibonacci(10+ 3);
  test_array();
  return 0;
}
