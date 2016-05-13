#ifndef FACEINFOMAN_H
#define FACEINFOMAN_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PHOTO_DIR      "faceinfo"
#define PHOTO_FILENAME "faceinfo.tar.gz"
/*
faceinfo目录下的文件
photo_a_w_h.bmp  //原始图片
photo_h_w_h.bmp  //灰度图片
photo1_w_h.bmp   //人脸照片1
……
photon_w_h.bmp   //人脸照片n
*/


enum E_COMP_STATUS
{
	E_COMP_SECCESS = 0x00,    // 比对成功
	E_COMP_FAIL    = 0x01,    // 比对失败
	E_COMP_NOCOMP  = 0xFF,    // 未比对
};


enum E_COMPRESS_FORMAT
{
	E_COMPRESS_NOCOMPRESS = 0x00, // 未压缩
	E_COMPRESS_RAR        = 0x01, // rar压缩
	E_COMPRESS_ZIP        = 0x02, // zip压缩
	E_COMPRESS_GZIP       = 0x03, // gzip压缩
	E_COMPRESS_BZ2        = 0x04, // bz2压缩

	E_COMPRESS_NOKNOW     = 0XFF, // 未知的压缩
};

enum E_DEV_TYPE
{
	E_DEV_4GWLROUTER      = 0x01,  // 4G无线加密路由器
	E_DEV_FDETECTTERM     = 0x02,  // 人脸检测终端设备
	E_DEV_MANSERVER       = 0x03,  // 管理服务器

	E_DEV_NOKNOW          = 0xFF,  // 未知设备
};

struct s_event_info
{
	unsigned char gps;             // GPS上传使能
	unsigned char gps_status;      // GPS状态
	unsigned char nfc;             // NFC上传使能
	unsigned char nfc_status;      // NFC状态
	unsigned char cam;             // CAM上传使能
	unsigned char cam_status;      // CAM状态
};

struct s_policy_info
{
	unsigned char net_restore;     // 网络恢复上传使用
	unsigned char gps;             // GPS上传使能
	unsigned char nfc;             // NFC上传使能
	unsigned char cam;             // CAM上传使能

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
	unsigned char province[98];		//省/行政区
	unsigned char city[98];			//城市/乡镇
	unsigned char organization[98];	//公司/团体
	unsigned char institution[98];	//部门/机构
	unsigned char place[256];		//场所
	unsigned char address[256];		//地址
	unsigned char manager[64];		//管理者
	unsigned char mobile[64];		//移动号码
	unsigned char telephone[64];	//固定号码
	unsigned char latitude[32];		//经度
	unsigned char longitude[32];	//纬度
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



//启动监听，接收服务端下发策略等请求
int start_listen(char * ip, unsigned short port);
//连接测试
int connect_test(char * ip, unsigned short port);

//制作压缩包文件
int make_compresspackage(char * dir);
//上传照片文件
int upload_faceinfo(char * ip, unsigned short port, int devtype, char *devid, char * taskid, char *latitude, char * longitude, int face_num, int comp_status, int compress_format, char * compress_file);
//上传事件
int unload_notify(char * ip, unsigned short port, int devtype, char *devid, struct s_event_info * event_info);
//上传比对分值
int unload_verify(char * ip, unsigned short port, int devtype, char *devid, char *taskid,  char* photoname, int score);
//注册回调
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




