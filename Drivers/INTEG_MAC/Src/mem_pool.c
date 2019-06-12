/* 
  mem_pool.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"
#include "stm32f4xx_hal.h"


unsigned char *mem_pool[MAX_MEM_POOL_SIZE];
unsigned int f_pool, r_pool;


// �޸� Ǯ �ʱ�ȭ
void mem_pool_init(void) {
  int i;
  f_pool = r_pool = 0;
  
  for( i = 0; i < MAX_MEM_POOL_SIZE; i++) {
    return_mem((unsigned char *)malloc(sizeof(char) * MEM_SIZE));
  }
}

// �޸� ��û
unsigned char *get_mem(void) {
  if (r_pool == f_pool)	// empty
      return(0);
   f_pool = (f_pool + 1) % MAX_MEM_POOL_SIZE;
   return(mem_pool[f_pool]);
}

// �޸� ��ȯ
unsigned char return_mem(unsigned char *mem) {
  if ((r_pool + 1) % MAX_MEM_POOL_SIZE == f_pool)       // full
      return(0);
   r_pool = (r_pool + 1) % MAX_MEM_POOL_SIZE;
   
   // �޸� Clean
   memset(mem, '\0', MEM_SIZE);
   mem_pool[r_pool] = mem;
   return(1);
}