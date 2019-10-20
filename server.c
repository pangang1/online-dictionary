#include"dict.h"

int TcpInit(char *s)
{
	int on=1;
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		perror("socket");
		exit(-1);	
	}
	
	struct sockaddr_in seraddr={
		.sin_family=AF_INET,
		.sin_port=htons(atoi(s)),
		.sin_addr.s_addr=htonl(INADDR_ANY)	
	};

	if (0 > setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) 
	{
		perror("setsockopt");
		return -1;
	}
	if(0>bind(sockfd,(struct sockaddr*)&seraddr,sizeof(seraddr)))
	{
		perror("bind");
		exit(-1);	
	}

	if(0>listen(sockfd,1024))
	{
		perror("listen");	
		exit(-1);
	}
	printf("listen.........\n");
	return sockfd;

}
int Login(sqlite3 *db,int connfd,char *name)
{
	char buf[512],answer='N';
	char sql[512];
	int nrow,ncolumn;
	char *errmsg=NULL;
	char **resultp=NULL;
	recv(connfd,buf,sizeof(buf),0);
	sprintf(sql,"select * from usr where name='%s';",buf);
	sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg);
	
	if(nrow==0)
	{
		send(connfd,&answer,sizeof(answer),0);
		return -1;
	}
	else
	{
		answer='Y';
		send(connfd,&answer,sizeof(answer),0);
	}
	strcpy(name,buf);
	recv(connfd,buf,sizeof(buf),0);
	if(strcmp(buf,resultp[3])==0)
		send(connfd,&answer,sizeof(answer),0);
	else 
	{
		answer='N';
		send(connfd,&answer,sizeof(answer),0);
		return -1;
	}
	return 0;
}
void SearchHistory(sqlite3 *db,int connfd,char *name)
{
	char buf1[512],sql[1024],**resultp,*errmsg;
	int nrow,ncolumn,i,j,count;

	sprintf(sql,"select * from history where name='%s';",name);
	sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg);
	count=(nrow+1)*(ncolumn);
	send(connfd,&count,sizeof(count),0);
	if(count==4)
		return ;
	count=0;
	for(i=0;i<nrow+1;i++)
		for(j=0;j<ncolumn;j++)
		{
			strcpy(buf1,resultp[count++]);
			send(connfd,buf1,sizeof(buf1),0);
		}
	return;
}
void SearchWord(sqlite3 *db,int connfd,char *name,char* buf1,char *buf2)
{
	char sql[1024],**resultp,*errmsg;
	int nrow,ncolumn;
	sprintf(sql,"select * from word where word='%s';",buf1);
	if(0!=sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg))
	{
		printf("test SearchWord:%s\n",errmsg);
		exit(-1);
	}
	if(0==nrow)
	{
		strcpy(buf2,"没有此单词!");
		send(connfd,buf2,512,0);	
	}
	else
	{
		strcpy(buf2,resultp[3]);
		send(connfd,buf2,512,0);
	}
}
void InsertHistory(sqlite3 *db,int connfd,char *name,char *buf1,char *buf2)
{
	char t1[64]={0},sql[1024],**resultp,*errmsg;
	int nrow,ncolumn;

	time_t t;
	time(&t);
	strcpy(t1,ctime(&t));
	t1[strlen(t1)-1]='\0';
	sprintf(sql,"insert into history values('%s','%s','%s','%s');",name,buf1,buf2,t1);
	if(0!=sqlite3_get_table(db,sql,&resultp,NULL,NULL,&errmsg))
	{
		fprintf(stderr,"insert history:",errmsg);
		exit(-1);
	}
	return;
}
void Search(sqlite3 *db,int connfd,char *name)
{
	char buf2[512],buf1[512],sql[1024],**resultp,*errmsg;//曾经在这里定
	//义指针变量s来代替buf2数组传参，结果出现很奇怪的错误。因为buf2数组
	//buf2指针是一片连续的空间，而s后面没有空间了。
	//还有一次将sql设置为
	//128字节然后发觉有些单词能行有些不行。
	//才发觉有些单词解释的长度加上单词时间等字节超出了sql的范围；
	//导致溢出覆盖了connfd；
	//查询了百度才知道这是类似与gets不安全的原因。
	int nrow,ncolumn;
	while(1)
	{

		if(0>=recv(connfd,buf1,sizeof(buf1),0))
			exit(-1);
		if(strcmp(buf1,"1")==0)
		{
			return ;
		}
		else if(strcmp(buf1,"2")==0)
		{
			SearchHistory(db,connfd,name);
			continue;
		}	
		else if(!strcmp(buf1,"3"))
		{
			sprintf(sql,"delete from history where name='%s';",name);	
			if(0!=sqlite3_exec(db,sql,0,0,&errmsg))
			{
				printf("%s\n",errmsg);
				exit(-1);
			}
			sprintf(sql,"delete from usr where name='%s';",name);	
			if(0!=sqlite3_exec(db,sql,0,0,&errmsg))
			{
				printf("%s\n",errmsg);
				exit(-1);

			}
			return ;	
		}
		else if(!strcmp(buf1,"4"))
		{
			sprintf(sql,"delete from history where name='%s';",name);	
			if(0!=sqlite3_exec(db,sql,0,0,&errmsg))
			{
				printf("sqlite3_get_table:%s\n",errmsg);
				exit(-1);
			}
			continue;
		}
		else
		{
			SearchWord(db,connfd,name,buf1,buf2);
			InsertHistory(db,connfd,name,buf1,buf2);
		}
	}
}
void Register(sqlite3 *db,int connfd)
{
	char answer='N';
	char *errmsg,**resultp;
	int nrow,ncolumn;	
	char buf1[512],sql[1024],buf2[512];
	if(0>=recv(connfd,buf1,sizeof(buf1),0))
		exit(0);	
	if(0>=recv(connfd,buf2,sizeof(buf2),0))
		exit(0);
	printf("%s,%s\n",buf1,buf2);
	sprintf(sql,"insert into usr values('%s','%s');",buf1,buf2);
	if(0!=sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg))
	{
		send(connfd,&answer,sizeof(answer),0);
		return;
	}
	else
	{
		answer='Y';
		send(connfd,&answer,sizeof(answer),0);
		return;
	}
}
void hander(int s)
{
	while(waitpid(-1,NULL,WNOHANG)>0);
}
int main(int argc,char *argv[])
{
	char name[64];
	pid_t pid;
	if(argc<2)
	{
		fprintf(stderr,"Usage:<%s><port>\n",argv[0]);	
		exit(-1);
	}
	signal(SIGCHLD,hander);
	int sockfd=TcpInit(argv[1]);
	struct sockaddr_in cliaddr;
	int addrlen,i;	
	int connfd;
	sqlite3 *db;
	sqlite3_open("dict.db",&db);
	while(1)
	{
		connfd=accept(sockfd,NULL,NULL);
		if(connfd<0)
		{
			perror("accept");
			continue;	
		}
		
		if((pid=fork())==0)	
		{
			while(1)
			{
				i=0;//保证每一次i的值都在else范围；
				if(0==recv(connfd,&i,sizeof(i),0))
					exit(0);	
				if(i==1)
				{
					if(Login(db,connfd,name)<0)
						continue;
					Search(db,connfd,name);
						continue;
				}
				else if(i==2)
				{
					Register(db,connfd);	
					continue;
				}
				else
					exit(0);
			}	
		}
	}
	return 0;
}
