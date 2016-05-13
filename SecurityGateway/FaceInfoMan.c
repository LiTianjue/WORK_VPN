#include "FaceInfoMan.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

//平台相关定
#ifdef WIN32
#include <io.h>
#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

#define MSG_NOSIGNAL 0

#else  // linux
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <net/if.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> 


#define ioctlsocket ioctl
#define closesocket close

#define O_BINARY 0
#define INVALID_SOCKET (-1)

#endif 




#define NET_CON_TIMEOUT       10   //连接超时
#define NET_REC_TIMEOUT       15   //接收超时



/***********************************************************************************************************
私有接口
1、create_socket     创建套接字
2、RecvData          接收数据
3、SendData          发送数据
4、build_netheader   制作网络头
、check_netheader    检验网络头的有效性
5、to_faceinfo_body  
***********************************************************************************************************/
static int create_socket(char *IP,unsigned short port);
static int RecvData(int sock, void * pdata, int len);
static int SendData(int sock, void * pdata, int len);

static void build_netheader(char * pbuff, int extlen);
static int check_netheader(char * pbuff, int *extlen);
static int to_faceinfo_body(int devtype, char *devid, char * taskid, char *latitude, char * longitude, int face_num, int comp_status, int compress_format, char * file_name, int file_len, char * pbody, int * len);
static int to_eventinfo_body(int devtype, char *devid, struct s_event_info *event_info, char * pbody, int * len);
static int to_verifyinfo_boby(int devtype, char *devid, char *taskid,  char* photoname, int score, char * pbody, int * len);

//反向查找字符串的函数 
int rfind(const char*source ,const char* match) 
{ 
	int i = 0;

	for( i=strlen(source); i>=0; i--) 
	{ 
		if(source[i]==match[0] && strncmp(source+i, match, strlen(match))==0) return i; 
	} 

	return -1;            

}

/*从字符串的左边截取n个字符*/  
char * left(char *dst,char *src, int n)  
{  
	char *p = src;  
	char *q = dst;  
	int len = strlen(src);  
	if(n>len) n = len;  
	while(n--) *(q++) = *(p++);  
	*(q++)='\0';  
	return dst;  
}

int create_socket(char *IP,unsigned short port)
{
	struct sockaddr_in addr;
	int sock;

#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 0);
	if(WSAStartup(wVersionRequested, &wsaData)!= 0)
	{
		return -1;
	}
#endif
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock != INVALID_SOCKET)
	{
		int  ret;
		int  optval;
		
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family   = AF_INET;
	    addr.sin_addr.s_addr = inet_addr(IP);
		addr.sin_port     = htons(port);
	
		optval = 1;
		ioctlsocket(sock, FIONBIO, &optval); 
		
		ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
		
		if(ret < 0)
		{
			int error = -1;
			struct timeval tm;
			fd_set set;	
			int len;

#ifndef WIN32
			
			if (errno != EWOULDBLOCK && errno != EINPROGRESS)
			{
				closesocket(sock);
				return -1;
			}
			
#endif
			
			//设置连接超时
			tm.tv_sec  = NET_CON_TIMEOUT;
			tm.tv_usec = 0;
			FD_ZERO(&set);
			FD_SET(sock, &set);
			if(select(sock + 1, NULL,&set, NULL, &tm) > 0)
			{
				len = sizeof(int);
				getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
				if(error != 0)
				{
					closesocket(sock);
					return -1;
				}
			} 
			else
			{
				closesocket(sock);
				return -1;
			}
		}
		//设置SOCK为阻塞
		optval = 0;
		ioctlsocket(sock, FIONBIO, &optval); 
		
		optval = NET_REC_TIMEOUT * 1000;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)(&optval), sizeof(optval));
	}
	
	return sock;
}

int create_listen(char *IP,unsigned short port)
{
	struct sockaddr_in addr;
	int sock;
	
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 0);
	if(WSAStartup(wVersionRequested, &wsaData)!= 0)
	{
		return -1;
	}
#endif
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock != INVALID_SOCKET)
	{
		int  ret;
		
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family   = AF_INET;
		addr.sin_addr.s_addr = inet_addr(IP);
		addr.sin_port     = htons(port);

		ret = bind(sock,(struct sockaddr *)&addr, sizeof(addr));
		if(ret == INVALID_SOCKET)
		{
			return -1;
		}
		
		ret = listen(sock, 8);
		if(ret==INVALID_SOCKET)
		{
		}
		return sock;

	}

	return -1;
}


int RecvData(int sock, void * pdata, int len)
{
	int ret, count;

	if(!pdata)
	{
		return -1;
	}
	
	count = 0;
	if(len == 0)
	{
		return 0;
	}
	
	while(len)
	{
		if((ret = recv(sock, (char*)pdata + count, len, 0)) <= 0)
		{
			return -1;
		}
		len  -= ret;
		count += ret;
	}
	return count;
}


int SendData(int sock, void * pdata, int len)
{
	
	int ret, count;

	if(!pdata)
	{
		return -1;
	}
	
	count = 0;
	if(len == 0)
	{
		return 0;
	}

	while(len > 0)
	{
		ret = send(sock, (char*)pdata + count, len, MSG_NOSIGNAL);
		if(ret > 0)
		{
			len -= ret;
			count += ret;
		}
		else
		{
			return -1;
		}
	}
	
	return count;
}

void build_netheader(char * pbuff, int extlen)
{
	if(!pbuff)
	{
		return;
	}

	pbuff[0] = 'L';
	pbuff[1] = 'Z';
	pbuff[2] = PROTOCOL_VER;

	pbuff[3] = 0xFF & (extlen >> 24);
	pbuff[4] = 0xFF & (extlen >> 16);
	pbuff[5] = 0xFF & (extlen >> 8);
	pbuff[6] = 0xFF & (extlen >> 0);
    /*
    int tmp = htonl(extlen);
    memcpy(pbuff+3,&tmp,4);
    */
}

//add by andy
void build_netheader2(char *pbuff,int extlen)
{
    build_netheader(pbuff,extlen);
}

int check_netheader(char * pbuff, int * extlen)
{
	if(!pbuff || !extlen)
	{
		return 0;
	}
	
	if( pbuff[0] == 'L' && pbuff[1] == 'Z')
	{
		if(pbuff[2] == PROTOCOL_VER)
		{
			int len = 0;
			unsigned char * tmp = pbuff;
			len |= tmp[3]  << 24;
			len |= tmp[4]  << 16;
			len |= tmp[5]  << 8;
			len |= tmp[6]  << 0;

			*extlen = len;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}
//add by andy
int check_netheader2(char *pbuff,int *extlen)
{
    return check_netheader(pbuff,extlen);
}

int to_faceinfo_body(int devtype, char *devid, char * taskid, char *latitude, char * longitude, int face_num, int comp_status, int compress_format, char * file_name, int file_len, char * pbody, int * len)
{
	char date_time[32] = {0};
	time_t curtime;

	if(!latitude || !longitude || !pbody)
	{
		return 0;
	}
	
	
	time(&curtime);
	strftime( date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime(&curtime) ); 

	//strcpy(date_time, "2015-7-22 18:20:00");

	sprintf(pbody,
	"<?xml version=\"1.0\"?>\r\n"
	"<Query>\r\n"
	"<DeviceType>0x%02x</DeviceType>\r\n"
	"<DeviceID>%s</DeviceID>\r\n"
	"<CmdType>FaceInfo</CmdType>\r\n"
	"<TaskID>%s</TaskID>\r\n"
	"<Latitude>%s</Latitude>\r\n"
	"<Longitude>%s</Longitude>\r\n"
	"<DateTime>%s</DateTime>\r\n"
	"<FaceNum>0x%04x</FaceNum>\r\n"
	"<CompStatus>0x%02x</CompStatus>\r\n"
	"<CompressFormat>0x%02x</CompressFormat>\r\n"
	"<FileName>%s</FileName>\r\n"
	"<FileSize>0x%x</FileSize>\r\n"	
	"</Query>\r\n", devtype, devid, taskid, latitude, longitude, date_time, face_num, comp_status, compress_format, file_name, file_len);

	*len = strlen(pbody);

	return 1;
}

int to_eventinfo_body(int devtype, char *devid, struct s_event_info *event_info, char * pbody, int * len)
{
	char tmp[64] = {0};
	
	if(!event_info || !len || !pbody)
	{
		return 0;
	}
	
	sprintf(pbody,
		"<?xml version=\"1.0\"?>\r\n"
		"<Notify>\r\n"
		"<DeviceType>0x%02x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>AlarmInfo</CmdType>\r\n", devtype, devid );

	if(event_info->gps)
	{
		sprintf(tmp, "<GPSStatus>%s</GPSStatus>\r\n", event_info->gps_status ? "OK" : "ERROR");
		strcat(pbody, tmp);
	}

	if(event_info->nfc)
	{
		sprintf(tmp, "<NFCStatus>%s</NFCStatus>\r\n", event_info->nfc_status ? "OK" : "ERROR");
		strcat(pbody, tmp);
	}

	if(event_info->cam)
	{
		sprintf(tmp, "<CAMStatus>%s</CAMStatus>\r\n", event_info->cam_status ? "OK" : "ERROR");
		strcat(pbody, tmp);
	}

    strcat(pbody, "</Notify>\r\n");
	
	*len = strlen(pbody);
	
	return 1;
}

int to_verifyinfo_boby(int devtype, char *devid, char *taskid,  char* photoname, int score, char * pbody, int * len)
{
	char tmp[64] = {0};

	if(!taskid || !photoname || !pbody)
	{
		return 0;
	}

	sprintf(pbody,
		"<?xml version=\"1.0\"?>\r\n"
		"<Notify>\r\n"
		"<DeviceType>0x%02x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>FaceInfoComp</CmdType>\r\n"
		"<TaskID>%s</TaskID>\r\n"
		"<PhotoName>%s</PhotoName>\r\n"
		"<Score>%d</Score>\r\n"
		"<FeatureID>%d</FeatureID>\r\n"
		"</Notify>\r\n", devtype, devid, taskid, photoname, score, 1009);

	*len = strlen(pbody);

	return 1;
}

int chec_no_data_ack(char * cmdtype, char * pbody)
{
	if(!cmdtype || !pbody)
	{
		return 0;
	}

	if(strstr(pbody, cmdtype))
	{
		char * p = strstr(pbody, "Result");
		if(p)
		{
			if(strstr(p, "OK"))
			{
				return 1;
			}
		}
	}

	return 0;
}

void no_data_ack(int iDevtye, char * pdevid, char * pCmdType, int bSuccess, char * pbody, int * len)
{
	if(!pbody || !pCmdType)
	{
		return;
	}
	sprintf(pbody,
		"<?xml version=\"1.0\"?>\r\n"
		"<Response>\r\n"
		"<DeviceType>0x%x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>%s</CmdType>\r\n"
		"<Result>%s</Result>\r\n"
		"</Response>\r\n", iDevtye, pdevid, pCmdType, bSuccess ? "OK" : "ERROR");
	
	*len = strlen(pbody);
}

void dev_info_ack(int iDevtye, char * pdevid, char * pCmdType, struct s_devstate_info *devstate_info, char * pbody, int * len)
{
	char tmp[64] = {0};

	if(!pbody || !pCmdType)
	{
		return;
	}
	sprintf(pbody,
		"<?xml version=\"1.0\"?>\r\n"
		"<Response>\r\n"
		"<DeviceType>0x%x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>%s</CmdType>\r\n", iDevtye, pdevid, pCmdType);

	sprintf(tmp, "<CPUUsage>%d</CPUUsage>\r\n", devstate_info->cpu_usage);
	strcat(pbody, tmp);

	sprintf(tmp, "<CPUTemperature>%d</CPUTemperature>\r\n", devstate_info->cpu_temperature);
	strcat(pbody, tmp);

	sprintf(tmp, "<MemorySituation>%s</MemorySituation>\r\n", devstate_info->mem_situat);
	strcat(pbody, tmp);


	sprintf(tmp, "<ComparisonState>%d</ComparisonState>\r\n", devstate_info->compare_state);
	strcat(pbody, tmp);

	sprintf(tmp, "<IPCNetworkState>%d</IPCNetworkState>\r\n", devstate_info->ipc_state);
	strcat(pbody, tmp);

	sprintf(tmp, "<IPCImageState>%d</IPCImageState>\r\n", devstate_info->ipc_image);
	strcat(pbody, tmp);

	sprintf(tmp, "<FaceDetectTime>%s</FaceDetectTime>\r\n", devstate_info->detect_time);
	strcat(pbody, tmp);

	strcat(pbody, "</Response>\r\n");

	*len = strlen(pbody);
}

void dev_catch_picture(int iDevtye, char * pdevid, char * pCmdType, struct s_picture_data *picture, char **pbody, int * len)
{
	char tmp[64] = {0};
	int nLenth = 0;
	char *datatmp = NULL;
	*pbody = (char*)malloc(picture->nsize + PROTOCOL_BUFF_LEN);
	memset(*pbody, 0, picture->nsize + PROTOCOL_BUFF_LEN);
	if(!*pbody || !pCmdType)
	{
		return ;
	}
	datatmp = *pbody + PROTOCOL_HEADER_LEN;

	sprintf(datatmp,
		"<?xml version=\"1.0\"?>\r\n"
		"<Response>\r\n"
		"<DeviceType>0x%x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>%s</CmdType>\r\n"
		"<CatchPic>", iDevtye, pdevid, pCmdType);

	nLenth = strlen(datatmp);
	memcpy(datatmp+nLenth, picture->pdata, picture->nsize);
	strcpy(tmp, "</CatchPic>\r\n</Response>\r\n");
	memcpy(datatmp + nLenth + picture->nsize, tmp, strlen(tmp));

	*len = strlen(datatmp);
}

int to_common_request(int iDevtye, char *pDevID, char * pCmdType, char * pbody, int * len)
{
	if(!pbody || !pCmdType)
	{
		return 0;
	}
	sprintf(pbody,
		"<?xml version=\"1.0\"?>\r\n"
		"<Down>\r\n"
		"<DeviceType>0x%x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>%s</CmdType>\r\n"
		"</Down>\r\n", iDevtye, pDevID, pCmdType);

	*len = strlen(pbody);

	return 1;
}

int to_apply_certificate(int iDevtye, char *pDevID, char * pCmdType, struct s_apparatus_info * apply_info, char * pbody, int * len)
{
	char tmp[64] = {0};
	if(!pbody || !pCmdType)
	{
		return;
	}

	sprintf(pbody,
		"<?xml version=\"1.0\"?>\r\n"
		"<Apply>\r\n"
		"<DeviceType>0x%x</DeviceType>\r\n"
		"<DeviceID>%s</DeviceID>\r\n"
		"<CmdType>%s</CmdType>\r\n", iDevtye, pDevID, pCmdType);

	sprintf(tmp, "<Province>%s</Province>\r\n", apply_info->province);
	strcat(pbody, tmp);

	sprintf(tmp, "<City>%s</City>\r\n", apply_info->city);
	strcat(pbody, tmp);

	sprintf(tmp, "<Organization>%s</Organization>\r\n", apply_info->organization);
	strcat(pbody, tmp);

	sprintf(tmp, "<Institution>%s</Institution>\r\n", apply_info->institution);
	strcat(pbody, tmp);

	sprintf(tmp, "<Place>%s</Place>\r\n", apply_info->place);
	strcat(pbody, tmp);

	sprintf(tmp, "<Address>%s</Address>\r\n", apply_info->address);
	strcat(pbody, tmp);

	strcat(pbody, "</Apply>\r\n");

	return 1;
}



/*****************************************************************************************
公有接口
1、start_listen
2、make_yuvfile
3、upload_faceinfo
4、connect_test
******************************************************************************************/

char g_ip[20] = {0};
unsigned short g_port = 0;
typedef void (*pFunc)(char *cmd_type, char *pbuff);
pFunc g_pfuc1 = NULL; 


int get_val(char * pbuff, char * ptype, char * pval)
{
	char * s, *e;
	int off = 0;
	char tmp[64] = {0};

	if(!pbuff || ! ptype || !pval)
	{
		return 0;
	}

	sprintf(tmp, "<%s>", ptype);
	off = strlen(tmp);

	s = strstr(pbuff, tmp);
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "</%s>", ptype);
	e = strstr(pbuff, tmp); 

	if(s && e)
	{
		s += off;
		memcpy(pval, s, e - s);
		return 1;
	}

	return 0;
}

int  analyse_body(int sock, char * pbody, int bodylen)
{
	char * xml_type = NULL;
	char  cmd_type[64] = {0};
	char  str_devtype[64] = {0};
	char  str_devID[64] = {0};
	int   dev_type;
	int   flag = 0;
	char buff[PROTOCOL_BUFF_LEN] = {0};
	int len = 0;

	if(!pbody)
	{
		return 0;
	}

	xml_type = strstr(pbody, "Query");
	if(!xml_type)
	{
		return 0;
	}

	if(get_val(pbody, "DeviceType", str_devtype))
	{
		dev_type = strtol(str_devtype, NULL, 16);
	}

	get_val(pbody, "DeviceID", str_devID);

	if(get_val(pbody, "CmdType", cmd_type))
	{
		if(0 == strcmp(cmd_type, "Policy"))
		{
			char tmp[64] = {0};
			struct s_policy_info policy_info;

			get_val(pbody, "NetRestore", tmp);

			if(0 == strcmp(tmp, "TRUE"))
			{
				policy_info.net_restore = 1;
			}
			else
			{
				policy_info.net_restore = 0;
			}


			memset(tmp, 0, sizeof(tmp));
			get_val(pbody, "GPS", tmp);
			
			if(0 == strcmp(tmp, "TRUE"))
			{
				policy_info.gps = 1;
			}
			else
			{
				policy_info.gps = 0;
			}

			memset(tmp, 0, sizeof(tmp));
			get_val(pbody, "NFC", tmp);
			
			if(0 == strcmp(tmp, "TRUE"))
			{
				policy_info.nfc = 1;
			}
			else
			{
				policy_info.nfc = 0;
			}

			memset(tmp, 0, sizeof(tmp));
			get_val(pbody, "CAM", tmp);
			
			if(0 == strcmp(tmp, "TRUE"))
			{
				policy_info.cam = 1;
			}
			else
			{
				policy_info.cam = 0;
			}	

			flag = 1;
			no_data_ack(dev_type, str_devID, cmd_type, flag, &buff[PROTOCOL_HEADER_LEN], &len);
			if(g_pfuc1)
			{
				g_pfuc1(CMDTYPE_POLICY,(char*)&policy_info);
			}
		}
		else if(0 == strcmp(cmd_type, "DeviceInfo"))
		{
			struct s_devstate_info devstate_info;
			flag = 1;
			g_pfuc1(CMDTYPE_DEVINFO,(char*)&devstate_info);
			dev_info_ack(dev_type, str_devID, cmd_type, &devstate_info, &buff[PROTOCOL_HEADER_LEN], &len);
		}
		else if (0 == strcmp(cmd_type, "CatchPicture"))
		{
			char *pImageData = NULL;
			struct s_picture_data picture_data;
			flag = 1;
			g_pfuc1(CMDTYPE_PICTURE, (char*)&picture_data);

			dev_catch_picture(dev_type, str_devID, cmd_type, &picture_data, &pImageData, &len);

			build_netheader(pImageData, len);

			if(SendData(sock, pImageData, PROTOCOL_HEADER_LEN + len) < 0)
			{
				flag = 0;
			}

			free(pImageData);
			free(picture_data.pdata);
			pImageData = NULL;
			picture_data.pdata = NULL;

			return flag;
		}
	}

	
	if(!flag)
	{
		no_data_ack(dev_type, str_devID, cmd_type, flag, &buff[PROTOCOL_HEADER_LEN], &len);
	}

	build_netheader(buff, len);

	if(SendData(sock, buff, PROTOCOL_HEADER_LEN + len) < 0)
	{
		flag = 0;
	}		

	return flag;

}

#ifdef WIN32
static unsigned long CALLBACK listen_thread(void *p)
#else
static void * listen_thread(void *p)
#endif
{
	int  len  = 0;
	int  sock_svr = -1;
	int  sock = -1;
	struct sockaddr_in client;

	sock_svr = create_listen(g_ip, g_port);
	len = sizeof(struct sockaddr_in);

	while(1)
	{
		sock = accept(sock_svr, (struct sockaddr*)(&client), &len);

		if(sock != -1)
		{
			int ret = 0;
			unsigned char buff[PROTOCOL_BUFF_LEN] = {0};
			int optval = NET_REC_TIMEOUT * 1000;

			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)(&optval), sizeof(optval));

			while(1)
			{
				ret = RecvData(sock, (char*)buff, PROTOCOL_HEADER_LEN);
				if(ret > 0)
				{
					int len = 0;
					if(buff[0] != 'L' || buff[1] != 'Z')
					{
						break;;
					}
					
					len |= buff[3] << 24;
					len |= buff[4] << 16;
					len |= buff[5] << 8;
					len |= buff[6] << 0;

					if(len > PROTOCOL_BUFF_LEN)
					{
						break;
					}
					
					memset(buff, 0, sizeof(buff));
					ret = RecvData(sock, buff, len);
					if(ret <= 0)
					{
						break;
					}
					analyse_body(sock, buff, len);
				}
				else
				{
					break;
				}
			}

			closesocket(sock);
		}
	}
	
	return 0;
}

int register_callback(void (*p)(char *cmd_type, char *pbuff))
//int register_callback(void (*p)(struct s_policy_info * policy_info))
{
	g_pfuc1 = p;
	return 1;
}

int start_listen(char * ip, unsigned short port)
{
#ifdef WIN32
	unsigned long dwThreadID = 0;
	HANDLE h = 0;
#else
	pthread_t th;
#endif

	if(!ip || port == 0 || port == 0xFFFF)
	{
		return 0;
	}

	strcpy(g_ip, ip);
	g_port = port;


#ifdef WIN32
	h = CreateThread(NULL, NULL, listen_thread, NULL, 0, &dwThreadID);
	if(h)
	{
		CloseHandle(h);
	}
	else
	{
		return 0;
	}
#else
	if(pthread_create(&th, NULL, (void *)listen_thread, NULL) != 0)
	{
		return 0;
	}
#endif

	return 1;
}


int make_compresspackage(char * dir)
{
	int fd = -1;
	char buff[512] = {0};
	int n = 0;

	if(!dir)
	{
		dir = PHOTO_DIR;
	}

#ifdef WIN32
	//sprintf(buff, "del %s.tar", dir);
	//system(buff);

	//n = rfind(dir ,"\\"); 

	//if (n > 0)
	//{
	//	left(path, dir, n+1);
	//}

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "rar a -ep1 -idcdpq %s.rar %s", dir, dir);
	system(buff);

// 	memset(buff, 0, sizeof(buff));
// 	sprintf(buff, "7z.exe a -ttar %s.tar %s", dir, dir);
// 	system(buff);
// 
// 	memset(buff, 0, sizeof(buff));
// 	sprintf(buff, "7z.exe a -tgzip %s.tar.gz %s.tar", dir, dir);
// 	system(buff);
#else
	sprintf(buff, "rm -f %s.tar.gz", dir);
	system(buff);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "tar czvf  %s.tar.gz %s", dir, dir);
	system(buff);	
#endif

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%s.rar", dir);
	fd = open(buff, O_RDONLY);
	if(fd < 0)
	{
		return 0;
	}

	close(fd);

	return 1;
}

int upload_faceinfo(char * ip, unsigned short port, int devtype, char *devid, char * taskid, char *latitude, char * longitude, int face_num, int comp_status, int compress_format, char * compress_file)
{
	int len = 0;
	char buff[PROTOCOL_BUFF_LEN] = {0};
	int sock = -1;
	int fd = -1;
	int file_len = 0;
	int ret_flag = 0;

	if(!latitude || !longitude)
	{
		return 0;
	}

	if(!compress_file)
	{
		compress_file = PHOTO_FILENAME;
	}

	fd = open(compress_file, O_RDONLY | O_BINARY);	
	if(fd < 0)
	{
		file_len = 0;
	}
	else
	{
		file_len = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
	}

	// 组织照片请求包
	if(!to_faceinfo_body(devtype, devid, taskid, latitude, longitude, face_num, comp_status, compress_format, compress_file, file_len, &buff[PROTOCOL_HEADER_LEN], &len))
	{
		goto err;
	}

	// 制作网络头
	build_netheader(buff, len);

	sock = create_socket(ip, port);
	
	if(sock < 0)
	{
		goto err;
	}

	//发送协议头+XML包
	if(SendData(sock, buff, PROTOCOL_HEADER_LEN + len) < 0)
	{
		goto err;
	}
	
	//发送照片压缩文件
	if(file_len > 0)
	{
		memset(buff, 0, sizeof(buff));
		while(len = read(fd, buff, sizeof(buff)))
		{
			if(SendData(sock, buff, len) < 0)
			{
				goto err;
			}
		}
	}

	//接收应答
	memset(buff, 0, sizeof(buff));

	if(RecvData(sock, buff, PROTOCOL_HEADER_LEN) < 0)
	{
		goto err;
	}

	len = 0;

	//检测协议头
	if(check_netheader(buff, &len))
	{
		memset(buff, 0, sizeof(buff));
		//接收应答包
		if(RecvData(sock, buff, len) < 0)
		{
			goto err;
		}
		//检测请求是否成功
		if(!chec_no_data_ack("FaceInfo", buff))
		{
			goto err;
		}
	}
	else
	{
		goto err;
	}

	ret_flag = 1;

err:
	if(fd != -1)
	{
		close(fd);
	}

	if(sock != -1)
	{
		closesocket(sock);
	}

	return ret_flag;
}

int connect_test(char * ip, unsigned short port)
{
	int sock = create_socket(ip, port);

	if(sock < 0)
	{
		return 0;
	}

	closesocket(sock);

	return 1;
}

int unload_notify(char * ip, unsigned short port, int devtype, char *devid, struct s_event_info * event_info)
{
	int len = 0;
	char buff[PROTOCOL_BUFF_LEN] = {0};
	int sock = -1;
	int ret_flag = 0;
	
	
	if(!to_eventinfo_body(devtype, devid, event_info, &buff[PROTOCOL_HEADER_LEN], &len))
	{
		goto err;
	}
	
	build_netheader(buff, len);
	
	sock = create_socket(ip, port);
	
	if(sock < 0)
	{
		goto err;
	}
	
	//发送协议头+XML包
	if(SendData(sock, buff, PROTOCOL_HEADER_LEN + len) < 0)
	{
		goto err;
	}
	

	//接收应答
	memset(buff, 0, sizeof(buff));
	
	if(RecvData(sock, buff, PROTOCOL_HEADER_LEN) < 0)
	{
		goto err;
	}
	
	len = 0;
	
	if(check_netheader(buff, &len))
	{
		memset(buff, 0, sizeof(buff));
		if(RecvData(sock, buff, len) < 0)
		{
			goto err;
		}
		if(!chec_no_data_ack("AlarmInfo", buff))
		{
			goto err;
		}
	}
	else
	{
		goto err;
	}
	
	ret_flag = 1;
	
err:
	
	if(sock != -1)
	{
		closesocket(sock);
	}
	
	return ret_flag;
}

int unload_verify(char * ip, unsigned short port, int devtype, char *devid, char *taskid,  char* photoname, int score)
{

	int len = 0;
	char buff[PROTOCOL_BUFF_LEN] = {0};
	int sock = -1;
	int ret_flag = 0;

	if(!to_verifyinfo_boby(devtype, devid, taskid, photoname, score, &buff[PROTOCOL_HEADER_LEN], &len))
	{
		goto err;
	}

	build_netheader(buff, len);

	sock = create_socket(ip, port);

	if(sock < 0)
	{
		goto err;
	}

	//发送协议头+XML包
	if(SendData(sock, buff, PROTOCOL_HEADER_LEN + len) < 0)
	{
		goto err;
	}


	//接收应答
	memset(buff, 0, sizeof(buff));

	if(RecvData(sock, buff, PROTOCOL_HEADER_LEN) < 0)
	{
		goto err;
	}

	len = 0;

	if(check_netheader(buff, &len))
	{
		memset(buff, 0, sizeof(buff));
		if(RecvData(sock, buff, len) < 0)
		{
			goto err;
		}
		if(!chec_no_data_ack("FaceInfoComp", buff))
		{
			goto err;
		}
	}
	else
	{
		goto err;
	}

	ret_flag = 1;

err:

	if(sock != -1)
	{
		closesocket(sock);
	}

	return ret_flag;

}

//2015-09-08 chen_zx
//公共信息上报
int common_report(char * ip, unsigned short port, char * input_aes, int * len, char * output)
{
	char header[PROTOCOL_HEADER_LEN] = {0};
	int sock = -1;
	int ret_flag = 0;

	char * buff = (char*)malloc(PROTOCOL_HEADER_LEN + *len);
	memset(buff, 0, PROTOCOL_HEADER_LEN + *len);

	build_netheader(header, *len);

	sock = create_socket(ip, port);

	if(sock < 0)
	{
		goto err;
	}

	memcpy(buff, header, PROTOCOL_HEADER_LEN);
	memcpy(buff + PROTOCOL_HEADER_LEN, input_aes, *len);

    //发送协议头+XML包
    if(SendData(sock,buff,PROTOCOL_HEADER_LEN+ *len) < 0)
    {
        goto err;
    }
    /*
    if(SendData(sock, buff+ PROTOCOL_HEADER_LEN, *len) < 0)
	{
		goto err;
    }
    */

	//接收应答

    //memset(input_aes, 0, sizeof(input_aes));

	if(RecvData(sock, output, PROTOCOL_HEADER_LEN) < 0)
	{
		goto err;
	}

	if(check_netheader(output, len))
	{
		memset(output, 0, sizeof(output));
		if(RecvData(sock, output, *len) < 0)
		{
			goto err;
		}

		//memcpy(output, input_aes, *len);
	}

	ret_flag = 1;

err:

	if(sock != -1)
	{
		closesocket(sock);
	}

	if (buff != NULL)
	{
		free(buff);
		buff = NULL;
	}

	return ret_flag;
}
