#ifndef GM_SKF_SDK_H
#define GM_SKF_SDK_H
#include "include/SKF.h"

#define DFT_LIB_NAME    "SKF_sd.dll"
#define GM_LOG  printf

#define CERT_TYPE_SIGN  1       //签名密钥
#define CERT_TYPE_ENC   0       //加密密钥

class GM_SKF_SDK
{

public:
    GM_SKF_SDK();
    ~GM_SKF_SDK();

private:
    HMODULE H_DLL;
    char *devName;
    PSKF_FUNCLIST   FunctionList;   //方法集合
    DEVHANDLE       *devHandle;     //设备句柄
    HCONTAINER      hContainer;     //容器句柄
    HAPPLICATION    hApplication;   //应用句柄

private:
    int sdk_is_ready;               //标记sdk 设备连接成功，应用和容器打开成功，可以进行读写操作

private:
    char skf_dll_name[256];

public:
    int GmSetDll(char *dllname);        //设置dll文件
    int GmLoadDll();                    //加载共享库
    int GmConnectDevice();              //连接设备
//    int GmGetDeviceState();              //获取设备状态

//    int GmVerifyPin(const char *pin);   //认证pin码

    int GmOpenApplication();            //打开应用
//    int GmCloseApplication();           //关闭应用

    int GmOpenContainer(char *cname);              //打开容器

    int GmExportCertificate(int type, u8 *data, u32 *size);          //导出证书


//    int GmImportCertificate();      //VPN不用导入证书，暂时不做
};

#endif // GM_SKF_SDK_H
