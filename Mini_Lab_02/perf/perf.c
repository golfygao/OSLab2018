#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <regex.h>
#define MAX_STR_SIZE 20

FILE * fdopen(int fildes, const char * mode);
int system(const char *command);
int pipe(int filedes[2]);
struct sysinfo timeinfo;
char call_name[100][30];
double call_time[100];
double total_time;
double calltime_perc[100];
int call_kind = 0;
int fd[2];
void init_record() {
	memset(call_name, '\0', sizeof(call_name));
	memset(call_time, 0.0, sizeof(call_time));
	memset(calltime_perc, 0.0, sizeof(calltime_perc));
	total_time = 0.0;
	call_kind = 0;
}

void calculate() {
	char c[500];
	char call_name_temp[MAX_STR_SIZE];
	double time_temp = 0.0;
	char call_time_temp[20];
	memset(c, '\0', sizeof(c));
	memset(call_name_temp, '\0', sizeof(call_name_temp));
	FILE *instream = fdopen(fd[0], "r");
	assert(instream!=NULL);
	fgets(c, 500, instream);
	//printf("line_readed,content:\n%s\n",&c[0]);
	sscanf(c, "%[0-9|a-z|A-Z]", call_name_temp);
	//sscanf(c, "%*[0-9|A-Z|a-z]%*^[=]= %*[0-9]<%lf>", &time_temp);
	int str_len = strlen(c);
	int a;
	for(int i = str_len - 1; ;i--) {
		if(c[i] != '<')
			continue;
		else {
			sscanf(&c[i], "%s", call_time_temp);
			a = i;
			break;
		}
	}
	//printf("i = %d\n", a);
	//printf("call_name_temp: %s, time_temp: %s  ", call_name_temp, call_time_temp);
	sscanf(call_time_temp, "<%lf>", &time_temp);
	//printf("time_temp = %lf\n", time_temp);
	total_time += time_temp;
	int i;
	for(i = 0; i <= call_kind; i++) {
		if(i == call_kind) {
			call_kind++;
			strcpy(call_name[i], call_name_temp);
			call_time[i] += time_temp;
			break;
		}
		else {
			//printf("%d attention\n", strcmp(call_name[i], call_name_temp));
			if(strcmp(call_name[i], call_name_temp)) {
				
				continue;
			}
			else {
				call_time[i] += time_temp;
				break;
			}
		}
	}
	//printf("i = %d, call_name = %s, call_time = %lf\n", i, call_name[i], call_time[i]);
	
	return;
}

int main(int argc, char *argv[], char *envp[]) {
  	for (int i = 0; i < argc; i++) {
    	assert(argv[i]); // specification
		printf("argv[%d] = %s\n", i, argv[i]);
 	}
  	assert(!argv[argc]); // specification

  	//regex_t reg;
  	//const char * pattern1 = "^\\w+";
  	//const char * pattern2 = "<[0-9|.]+>$";

  	int result = -1;
  	int *write_fd = &fd[1];
  	int *read_fd = &fd[0];

  	//char command[MAX_STR_SIZE];
  	//memset(command, '\0', sizeof(command));
  	//strcpy(command, "strace -T ");
  	//strcat(command, argv[1]);
  	//printf("%s+%s\n", command, command_2);
  	result = pipe(fd);
  	if(result != 0) {
  		printf("fail to create pipe\n");
  		return -1;
  	}
  	//printf("test1\n");
  	
  	pid_t pid;
  	pid = fork();
  	if(pid == 0) {
  		char add_argv[3][20] = {">", "/dev/null"};
  		char * new_argv[] = {"/usr/bin/strace", "-T", argv[1], ">/dev/null", NULL};
  		/*
  		new_argv[0] = "/usr/bin/strace";
  		new_argv[1] = add_argv[0];
  		//new_argv[1] = "-T";
  		new_argv[argc + 1] = add_argv[1];
  		printf("argc = %d\n", argc);
  		for(int i = 1; i < argc; i++)
  		//	new_argv[i + 1] = argv[i];
  			new_argv[i] = argv[i];
  		new_argv[argc] = add_argv[2];
  		//new_argv[argc + 2] = NULL;
  		printf("test2\n");
*/
  		//*new_argv = {"usr/bin/strace", argv[1], ">", "/dev/null"};
  		//new_argv[0] = "usr/bin/strace";
  		//new_argv[1] = argv[1];
  		//new_argv[2] = ">";
  		//new_argv[3] = "/dev/null";
	//	for (int i = 0; i < argc + 3; i++) {
    	//	assert(new_argv[i]); // specification
	//		printf("new_argv[%d] = %s\n", i, new_argv[i]);
 		
  		//assert(!argv[argc]); // specification

 	//	printf("test3\n");
 		close(fd[0]);
  		dup2(fd[1], 2);

  	//	printf("test4\n");
		execve("/usr/bin/strace", new_argv, envp);
		assert(0);

		perror("/usr/bin/strace");
		//???????什么情况？？
	}
	else {
	//	printf("test6\n");
		init_record();
		close(fd[1]);
		int *status = NULL;
	//	int old = dup(STDIN_FILENO);
		dup2(fd[0], 0);
	//	printf("fd[0]=%d\n",fd[0]);
		//printf
		while(waitpid(pid, status, WNOHANG)==0) {
			clock_t prev = clock();
			calculate();
			if(clock() - prev > 500) {
				prev = clock();
				for(int j = 0; j < call_kind; j++)
					printf("%s: %lf%%\n", call_name[j], call_time[j]/total_time*100);
				printf("total_time: %lf\n", total_time);
				printf("---------------------------------\n");
			}
		}
				for(int j = 0; j < call_kind; j++)
					printf("%s: %lf%%\n", call_name[j], call_time[j]/total_time*100);
				printf("total_time: %lf\n", total_time);
				printf("---------------------------------\n");
	}
  	return 0;
}