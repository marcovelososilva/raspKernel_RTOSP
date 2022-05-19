/*
 * Test the rtosp syscall (#449)
 */
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define SYS_rtosp 449

int main()
{
  pid_t user_array[10];
  int size = 0;
  long res;
  res = syscall(SYS_rtosp, &user_array, &size);

  printf("Syscall returned %d\n", res);
  printf("%d processes w/ RTOSP set to 1\n", size);

  for(int i = 0; i<size; i++)
  {
    printf("PID: %d\n", user_array[i]);
  }
  return 0;
}