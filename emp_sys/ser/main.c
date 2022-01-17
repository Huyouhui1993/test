#include "serStaffTcp.h"

int main(int argc, const char *argv[])
{
	long res;
	sqlite3 *db;
	const char *IP = argv[2];
	const char *PORT = argv[1];
	//1.外部传参
	if(argc != 3)
	{
		printf("外部传参错误,请传参端口号及ip地址\n");
		return -1;
	}
//	IP = argv[2];
//	PORT = argv[1];
	
	//2.数据库初始化
	res = sqlite3Initialize(&db);
	if(res < 0)
	{
		printf("sqlite3Initialize failed\n");
		return -1;
	}
//	db = (sqlite3 *)res;
//	printf("db = %p\n", db);
//	printf("res = %lx\n", res);

	//3.网络初始化
	res = socketInitialize(IP, PORT, db);
	if(res < 0)
	{
		printf("socketInitialize failed\n");
		return -1;
	}
	
	

	
	return 0;
}
