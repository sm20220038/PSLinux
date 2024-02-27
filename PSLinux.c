#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>

#define CMDLINE_PATH_MAXSIZE 100

//Funkcija koja proverava da li string sadrzi samo brojeve
int is_number(const char *str) {
    while (*str) {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}


//Funkcija koja ispisuje informacije o procesu
void print_process_info(const char *pid){
	char cmdline_path[CMDLINE_PATH_MAXSIZE];
	FILE *cmdline_file;
	char cmdline[CMDLINE_PATH_MAXSIZE], line[CMDLINE_PATH_MAXSIZE];
	char user[64];
	unsigned int pid_val;
	char state;
	unsigned int usertime, systemtime;
	long int rss;
	long int vsz;
	char tty[8];

	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/cmdline", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja cmdline fajla za proces: %s\n", pid);
		return;
	}
	if(fgets(cmdline, CMDLINE_PATH_MAXSIZE, cmdline_file) != NULL){
		//Brisanje pratecih karaktera za novi red
		cmdline[strcspn(cmdline, "\n")] = '\0';
	}
	fclose(cmdline_file);

	//Pronalazenje username vlasnika procesa
	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/status", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja status fajla za proces: %s\n", pid);
		return;
	}
	while(fgets(cmdline_path, CMDLINE_PATH_MAXSIZE, cmdline_file)){
		if(strncmp(cmdline_path, "Uid:", 4) == 0){
			__uid_t uid;
			sscanf(cmdline_path, "%*s %u", &uid);
			struct passwd *pw = getpwuid(uid);
			if(pw){
				strncpy(user, pw->pw_name, 64);
				user[8] = '\0'; //Osiguravamo da je username null-terminated i da je duzine 9 karaktera
			}
			break;
		}
	}
	fclose(cmdline_file);
	//Pronalazenje statusa procesa
	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/status", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja status fajla za proces: %s\n", pid);
		return;
	}
	while(fgets(line, CMDLINE_PATH_MAXSIZE, cmdline_file) != NULL){
		if(sscanf(line, "State:\t%c", &state) == 1){
			break;
		}
	}
	fclose(cmdline_file);
	//Pronalazenje rss procesa
	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/status", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja status fajla za proces: %s\n", pid);
		return;
	}
	while(fgets(line, CMDLINE_PATH_MAXSIZE, cmdline_file) != NULL){
		if(sscanf(line, "VmRSS: %li KB\n", &rss) == 1){
			break;
		}
	}
	fclose(cmdline_file);
	//Pronalazenje vsz procesa
	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/status", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja status fajla za proces: %s\n", pid);
		return;
	}
	while(fgets(line, CMDLINE_PATH_MAXSIZE, cmdline_file) != NULL){
		if(sscanf(line, "VmSize: %li KB\n", &vsz) == 1){
			break;
		}
	}
	while(vsz > 999999){
		vsz /= 10;
	}
	fclose(cmdline_file);
	printf("%-8s %5s %6li %6li    ?     %3c   %-3s\n", user, pid, vsz, rss, state, cmdline);
}

void list_processes(){
	DIR *proc_dir;
	struct dirent *entry;
	//Otvara /proc direktorijum
	proc_dir = opendir("/proc");
	if(proc_dir == NULL){
		perror("Error while opening /proc directory\n");
		return;
	}
	//Cita sadrzaj iz /proc direktorijuma
	while((entry = readdir(proc_dir)) != NULL){
		//Proverava da li je sadrzaj direktorijum i da li se njegovo ime sastoji samo od brojeva
		if(entry->d_type == DT_DIR && is_number(entry->d_name)){
			print_process_info(entry->d_name);
		}
	}
	closedir(proc_dir);
}

int main(int argc, char *argv[]){
	if(argv[1] != NULL && !strcmp(argv[1], "--help")){
		printf(" main [options]\n\n Try 'main --help <simple|list|output|threads|misc|all>'\n  or 'main --help <s|l|o|t|m|a>'\n for additional help text.\n");
		return 0;
	}
	//printf("Prvi argument: %s, Drugi argument: %s, i broj argumenata je:%d\n", argv[0],argv[1], argc);
	printf("USER\tPID\tVSZ\tRSS\tTTY\tSTAT\tCOMMAND\n");
	list_processes();
	return 0;
}
