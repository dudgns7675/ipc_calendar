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

int sf1_cal_manage(){
	int fd;
	int funcmode;
	char filename[40] = {'\0',};
	char filepath[100] = {'\0',};
	char command[100] = {'\0',};
	char *tp;

	printf("캘린더를 관리합니다.\n");
	printf("캘린더를 생성합니까? 삭제합니까?(1:생성, 2:삭제) : ");
	scanf("%d", &funcmode);
	getchar();
	printf("캘린더 이름을 입력해 주세요 : ");
	scanf("%s", filename);

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
			close(fd);
			system("clear");
			printf("캘린더가  생성되었습니다.\n");
			return 1;
		}
		else{
			system("clear");
			printf("해당하는 캘린더가 이미 존재합니다.\n");
			return -1;
		}
	}
	//mode 2 is delete mode
	else if(funcmode == 2){
		if(filecheck(filename) == 1){
			system(command);
			system("clear");
			printf("캘린더가 삭제되었습니다.\n");
			return 1;
		}
		else{
			system("clear");
			printf("해당하는 캘린더가 존재하지 않습니다.\n");
			return -1;
		}
	}
}

int sf2_save_cal(t_caldata caldata){
	int i;
	int check = 0;
	char buf[BUFF_SIZE] = {'\0',};
	FILE *fp;
	FILE *ftemp;
	char command[100] = {'\0',};
	char filepath[100] = {'\0',};
	char temppath[100] = {'\0',};
	char *tp = filepath;
	if(filecheck(caldata.calName) != 1){
		return -1;
	}
	else{
		strcat(tp, "./calendars/");
		strcat(tp, caldata.calName);
	}

	if(caldata.flag == 1){
		fp = fopen(filepath, "r+t");
		if(fp == NULL){
			printf("\n파일 오픈 실패!\n");
			return -1;
		}
		tp = temppath;
		strcat(tp, "./calendars/temp");
		ftemp = fopen(temppath, "wt");
		if(ftemp == NULL){
			printf("\n임시파일 오픈 실패!\n");
			return -1;
		}

		while(!feof(fp)){
			fgets(buf, sizeof(buf), fp);
			if(atoi(buf) == caldata.date){
				check = 1;
				fputs(buf, ftemp);
				for(i=0; i<MEMO_NUMS; i++){
					fgets(buf, sizeof(buf), fp);
					fputs(buf, ftemp);
					if(atoi(buf) == caldata.memoNum){
						fgets(buf, sizeof(buf), fp);
						fprintf(ftemp, "%s\n", caldata.memo[i]);
						continue;
					}
					fgets(buf, sizeof(buf), fp);
					fputs(buf, ftemp);
				}
			}
			fputs(buf, ftemp);
		}
		if(check != 1){
			fprintf(fp, "%d\n", caldata.date);
			for(i=0; i<MEMO_NUMS; i++){
				fprintf(fp, "%d\n", i+1);
				if((i+1) == caldata.memoNum){
					fprintf(fp, "%s\n", caldata.memo[i]);
				}
				else
					fputc('\n', fp);
			}
			tp = command;
			strcat(tp, "rm ");
			strcat(tp, temppath);
			system(command);
			fclose(fp);
			fclose(ftemp);
			return 1;
		}
		else{
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
			return 1;
		}
	}
	else if(caldata.flag == 2){
		fp = fopen(filepath, "r+t");
		if(fp == NULL){
			printf("\n파일 오픈 실패!\n");
			return -1;
		}
		tp = temppath;
		strcat(tp, "./calendars/temp");
		ftemp = fopen(temppath, "wt");
		if(ftemp == NULL){
			printf("\n임시파일 오픈 실패!\n");
			return -1;
		}
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
		return 1;
	}
}

int sf3_transfer_cal(t_caldata caldata, int msqid){
	t_caldata sndcal;

	int i;
	char buf[BUFF_SIZE] = {'\0',};
	FILE *fp;
	char filepath[100] = {'\0',};
	char *tp = filepath;
	sndcal.date = caldata.date;
	sndcal.flag = ((int)caldata.flag * caldata.pid);

	if(filecheck(caldata.calName) != 1){
		//file not exist
		sndcal.memoNum = -1;
		if(-1 == msgsnd(msqid, &sndcal, sizeof(t_caldata)-sizeof(long), 0)){
				return -1;
			}
		return 1;
	}
	else{
		strcat(tp, "./calendars/");
		strcat(tp, caldata.calName);
	}

	//init
	tp = sndcal.memo[0];
	for(i=0; i<sizeof(sndcal.memo); i++)
		*(tp+i) = '\0';
	
	fp = fopen(filepath, "rt");
	while(!feof(fp)){
		fgets(buf, BUFF_SIZE, fp);
		if(atoi(buf) == caldata.date){
			for(i=0; i<MEMO_NUMS; i++){
				fgets(buf, BUFF_SIZE, fp);
				fgets(buf, BUFF_SIZE, fp);
				tp = sndcal.memo[i];
				strcat(tp, buf);
			}
			fclose(fp);
			if(-1 == msgsnd(msqid, &sndcal, sizeof(t_caldata)-sizeof(long), 0)){
				return -1;
			}
			return 1;
		}
	}
	//memo not exist
	sndcal.memoNum = -2;
	if(-1 == msgsnd(msqid, &sndcal, sizeof(t_caldata)-sizeof(long), 0)){
		return 1;
	}

	fclose(fp);
	return 1;
}

int sf4_see_cal(){
	int i;
	int calDate;
	char calName[40] = {'\0',};
	char buf[BUFF_SIZE];
	char filepath[100] = {'\0',};
	char *tp = filepath;
	FILE *fp;

	printf("캘린더를 조회합니다.\n");
	printf("조회할 캘린더 이름을 입력해 주세요 : ");
	scanf("%s", calName);
	printf("조회할 날짜를 입력해 주세요 : ");
	scanf("%d", &calDate);
	getchar();

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
				printf("엔터를 누르면 메인 화면으로 돌아갑니다.\n");
				getchar();
				system("clear");
				return 1;
			}
		}
		system("clear");
		printf("지정한 날짜의 메모가 존재하지 않습니다.\n");
		fclose(fp);
		return 1;
	}
	else{
		system("clear");
		printf("캘린더가 존재하지 않습니다!\n");
		return -1;
	}
}

void *catch_thread(void *rcv_msqid){
	t_caldata rcvdata;
	int check = 0;
	int msqid = *((int*)rcv_msqid);
	
	rcvdata.flag = 0;
	while(1){
		sleep(1);
		msgrcv(msqid, &rcvdata, sizeof(t_caldata)-sizeof(long), 1, IPC_NOWAIT);
		if(rcvdata.flag == 1){
			if(-1 == (check = sf2_save_cal(rcvdata)))
				printf("\n쓰레드에서 오류가 발생했습니다.\n");
			rcvdata.flag = 0;
		}
		msgrcv(msqid, &rcvdata, sizeof(t_caldata)-sizeof(long), 2, IPC_NOWAIT);
		if(rcvdata.flag == 2){
			if(-1 == (check = sf2_save_cal(rcvdata)))
				printf("\n쓰레드에서 오류가 발생했습니다.\n");
			rcvdata.flag = 0;
		}
		msgrcv(msqid, &rcvdata, sizeof(t_caldata)-sizeof(long), 3, IPC_NOWAIT);
		if(rcvdata.flag == 3){
			if(-1 == (check = sf3_transfer_cal(rcvdata, msqid)))
				printf("\n쓰레드에서 오류가 발생했습니다.\n");
			rcvdata.flag = 0;
		}
	}
}

void main(){
	int i;
	int funcNum;
	int f1_mode;
	int calDate;
	char calName[40];
	int msqid;
	int th_check;
	pthread_t p_thread;
	pthread_attr_t attr;
	
	if(-1 == (msqid = msgget((key_t)KEY, IPC_CREAT|0666))){
		printf("큐 생성 실패!\n");
		exit(1);
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	if(0 != (th_check = pthread_create(&p_thread, &attr, catch_thread, (void *)&msqid))){
		printf("쓰레드 생성 실패!\n");
		exit(1);
	}

	system("clear");
	while(1){
		printf("원하시는 기능을 선택해 주세요.\n");
		printf("1: 캘린더 관리\n");
		printf("2: 캘린더 조회\n");
		printf("9: 종료\n");
		printf("원하시는 기능 번호를 입력해 주세요 : ");
		scanf("%d", &funcNum);
		getchar();

		if(funcNum == 1){
			sf1_cal_manage();
		}
		else if(funcNum == 2){
			sf4_see_cal();
		}
		else if(funcNum == 9){
			msgctl(msqid, IPC_RMID, 0);
			pthread_cancel(p_thread);
			exit(1);
		}
		else{
			system("clear");
			printf("잘못된 입력입니다! 다시 입력해 주세요.\n");
		}
	}
}
