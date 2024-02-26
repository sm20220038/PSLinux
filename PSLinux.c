#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#define CMDLINE_PATH_MAXSIZE 1024
//Function to check if a string contains only digits
int is_number(const char *str) {
    while (*str) {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

//Function to print process information
void print_process_info(const char *pid){
	char cmdline_path[CMDLINE_PATH_MAXSIZE];
	FILE *cmdline_file;
	char cmdline[CMDLINE_PATH_MAXSIZE];

	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/cmdline", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Error while opening cmdline file for process %s\n", pid);
	}

	if(fgets(cmdline, CMDLINE_PATH_MAXSIZE, cmdline_file) != NULL){
		//Remove trailing newline characters
		cmdline[strcspn(cmdline, "\n")] = '\0';
		printf("%s\t%s\n", pid, cmdline);
	}

	fclose(cmdline_file);
}

void list_processes(){
	DIR *proc_dir;
	struct dirent *entry;
	//Opens /proc directory
	proc_dir = opendir("/proc");
	if(proc_dir == NULL){
		perror("Error while opening /proc directory\n");
		return;
	}
	//Read entries from /proc directory
	while((entry = readdir(proc_dir)) != NULL){
		//Checks if the entry is a directory and if its name consist only of a digits
		if(entry->d_type == DT_DIR && is_number(entry->d_name)){
			print_process_info(entry->d_name);
		}
	}

	closedir(proc_dir);
}

int main(int argc, char *argv[]){
	printf("PID\tCOMMAND\n");
	list_processes();	
	return 0;
}
