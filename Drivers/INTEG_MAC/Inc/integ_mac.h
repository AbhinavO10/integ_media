/*

      integ_mac.h


*/
#define MEDIA_NUM 3             // ��� ��ü ��
#define LIFI 0                          // Media Type
#define BLUETOOTH 1
#define CC2530 2
#define OPT_MEDIA 0xF0        // ���� ������ ��ü�� ���

static char *media_name[MEDIA_NUM] = {"LI-FI", "BLUETOOTH", "CC2530"};
extern unsigned char cur_media;  // ���� ����� ��ũ
extern unsigned char opt_media;  // ���� ������ ��ũ

// ���� ���� ���� ���̺�
#define STATUS_NUM 2
#define INIT_STATUS 0
#define CONNECT_STATUS 1
#define CON 1
#define DISCON 0
static char *STATUS_NAME[STATUS_NUM] = {"Init", "Connect"};
extern unsigned char STATUS_TABLE[STATUS_NUM][MEDIA_NUM];


#define FIND_OPT_PERIOD 5 // ���� ��� �˻� �ֱ� 500ms
#define RETRANSMIT_TIME 3      // ������ �ֱ� 300ms
#define R_SUCCESS 1
#define R_FAIL 0
static char *result_string[2] = {"FAIL", "SUCCESS"};

#define INTEG_ADDR_LEN 6     // ���� �� �ּ� ����
#define LIFI_ADDR_LEN 6     // lifi �� �ּ� ����
#define BLUETOOTH_ADDR_LEN 6     // BLUETOOTH �� �ּ� ����
#define CC2530_ADDR_LEN 8     // CC2530 �� �ּ� ����
static unsigned char media_addr_len[MEDIA_NUM] = {LIFI_ADDR_LEN, BLUETOOTH_ADDR_LEN, CC2530_ADDR_LEN};

#define INTEG_FRAME_HEADER_LEN 17 // ���� �� ������ ��� ����
#define INTEG_FRAME_DATA_LEN 40  
#define INTEG_FRAME_TOTAL_LEN 57

#define INTEG_FRAME_LEN_FIELD 0

// messageType
#define DATA_MSG 0x01
#define ACK_MSG 0x02
#define PASS_MSG 0xFF

// deviceType
#define MASTER 0x00
#define SLAVE 0x01


extern unsigned char my_integ_address[INTEG_ADDR_LEN];
#define HOOD_HASH_ID 0x10
extern unsigned char hood_integ_address[INTEG_ADDR_LEN];

#define MAX_SEQ_NUMBER 10                   // ���� ��ȣ �ִ�
#define DEFAULT_SEQ_NUMBER 1            // ���� ��ȣ �ʱⰪ
extern unsigned char seqNumber;         // ���� ��ȣ

#define STATIC_ADDR 0
#define DYNAMIC_ADDR 1
static char *addr_type_name[2] = {"STATIC", "DYNAMIC"};

// ���� MAC ���̺� ����ü
typedef struct integ_table {
  unsigned char integ_addr[INTEG_ADDR_LEN];
  unsigned char addr_type; // static : 0, dynamic : 1
  unsigned char *media_addr[MEDIA_NUM];
} INTEG_TABLE;


// ���� MAC ������ ����ü
typedef struct integ_frame {
  unsigned char frame_length;
  unsigned char message_type;
  unsigned char src_address[INTEG_ADDR_LEN];
  unsigned char dest_address[INTEG_ADDR_LEN];
  unsigned char media_type;
  unsigned char seqNumber;
  unsigned char ackNumber;
  unsigned char data[INTEG_FRAME_DATA_LEN];
} INTEG_FRAME;

unsigned char get_seq_number(void);
void integ_mac_handler(void * arg);
void integ_retransmit_handler(void * arg);
void integ_find_opt_link(void *);
void integ_mac_init(void);
void integ_print_frame(INTEG_FRAME *frame);

/** Get the Least Significant Byte (LSB) of an unsigned int*/
#define LSB(num) ((num) & 0xFF)

/** Get the Most Significant Byte (MSB) of an unsigned int*/
#define MSB(num) ((num) >> 8)
