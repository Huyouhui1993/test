#ifndef __SER_STAFF_TCP_H__
#define __SER_STAFF_TCP_H__


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <unistd.h>

#define CIPHER "999"
#define N 128
#define M 256
#define MN 512
#define MAX 50
#define LEN 20
#define ERR_MSG(msg) do{\
	printf("error line: %d\n",__LINE__);\
	perror(msg);\
}while(0)

/*
 * 协议
 * R:管理员注册
 * L:管理员登录 	l:员工登录
 * A:管理员增加数据
 * D:管理员删除数据
 * C:管理员修改数据 c:员工修改数据
 * S:管理员查询数据 s:员工查询数据
*/


typedef struct{
	char name[LEN];
	int age;
	char sex;
	char tel[N];
	char addr[LEN];
	int salary;
	char cipher[LEN];
	int state;
}__attribute__((packed)) Adata;


typedef struct{
	char type;//协议
	char account[LEN];//账号
	char cipher[LEN];//密码
	char state;//注册/登录状态/查询/修改 Y表示成功 N表示失败
//	char abuf[M];//管理员要的数据，员工可查询的数据
	Adata adata;//管理员要的数据，员工可查询的数据
	char sbuf[N];//员工要修改数据
}__attribute__((packed)) MsgTran;

typedef struct{
	int newfd;
	struct sockaddr_in cin;
	sqlite3 *db;
}CliMsg;



//网络初始化
int socketInitialize(const char *IP, const char *PORT);

//菜单函数
int mainMenuFunction();

//数据包发送函数
int doSendMsg(MsgTran push, int newfd);

//数据包接收函数
int  doRecvMsg(MsgTran *push, int newfd);

//管理员注册函数
int administratorRegisterCli(int newfd);
				
//管理员登录
int administratorLogin(int newfd);

//管理员菜单函数
int administratorMenuFunction();

//管理员增加数据函数
int administratorAddData(int newfd);


//管理员增加数据函数
int administratorDelateData(int newfd);

//管理员修改数据函数
int administratorModifyData(int newfd);

//管理员查询数据函数
int administratorSearchData(int newfd);

//员工登录
int staffLogin(int newfd);

//员工菜单函数
int staffMenuFunction();
#endif
