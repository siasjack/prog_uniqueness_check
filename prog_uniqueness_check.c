

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_PATH_LEN	1024
#define MAX_PROG_NAME_LEN 32	
int tools_write_file(char *filename,char* open_mode,const char *data,int data_len)
{
	FILE *fd= NULL;
	if(!filename || !data || !data_len ||!open_mode ||
			!strlen(data) || !strlen(filename)){
		return -1;
	}

	fd = fopen(filename,open_mode);
	if(!fd){
		return -2;
	}

	fwrite(data, data_len, 1, fd);
	fclose(fd);
	printf("%s write [%s]in file [%s]\n",__FUNCTION__,data,filename);
	return 0;
}
int get_process_name (char * dest,int len)
{
	char process_path[MAX_PATH_LEN] = {0};
	char *process_name=NULL;

	if(!dest || !len){
		return -1;	
	}

	if(readlink("/proc/self/exe", process_path,sizeof(process_path)) <=0)
	{
		printf("read link err!!\n");
		return -1;
	}

	process_name = strrchr(process_path, '/');
	if(!process_name)
	{
		strncpy(dest,process_name,len);
	}
	else
	{
		strncpy(dest,++process_name,len);
	}
	return 0;
}

int tools_read_file(char *file_name,char **dest_str)
{
	FILE *f=NULL;
	long len=0;

	if(!dest_str || !file_name){
		return -1;
	}
	f=fopen(file_name,"rb");
	if(!f){
		return -2;
	}   

	fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	*dest_str = (char*)malloc(len+1);
	if(!(*dest_str) ){
		exit(-1);	
	}
	fread(*dest_str,1,len,f);
	fclose(f);
	return len;
}   

int pid_uniqueness_check()
{
	char prog_name[MAX_PROG_NAME_LEN]={0}, run_path[MAX_PATH_LEN]={0};
	char str_pid[16]={0};
	char *last_pid=NULL;
	int __pid = 0;
	get_process_name(prog_name,sizeof(prog_name));

	if(!strlen(prog_name)){
		return -1;
	}

	snprintf(run_path,sizeof(run_path),"/var/run/%s.pid",prog_name);
	
	tools_read_file(run_path,&last_pid);
	if(last_pid && strlen(last_pid)){
		__pid = atoi(last_pid);
		free(last_pid);
		return __pid;
	}else{
		return 0;	
	}

}

int save_pid_file()
{
	pid_t this_pid = getpid();
	char prog_name[MAX_PROG_NAME_LEN]={0}, run_path[MAX_PATH_LEN]={0};
	char str_pid[16]={0};
	printf("pid=%d\n",this_pid);
	get_process_name(prog_name,sizeof(prog_name));

	if(!strlen(prog_name)){
		return -1;
	}

	snprintf(run_path,sizeof(run_path),"/var/run/%s.pid",prog_name);
	
	snprintf(str_pid,sizeof(str_pid),"%d",this_pid);

	tools_write_file(run_path,"w",str_pid,strlen(str_pid));

	return (int)this_pid;

}

int check_last_pid_state(int pid)
{
	char  state_file[MAX_PATH_LEN] = {0};
	char *content=NULL;

	if(!pid){
		return -1;	
	}
	snprintf(state_file,sizeof(state_file),"/proc/%d/status",pid);
	
	tools_read_file(state_file,&content);
	if(!content){
		return 0;	
	}else{
		free(content);
		return 1;
	}

}

void check_uniqueness_pid_and_save()
{
	int last_pid = pid_uniqueness_check();
	if(0 == last_pid)
	{
		save_pid_file();
	}else{

		if(check_last_pid_state(last_pid) == 0){
			save_pid_file();
			return;
		}
		fprintf(stderr,"same programe is running,pid=%d,abort...\n",last_pid);
		exit(0);
	}

}

int main()
{
	char name[MAX_PATH_LEN]={0};
	pid_uniqueness_check();
	get_process_name(name,MAX_PATH_LEN);
	printf("name:%s\n",name);

	check_uniqueness_pid_and_save();
	while(1)
		sleep(100);
}
