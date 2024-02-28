#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define _XOPEN_SOURCE 700
#define CMDLINE_PATH_MAXSIZE 81
#define MAX_STAT_LEN 128
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
	char state;
	/* unsigned int usertime, systemtime;  */
	long int rss;
	long int vsz;
	int flag = 0;
	//char tty[8];
	
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

	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/status", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja status fajla za proces: %s\n", pid);
		return;
	}
	while(fgets(line, CMDLINE_PATH_MAXSIZE, cmdline_file) != NULL){
		//Pronalazenje statusa procesa
		if(sscanf(line, "State:\t%c", &state) == 1){
			flag++;
		}
		//Pronalazenje rss procesa
		if(sscanf(line, "VmRSS: %li KB\n", &rss) == 1){
			flag++;
		}
		//Pronalazenje vsz procesa
		if(sscanf(line, "VmSize: %li KB\n", &vsz) == 1){
			flag++;
		}
		if(flag == 3){
			break;
		}
	}
	fclose(cmdline_file);
	//Skracivanje na sestocifreni broj radi lepseg prikaza
	while(vsz > 999999){
		vsz /= 10;
	}
	//Citanje START vremena
	snprintf(cmdline_path, CMDLINE_PATH_MAXSIZE, "/proc/%s/stat", pid);
	cmdline_file = fopen(cmdline_path, "r");
	if(cmdline_file == NULL){
		fprintf(stderr, "Greska prilikom otvaranja stat fajla za proces: %s\n", pid);
		return;
	}
	char buffer[MAX_STAT_LEN];
	fgets(buffer, MAX_STAT_LEN, cmdline_file);
	fclose(cmdline_file);

	char *token = strtok(buffer, " ");
	char utime[15];
	char stime[15];
    int i = 1;
    while (token != NULL) {
        if (i == 14) {
        	strcpy(utime,token);
        }else if (i == 15) {
        	strcpy(stime, token);
            break;
        }
        token = strtok(NULL, " ");
        i++;
    }
	long int usertime = atoi(utime);
	long int systime = atoi(stime);
	int collective = usertime + systime;
	int minute = 0;
	int hour = 0;
	while(collective >= 60){
		minute++;
		collective = collective - 60;
	}
	while(minute >= 60){
		hour++;
		minute = minute - 60;
	}
	//Uzimanje vremena i poredjenje sa trenutnim danom
	time_t currentTime = time(NULL);
	struct tm tmCurr = *localtime(&currentTime);
	struct stat attr;
	stat(cmdline_path, &attr);
	struct tm tmProc = *localtime(&(attr.st_ctime));
	char time[10];
	strftime(time, 10, "%H:%M", localtime(&(attr.st_ctime)));

	//Pravljenje %CPU sekcije
	unsigned long long int starttime = atoi(time);
	struct timespec current;
	clock_gettime(CLOCK_MONOTONIC, &current);
	unsigned long long currentSeconds = current.tv_sec;
	unsigned long long elapsedTime = currentSeconds - (starttime / sysconf(_SC_CLK_TCK));
	float cpuUtilization = ((usertime + systime)*100.0)/(elapsedTime * sysconf(_SC_CLK_TCK));
	
	//Pravljenje %MEM sekcije
	long int pages = sysconf(_SC_PHYS_PAGES);
	long int pageSize = sysconf(_SC_PAGE_SIZE);
	long int phsyicalMem = pages * pageSize;
	float memUtilization = ((float)rss*1000.0 / (float)phsyicalMem) * 100.0;
	if(tmCurr.tm_mday > tmProc.tm_mday){
		printf("%-8s %5s %.1f %.1lf %6li %6li    ?     %3c %s %i.%i %i:%i %-3s\n", user, pid, cpuUtilization, memUtilization, vsz, rss, state, tmProc.tm_mday, tmProc.tm_mon, hour, minute, cmdline);
		return;
	}
	printf("%-8s %5s %.1f %.1lf %6li %6li    ?     %3c %s %i:%i  %-3s\n", user, pid, cpuUtilization, memUtilization, vsz, rss, state, time, hour, minute, cmdline);
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
	printf("USER\tPID   %%CPU %%MEM   VSZ     RSS   TTY  STAT  START TIME\tCOMMAND\n");
	list_processes();
	return 0;
}
