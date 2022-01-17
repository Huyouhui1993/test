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

/*
typedef struct{
	
}__attribute__((packed)) Sdata;
*/
typedef struct{
	char type;//协议
	char account[LEN];//账号
	char cipher[LEN];//密码
	char state;//注册/登录状态/查询/修改 Y表示成功 N表示失败
	Adata adata;//管理员要的数据，员工可查询的数据
	char sdata[N];//员工要修改数据
}__attribute__((packed)) MsgTran;

typedef struct{
	int newfd;
	struct sockaddr_in cin;
	sqlite3 *db;
}CliMsg;


//数据包发送函数
int doSendMsg(MsgTran push, int newfd);

//管理员注册
int administratorRegister(sqlite3 *db, MsgTran push, int newfd);


//管理员登录
int administratorLogin(sqlite3 *db, MsgTran push, int newfd);

//管理员增加员工数据
int administratorAddData(sqlite3 *db, MsgTran push, int newfd);

//管理员删除员工数据
int administratorDelateData(sqlite3 *db, MsgTran push, int newfd);

//管理员修改数据
int administratorModifyData(sqlite3 *db, MsgTran push, int newfd);

//管理员查询数据
int administratorSearchData(sqlite3 *db, MsgTran push,int newfd);

//员工登录
int staffLogin(sqlite3 *db, MsgTran push, int newfd);
/*
//员工修改数据
int staffModifyData();

//员工查询数据
int staffSearchData();
*/

//网络初始化
int socketInitialize(const char *IP, const char *PORT, sqlite3 *db);

//分支线程接收分析发送数据
void *doSendRecive(void *arg);

//数据库初始化
int sqlite3Initialize(sqlite3 **mdb);

//创建管理员账号密码表
int createAdministratorAccountSheet(sqlite3 *db);

//创建员工账号、密码、数据表
int createStaffDataSheet(sqlite3 *db);

//客户端退出函数
int cliQuit(sqlite3 *db, MsgTran push);

#endif
