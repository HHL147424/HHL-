#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
int Check_pram(const char*srcfile/*源文件*/,int PRONUM/*进程个数是否合法*/,int argc)//参数验证模块
{

	if(access(srcfile,F_OK)!=0)//文件是否存在验证
	{
		printf("源文件验证失败\n");
		return -1;
		}
		
	if(argc<3)//参数数量验证
	{
		printf("参数验证失败\n");
		return -1;
	}
	if(PRONUM<2)//进程个数验证
	{
		printf("进程个数验证失败\n");
		return -1;
		}
	return 0;
}

int Block_cur(const char*srcfile,int PRONUM)//任务切割，根据进程数量切片,返回切片大小
{
	struct stat st={};
	if(stat(srcfile,&st)==-1)
	{
		perror("获取文件属性失败");
		return -1;
	}
	printf("文件大小 %d\n",st.st_size);
	int Block_len=0;
	if(st.st_size%PRONUM)
		Block_len=(st.st_size/PRONUM+1);
	else
		Block_len=st.st_size/PRONUM;
	return Block_len;

}

int Process_create(const char*srcfile,const char*destfile,int PRONUM,int Block_len)//多进程创建
{
	int fd_src=open(srcfile,O_RDONLY);
	if(fd_src==-1)
	{
		printf("打开源文件失败\n");
		return -1;
	}
	int fd_dest=open(destfile,O_WRONLY|O_CREAT|O_TRUNC,0666);
	if(fd_dest==-1)
	{
		printf("打开目标文件失败\n");
		close(fd_src);
		return -1;
	}
	pid_t pid=0;
	for(int i=0;i<PRONUM;i++)//创建子进程，重载复制函数
	{
		pid=fork();
		if(pid==0)
		{
			int pos=i*Block_len;//起始地址//LINUX没有itoa,只有atoi
			char strpos[10]="";
			char strblock_len[10]="";
			sprintf(strpos,"%d",pos);
			sprintf(strblock_len,"%d",Block_len);
			execl("/home/colin/five/COPY","COPY",srcfile,destfile,strpos,strblock_len,NULL);
		}

	}
	return 0;


}

void Process_wait()
{
	int pid=0;
	while((pid=wait(NULL))!=-1)
	{
		printf("子进程PID %d\n",pid);
	}
}

int main(int argc,char*argv[])
{
	int pronum=0;
	if(argv[3]==0)//进程个数
	{
		pronum=2;
		}
	else
		pronum=atoi(argv[3]);
	if(Check_pram(argv[1],pronum,argc)==-1)//验证模块
	{
		printf("验证模块失败\n");
		return 0;
	}

	printf("进程数%d\n",pronum);

	int len=0;
	if((len=Block_cur(argv[1],pronum))==-1)
	{
		printf("任务切片失败");
	}

	printf("任务切片成功 大小：%d\n",len);

	if(Process_create(argv[1],argv[2],pronum,len)==-1)
	{
		printf("创建子进程并拷贝环节失败\n");
		return 0;
	}

	printf("创建子进程成功\n");

	Process_wait();

	return 0;

}
