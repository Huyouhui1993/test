#include "cliStaffTcp.h"

int main(int argc, const char *argv[])
{
	const char *IP = argv[2];
	const char *PORT = argv[1];
	int sfd;

	//1.外部传参端口号以及IP地址
	if(argc != 3)
	{
		printf("请外部传参本机端口号及ip");
		return -1;
	}

	//2.网络初始化
	sfd = socketInitialize(IP, PORT);
	if(sfd < 0)
	{
		printf("socketInitialize failed\n");
		return -1;
	}
	
	//3.选择界面
	char choose;
	while(1)
	{
		//菜单函数
		mainMenuFunction();

		//选择函数
		scanf("%c", &choose);
		while(getchar() != 10);

		switch(choose)
		{
			case '1':
				//管理员登录
				if(administratorLogin(sfd) < 0)
				{
					printf("管理员登录函数调用失败\n");
					return -1;
				}

				break;
			case '2':
				//员工登录
				if(staffLogin(sfd) < 0)
				{
					printf("员工登录函数调用失败\n");
					return -1;
				}
				

				break;
			case '3':
				//管理员注册
				if(administratorRegisterCli(sfd) < 0)
				{
					printf("管理员注册函数调用失败\n");
					return -1;
				}

				break;
			case '0':
				//退出
				close(sfd);
				return 0;
			default:
				printf("value error,please input right choose\n");
				break;
		}
		printf("Enter any key clear screen\n");
		while(getchar() != 10);
		//system("clear");
	}
	return 0;
}
