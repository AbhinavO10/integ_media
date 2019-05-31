#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "integ_mac.h"
#include "frame_queue.h"
#include "task.h"
#include "timer.h"
#include "hash.h"
#include "bluetooth.h"                // blluetooth
#include "lifi.h"                          // lifi
#include "mac_interface.h"      // CC2530
#include "uart.h"
#include "display.h"
#include "mem_pool.h"

#define STM32_UUID ((uint32_t *)0x1FFF7A10)

unsigned char testBuf_2[120] = {'1','1','1','1','1','1','1','1',
'2','2','2','2','2','2','2','2',
'3','3','3','3','3','3','3','3',
'4','4','4','4','4','4','4','4',
'5','5','5','5','5','5','5','5',
'6','6','6','6','6','6','6','6',
'7','7','7','7','7','7','7','7',
'8','8','8','8','8','8','8','8',
'9','9','9','9','9','9','9','9',
'1','1','1','1','1','1','1','1',
'2','2','2','2','2','2','2','2',
'3','3','3','3','3','3','3','3',
'4','4','4','4','4','4','4','4',
'5','5','5','5','5','5','5','5',
'6','6','6','6','6','6','6','6'
};

unsigned char testBuf_recv[120];

// dispaly �� ����
char message_buffer[COL_NUMS];

// �� ��ü �� �Լ� ������
unsigned char (*fun_init[MEDIA_NUM])(unsigned char) = {lifi_init, bluetooth_init, startMac};   // �� ��ü �ʱ�ȭ �Լ� ������
unsigned char (*fun_send[MEDIA_NUM])(unsigned char* , unsigned char* , int ) = {lifi_send, bluetooth_send, macDataReq};    // �� ��ü ������ ���� �Լ� ������
unsigned char* (*fun_get_addr[MEDIA_NUM])(unsigned char) = {lifi_get_mac_addr, bt_get_mac_addr, cc2530_get_mac_addr};    // �� ��ü �ּ� ��� �Լ� ������


// �̿� �ּ�
unsigned char hood_integ_address[INTEG_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/*
unsigned char hood_lifi_address[LIFI_ADDR_LEN] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
unsigned char hood_bluetooth_address[BLUETOOTH_ADDR_LEN] =  {0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
unsigned char hood_cc2530_address[CC2530_ADDR_LEN] = {0x11, 0x67, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
*/

// �� �ּ�
unsigned char integ_macAddr[INTEG_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char my_integ_address[INTEG_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char my_lifi_address[LIFI_ADDR_LEN] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
unsigned char my_bluetooth_address[BLUETOOTH_ADDR_LEN] =  {0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
unsigned char my_cc2530_address[CC2530_ADDR_LEN] = {0x11, 0x67, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

// ��ε�ĳ��Ʈ �� ���� MAC �ּ�
unsigned char integ_broadcast_addr[INTEG_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

INTEG_FRAME advertising_frame;
INTEG_FRAME t_frame;          // �ӽÿ� ������ ����

unsigned char seqNumber;        // ���� ��ȣ
unsigned char cur_media;              // ���� ����ϴ� ��ü
unsigned char prev_media;             // ������ ����� ��ü
unsigned char opt_media;              // ������ ��ü
unsigned char deviceType;            // ��ġ ����

// ��ü ���� ���̺�
unsigned char STATUS_TABLE[STATUS_NUM][MEDIA_NUM] = {{R_FAIL, R_FAIL, R_FAIL}, {DISCON, DISCON, DISCON}};

// ���� MAC �ʱ�ȭ ���� (1 : �ʱ�ȭ �Ϸ�)
unsigned char integ_init_state = 0;
unsigned char fragment_id;

#define TRANSMIT_FRAME 1
#define RECEIVE_FRAME 0
void integ_mac_handler(void * arg)
{
  struct task task, retrans_task;
  struct node *table = NULL;   // MAC ���̺� ������ ����
  INTEG_FRAME *frame = NULL;    // ���� ������ ť���� ������ �������� ����Ű�� ������
  unsigned char message_type;   // �޽��� ���� 
  unsigned char result;         // �Լ� ���� ��� ����� ����
  unsigned char frame_state; // �۽ſ� ���ſ� ����
  unsigned char mac_table_key;  // �����̺� ���� �� Ű
  int i;        // for �ݺ��� ��
  
  while((frame = frame_queue_delete()) != NULL) {
    //integ_print_frame(frame);
    message_type = frame->message_type;
    
    // �ٿ��� �ּ� �ʵ�� �ڽ��� �ּ� ��
    result = memcmp(my_integ_address, frame->src_address, INTEG_ADDR_LEN);
    if(result == 0) {
      frame_state = TRANSMIT_FRAME;
    }
    else {
      frame_state = RECEIVE_FRAME;
    }
    
    // �����̺� ���� �� Ű �ּ� ù����Ʈ
    mac_table_key = frame->dest_address[0];
    
    // �����̺� ��������
    table = FindHashData(mac_table_key);
    
    switch(message_type) {
    case DATA_MSG:      
      // ������ �۽� ��� 
      if(frame_state == TRANSMIT_FRAME) {
        cur_media = frame->media_type;
        
        // ������ �������� ��� ������ ��ü�� ����
        if((cur_media & OPT_MEDIA) == OPT_MEDIA) {
          prev_media = cur_media & 0x0F;
          
          // ackNumber �ʵ� �� �������� Ƚ���� ����Ѵ�. (�ӽ�)
          frame->ackNumber++;
          
          // ������ ��⿭�� ������ �߰�
          re_frame_queue_insert((unsigned char *)frame);
          sprintf(message_buffer, "%d �� ������ ���� ���� \r\n", frame->ackNumber);
          insert_display_message(prev_media, message_buffer);
          
          cur_media = prev_media;
          // ������ Ƚ���� 3ȸ�̸� ��ü ���� ���� �������� ����
          if(frame->ackNumber >= RETRANSMIT_NUM) {
            // ���� ���� ��ü ���� ���� ����
            STATUS_TABLE[CONNECT_STATUS][prev_media] = DISCON;
            integ_find_opt_link(NULL);
            cur_media = opt_media;
          }
          frame->media_type = OPT_MEDIA | cur_media;
          
          // ����ȭ ��Ŷ�̸鼭 ������ü�� ����� ���
          if((frame->fragment_offset & 0x80) == 0x80 && (prev_media != cur_media)) {
            
            // Don't fragment 
            if((frame->fragment_offset & 0x40) != 0x40) {

              if (media_mtu_size[prev_media] < media_mtu_size[cur_media]) { //  ����ȭ ������ ��ħ
                int max_octet_length = (media_mtu_size[cur_media]  - INTEG_FRAME_HEADER_LEN) / 8;
                frame->frame_length = INTEG_FRAME_HEADER_LEN + max_octet_length * 8;
                
                // ������ �����ӵ� ����
                int count = 0;
                while (count != ((frame->frame_length - INTEG_FRAME_HEADER_LEN) / (MIN_MTU_SIZE - INTEG_FRAME_HEADER_LEN))) {                
                  // ���� ������ ť���� ����
                  frame_queue_remove((frame->seqNumber + count) % MAX_SEQ_NUMBER);
                  // ������ ��⿭�� ������ ����
                  re_frame_queue_remove((frame->seqNumber + count) % MAX_SEQ_NUMBER);
                }
                frame->message_type = DATA_MSG;
              }
              
              else {      //����ȭ ������ ����
                int count = (media_mtu_size[prev_media] - INTEG_FRAME_HEADER_LEN) / (media_mtu_size[cur_media] - INTEG_FRAME_HEADER_LEN);
                int cur_octet_length = (media_mtu_size[cur_media]  - INTEG_FRAME_HEADER_LEN) / 8;
                for (i = 0; i < count; i++) {
                  // ���� ������ ť���� ����
                  frame_queue_remove((frame->seqNumber + i) % MAX_SEQ_NUMBER);
                  // ������ ��⿭�� ������ ����
                  re_frame_queue_remove((frame->seqNumber + i) % MAX_SEQ_NUMBER);
                  
                  memcpy(&t_frame, frame, INTEG_FRAME_HEADER_LEN);
                  t_frame.seqNumber = (frame->seqNumber + i) % MAX_SEQ_NUMBER;
                  t_frame.frame_length = INTEG_FRAME_HEADER_LEN + (cur_octet_length * 8);
                  t_frame.message_type = DATA_MSG;
                  t_frame.fragment_offset = t_frame.fragment_offset + (i * cur_octet_length);
                  t_frame.fragment_offset |= 0x40;        // DF ����
                  t_frame.data = get_mem();
                  memcpy(t_frame.data, frame->data + (i * 8), cur_octet_length * 8);
                  frame_queue_insert((unsigned char *)&t_frame);
                  
                  
                }
              }
            }
          }
          
          sprintf(message_buffer, "[SEQ : %d] ������ ������ (�� ������ : %02X)\r\n", frame->seqNumber, frame->dest_address[0]);
          // sprintf(message_buffer, "[SEQ : %d] ������ ������ (�� ������ : %02X) : %s\r\n", frame->seqNumber, frame->dest_address[0], frame->data);
          insert_display_message(cur_media, message_buffer);
        }
        // ó�� ������ ���
        else {
          // �����Ϸ��� �������� �� ��ü �ּ� MTU ũ�⺸�� ū ��� 
          if(frame->frame_length > MIN_MTU_SIZE) {
            
            // ����ȭ �غ�
            int i;
            int total_data_len = frame->frame_length - INTEG_FRAME_HEADER_LEN;
            
            int fragment_count = ceil((total_data_len / (double) (MIN_MTU_SIZE - INTEG_FRAME_HEADER_LEN)));
            
            //printf("%d %d %d", fragment_count, frame->frame_length, MIN_MTU_SIZE);
            
            memcpy(&t_frame, frame, INTEG_FRAME_HEADER_LEN);
            t_frame.fragment_id = DEFAULT_FRAGMENT_ID;
            t_frame.fragment_offset = 0;       
            t_frame.media_type = opt_media;
            //t_frame.media_type = 0xF0 | 0x01;
            
            int min_octet_length = (MIN_MTU_SIZE - INTEG_FRAME_HEADER_LEN) / 8;
            int max_octet_length = (MAX_MTU_SIZE - INTEG_FRAME_HEADER_LEN) / 8;
            for (i = 0; i < fragment_count; i++) {
              t_frame.frame_length = INTEG_FRAME_HEADER_LEN + min_octet_length * 8;
              if (i >= 1) {
                t_frame.seqNumber = get_seq_number();
              }
              
              t_frame.fragment_offset = i * min_octet_length; 
              if (i != (fragment_count - 1)) {             // ������ ��Ŷ �ƴϸ� MF ����
                t_frame.fragment_offset |= 0x80;
              }
              else {
                t_frame.fragment_offset |= 0x40;        // ������ ��Ŷ DF ����
              } 
              
              t_frame.data = get_mem();
              memcpy(t_frame.data, frame->data + ((t_frame.fragment_offset & 0x3F) * 8), min_octet_length * 8);
              frame_queue_insert((unsigned char *)&t_frame);
            }
            continue;
          }
          // �����Ϸ��� �������� �� ��ü �ּ� MTU ũ�⺸�� ���� ��� ����ȭ ���� �ٷ� ����
          else {
            frame->media_type ^= OPT_MEDIA;  
            re_frame_queue_insert((unsigned char *)frame);
            frame->media_type = cur_media;
            
            sprintf(message_buffer, "[SEQ : %d] ������ ���� ( �� ������ : %02X) \r\n", frame->seqNumber, frame->dest_address[0]);
            // sprintf(message_buffer, "[SEQ : %d] ������ ���� ( �� ������ : %02X) : %s\r\n", frame->seqNumber, frame->dest_address[0], frame->data);
            insert_display_message(cur_media, message_buffer);
          }
        }
        
        // ������ Task �߰�
        retrans_task.fun = integ_retransmit_handler;
        strcpy(retrans_task.arg, "");
        insert_timer(&retrans_task, RETRANSMIT_TIME);
        
        // INTEG ADDR -> MAC ADDR ��ȯ
        
        if(table != NULL) {
          // �۽�
          //printf("cur_media = %d\r\n", cur_media);
          fun_send[cur_media](table->data.media_addr[cur_media], (unsigned char *)frame, frame->frame_length);
        }
      }
      // ������ ���� �� ACK ����
      else if(frame_state == RECEIVE_FRAME) {
        //printf("** Data ����\r\n");
        
        // ����ȭ ������ ���� ��
        // MF�� ���õǾ� �ְų�, fragment_offset�� 0�� �ƴ� ���
        unsigned char more_flag = frame->fragment_offset & 0x80;
        unsigned char offset = frame->fragment_offset & 0x3F;
        
        // ����
        if ((more_flag == 0x80 )|| (offset != 0)){
          memcpy(testBuf_recv + (offset * 8), frame->data, frame->frame_length - INTEG_FRAME_HEADER_LEN);
        }

        
        // ACK ��Ŷ ����
        t_frame.frame_length = frame->frame_length;
        t_frame.message_type = ACK_MSG;
        t_frame.media_type = frame->media_type & 0x0F;
        t_frame.ackNumber = frame->seqNumber + 1;
        t_frame.data = NULL;
        memcpy(t_frame.dest_address, frame->src_address, INTEG_ADDR_LEN);
        memcpy(t_frame.src_address, my_integ_address, INTEG_ADDR_LEN);
        
        sprintf(message_buffer, "[SEQ : %d] ������ ���� ( �� �ٿ��� : %02X) \r\n",  frame->seqNumber, frame->src_address[0]);
        // sprintf(message_buffer, "[SEQ : %d] ������ ���� ( �� �ٿ��� : %02X) : %s\r\n",  frame->seqNumber, frame->src_address[0], frame->data);
        insert_display_message(t_frame.media_type, message_buffer);
        
        // �޸� -> Ǯ ��ȯ
        return_mem(frame->data);
        frame->data = NULL;
        
        // ���� ��ü ���� ���� ����
        STATUS_TABLE[CONNECT_STATUS][t_frame.media_type] = CON;
        
        frame_queue_insert((unsigned char *)&t_frame);
        //printf("** ACK ����\r\n");
      }
      break;
    case ACK_MSG:
      // ACK �۽� ���
      if(frame_state == TRANSMIT_FRAME) {
        //printf("** ACK �۽�\r\n");
        
        // ACK �۽��� ���� ��ü��
        cur_media = frame->media_type;
        
        // INTEG ADDR -> MAC ADDR
        if(table != NULL) {
          // �۽�
          //printf("cur_media = %d\r\n", cur_media);
          fun_send[cur_media](table->data.media_addr[cur_media], (unsigned char *)frame, frame->frame_length);
        }
        
        sprintf(message_buffer, "[ACK : %d] ACK �۽� ( �� ������ : %02X) \r\n",  frame->ackNumber, frame->dest_address[0]);
        insert_display_message(cur_media, message_buffer);
        
      }
      // ACK ���� ��
      else if(frame_state == RECEIVE_FRAME) {
        int ackNumber;
        
        // ���� ��ü ���� ���� ����
        STATUS_TABLE[CONNECT_STATUS][frame->media_type] = CON;
        
        ackNumber = frame->ackNumber - 1;
        // ������ ��⿭�� ������ ����
        re_frame_queue_remove(ackNumber % MAX_SEQ_NUMBER);
        
        sprintf(message_buffer, "[ACK : %d] ACK ���� ( ��ٿ��� : %02X) \r\n",  frame->ackNumber, frame->src_address[0]);
        insert_display_message(frame->media_type, message_buffer);
        
        // �޸� -> Ǯ ��ȯ
        return_mem(frame->data);
        frame->data = NULL;
      }
      break;
    case ADV_MSG:
      // ADV_MSG �۽� ������
      if(frame_state == TRANSMIT_FRAME) {
        // INTEG ADDR -> MAC ADDR ��ȯ
        if(table != NULL) {
          // �۽�
          
          for(i = 0; i < MEDIA_NUM; i++) {
            frame->media_type = i;
            //printf("%s ADV_MSG �۽�\r\n", media_name[frame->media_type]);
            fun_send[i](table->data.media_addr[i], (unsigned char *)frame, frame->frame_length);
            HAL_Delay(30);
          }
        }
        frame->data = NULL;
        //frame_queue_insert((unsigned char *)frame);
      }
      // ADV_MSG ���� ���� ��� 
      else if(frame_state == RECEIVE_FRAME) {
        //printf("%s ADV ����\r\n", media_name[frame->media_type]);
        
        // ADV_MSG �۽����� �����̺� ��������
        mac_table_key = frame->src_address[0];
        table = FindHashData(mac_table_key);
        
        // ���ο� �̿��� �����̺� �߰�
        if(table == NULL) {
          // �̿� �� ���̺� ����
          table = get_hashNode();
          table->id = mac_table_key;
          table->data.addr_type = DYNAMIC_ADDR;
          memcpy(table->data.integ_addr, frame->src_address, INTEG_ADDR_LEN);
          memcpy(table->data.media_addr, frame->data, MEDIA_ADDR_LEN_MAX * MEDIA_NUM);
          AddHashData(table->id, table);
          
          
          sprintf(message_buffer, "���ο� �̿���� (���� MAC �ּ� : %02X) �߰� MAC TABLE �߰�\r\n", table->data.integ_addr[0]);
          insert_display_message(INTEG_MSG, message_buffer);
        }
        
        // ADV_MSG �۽��ڿ��� MAC ���̺��� �����ؼ� ADV_IND ����
        t_frame.data = get_mem();
        t_frame.frame_length = frame->frame_length;
        t_frame.message_type = ADV_IND;
        t_frame.media_type = frame->media_type;
        memcpy(t_frame.src_address, my_integ_address, INTEG_ADDR_LEN);
        memcpy(t_frame.dest_address, frame->src_address, INTEG_ADDR_LEN);
        memcpy(t_frame.data, advertising_frame.data, MEDIA_ADDR_LEN_MAX * MEDIA_NUM);
        frame_queue_insert((unsigned char *)&t_frame);
        
        // �޸� -> Ǯ ��ȯ
        return_mem(frame->data);
        frame->data = NULL;
      }
      break;
      
    case ADV_IND:
      // ADV_IND �۽� ������
      if(frame_state == TRANSMIT_FRAME) {
        //printf("%s ADV_IND �۽�\r\n", media_name[frame->media_type]);
        
        // INTEG ADDR -> MAC ADDR ��ȯ
        if(table != NULL) {
          // �۽�
          HAL_Delay(30);
          fun_send[frame->media_type](table->data.media_addr[frame->media_type], (unsigned char *)frame, frame->frame_length);
        }
        // �޸� -> Ǯ ��ȯ
        return_mem(frame->data);
        frame->data = NULL;
      }
      // ADV_IND ���� ���� ��� 
      else if(frame_state == RECEIVE_FRAME) {
        //printf("%s ADV_IND ����\r\n", media_name[frame->media_type]);
        
        // ADV_IND �۽����� �����̺� ��������
        mac_table_key = frame->src_address[0];
        table = FindHashData(mac_table_key);
        
        // ���ο� �̿��� �����̺� �߰�
        if(table == NULL) {
          // �̿� �� ���̺� ����
          table = get_hashNode();
          table->id = mac_table_key;
          table->data.addr_type = DYNAMIC_ADDR;
          memcpy(table->data.integ_addr, frame->src_address, INTEG_ADDR_LEN);
          memcpy(table->data.media_addr, frame->data, MEDIA_ADDR_LEN_MAX * MEDIA_NUM);
          AddHashData(table->id, table);
          
          sprintf(message_buffer, "���ο� �̿���� (���� MAC �ּ� : %02X) �߰� MAC TABLE �߰�\r\n", table->data.integ_addr[0]);
          insert_display_message(INTEG_MSG, message_buffer);
        }
        
        // �޸� -> Ǯ ��ȯ
        return_mem(frame->data);
        frame->data = NULL;
      }
      break;
      
    case PASS_MSG:
      //printf("������ ���\r\n");
      
      // �޸� -> Ǯ ��ȯ
      return_mem(frame->data);
      frame->data = NULL;
      break;
    default:
      sprintf(message_buffer, "�߸��� ���� MAC ������ (Error Type : %02X)\r\n", frame->message_type);
      insert_display_message(INTEG_MSG, message_buffer);
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
  
  insert_display_message(INTEG_MSG, "���� MAC �ʱ�ȭ ����\r\n");
  
  fragment_id = DEFAULT_FRAGMENT_ID;
  seqNumber = DEFAULT_SEQ_NUMBER;        // ������ȣ �ʱ�ȭ
  frame_queue_init();                                   // ���� ������ ť �ʱ�ȭ
  re_frame_queue_init();                                // ������ ������ ť �ʱ�ȭ
  
  
  advertising_frame.data = get_mem();
  
  // �ӽ� 
  if (LSB(STM32_UUID[0]) == 0x2c) {
    hood_integ_address[0] = 0x2E;
    /*
    hood_cc2530_address[0] = 0x2E;
    hood_lifi_address[0] = 0x2E;
    hood_bluetooth_address[0] = 0x2E;
    */
    deviceType = MASTER;
  }
  else {
    hood_integ_address[0] = 0x2c;
    /*
    hood_cc2530_address[0] = 0x2c;
    hood_lifi_address[0] = 0x2c;
    hood_bluetooth_address[0] = 0x2c;
    */
    deviceType = SLAVE;
  }
  
  // MCU <---> ��ü ��� �ʱ�ȭ
  
  // ��ü �ʱ�ȭ
  for(i = 0; i < MEDIA_NUM; i++) {
    result = fun_init[i](deviceType);
    STATUS_TABLE[INIT_STATUS][i] = result;
    STATUS_TABLE[CONNECT_STATUS][i] = result;
    sprintf(message_buffer, "�ʱ�ȭ %s \r\n", result_string[result]);
    insert_display_message(i, message_buffer); 
  }
  
  // �� ��ü �ּ� ��������
  memcpy(my_integ_address, integ_get_mac_addr(MAC_ADDR), INTEG_ADDR_LEN);               // ����
  memcpy(my_cc2530_address, cc2530_get_mac_addr(MAC_ADDR), CC2530_ADDR_LEN);        // CC2530
  memcpy(my_bluetooth_address, bt_get_mac_addr(MAC_ADDR), BLUETOOTH_ADDR_LEN);    // BT
  memcpy(my_lifi_address, lifi_get_mac_addr(MAC_ADDR), LIFI_ADDR_LEN);                           // LI-FI
  
  // advertising frame ����
  advertising_frame.frame_length = INTEG_FRAME_HEADER_LEN + MEDIA_NUM * MEDIA_ADDR_LEN_MAX;
  advertising_frame.message_type = ADV_MSG;
  memcpy(advertising_frame.src_address, my_integ_address, INTEG_ADDR_LEN);
  memcpy(advertising_frame.dest_address, integ_broadcast_addr, INTEG_ADDR_LEN);
  for(i = 0; i < MEDIA_NUM; i++) {
    memcpy(advertising_frame.data + (i * MEDIA_ADDR_LEN_MAX), fun_get_addr[i](MAC_ADDR), MEDIA_ADDR_LEN_MAX);
  }
  
  // advertising �۽� frame ����
  frame_queue_insert((unsigned char *)&advertising_frame);
  
  // �ڽ��� �� ���̺� ����
  table = get_hashNode();
  table->id = LSB(STM32_UUID[0]);
  
  table->data.addr_type = STATIC_ADDR;
  memcpy(table->data.integ_addr, my_integ_address, INTEG_ADDR_LEN);
  memcpy(table->data.media_addr, advertising_frame.data, MEDIA_ADDR_LEN_MAX * MEDIA_NUM);
  AddHashData(table->id, table);
  
  // Boradcast MAC table ����
  table = get_hashNode();
  table->id = 0xFF;
  
  table->data.addr_type = STATIC_ADDR;
  memcpy(table->data.integ_addr, integ_broadcast_addr, INTEG_ADDR_LEN);
  for(i = 0; i < MEDIA_NUM; i++) {
    memcpy(table->data.media_addr[i], fun_get_addr[i](BROADCAST_ADDR), MEDIA_ADDR_LEN_MAX);
  }
  AddHashData(table->id, table);
  
  
  /*
  // �̿� �� ���̺� ����
  table = get_hashNode();
  table->id = HOOD_HASH_ID;
  table->data.addr_type = DYNAMIC_ADDR;
  memcpy(table->data.integ_addr, hood_integ_address, INTEG_ADDR_LEN);
  
  memcpy(table->data.media_addr[LIFI], hood_lifi_address, MEDIA_ADDR_LEN_MAX);
  memcpy(table->data.media_addr[BLUETOOTH], hood_bluetooth_address, MEDIA_ADDR_LEN_MAX);
  memcpy(table->data.media_addr[CC2530], hood_cc2530_address, MEDIA_ADDR_LEN_MAX);
  AddHashData(table->id, table);
  */
  
  // ���� ��ü ����
  integ_find_opt_link(NULL);
  
  // �ʱ�ȭ �Ϸ� ����
  integ_init_state = 1;
  insert_display_message(INTEG_MSG, "���� MAC �ʱ�ȭ �Ϸ�\r\n");
  
  // ù ��� ������ ���� ��ü ����
  cur_media = opt_media;
  
  // ���� MAC Handler TASK ����
  integ_mac_handler("");
  
  
  /*
  struct task task;
  task.fun = integ_find_opt_link;
  strcpy(task.arg, "");
  insert_timer(&task, FIND_OPT_PERIOD);
  
  */
}

void integ_find_opt_link(void * arg)
{
  struct task task;
  int i,prev_media;
  
  
  prev_media = opt_media;
  
  // �ֱ����� TASK ����
  task.fun = integ_find_opt_link;
  strcpy(task.arg, "");
  //insert_timer(&task, FIND_OPT_PERIOD);
  
  for(i = 0; i < MEDIA_NUM; i++) {
    if(STATUS_TABLE[INIT_STATUS][i] && STATUS_TABLE[CONNECT_STATUS][i]) {
      opt_media = i;
      break;
      //printf("������ü : %s\r\n", media_name[opt_media]);
    }
  }
  
  // ����� ��ü�� ���� ���
  if(i == MEDIA_NUM) {
    opt_media = rand() % MEDIA_NUM;
    sprintf(message_buffer, "���� ���� ��ü ����. [%s] ���� ��ü ���� \r\n", media_name[opt_media]);
    insert_display_message(INTEG_MSG, message_buffer);
  }
  // ����� ��ü�� �ִ� ���
  // ���ο� ������ü �� ��� ���� �˸�
  else if (prev_media != opt_media) {
    sprintf(message_buffer, "[%s] ���� [%s] ���� ���� ��ü ���� \r\n",  media_name[prev_media],  media_name[opt_media]);
    insert_display_message(INTEG_MSG, message_buffer);
    return;
  }
  // ���� ������ü�� ���� ��ü�� ���� ��� PASS
  else {
    
  }
  
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

// ���� MAC �ּ� GET
unsigned char* integ_get_mac_addr(unsigned char addr_type) {
  if (addr_type == BROADCAST_ADDR) {
    return integ_broadcast_addr;
  }
  else {
    integ_macAddr[0] = LSB(STM32_UUID[0]);
    return integ_macAddr;
  }
}

unsigned char get_seq_number(void)
{
  unsigned char return_value = seqNumber;
  seqNumber = (seqNumber + 1) % MAX_SEQ_NUMBER;
  return return_value;
}