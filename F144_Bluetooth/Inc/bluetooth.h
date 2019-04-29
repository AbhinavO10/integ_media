/*      
    bluetooth.h
*/

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