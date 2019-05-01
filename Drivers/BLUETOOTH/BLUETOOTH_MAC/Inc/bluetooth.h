/* 
  bluetooth.h
*/
extern unsigned char bt_state;
extern unsigned char bt_index;
extern unsigned char bt_count;
extern unsigned char btBuf[100];
unsigned char bluetooth_init(unsigned char deviceType);
unsigned char bluetooth_send(unsigned char* dest_addr, unsigned char* data, int data_length);

// BlueTooth Init
void BT_M_Init();
void BT_S_Init();
// AT Ŀ�Ǵ� ����
void BT_AT();
//�ӵ� ���� - 115200
void BT_SetBaud();
// Mac �ּ� �����
void BT_MacAddress();
// ��� ����
void BT_ControlMode();
// ������ / �����̺� ����
void BT_SetChangeRole();
// Master ����
void BT_SetMaster();
// SLAVE ����
void BT_SetSlave();
// �ʱ�ȭ
void BT_Reset();
//IMME 1
void BT_IMME();
//START
void BT_START();
//CONNECT
void BT_CONNET();