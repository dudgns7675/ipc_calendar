#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include "calHeader.h"

int filecheck(char* filename){
	DIR *dir;
	struct dirent *ent;

	//directory check
	dir = opendir("./");
	if(dir != NULL){
		while((ent = readdir(dir)) != NULL){
			if(!strcmp(ent->d_name, "calendars"))
				break;
		}
		if(ent == NULL)
			system("mkdir ./calendars");
	}
	closedir(dir);

	//file check
	dir = opendir("./calendars/");
	if (dir != NULL){
		while ((ent = readdir(dir)) != NULL){
			if(!strcmp(ent->d_name, filename)){
				closedir(dir);
				return 1;
			}
		}
		closedir(dir);
		return -1;
	}
	else{
		printf("디렉토리  열기  실패!");
		return -1;
	}
}

int sf1_cal_manage(char* filename, int funcmode){
	int fd;
	char filepath[100] = {'\0',};
	char command[100] = {'\0',};
	char *tp;

	tp = filepath;
	strcat(tp, "./calendars/");
	strcat(tp, filename);

	tp = command;
	strcat(tp, "rm ");
	strcat(tp, filepath);

	//mode 1 is create mode
	if(funcmode == 1){
		if(filecheck(filename) != 1){
			fd = creat(filepath, 0666);
			printf("캘린더가  생성되었습니다.\n");
			close(fd);
			return 1;
		}
		else{
			printf("해당하는 캘린더가 이미 존재합니다.\n");
			return -1;
		}
	}
	//mode 2 is delete mode
	else if(funcmode == 2){
		if(filecheck(filename) == 1){
			system(command);
			printf("캘린더가 삭제되었습니다.\n");
			return 1;
		}
		else{
			printf("해당하는 캘린더가 존재하지 않습니다.\n");
			return -1;
		}
	}
}

int sf2_save_cal(t_caldata caldata){
	int i;
	char buf[BUFF_SIZE] = {'\0',};
	FILE *fp;
	//FILE *ftemp;
	//char command[100] = {'\0',};
	char filepath[100] = {'\0',};
	//char temppath[100] = {'\0',};
	char *tp = filepath;
	if(filecheck(caldata.calName) != 1){
		printf("요청받은 캘린더가 존재하지 않습니다!\n");
		return -1;
	}
	else{
		strcat(tp, "./calendars/");
		strcat(tp, caldata.calName);
	}

	if(caldata.flag == 1){
		printf("flag1 진입\n");
		fp = fopen(filepath, "r+");
		if(fp == NULL){
			printf("파일 오픈 실패!");
			exit(1);
		}
		while(!feof(fp)){
			fgets(buf, sizeof(buf), fp);
			if(atoi(buf) == caldata.date){
				for(i=0; i<MEMO_NUMS; i++){
					fgets(buf, sizeof(buf), fp);
					if(atoi(buf) == caldata.memoNum){
						fprintf(fp, "%s\n", caldata.memo[i]);
						printf("캘린더를 수정했습니다.\n");
						fclose(fp);
						return 1;
					}
					fgets(buf, sizeof(buf), fp);
				}
			}
		}
		fprintf(fp, "%d\n", caldata.date);
		for(i=0; i<MEMO_NUMS; i++){
			fprintf(fp, "%d\n", i+1);
			if((i+1) == caldata.memoNum){
				fprintf(fp, "%s\n", caldata.memo[i]);
			}
			else
				fputc('\n', fp);
		}
		printf("캘린더를 수정했습니다.\n");
		fclose(fp);
		return 1;
	}/*
	else if(caldata.flag == 2){
		printf("flag2 진입\n");
		fp = fopen(filepath, "r+");
		if(fp == NULL){
			printf("파일 오픈 실패!");
			exit(1);
		}
		tp = temppath;
		strcat(tp, "./calendars/temp");
		ftemp = fopen(temppath, "w");
		while(!feof(fp)){
			fgets(buf, sizeof(buf), fp);
			if(atoi(buf) == caldata.date){
				for(i=0; i<MEMO_NUMS; i++){
					fgets(buf, sizeof(buf), fp);
					fgets(buf, sizeof(buf), fp);
				}
			}
			fputs(buf, ftemp);
		}
		tp = command;
		strcat(tp, "rm ");
		strcat(tp, filepath);
		system(command);
		tp[0] = '\0';
		strcat(tp, "mv ");
		strcat(tp, temppath);
		strcat(tp, " ");
		strcat(tp, filepath);
		system(command);
		
		fclose(fp);
		fclose(ftemp);
		printf("메모가 삭제되었습니다.\n");
		return 1;
	}*/
}

int sf4_see_cal(char* calName, int calDate){
	int i;
	char buf[BUFF_SIZE];
	char filepath[100] = {'\0',};
	char *tp = filepath;
	FILE *fp;
	strcat(tp, "./calendars/");
	strcat(tp, calName);

	if(filecheck(calName) == 1){
		fp = fopen(filepath, "rt");
		while(!feof(fp)){
			fgets(buf, BUFF_SIZE, fp);
			if(atoi(buf) == calDate){
				printf("%d의 메모\n", calDate);
				for(i=0; i<MEMO_NUMS; i++){
					fgets(buf, BUFF_SIZE, fp);
					fgets(buf, BUFF_SIZE, fp);
					printf("%d. %s", i+1, buf);
				}
				fclose(fp);
				return 1;
			}
		}
		printf("지정한 날짜의 메모가 존재하지 않습니다.\n");
		fclose(fp);
		return 1;
	}
	else{
		printf("캘린더가 존재하지 않습니다!\n");
		return -1;
	}
}

void main(){
	int i;
	int msqid;
	t_caldata caldata;
	t_caldata rcvdata;
	
	if(-1 == (msqid = msgget((key_t)KEY, IPC_CREAT|0666))){
		printf("큐 생성 실패!\n");
		exit(1);
	}
	
	rcvdata.flag = 0;
	while(1){
		printf("...\n");
		sleep(1);
		msgrcv(msqid, &rcvdata, sizeof(t_caldata)-sizeof(long), 1, 0);
		if(rcvdata.flag == 1){
			printf("flag1 catch!\n");
			sf2_save_cal(rcvdata);
			rcvdata.flag = 0;
		}
	//	msgrcv(msqid, &rcvdata, sizeof(t_caldata)-sizeof(long), 2, IPC_NOWAIT);
	//	if(rcvdata.flag == 2){
	//		printf("flag2 catch!\n");
	//		sf2_save_cal(rcvdata);
	//		rcvdata.flag = 0;
	//	}
	}
}
