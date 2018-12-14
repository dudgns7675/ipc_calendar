/*
	t_caldata flags
	1 : client -> server memo write request
	2 : client -> server memo delete request
	3 : client -> server memo see request
	3*pid : server -> client memo see response
*/
#define BUFF_SIZE 32
#define MEMO_NUMS 10
#define KEY 7675

typedef struct {
	long flag;
	int pid;
	char calName[BUFF_SIZE];
	int date;
	char memo[MEMO_NUMS][BUFF_SIZE];
	int memoNum;
}t_caldata;
