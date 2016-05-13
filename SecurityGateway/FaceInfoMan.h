#ifndef FACEINFOMAN_H
#define FACEINFOMAN_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PHOTO_DIR      "faceinfo"
#define PHOTO_FILENAME "faceinfo.tar.gz"
/*
faceinfoĿ¼�µ��ļ�
photo_a_w_h.bmp  //ԭʼͼƬ
photo_h_w_h.bmp  //�Ҷ�ͼƬ
photo1_w_h.bmp   //������Ƭ1
����
photon_w_h.bmp   //������Ƭn
*/


enum E_COMP_STATUS
{
	E_COMP_SECCESS = 0x00,    // �ȶԳɹ�
	E_COMP_FAIL    = 0x01,    // �ȶ�ʧ��
	E_COMP_NOCOMP  = 0xFF,    // δ�ȶ�
};


enum E_COMPRESS_FORMAT
{
	E_COMPRESS_NOCOMPRESS = 0x00, // δѹ��
	E_COMPRESS_RAR        = 0x01, // rarѹ��
	E_COMPRESS_ZIP        = 0x02, // zipѹ��
	E_COMPRESS_GZIP       = 0x03, // gzipѹ��
	E_COMPRESS_BZ2        = 0x04, // bz2ѹ��

	E_COMPRESS_NOKNOW     = 0XFF, // δ֪��ѹ��
};

enum E_DEV_TYPE
{
	E_DEV_4GWLROUTER      = 0x01,  // 4G���߼���·����
	E_DEV_FDETECTTERM     = 0x02,  // ��������ն��豸
	E_DEV_MANSERVER       = 0x03,  // ���������

	E_DEV_NOKNOW          = 0xFF,  // δ֪�豸
};

struct s_event_info
{
	unsigned char gps;             // GPS�ϴ�ʹ��
	unsigned char gps_status;      // GPS״̬
	unsigned char nfc;             // NFC�ϴ�ʹ��
	unsigned char nfc_status;      // NFC״̬
	unsigned char cam;             // CAM�ϴ�ʹ��
	unsigned char cam_status;      // CAM״̬
};

struct s_policy_info
{
	unsigned char net_restore;     // ����ָ��ϴ�ʹ��
	unsigned char gps;             // GPS�ϴ�ʹ��
	unsigned char nfc;             // NFC�ϴ�ʹ��
	unsigned char cam;             // CAM�ϴ�ʹ��

};

struct s_devstate_info
{
	unsigned int cpu_usage;
	unsigned int cpu_temperature;
	unsigned char mem_situat[16];
	unsigned char compare_state;
	unsigned char ipc_state;
	unsigned char ipc_image;
	unsigned char detect_time[24];
};

struct s_picture_data
{
	char *pdata;
	int nsize;
};

struct s_down_config
{
	unsigned char time_ser[32];
	unsigned char ca_ser[32];
	unsigned char monitor_ser[32];
	unsigned char center_ser[32];

};

struct s_apparatus_info
{
	unsigned char province[98];		//ʡ/������
	unsigned char city[98];			//����/����
	unsigned char organization[98];	//��˾/����
	unsigned char institution[98];	//����/����
	unsigned char place[256];		//����
	unsigned char address[256];		//��ַ
	unsigned char manager[64];		//������
	unsigned char mobile[64];		//�ƶ�����
	unsigned char telephone[64];	//�̶�����
	unsigned char latitude[32];		//����
	unsigned char longitude[32];	//γ��
};

#define PROTOCOL_VER         0x01
#define PROTOCOL_HEADER_LEN  28
#define PROTOCOL_BUFF_LEN    1024

#define OSTYPE_WINDOWS		"windows"
#define OSTYPE_LINUX		"linux"
#define OSTYPE_MAC			"mac"

#define CMDTYPE_POLICY		"Policy"
#define CMDTYPE_DEVINFO		"DeviceInfo"
#define CMDTYPE_PICTURE		"CatchPicture"



//�������������շ�����·����Ե�����
int start_listen(char * ip, unsigned short port);
//���Ӳ���
int connect_test(char * ip, unsigned short port);

//����ѹ�����ļ�
int make_compresspackage(char * dir);
//�ϴ���Ƭ�ļ�
int upload_faceinfo(char * ip, unsigned short port, int devtype, char *devid, char * taskid, char *latitude, char * longitude, int face_num, int comp_status, int compress_format, char * compress_file);
//�ϴ��¼�
int unload_notify(char * ip, unsigned short port, int devtype, char *devid, struct s_event_info * event_info);
//�ϴ��ȶԷ�ֵ
int unload_verify(char * ip, unsigned short port, int devtype, char *devid, char *taskid,  char* photoname, int score);
//ע��ص�
int register_callback(void (*p)(char *cmd_type, char *pbuff));

//Add 2015-09-08 chen_zx
int common_report(char * ip, unsigned short port, char * input_aes, int * len , char * output);



//add by andy,build and check header out outsize
void build_netheader2(char *pbuff,int extlen);
int check_netheader2(char *pbuff,int *extlen);

#ifdef __cplusplus
}
#endif


#endif




