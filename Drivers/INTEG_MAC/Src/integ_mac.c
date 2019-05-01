#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "integ_mac.h"
#include "frame_queue.h"
#include "task.h"
#include "timer.h"
#include "hash.h"
#include "bluetooth.h"                // blluetooth
#include "lifi.h"                          // lifi
#include "mac_interface.h"      // CC2530

#define STM32_UUID ((uint32_t *)0x1FFF7A10)

// �� ��ü �� �Լ� ������
unsigned char (*fun_init[MEDIA_NUM])(unsigned char) = {lifi_init, bluetooth_init, startMac};   // �ʱ�ȭ
unsigned char (*fun_send[MEDIA_NUM])(unsigned char* , unsigned char* , int ) = {lifi_send, bluetooth_send, macDataReq};    // ������ ����

// �̿� �ּ�
unsigned char hood_integ_address[INTEG_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char hood_lifi_address[LIFI_ADDR_LEN] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
unsigned char hood_bluetooth_address[BLUETOOTH_ADDR_LEN] =  {0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
unsigned char hood_cc2530_address[CC2530_ADDR_LEN] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

// �� �ּ�
unsigned char my_integ_address[INTEG_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char my_lifi_address[LIFI_ADDR_LEN] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
unsigned char my_bluetooth_address[BLUETOOTH_ADDR_LEN] =  {0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
unsigned char my_cc2530_address[CC2530_ADDR_LEN] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

unsigned char seqNumber;        // ���� ��ȣ
unsigned char cur_media;              // ���� ����ϴ� ��ü
unsigned char opt_media;              // ������ ��ü
unsigned char deviceType;            // ��ġ ����


#define TRANSMIT_FRAME 1
#define RECEIVE_FRAME 0
void integ_mac_handler(void * arg)
{
  struct task task, retrans_task;
  INTEG_FRAME *frame = NULL;
  INTEG_FRAME t_frame;
  unsigned char message_type;
  unsigned char result;
  unsigned char frame_state; // �۽ſ� ���ſ�
  
  while((frame = frame_queue_delete()) != NULL) {
    integ_print_frame(frame);
    message_type = frame->message_type;
    
    // �ٿ��� �ּ� �ʵ�� �ڽ��� �ּ� ��
    result = memcmp(my_integ_address, frame->src_address, INTEG_ADDR_LEN);
    if(result == 0) {
      frame_state = TRANSMIT_FRAME;
    }
    else {
      frame_state = RECEIVE_FRAME;
    }
    
    switch(message_type) {
    case DATA_MSG:      
      // ������ �۽� ��� 
      if(frame_state == TRANSMIT_FRAME) {
        printf("** Data �۽� seqNum %d\r\n", frame->seqNumber);
        cur_media = frame->media_type;
        
        // ������ ��⿭�� ������ �߰�
        frame->media_type = OPT_MEDIA;  // ������ �� ��� ������ ��ü�� ����ؼ� ����
        re_frame_queue_insert((unsigned char *)frame);
        frame->media_type = cur_media;
        
        // ������ Task �߰�
        retrans_task.fun = integ_retransmit_handler;
        strcpy(retrans_task.arg, "");
        insert_timer(&retrans_task, RETRANSMIT_TIME);
        
        // ������ �������� ��� ������ ��ü�� ����
        if(cur_media == OPT_MEDIA) {
          cur_media = opt_media;
          frame->media_type = cur_media;
        }
        
        // INTEG ADDR -> MAC ADDR ��ȯ
        struct node *table = FindHashData(HOOD_HASH_ID);
        
        if(table != NULL) {
          // �۽�
          fun_send[cur_media](table->data.media_addr[cur_media], (unsigned char *)frame, frame->frame_length);
        }
      }
      // ������ ���� �� ACK �۽�
      else if(frame_state == RECEIVE_FRAME) {
        printf("** Data ����\r\n");
        // ACK ��Ŷ ����
        t_frame.frame_length = frame->frame_length;
        t_frame.message_type = ACK_MSG;
        t_frame.media_type = frame->media_type;
        t_frame.ackNumber = frame->seqNumber + 1;
        memcpy(t_frame.dest_address, frame->src_address, INTEG_ADDR_LEN);
        memcpy(t_frame.src_address, my_integ_address, INTEG_ADDR_LEN);
        frame_queue_insert((unsigned char *)&t_frame);
        printf("** ACK ����\r\n");
      }
      break;
    case ACK_MSG:
      // ACK �۽� ���
      if(frame_state == TRANSMIT_FRAME) {
        printf("** ACK �۽�\r\n");
        
        // ACK �۽��� ���� ��ü��
        cur_media = frame->media_type;
        
        // INTEG ADDR -> MAC ADDR
        struct node *table = FindHashData(HOOD_HASH_ID);
        if(table != NULL) {
          // �۽�
          fun_send[cur_media](table->data.media_addr[cur_media], (unsigned char *)frame, frame->frame_length);
        }
      }
      // ACK ���� ��
      else if(frame_state == RECEIVE_FRAME) {
        int ackNumber;
        ackNumber = frame->ackNumber - 1;
        printf("** ACK ���� ackNum : %d\r\n", ackNumber + 1);
        // ������ ��⿭�� ������ ����
        re_frame_queue_remove(ackNumber % MAX_SEQ_NUMBER);
      }
      break;
    case PASS_MSG:
      printf("������ ���\r\n");
      break;
    }
  }
  HAL_Delay(1);
  task.fun = integ_mac_handler;
  strcpy(task.arg, "");
  task_insert(&task);
}


/*
������ ť�� �ִ� ������ �ϳ��� ������ ť�� ����.
*/
void integ_retransmit_handler(void * arg)
{
  INTEG_FRAME* t_frame;
  t_frame = re_frame_queue_delete();
  
  if(t_frame != NULL) {         //  ������ �� ������ �ִ� ��� ������ ������ ť�� ����
    frame_queue_insert((unsigned char *)t_frame);
  }
}

void integ_mac_init(void)
{
  int i, result;
  struct node *table;   // MAC Table ����
  
  seqNumber = DEFAULT_SEQ_NUMBER;
  frame_queue_init();
  re_frame_queue_init();
  
  my_integ_address[0] = LSB(STM32_UUID[0]);
  my_cc2530_address[0] = LSB(STM32_UUID[0]);
  my_lifi_address[0] = LSB(STM32_UUID[0]);
  my_bluetooth_address[0] = LSB(STM32_UUID[0]);
  if (LSB(STM32_UUID[0]) == 0x2c) {
    hood_integ_address[0] = 0x2E;
    hood_cc2530_address[0] = 0x2E;
    hood_lifi_address[0] = 0x2E;
    hood_bluetooth_address[0] = 0x2E;
    deviceType = MASTER;
  }
  else {
    hood_integ_address[0] = 0x2c;
    hood_cc2530_address[0] = 0x2c;
    hood_lifi_address[0] = 0x2c;
    hood_bluetooth_address[0] = 0x2c;
    deviceType = SLAVE;
  }
  
  table = get_hashNode();
  table->id = LSB(STM32_UUID[0]);
  table->data.addr_type = STATIC_ADDR;
  memcpy(table->data.integ_addr, my_integ_address, INTEG_ADDR_LEN);
  table->data.media_addr[LIFI] = my_lifi_address;
  table->data.media_addr[BLUETOOTH] = my_bluetooth_address;
  table->data.media_addr[CC2530] = my_cc2530_address;
  AddHashData(table->id, table);
  
  table = get_hashNode();
  table->id = HOOD_HASH_ID;
  table->data.addr_type = DYNAMIC_ADDR;
  memcpy(table->data.integ_addr, hood_integ_address, INTEG_ADDR_LEN);
  table->data.media_addr[LIFI] = hood_lifi_address;
  table->data.media_addr[BLUETOOTH] = hood_bluetooth_address;
  table->data.media_addr[CC2530] = hood_cc2530_address;
  AddHashData(table->id, table);
  
  // MCU <---> ��ü ��� �ʱ�ȭ
  
  // ��ü �ʱ�ȭ
  for(i = 0; i < MEDIA_NUM; i++) {
    result = fun_init[i](deviceType);
    printf("* [%s] �ʱ�ȭ %s \r\n", media_name[i], result_string[result]); 
  }
  // ���� ��ü ����
  //opt_media = cur_media = BLUETOOTH;
  opt_media = cur_media = BLUETOOTH;
  
  integ_mac_handler("");
  
  struct task task;
  task.fun = integ_find_opt_link;
  strcpy(task.arg, "");
  insert_timer(&task, FIND_OPT_PERIOD);
}

void integ_find_opt_link(void * arg)
{
  struct task task;
  task.fun = integ_find_opt_link;
  strcpy(task.arg, "");
  insert_timer(&task, FIND_OPT_PERIOD);
  opt_media = (rand() % 2) + 1;
  //opt_media = CC2530;
  //printf("������ũ : %s\r\n", media_name[opt_media]);
}

// ������ ���
void integ_print_frame(INTEG_FRAME *frame)
{
  int i;
  
  printf("----------\r\n");
  printf("Source Address : ");
  for(i = 0; i < INTEG_ADDR_LEN; i++) {
    printf("%02x ", frame->src_address[i]);
  }
  printf(" | Dest Address : ");
  for(i = 0; i < INTEG_ADDR_LEN; i++) {
    printf("%02x ", frame->dest_address[i]);
  }
  printf("\r\nLength : %d | msgType : %d | mediaType : %d | seqNumber : %d | ackNumber : %d\r\n", frame->frame_length, frame->message_type, frame->media_type, frame->seqNumber, frame->ackNumber);
  printf("----------\r\n");
  
}

unsigned char get_seq_number(void)
{
  unsigned char return_value = seqNumber;
  seqNumber = (seqNumber + 1) % MAX_SEQ_NUMBER;
  return return_value;
}