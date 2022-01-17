#include "serStaffTcp.h"

/*

//管理员注册
int administratorRegister();

//管理员登录
int administratorLogin();

//员工登录
int staffLogin();

//管理员增加员工数据
int administratorAddData();

//管理员删除员工数据
int administratorDelateData();

//管理员修改数据
int administratorModifyData();

//员工修改数据
int staffModifyData();

//管理员查询数据
int administratorSearchData();

//员工查询数据
int staffSearchData();

*/

//数据库初始化
int sqlite3Initialize(sqlite3 **mdb)
{
	int res;
	//打开数据库（数据库不存在自动创建数据库并打开）
	if(sqlite3_open("./staff.db", mdb) != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_open: %s\n",sqlite3_errmsg(*mdb));
		return -1;
	}
	sqlite3 *db = *mdb;
	//创建管理员账号密码表
	res = createAdministratorAccountSheet(db);
	if(res < 0)
	{
		printf("创建管理员账号密码表失败\n");
		return -1;
	}

	//创建员工账号、密码、数据表
	res = createStaffDataSheet(db);
	if(res < 0)
	{
		printf("创建员工账号、密码、信息表失败\n");
		return -1;
	}

	//	printf("db = %ld\n", (long)db);
	//	printf("db = %p\n", db);
	//	return (long)db;
	return 0;
}

//创建员工账号、密码、数据表
int createStaffDataSheet(sqlite3 *db)
{
	//创建表格
	char *errmsg = NULL;
	char sql[M] = "create table if not exists staff(name char primary key, age int, sex char, telephone char, address char, salary int,cipher char, stastate int);";
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec: %s line = %d\n", errmsg, __LINE__);
		return -1;
	}
	printf("创建员工账号、密码、数据表完毕\n");	

	//初始化表格状态
	sprintf(sql, "update staff set stastate=0 where stastate=1;");
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec: %s line = %d\n", errmsg, __LINE__);
		return -1;
	}
	printf("管理员账号密码表初始化完毕\n");
	return 0;

}

//创建管理员账号密码表
int createAdministratorAccountSheet(sqlite3 *db)
{
	//创建表格
	char *errmsg = NULL;
	char sql[N] = "create table if not exists admin(name char primary key, cipher char, admstate int);";
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec: %s line = %d\n", errmsg, __LINE__);
		return -1;
	}
	printf("管理员账号密码表创建完毕\n");

	//初始化表格状态
	sprintf(sql, "update admin set admstate=0 where admstate=1;");
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_exec: %s line = %d\n", errmsg, __LINE__);
		return -1;
	}
	printf("管理员账号密码表初始化完毕\n");
	return 0;

}

//网络初始化
int socketInitialize(const char *IP, const char *PORT, sqlite3 *db)
{
	int reuse = 1;
	//1.创建字节流式套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)
	{
		ERR_MSG("socket");
		return -1;
	}

	//2.允许端口快速重用
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
	{
		ERR_MSG("setsockopt");
		return -1;
	}

	//3.绑定IP和端口
	struct sockaddr_in sin;
	socklen_t addrlen = sizeof(sin);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(atoi(PORT));
	sin.sin_addr.s_addr = inet_addr(IP);
	printf("%d %s\n", atoi(PORT), IP);
	if(bind(sfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		ERR_MSG("bind");
		return -1;
	}

	//4.设置为监听模式
	if(listen(sfd, MAX))
	{
		ERR_MSG("listen");
		return -1;
	}

	//5.accept
	int newfd;
	struct sockaddr_in cin;
	pthread_t pid;
	CliMsg info;
	while(1)
	{
		newfd = accept(sfd, (struct sockaddr *)&cin, &addrlen);
		if(newfd < 0)
		{
			ERR_MSG("accept");
			return -1;
		}
		printf("[%s:%d] newfd = %d\n", inet_ntoa(cin.sin_addr),\
				ntohs(cin.sin_port), newfd);


		//创建分支线程用于通信
		info.newfd = newfd;
		info.cin = cin;
		info.db = db;
		if(pthread_create(&pid, 0, doSendRecive, (void *)&info))
		{
			ERR_MSG("pthread_create");
			return -1;
		}

	}
}

//分支线程接收分析发送数据
void *doSendRecive(void *arg)
{
	pthread_detach(pthread_self());
	CliMsg info = *(CliMsg *)arg;
	struct sockaddr_in cin = info.cin;
	int newfd = info.newfd;
	sqlite3 *db = info.db;

	//接收发送数据包
	MsgTran push;
	ssize_t res;
	while(1)
	{
		memset(&push, 0, sizeof(push));
		res = recv(newfd, &push, sizeof(push), 0);
		if(res < 0)
		{
			ERR_MSG("recv");
			return NULL;
		}
		else if(res == 0)
		{
			printf("客户端退出\n");
			//客户端退出函数，数据库内登录状态清零。
			if(cliQuit(db, push) < 0)
			{
				printf("cliQuit error: %d\n", __LINE__);
				return NULL;
			}
			close(newfd);
			pthread_exit(NULL);
			return NULL;
		}

		//解析数据包
		switch(push.type)
		{
		case 'R':
			//管理员注册
			if(administratorRegister(db, push, newfd) < 0)
			{
				printf("管理员注册函数调用失败\n");
				return NULL;
			}

			break;
		case 'L':
			//管理员登录
			if(administratorLogin(db, push, newfd) < 0)
			{
				printf("管理员登录函数调用失败\n");
				return NULL;
			}
			break;

		case 'l':
			//员工登录
			if(staffLogin(db, push, newfd) < 0)
			{
				printf("员工登录函数调用失败\n");
				return NULL;
			}
			

			break;
		case 'A':
			//管理员增加数据
			if(administratorAddData(db, push, newfd) < 0)
			{
				printf("管理员登录函数调用失败\n");
				return NULL;
			}
			break;
		case 'D':
			//管理员删除数据
			if(administratorDelateData(db, push, newfd) < 0)
			{
				printf("管理员登录函数调用失败\n");
				return NULL;
			}

			break;
		case 'C':
			//管理员修改数据
			if(administratorModifyData(db, push, newfd) < 0)
			{
				printf("管理员修改数据函数调用失败\n");
				return NULL;
			}
			break;
		case 'c':
			//员工修改数据
			break;
		case 'S':
			//管理员查询数据
			if(administratorSearchData(db, push, newfd) < 0)
			{
				printf("管理员查询函数调用失败\n");
				return NULL;
			}				

			break;
		case 's':
			//员工查询数据
			break;
		default:
			break;
		}

	}
}

//客户端退出函数
int cliQuit(sqlite3 *db, MsgTran push)
{
	char *errmsg = NULL;
	char sql[N] = "";
	int flag = 0;
	char **presult = NULL;
	int row, column;

	if(push.account != NULL)
	{
		//查询账号是否在表内
		sprintf(sql, "select * from admin where name=\"%s\";", push.account);
		if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
		{
			printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
			return -1;
		}
		if(row > 0)
		{
			flag = 1;
		}

		sprintf(sql, "select * from staff where name=\"%s\";", push.account);
		if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
		{
			printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
			return -1;
		}
		if(row > 0)
		{
			flag = 2;
		}

		if(flag == 2)
		{
			//清空员工表状态
			sprintf(sql, "update staff set stastate=0 where name=\"%s\";", push.account);
			if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
			{
				printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
				return -1;
			}
		}
		else if(flag == 1)
		{
			//清空管理员表状态
			sprintf(sql, "update admin set admstate=0 where name=\"%s\";", push.account);
			if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
			{
				printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
				return -1;
			}
		}
	}
	return 0;
}

//管理员注册函数
int administratorRegister(sqlite3 *db, MsgTran push, int newfd)
{
	char sql[M] = "";
	char **presult = NULL;
	int row,column;
	char *errmsg = NULL;
	//解析数据包
	sprintf(sql, "select * from admin where name=\"%s\";", push.account);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}

	//数据存在则注册失败
	if(row > 0)
	{
		push.state = 'N';
	}
	//数据插入数据库
	if(row == 0)
	{
		push.state = 'Y';
		sprintf(sql, "insert into admin values (\"%s\", \"%s\", 0);", push.account, push.cipher);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
			return -1;
		}
	}

	printf("row: %d\n", row);

	//释放查询到的结果
	sqlite3_free_table(presult);

	//	printf("push.state: %c\n", push.state);
	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	//	printf("push.cipher: %s\n", push.cipher);

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

//管理员登录函数
int administratorLogin(sqlite3 *db, MsgTran push, int newfd)
{
	char sql[M] = "";
	char *errmsg = NULL;
	char **presult = NULL;
	int row, column;
	int res = 0;

	//查询数据是否存在
	sprintf(sql, "select * from admin where name=\"%s\";", push.account);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}

	printf("presult[2]: %s %s %d presult[5]:%d\n", presult[3], presult[4], *presult[5], *presult[5]);
	//数据不存在登录失败
	if(row == 0)
	{
		push.state = 'N';
		//释放查询到的结果
		sqlite3_free_table(presult);
		//发送数据包
		if(doSendMsg(push, newfd) < 0)
		{
			printf("doSendMsg error: %d\n", __LINE__);
			return -1;
		}
	}
	//数据存在密码错误或者重复登录失败
	else
	{
		//	printf("presult[4]: %s\n", presult[4]);
		res = strcmp(presult[4], push.cipher);
		if(res != 0)
		{
			push.state ='N';
		}
		else
		{
			if(*presult[5] == '1')
			{
				push.state = 'N';
				//释放查询到的结果
				sqlite3_free_table(presult);
				//发送数据包
				if(doSendMsg(push, newfd) < 0)
				{
					printf("doSendMsg error: %d\n", __LINE__);
					return -1;
				}
				return 0;
			}
			else
			{
				push.state = 'Y';
				//数据存在登录成功
				memset(sql, 0, sizeof(sql));
				sprintf(sql, "update admin set admstate=1 where name=\"%s\";", push.account);
				if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
				{
					printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
					return -1;
				}
//	printf("presult[2]: %s %s %d presult[5]:%d\n", presult[3], presult[4], *presult[5], *presult[5]);
				//释放查询到的结果
				sqlite3_free_table(presult);

				//发送数据包
				if(doSendMsg(push, newfd) < 0)
				{
					printf("doSendMsg error: %d\n", __LINE__);
					return -1;
				}

			}
		}
	}
	return 0;
}

//管理员增加员工数据
int administratorAddData(sqlite3 *db, MsgTran push, int newfd)
{
	char sql[M] = "";
	char **presult = NULL;
	int row,column;
	char *errmsg = NULL;

	//解析数据包
	sprintf(sql, "select * from admin where name=\"%s\";", push.adata.name);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}

	//数据存在则插入失败
	if(row > 0)
	{
		push.state = 'N';
	}
	//数据插入数据库
	if(row == 0)
	{
		push.state = 'Y';
		sprintf(sql, "insert into staff values (\"%s\", \"%d\", \"%c\", \"%s\", \"%s\", \"%d\", \"%s\", 0);",\
				push.adata.name, push.adata.age, push.adata.sex, push.adata.tel, push.adata.addr, push.adata.salary, push.adata.cipher);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
			return -1;
		}

	}

	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}
	return 0;
}


//管理员删除员工数据
int administratorDelateData(sqlite3 *db, MsgTran push, int newfd)
{
	char sql[M] = "";
	char **presult = NULL;
	int row,column;
	char *errmsg = NULL;

	//解析数据包
	sprintf(sql, "select * from staff where name=\"%s\";", push.adata.name);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}
	//数据不存在则删除失败
	if(row == 0)
	{
		push.state = 'N';
	}

	//数据存在删除数据
	if(row > 0)
	{
		push.state = 'Y';
		sprintf(sql, "delete from staff where name=\"%s\";", push.adata.name);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
			return -1;
		}
	}
	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	return 0;
}

//管理员修改数据
int administratorModifyData(sqlite3 *db, MsgTran push, int newfd)
{
	char sql[MN] = "";
	char **presult = NULL;
	int row,column;
	char *errmsg = NULL;

	//解析数据包
	sprintf(sql, "select * from staff where name=\"%s\";", push.adata.name);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}

	//数据不存在则修改失败
	if(row == 0)
	{
		push.state = 'N';
	}

	//数据存在修改数据
	if(row > 0)
	{
		push.state = 'Y';
		sprintf(sql, "update staff set age=\"%d\", sex=\"%c\", telephone=\"%s\", address=\"%s\", salary=\"%d\", cipher=\"%s\" where name=\"%s\";",\
				push.adata.age, push.adata.sex, push.adata.tel, push.adata.addr, push.adata.salary, push.adata.cipher, push.adata.name);
		if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		{
			printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
			return -1;
		}
	}
	//发送数据包
	if(doSendMsg(push, newfd) < 0)
	{
		printf("doSendMsg error: %d\n", __LINE__);
		return -1;
	}

	return 0;
}

//管理员查询数据
int administratorSearchData(sqlite3 *db, MsgTran push,int newfd)
{
	char sql[MN] = "";
	char **presult = NULL;
	int row,column;
	char *errmsg = NULL;
	int i,j;

	//解析数据包
	sprintf(sql, "select * from staff where name=\"%s\";", push.adata.name);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}

	//数据不存在则查询失败
	if(row == 0)
	{
		push.state = 'N';
	}

	//数据存在修改数据
	if(row > 0)
	{
		push.state = 'N';
		push.state = 'Y';
		for(i = 1; i<=row; i++)
		{

			for(j=8*i; j<8*(i+1);)
			{
				//	printf("-------------------\n");
				strcpy(push.adata.name, presult[j++]);
				//	printf("%s\t", presult[i]);
				push.adata.age = *presult[j++];
				//	printf("%d\t", *presult[i]);
				push.adata.sex = *presult[j++];
				//	printf("%c\t", *presult[i]);
				strcpy(push.adata.tel, presult[j++]);
				//	printf("%s\t", presult[i]);
				strcpy(push.adata.addr, presult[j++]);
				//	printf("%s\t", presult[i]);
				push.adata.salary = *presult[j++];
				//	printf("%d\t", *presult[i]);
				strcpy(push.adata.cipher, presult[j++]);
				//	printf("%s\t", presult[i]);
				push.adata.state = *presult[j++];
				//	printf("%d\t", *presult[i]);
			}
			//发送数据包
			if(doSendMsg(push, newfd) < 0)
			{
				printf("doSendMsg error: %d\n", __LINE__);
				return -1;
			}
		}
	}

	return 0;
}

//员工登录
int staffLogin(sqlite3 *db, MsgTran push, int newfd)
{
	char sql[M] = "";
	char *errmsg = NULL;
	char **presult = NULL;
	int row, column;
	int res = 0;
	
	//查询数据是否存在
	sprintf(sql, "select * from staff where name=\"%s\";", push.account);
	if(sqlite3_get_table(db, sql, &presult, &row, &column, &errmsg) != SQLITE_OK)
	{
		printf("sqlite3_get_table: %s line: %d\n", errmsg, __LINE__);
		return -1;
	}
	
	printf("presult[15] = %d\n", *presult[15]);

	//数据不存在登录失败
	if(row == 0)
	{
		push.state = 'N';
		//释放查询到的结果
		sqlite3_free_table(presult);
		//发送数据包
		if(doSendMsg(push, newfd) < 0)
		{
			printf("doSendMsg error: %d\n", __LINE__);
			return -1;
		}
	}
	//数据存在密码错误或者重复登录失败
	else
	{
		//	printf("presult[14]: %s\n", presult[4]);
		res = strcmp(presult[14], push.cipher);
		if(res != 0)
		{
			push.state ='N';
		}
		else
		{
			if(*presult[15] == '1')
			{
				push.state = 'N';
				//释放查询到的结果
				sqlite3_free_table(presult);
				//发送数据包
				if(doSendMsg(push, newfd) < 0)
				{
					printf("doSendMsg error: %d\n", __LINE__);
					return -1;
				}
				return 0;
			}
			else
			{
				push.state = 'Y';
				//数据存在登录成功
				memset(sql, 0, sizeof(sql));
				sprintf(sql, "update staff set stastate=1 where name=\"%s\";", push.account);
				if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
				{
					printf("sqlite3_exec: %s line: %d\n", errmsg, __LINE__);
					return -1;
				}
//	printf("presult[2]: %s %s %d presult[5]:%d\n", presult[3], presult[4], *presult[5], *presult[5]);
				//释放查询到的结果
				sqlite3_free_table(presult);

				//发送数据包
				if(doSendMsg(push, newfd) < 0)
				{
					printf("doSendMsg error: %d\n", __LINE__);
					return -1;
				}

			}
		}
	}
	return 0;

}

