/*      

task.c

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "uart.h"
#include "task.h"
#include "integ_mac.h"
#include "frame_queue.h"
#include "hash.h"
#include "bluetooth.h"

struct task Task_q[MAX_TASK];
volatile int Task_f, Task_r;

void task_init()
{
  Task_f = Task_r = 0;
}

int task_insert(struct task *tskp)
{
  if ((Task_r + 1) % MAX_TASK == Task_f)
    return(0);
  Task_r = (Task_r + 1) % MAX_TASK;
  Task_q[Task_r] = *tskp;
  return(1);
}

int task_delete(struct task *tskp)
{
  if (Task_r == Task_f)
    return(0);
  Task_f = (Task_f + 1) % MAX_TASK;
  *tskp = Task_q[Task_f];
  return(1);
}

void task_cmd(void *arg)
{
  char buf[64], *cp0, *cp1, *cp2;
  int deviceType = 0;
  if (fgets(buf, 64, stdin) == NULL) {
    printf("$ ");
    return;
  }
  cp0 = strtok(buf, " \t\n\r");
  cp1 = strtok(NULL, " \t\n\r");
  
  // �ƹ��͵� �Է¾��� ���
  if (cp0 == NULL) {
    printf("$ ");
    return;
  }
  else if(!strcmp(cp0, "info")) {
    PrintAllHashData();
    cur_media = opt_media = BLUETOOTH;
  }
  else if(!strcmp(cp0, "s")) {
    INTEG_FRAME frame;
    frame.frame_length = INTEG_FRAME_HEADER_LEN + 0x05;
    memcpy(frame.src_address, my_integ_address, INTEG_ADDR_LEN); 
    memcpy(frame.dest_address, hood_integ_address, INTEG_ADDR_LEN); 
    frame.media_type = cur_media;
    frame.message_type = DATA_MSG;
    frame.data[0] = 0x65;
    frame.seqNumber = get_seq_number();
    frame_queue_insert((unsigned char *)&frame);
    //macDataReq(broadcast_addr, &frame, frame.frame_length);
  }
  else if (!strcmp(cp0, "init")) {
    // �߰� ���� �Է����� ���� ���
    if (cp1 == NULL) {
      printf("!!!- Usage : init deviceType([0]MASTER, [1]SLAVE\n");
      printf("$ ");
      return;
    }
    deviceType = atoi(cp1);
    if (deviceType < 0 || deviceType > 1) {
      printf("!!!- Wrong deviceType : [0]MASTER, [1]SLAVE\n");
      printf("$ ");
      return;
    }
  }
  // �߸��� ��ɾ� �Է��� ��� 
  else {
    printf("!!!-Wrong command\n");
  }
  printf("$ ");
}