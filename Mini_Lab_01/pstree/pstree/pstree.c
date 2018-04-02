#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_NUM_PROC 100000
#define MAX_NUM_CHILD 100
#define MAX_NUM_TASK 100
#define MAX_NUM_NAME 100

struct node
{
	int *child;
	int *task;
	int pid, ppid;
	int task_num, child_num;
};
struct node *graph = NULL;

int pid_search[MAX_NUM_PROC];
int curr;
char file_name[1024];
char buf[2048];
char name[100000][20];
int x[10];			//to store the different levels of number of spaces 
int x_level = 0;

void init() {
	graph = (struct node*)malloc(sizeof(struct node) * MAX_NUM_PROC);
	memset(pid_search, -1, sizeof(pid_search));
	for(int i = 0; i < MAX_NUM_PROC; i++) {
		graph[i].pid = graph[i].ppid = -1;
		graph[i].task_num = graph[i].child_num = 0;
		graph[i].child = (int *)malloc(sizeof(int)*MAX_NUM_CHILD);
		graph[i].task = (int *)malloc(sizeof(int)*MAX_NUM_TASK);
	}
	pid_search[0] = 0;
	graph[0].pid = 0;

}

bool num_check(char *name) {
	for (int i = 0; i < strlen(name); i++) 
		if(name[i] < '0' || name[i] > '9')
			return 0;
	return 1;
}

int read_ppid(int pid, char *Name) {
	sprintf(file_name, "/proc/%d/stat", pid);
	int fd = open(file_name, O_RDONLY);
	int readed_bytes = read(fd, buf, sizeof(buf));
	close(fd);
	int Pid, PPid;
	memset(Name, '\0', sizeof(Name));
	sscanf(buf, "%d%s%*s%d", &Pid, Name, &PPid);
	assert(Pid == pid);
		//test 2
	//printf("test2: name = %s, ppid = %d\n", Name, PPid);
	return PPid;
}

void get_task(int pid) {
	int temp_node_id = pid_search[pid];
	char dir_name[20];
	sprintf(dir_name, "/proc/%d/task", pid);
	DIR *d = opendir(dir_name);
	struct dirent *dir;
	while((dir=readdir(d)) != NULL) {
		//printf("test11: dir->name = %s\n", dir->d_name);
		if(num_check(dir->d_name)) {
			int task_id;
			sscanf(dir->d_name, "%d", &task_id);
			//printf("test10: task_id = %d\n", task_id);
			if(task_id != pid) {
				graph[temp_node_id].task[graph[temp_node_id].task_num] = task_id;
				graph[temp_node_id].task_num++;
				//printf("test9: graph[temp_node_id].task_num = %d, task_id = %d\n", graph[temp_node_id].task_num, task_id);

				sprintf(file_name, "/proc/%d/task/%d/status", pid, task_id);
				int fd = open(file_name, O_RDONLY);
				int readed_bytes = read(fd, buf, sizeof(buf));
				close(fd);
				int i = 0;
				while(buf[i + 6] != '\n') {
					name[task_id][i] = buf[i + 6];
					i++;
				}
				name[task_id][i] = '\0';
			}
		}
	}
}

void get_info_from_dir() {
	DIR *d;
	struct dirent *dir;
	d = opendir("/proc");
	while((dir = readdir(d))!=NULL) {
		if(num_check(dir->d_name)) {
			int pid;
			sscanf(dir->d_name, "%d", &pid);
			//		test1
			//printf("test1: pid = %d\n", pid);
			//
			char nname[MAX_NUM_NAME];
			int ppid = read_ppid(pid, nname);
			//continued...
			if(pid_search[pid] == -1) {
				graph[curr].pid = pid;
				graph[curr].ppid = ppid;
				pid_search[pid] = curr;
				curr++;
			}
			else
				graph[curr].ppid = ppid;
			strcpy(name[pid], nname);
			if(pid_search[ppid] == -1) {
				graph[curr].pid = ppid;
				pid_search[ppid] = curr;
				curr++;
			}
			int cur_node_ppid = pid_search[ppid];
			//printf("test5: name = %s, pid = %d\n", name[pid], pid);
			//printf("test15: child_num = %d\n", graph[cur_node_ppid].child_num);
			graph[cur_node_ppid].child[graph[cur_node_ppid].child_num] = pid;
			graph[cur_node_ppid].child_num++;
			get_task(pid);
		}

	}
	closedir(d);
	//printf("test3: get_info_from_dir\n");
}

void print_spaces() {
	for(int j = 0; j < x_level; j++) {
		for(int i = 0; i < x[j]; i++) {
			printf(" ");
		}
	}
}

void print_task(int id) {
	printf("%s(%d)", name[id], id);
}

void free_graph() {
	free(graph);
}

void print_progress(int id) {
	if(id == 2)
		return;
	int temp_node_id = pid_search[id];
		//space_dif_num
	char print_info[40];
	if(graph[temp_node_id].child_num + graph[temp_node_id].task_num > 0) {
		sprintf(print_info, "%s(%d)", name[graph[temp_node_id].pid], id);
		printf("%s", print_info);
	}

	x[x_level] += strlen(print_info) + 3;
	x_level++;
	//printf("test6: spaces = %d\n", spaces);

	assert(temp_node_id != -1);
	int first = 1;
	//printf("test8: task_num = %d\n", graph[temp_node_id].task_num);
	for(int i = 0; i < graph[temp_node_id].task_num; i++) {
		//printf("test12: true\n");
		if(i == 0) {
			printf("   ");
			print_task(graph[temp_node_id].task[i]);
			//printf("test7: i = %d, .task[i] = %d\n", i, graph[temp_node_id].task[i]);
			first = 0;
		}
		else {
			print_spaces();
			print_task(graph[temp_node_id].task[i]);
		}
		printf("\n");

		//if((i == graph[temp_node_id].task_num - 1) && (graph[temp_node_id].child_num == 0)) {
		//	print_spaces();
		//	printf("\n");
		//}
	}
	//printf("test13: temp_node_id = %d, child_num = %d\n", temp_node_id, graph[temp_node_id].child_num);
	//for(int i = 0; i < graph[temp_node_id].child_num; i++)
	//printf("test14: i = %d, child[i] = %d\n", i, graph[temp_node_id].child[i]);
	for(int i = 1; i < graph[temp_node_id].child_num; i++) {
		if(i == 1 && first) {
			printf("   ");
			print_progress(graph[temp_node_id].child[i]);
		//else if(i == 0 && !first) {
		//	print_spaces(spaces, re_x, space_dif_num);
		//	print_progress(graph[temp_node_id].child[i], spaces, re_x, re_sdn);
		//}
		}
		else {
			print_spaces();
			print_progress(graph[temp_node_id].child[i]);
				printf("\n");
		}
		int temp = pid_search[graph[temp_node_id].child[i]];
		if(graph[temp].child_num + graph[temp].task_num == 0)
			printf("\n");
		if(i == graph[temp_node_id].child_num - 1) {
			print_spaces();
			printf("\n");
		}
	}
	x_level--;
	x[x_level] = 0;	
}

int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // specification
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // specification

  //addtion part
  memset(x, 0, sizeof(x));
  init();
  get_info_from_dir();
  //printf("test4: curr = %d\n", curr);
  //for(int i=0; i<curr; i++)
  	//printf("i = %d, graph[i].pid = %d, graph[i].ppid = %d\n", i, graph[i].pid, graph[i].ppid);

  print_progress(1);
  free_graph();

  return 0;
}
