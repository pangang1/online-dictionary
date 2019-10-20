#include"dict.h"

int TcpInit(char *addr,char *port)
{
	int sockfd;
	if(0>(sockfd=socket(AF_INET,SOCK_STREAM,0)))
	{
		perror("socket");	
		exit(-1);
	}
	struct sockaddr_in seraddr={
		.sin_family=AF_INET,
		.sin_port=htons(atoi(port)),
		.sin_addr.s_addr=inet_addr(addr)
	};
	if(0>connect(sockfd,(struct sockaddr*)&seraddr,sizeof(seraddr)))
	{
		perror("connect");
		exit(-1);
	}
	printf("connect........\n");
	return sockfd;

}
int GraphicInit(int sockfd)
{
	int a;
	system("clear");
	printf("***********************************\n");
	printf("1.登录2.注册3.退出\n");
	printf("请输入相应的数字来得到你想要的功能:\n");
	scanf("%d",&a);
	getchar();
	if(a!=1&&a!=2&&a!=3)
	{
		printf("输入错误，请按规则重新输入！\n");
		sleep(1);
		return -1;
	}
	else
		send(sockfd,&a,sizeof(a),0);
	return a;
}
void Register(int sockfd)
{
	int a;
	int flag;
	char buf[512];
	char answer;
	system("clear");	
	printf("用户名：");
	fgets(buf,sizeof(buf),stdin);
	buf[strlen(buf)-1]='\0';
	send(sockfd,buf,sizeof(buf),0);
	while(1)
	{
		flag=0;
		printf("密码：");
		fgets(buf,sizeof(buf),stdin);
		buf[strlen(buf)-1]='\0';
		for(a=0;buf[a]!='\0';a++)
		{
			if(!((buf[a]>='a'&&buf[a]<='z')||(buf[a]>='A'&&buf[a]<='Z')||(buf[a]>='0'&&buf[a]<='9')))
			{
				printf("密码格式只能为数字字母!\n");
				flag=1;
				break;
			}
		}
		if(flag==1)
			continue;
		else
			break;
	}
	send(sockfd,buf,sizeof(buf),0);

	recv(sockfd,&answer,1,0);
	if(answer=='N')
	{
		printf("此用户已被注册！\n1秒后退回主界面\n");
		sleep(1);
		return;
	}
	printf("注册成功！\n1秒后退回主界面\n");
	sleep(1);

	return ;
}


int Login(int sockfd)
{
	int a;
	char answer;
	char buf[512];
	system("clear");
	printf("用户名：");
	fgets(buf,sizeof(buf),stdin);
	buf[strlen(buf)-1]='\0';
	send(sockfd,buf,sizeof(buf),0);	
	recv(sockfd,&answer,1,0);
	if(answer=='N')
	{
		printf("没有此用户！\n1秒后退回主界面\n");
		sleep(1);
		return -1;
	}

	printf("密码：");
	fgets(buf,sizeof(buf),stdin);
	buf[strlen(buf)-1]='\0';
	send(sockfd,buf,sizeof(buf),0);
	recv(sockfd,&answer,1,0);
	if(answer=='N')
	{
		printf("密码错误！\n1秒后退回主界面\n");
		sleep(1);
		return -1;
	}
	return 0;
}
int SearchWorld(int sockfd)
{
	char buf[512];
	int count,i,j;
	system("clear");
	printf("输入1. 退出 返回到登录界面！\n");
	printf("输入2. 查询 查询历史记录！\n");
	printf("输入3. 注销 注销当前用户并退回登录界面！\n");
	printf("输入4. 删除 删除历史记录！\n");
	printf("输入5. 查看帮助！");
	printf("请按要求输入数字！\n");
	while(1)
	{
		printf("请输入要查询的单词:");
		fgets(buf,sizeof(buf),stdin);
		for(i=0;buf[i]==' ';i++);
		for(j=0;buf[i]!=' '&&buf[i]!='\n';j++,i++)
			buf[j]=buf[i];
		buf[j]='\0';
		if(!strcmp(buf,"5"))
		{
			printf("输入1. 退出 返回到登录界面！\n");
			printf("输入2. 查询 查询历史记录！\n");
			printf("输入3. 注销 注销当前用户并退回登录界面！\n");
			printf("输入4. 删除 删除历史记录！\n");
			printf("输入5. 查看帮助！\n");
			continue;
		}
		send(sockfd,buf,sizeof(buf),0);
		if(strcmp(buf,"1")==0)
			return -1;
		else if(strcmp(buf,"2")==0)
		{
			if(0==recv(sockfd,&count,sizeof(count),0))
				exit(-1);
			if(count==0)
			{
				printf("还没有查词记录!\n");
				continue;
			}
			for(i=0;i<(count/4);i++)
				for(j=0;j<4;j++)	
				{
					recv(sockfd,buf,sizeof(buf),0);
					if(0==i)
						continue;
					else if(0==j)
						printf("user:%s\n",buf);
					else if(1==j)
						printf("word:%s\n",buf);
					else if(2==j)
						printf("explain:%s\n",buf);
					else if(3==j)
						printf("time:%s\n\n",buf);

				}
			continue;
		}
		else if(!strcmp(buf,"3"))
			return -1;	
		else if(!strcmp(buf,"4"))
			continue;
		else	
		{
			recv(sockfd,buf,sizeof(buf),0);
			printf("%s\n",buf);
		}
	}
}
int main(int argc,char *argv[])
{
	int a;
	char buf[512];
	if(argc<3)
	{
		fprintf(stderr,"Usage:<%s> <ip> <port>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	int sockfd=TcpInit(argv[1],argv[2]);

	while(1)
	{
		a=GraphicInit(sockfd);
		if(a==1)
		{
			if(Login(sockfd)<0)
				continue;
			printf("process is here\n");
			if(SearchWorld(sockfd)<0)
				continue;
		}
		else if(a==2)
		{
				Register(sockfd);
					continue;
		}
		else if(a==3) 
			exit(0);	
		else
			continue;
	}
	return 0;
}
