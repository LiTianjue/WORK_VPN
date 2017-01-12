#ifndef CX_SKF_SDK_H
#define CX_SKF_SDK_H

#define CERT_TYPE_SIGN  1       //签名密钥
#define CERT_TYPE_ENC   0       //加密密钥

#define  DEV_ABSENT_STATE 	0x00000000 	//设备不存在
#define  DEV_PRESENT_STATE 	0x00000001 	//设备存在
#define  DEV_UNKNOW_STATE 	0x00000002 	//设备状态未知

//容器类型
#define CONTAINER_TYPE_NONE      0   // 未定
#define CONTAINER_TYPE_RSA       1   // RSA
#define CONTAINER_TYPE_ECC       2   // ECC

#define SDK_ERROR_NOT_INIT      0x000000FF  //设备未初始化


//设置共享库路径
int CX_SKF_SDK_SetDll(char *dllname);

//载入共享库
int CX_SKF_SDK_LoadDll();

//连接设备
int CX_SKF_SDK_ConnectDev();

//断开设备连接
int CX_SKF_SDK_DisConnectDev();

//获取设备信息
int CX_SKF_SDK_GetDevInfo(char *info,int info_len);

//获取设备状态
int CX_SKF_SDK_GetDevState();

//验证pin码
int CX_SKF_SDK_VerifyPin(char *pin,unsigned int *retry);

//打开应用
int CX_SKF_SDK_OpenApplication();

//关闭应用
int CX_SKF_SDK_CloseApplication();

//枚举容器
int CX_SKF_SDK_EnumContainer(char *cname,unsigned int *csize);

//打开容器
int CX_SKF_SDK_OpenContainer(char *cname);

//创建容器
int CX_SKF_SDK_CreateContainer(char *cname);

//删除容器
int CX_SKF_SDK_DeleteContainer(char *cname);

//关闭容器
int CX_SKF_SDK_CloseContainer(char *cname);

//获取容器类型
int CX_SKF_SDK_GetContainerType(char *cname,unsigned int *ctype);

//导出证书
int CX_SKF_SDK_ExportCertificate(int type,unsigned char *data,unsigned int *size);

//导入密钥和证书
int CX_SKF_SDK_ImportSM2Certificate(unsigned char *keypair,unsigned char *cert,int key_len,int cert_len);




//密码验证结束后快速导出证书
int  CX_SKF_SDK_ExportCertificate_EX(char *cname,int type,unsigned char *data,unsigned int *size);

#endif // CX_SKF_SDK_H
