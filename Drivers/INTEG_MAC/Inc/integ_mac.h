/*

      integ_mac.h


*/
// �Ϲ� �׽�Ʈ ��  :  Hello
extern unsigned char testBuf[5];
// ����ȭ �׽�Ʈ �� ������
extern unsigned char testBuf_2[120];

#define MEDIA_NUM 3             // ��� ��ü ��
#define LIFI 0                          // Media Type
#define BLUETOOTH 1
#define CC2530 2
#define OPT_MEDIA 0xF0        // ���� ������ ��ü�� ���

static char *media_name[MEDIA_NUM] = {"LI-FI", "BLUETOOTH", "CC2530"};
extern unsigned char cur_media;  // ���� ����� ��ũ
extern unsigned char opt_media;  // ���� ������ ��ũ
extern unsigned char integ_init_state; // ���� MAC �ʱ�ȭ ����

// ���� ���� ���� ���̺�
#define STATUS_NUM 2    // ���� ����
#define INIT_STATUS 0     
#define CONNECT_STATUS 1
#define CON 1
#define DISCON 0
static char *STATUS_NAME[STATUS_NUM] = {"Init", "Connect"};
extern unsigned char STATUS_TABLE[STATUS_NUM][MEDIA_NUM];


#define FIND_OPT_PERIOD 5 // ���� ��� �˻� �ֱ� 500ms
#define RETRANSMIT_TIME 5      // ������ �ֱ� 500ms
#define RETRANSMIT_NUM 3      // 3�� ACK �ȿ��� ��� ��ü����
#define R_SUCCESS 1     // ���� ��� : ����
#define R_FAIL 0                // ���� ��� : ����
static char *result_string[2] = {"FAIL", "SUCCESS"};

#define INTEG_ADDR_LEN 6     // ���� �� �ּ� ����
#define LIFI_ADDR_LEN 6     // lifi �� �ּ� ����
#define BLUETOOTH_ADDR_LEN 6     // BLUETOOTH �� �ּ� ����
#define CC2530_ADDR_LEN 8     // CC2530 �� �ּ� ����
#define MEDIA_ADDR_LEN_MAX 8    // �� ��ü �ּ��� ���̰��� �� ��
static unsigned char media_addr_len[MEDIA_NUM] = {LIFI_ADDR_LEN, BLUETOOTH_ADDR_LEN, CC2530_ADDR_LEN};  /// �� ��ü �ּ� ���� �ε����� ���� ��


// messageType
#define DATA_MSG 0x01   // �Ϲ� �����͸� ���� �޽���
#define ACK_MSG 0x02    // ���� �޽���
#define ADV_MSG 0x04    // ��� �ڽ��� ���縦 ��ε�ĳ���� �Ҷ�
#define ADV_IND 0x05      // ADV_MSG�� ���� ���� 
#define PASS_MSG 0xFF   // �ƹ��͵� ���� �޽��� 

// deviceType
#define MASTER 0x00
#define SLAVE 0x01

// fragmentaion ����
static int media_mtu_size[MEDIA_NUM] = {28, 44, 116};
#define MIN_MTU_SIZE 44 // ��ü�� MTU ũ�� �� ���� ���� ��
#define MAX_MTU_SIZE 116 // ��ü�� MTU ũ�� �� ���� ū ��

// SEQ
#define MAX_SEQ_NUMBER 1024                   // ���� ��ȣ �ִ�
#define DEFAULT_SEQ_NUMBER 0            // ���� ��ȣ �ʱⰪ
extern unsigned char seqNumber;         // ���� ��ȣ


#define STATIC_ADDR 0   // �������� ���� �ּ� (�ڽ��� �ּ�, ��ε�ĳ��Ʈ)
#define DYNAMIC_ADDR 1  // �ܺη� ���� ������ ����
static char *addr_type_name[2] = {"STATIC", "DYNAMIC"};
extern unsigned char my_integ_address[INTEG_ADDR_LEN];
extern unsigned char hood_integ_address[INTEG_ADDR_LEN];

// ���� MAC ���̺� ����ü
typedef struct integ_table {
  unsigned char integ_addr[INTEG_ADDR_LEN];     // ���� MAC �ּ�
  unsigned char addr_type; // static : 0, dynamic : 1
  unsigned char media_addr[MEDIA_NUM][MEDIA_ADDR_LEN_MAX];      // �� ��ü �ּ� 
} INTEG_TABLE;

#define INTEG_FRAME_HEADER_LEN 20 // ���� �� ������ ��� ����
#define INTEG_FRAME_DATA_LEN 39     // ���� �� ������ ������ ����
#define INTEG_FRAME_TOTAL_LEN 59 + 1   // ���� �� ������ ��� + ������ ����
#define INTEG_FRAME_LEN_FILED_SIZE 2    // ������ ���� �ʵ� ũ�� 2����Ʈ

#define INTEG_FRAME_LEN_FIELD 0     // ������ ���� �ʵ�� ���� �������� 0��


// ���� MAC ������ ����ü
typedef struct integ_frame {
  unsigned char frame_length[2];   // ������ ����
  unsigned char message_type;   // �޽��� ����
  unsigned char src_address[INTEG_ADDR_LEN];    // �ٿ��� �ּ�
  unsigned char dest_address[INTEG_ADDR_LEN];   // ������ �ּ�
  unsigned char media_type;     // ��ü ����
  unsigned char seqNumber;      // ���� ��ȣ
  unsigned char ackNumber;      // ���� ��ȣ
  unsigned char fragment_number;            // ����ȭ ��ȣ
  unsigned char fragment_offset;        // ������ ��Ʈ ������
  unsigned char *data;// ���̷ε�
} INTEG_FRAME;


extern INTEG_FRAME advertising_frame;

unsigned char get_seq_number(void);
void integ_mac_handler(void * arg);
void integ_retransmit_handler(void * arg);
void integ_find_opt_link(void *);
void integ_mac_init(void);
void integ_print_frame(INTEG_FRAME *frame);

#define MAC_ADDR 0x00
#define BROADCAST_ADDR 0xFF
unsigned char* integ_get_mac_addr(unsigned char addr_type);

#define LENGTH_LSB 0
#define LENGTH_MSB 1

/** Get the Least Significant Byte (LSB) of an unsigned int*/
#define LSB(num) ((num) & 0xFF)

/** Get the Most Significant Byte (MSB) of an unsigned int*/
#define MSB(num) ((num) >> 8)

/** Convert two unsigned chars to an unsigned int, LSB first*/
#define CONVERT_TO_INT(lsb,msb) ((lsb) + 0x0100*(msb))
