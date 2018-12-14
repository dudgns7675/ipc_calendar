#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "calHeader.h"

void printcal(t_caldata caldata){
	int i;
	printf("%d의 메모\n", caldata.date);
	for(i=0; i<MEMO_NUMS; i++)
		printf("%d. %s\n", i+1, caldata.memo[i]);
}

int cf1_see_cal(int msqid){
	t_caldata caldata;
	t_caldata rcvdata;

	caldata.pid = getpid();
	caldata.flag = 3;
	system("clear");
	printf("캘린더의 메모를 조회합니다.\n");
	printf("메모를 조회할 캘린더 이름을 입력하세요 : ");
	scanf("%s", caldata.calName);
	getchar();
	printf("메모를 조회할 날짜를 입력하세요 : ");
	scanf("%d", &caldata.date);
	getchar();
	printf("서버로 메모 조회를 요청합니다...\n");

	if(-1 == msgsnd(msqid, &caldata, sizeof(t_caldata)-sizeof(long), 0)){
		system("clear");
		printf("요청에 실패했습니다!\n");
		return -1;
	}
	sleep(1);
	printf("요청에 성공했습니다. 응답을 기다립니다...\n");
	rcvdata.flag = 0;
	while(rcvdata.flag == 0){
		msgrcv(msqid, &rcvdata, sizeof(t_caldata)-sizeof(long), (3*caldata.pid), 0);
		sleep(1);
	}
	if(rcvdata.memoNum == -1){
		printf("오류 응답이 돌아왔습니다.\n");
		printf("입력한 캘린더가 존재하지 않습니다.\n");
		printf("엔터를 입력하면 처음으로 돌아갑니다.\n");
		getchar();
		system("clear");
		return -1;
	}
	if(rcvdata.memoNum == -2){
		printf("오류 응답이 돌아왔습니다.\n");
		printf("입력한 날짜에 메모가 존재하지 않습니다..\n");
		printf("엔터를 입력하면 처음으로 돌아갑니다.\n");
		getchar();
		system("clear");
		return -1;
	}
	printf("조회한 %d의 메모를 출력합니다.\n", rcvdata.date);
	printcal(rcvdata);
	printf("엔터를 입력하면 처음으로 돌아갑니다.\n");
	getchar();
	system("clear");
	return 1;
}

int cf2_write_cal(int msqid){
	t_caldata caldata;
	int mode;
	int i;
	char *tp;

	tp = caldata.memo[0];
	for(i=0; i<(MEMO_NUMS*BUFF_SIZE); i++)
		*(tp+i) = '\0';

	system("clear");
	while(1){
		printf("캘린더의 메모를 수정합니다.\n");
		printf("메모를 추가하시겠습니까? 삭제하시겠습니까?\n");
		printf("(1: 추가, 2: 삭제) : ");
		scanf("%d", &mode);
		getchar();

		if(mode == 1){
			caldata.flag = 1;
			printf("메모를 추가할 캘린더 이름을 입력하세요 : ");
			scanf("%s", caldata.calName);
			getchar();
			printf("메모를 추가할 날짜를 입력하세요 : ");
			scanf("%d", &caldata.date);
			getchar();
			printf("추가할 메모 번호를 입력하세요 : ");
			scanf("%d", &caldata.memoNum);
			getchar();
			printf("추가할 메모 내용을 입력하세요 : ");
			scanf("%[^\n]s", caldata.memo[caldata.memoNum-1]);
			getchar();
			printf("서버로 메모 추가를 요청합니다...\n");

			if(-1 == msgsnd(msqid, &caldata, sizeof(t_caldata)-sizeof(long), 0)){
				system("clear");
				printf("요청에 실패했습니다!\n");
				return -1;
			}
			sleep(1);
			system("clear");
			printf("서버로 메모 추가를 요청했습니다.\n");
			return 1;
		}
		else if(mode == 2){
			caldata.flag = 2;
			printf("메모를 삭제할 캘린더 이름을 입력하세요 : ");
			scanf("%s", caldata.calName);
			getchar();
			printf("삭제할 메모 날짜를 입력하세요 : ");
			scanf("%d", &caldata.date);
			getchar();

			if(-1 == msgsnd(msqid, &caldata, sizeof(t_caldata)-sizeof(long), 0)){
				system("clear");
				printf("요청에 실패했습니다!\n");
				return -1;
			}
			sleep(1);
			system("clear");
			printf("서버로 메모 삭제를 요청했습니다.\n");
			return 1;
		}
		else{
			system("clear");
			printf("잘못된 입력입니다! 다시 입력해 주세요.\n");
		}
	}
}

void main(){
	int msqid;
	int funcNum;

	system("clear");

	if((msqid = msgget((key_t)KEY, IPC_CREAT|IPC_EXCL) != -1)){
		printf("메시지 큐가 없습니다!\n");
		printf("서버 프로그램이 켜져 있는지 확인해 주세요.\n");
		msqid = msgget((key_t)KEY, IPC_CREAT);
		msgctl(msqid, IPC_RMID, 0);
		exit(1);
	}
	else{
		if(-1 == (msqid = msgget((key_t)KEY, IPC_CREAT|0666))){
			printf("메시지 큐 접속에 실패했습니다.\n");
			exit(1);
		}
		else
			printf("메시지 큐 접속에 성공했습니다.\n");
	}

	while(1){
		printf("1: 캘린더의 메모 조회하기\n");
		printf("2: 캘린더의 메모 수정하기\n");
		printf("9: 종료\n");
		printf("원하시는 기능 번호를 입력해 주세요 : ");
		scanf("%d", &funcNum);
		getchar();

		if(funcNum == 1){
			cf1_see_cal(msqid);
		}
		else if(funcNum == 2){
			cf2_write_cal(msqid);
		}
		else if(funcNum == 9){
			exit(1);
		}
		else{
			system("clear");
			printf("잘못된 입력입니다! 다시 입력해 주세요.\n");
		}
	}
}
