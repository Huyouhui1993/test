#include "cliStaffTcp.h"

int socketInitialize(const char *IP,const char *PORT)
{
	int sfd;
	//创建字节流套接字
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	//链接服务器
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(atoi(PORT));
	sin.sin_addr.s_addr = inet_addr(IP);

	socklen_t addrlen = sizeof(sin);

	if(connect(sfd, (struct sockaddr *)&sin, addrlen))
	{
		ERR_MSG("connect");
		return -1;
	}

	return sfd;
}

//菜单函数
int mainMenuFunction()
{
	printf("===========================菜单==================================\n");
	printf("========================1.管理员登录=============================\n");
	printf("========================2.员工登录===============================\n");
	printf("========================3.管理员注册=============================\n");
	printf("========================0.退出===================================\n");
	printf("=================================================================\n");
	printf("=================================================================\n");
	printf("Choose>>>\n");
	return 0;
}

//数据包发送函数
int doSendMsg(MsgTran push, int newfd)
{
	if(send(newfd, &push, sizeof(push), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	return 0;
}

//数据包接收函数
int doRecvMsg(MsgTran *push, int newfd)
{
	if(recv(newfd, push, sizeof(*push), 0) < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	return 0;
}

//管理员注册函数
int administratorRegisterCli(int newfd)
{
	char str[N] = "";
	MsgTran push;
	int res;
	//注册码验证
	while(1)
	{
		printf("请输入注册码>>>\n");
		scanf("%s", str);
		res = strcmp(CIPHER, str);
		if(res != 0)
		{
			printf("验证码错误，无法注册,请重新输入注册码\n");
			printf("或者输入‘#’退出注册\n");
		}
		printf("str: %s\n", str);
		if((strlen(str) == 1) && (str[0] == '#'))
		{
			printf("退出注册\n");
			return 0;
		}

		if(res == 0)
		{
			break;
		}
	}
	printf("请输入账号>>>\n");
	scanf("%s", push.account);
	printf("请输入密码>>>\n");
	scanf("%s", push.cipher);
	push.type = 'R';

	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}

	printf("line: %d\n", __LINE__);
	printf("push.account: %s\n", push.account);
	printf("push.state: %c\n", push.state);

	if(push.state == 'N')
	{
		printf("管理员注册失败\n");
	}
	else if(push.state == 'Y')
	{
		printf("管理员账户注册成功\n");
	}
	return 0;
}


//管理员登录
int administratorLogin(int newfd)
{
//	char str[N] = "";
	MsgTran push;
	int res;
	char choose;

	push.type = 'L';

	//账号密码输入
	printf("请输入账号>>>\n");
	scanf("%s", push.account);
	printf("请输入密码>>>\n");
	scanf("%s", push.cipher);

	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}

	if(push.state == 'N')
	{
		printf("管理员登录失败\n");
		return 0;
	}
//	printf("=============%d\n",__LINE__);
//	printf("push.state: %c\n", push.state);
	
	//登录成功
	//管理员菜单函数
	while(1)
	{
		administratorMenuFunction();
		//选择函数
		while(getchar() != 10);

		scanf("%c", &choose);
		while(getchar() != 10);

		switch(choose)
		{
			case '1':
				//查询数据
				if(administratorSearchData(newfd) < 0)
				{
					printf("管理员查询数据函数调用失败\n");
					return -1;
				}	

				break;
			case '2':
				//修改员工数据
				if(administratorModifyData(newfd) < 0)
				{
					printf("管理员修改数据函数调用失败\n");
					return -1;
				}	
				

				break;
			case '3':
				//修改管理员密码
				break;
			case '4':
				//删除员工数据
				if(administratorDelateData(newfd) < 0)
				{
					printf("管理员删除数据函数调用失败\n");
					return -1;
				}	

				break;
			case '5':
				//增加员工数据
				if(administratorAddData(newfd) < 0)
				{
					printf("管理员增加数据函数调用失败\n");
					return -1;
				}	
				break;
			case '0':
				//返回上一级
				return 0;
			default:
				break;
		}

	}
	
	return 0;
}

//管理员增加数据函数
int administratorAddData(int newfd)
{
	MsgTran push;
	push.type = 'A';
	printf("请依次输入员工姓名、年龄、性别、电话、地址、薪资、密码\n");
	scanf("%s%d%*c%c%s%s%d%s", push.adata.name, &push.adata.age,\
			&push.adata.sex, push.adata.tel, push.adata.addr, &push.adata.salary, push.adata.cipher);
	
	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}

	if(push.state == 'N')
	{
		printf("管理员增加数据失败\n");
		return 0;
	}

	printf("管理员增加数据成功\n");
	return 0;
}


//管理员删除数据函数
int administratorDelateData(int newfd)
{
	MsgTran push;
	push.type = 'D';
	printf("请输入要删除员工姓名\n");
	printf("请输入>>>\n");
	scanf("%s", push.adata.name);

	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}

	if(push.state == 'N')
	{
		printf("管理员删除数据失败\n");
		return 0;
	}

	printf("管理员删除数据成功\n");
	return 0;
}

//管理员修改数据函数
int administratorModifyData(int newfd)
{
	MsgTran push;
	push.type = 'C';
	printf("请依次输入员工姓名、年龄、性别、电话、地址、薪资、密码\n");
	scanf("%s%d%*c%c%s%s%d%s", push.adata.name, &push.adata.age,\
			&push.adata.sex, push.adata.tel, push.adata.addr, &push.adata.salary, push.adata.cipher);
	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}
	
	if(push.state == 'N')
	{
		printf("管理员修改数据失败\n");
	}

	printf("管理员修改数据成功\n");

	return 0;
}

//管理员查询数据函数
int administratorSearchData(int newfd)
{
	MsgTran push;
	push.type = 'S';
	int flag = 0;
	printf("请输入要查询员工的姓名>>>\n");
	scanf("%s", push.adata.name);

	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	
	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}

	if(push.state == 'N')
	{
		printf("管理员查询数据失败\n");
		return 0;
	}

	printf("管理员查询数据成功\n");

	if(flag == 0)
	{
		printf("name\tage\tsex\ttelephone\taddress\tsalary\tcipher\tstate\n");
		flag = 1;
	}
	printf("%s\t%d\t%c\t%s\t\t%s\t%d\t%s\t%d\n",\
			push.adata.name, push.adata.age, push.adata.sex, push.adata.tel, push.adata.addr, push.adata.salary, push.adata.cipher, push.adata.state);

	return 0;
}


//管理员菜单函数
int administratorMenuFunction()
{
	printf("===========================菜单==================================\n");
	printf("========================1.查询数据===============================\n");
	printf("========================2.修改员工数据===========================\n");
	printf("========================3.修改管理员密码=========================\n");
	printf("========================4.删除员工数据===========================\n");
	printf("========================5.增加员工数据===========================\n");
	printf("========================0.返回上一级=============================\n");
	printf("=================================================================\n");
	printf("=================================================================\n");
	printf("Choose>>>\n");
	return 0;	
}


//员工登录
int staffLogin(int newfd)
{
	MsgTran push;
	int res;
	char choose;
	
	push.type = 'l';

	printf("请输入员工账号>>>\n");
	scanf("%s", push.account);
	printf("请输入员工密码>>>\n");
	scanf("%s", push.cipher);

	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//接收数据包
	memset(&push, 0, sizeof(push));
	if(doRecvMsg(&push, newfd) < 0)
	{
		printf("doRecvMsg error: %d\n", __LINE__);
		return -1;
	}

	//判断登录是否成功
	if(push.state == 'N')
	{
		printf("员工登录失败\n");
		return 0;
	}

	while(1)
	{
		staffMenuFunction();

		//选择函数
		while(getchar() != 10);

		scanf("%c", &choose);
		while(getchar() != 10);

		switch(choose)
		{
			case '1':
				//查询数据
				if(administratorSearchData(newfd) < 0)
				{
					printf("管理员查询数据函数调用失败\n");
					return -1;
				}	

				break;
			case '2':
				//修改数据
/*				if(administratorModifyData(newfd) < 0)
				{
					printf("管理员修改数据函数调用失败\n");
					return -1;
				}
*/			case '0':
				//返回上一级
				return 0;
				break;
			default:
				break;
		}
		
	}

	return 0;	
}


//员工菜单函数
int staffMenuFunction()
{
	printf("===========================菜单==================================\n");
	printf("========================1.查询数据===============================\n");
	printf("========================2.修改数据===========================\n");
	printf("========================0.返回上一级=============================\n");
	printf("=================================================================\n");
	printf("=================================================================\n");
	printf("Choose>>>\n");

	return 0;
	
}
