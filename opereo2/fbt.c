#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


#define dprintf(x) printf x
#include "ssbanger.inc"
#define SSBLKS 16

typedef int16_t fbt_t;


fbt_t fbt[SSBLKS];
fbt_t fbthead;

#define FILENO 30

struct file {
	int name;
	int head;
	int blks;
}Files[FILENO];

int fbfree;

void fbt_list(int);
void fbt_dump();
void fbt_interactive();

void
fbt_init()
{
	int i;

	for (i = 0; i < SSBLKS; i++) {
		fbt[i] = i + 1;
	}
	fbt[SSBLKS - 1] = -1;

	fbthead = 0;

	fbfree = SSBLKS;
	for (int i = 0; i < FILENO; i++) {
		Files[i].name = i;
		Files[i].head = -1;
		Files[i].blks = 0;

	}
	srand((unsigned int)time(NULL));

	printf("fbt system initialized\n");


}

int
fbt_alloc(int nblks)
{
	int i;
	int fb ;
	int b4fb ;
	int b4head;

	printf("fbt_alloc: fbthead %d nblks %d \n",fbthead, nblks);



	if (nblks > fbfree) {
		printf("fbt_alloc: Error nblks > fbfree\n");
		return -1;
	}


	fb = fbthead;

	if (fb >= SSBLKS) {
		printf("fbt_alloc: Error fbthead is over SSBLKS. Please select again\n");
		return -1;
	}


	for (i = 0; i < nblks; i++) {
		b4fb = fb;
		fb = fbt[fb];
	}

	b4head = fbthead;

	fbt[b4fb] = -1;

	fbthead = fb;

	fbfree -= nblks;

	printf("fbt_alloc: allocated list: ");
	fbt_list(b4head);
	printf("fbt_alloc: free list     : ");
	fbt_list(-1);

	return b4head;
}



int
fbt_free(int blkno, int nblks) {
	int i;
	int fb;

	printf("fbt_free: blkno %d, nblks %d\n", blkno, nblks);

	if (blkno > SSBLKS) {
		printf("fbt_free: blkno(%d) doesn't exist\n", blkno);
	}
	if (nblks <= 0 || nblks > SSBLKS) {
		printf("fbt_free: %dblcks is over SSBLKS or -1\n", nblks);
	}

	fb = blkno;
	for (fb = blkno, i = 1; i < nblks; i++) {
		fb = fbt[fb];
		if (fb < 0) {								
			printf("fbt_free: blks is not appropriate \n");
			fbt_init();
			fbt_interactive();
		}
	}

	if (fbt[fb] != -1) {
		printf("fbt_free: Error this list is not configured of %dblocks\n", i);
		fbt_init();
		fbt_interactive();
	}


	fbt[fb] = fbthead;
	fbthead = blkno;

	fbfree += nblks;

	printf("fbt_alloc: free list    : ");
	fbt_list(-1);
	
	return 0;
}


void
fbt_list(int blkno)
{
	int fb;
		for (fb = (blkno >= 0) ? blkno : fbthead; fb >= 0; fb = fbt[fb]) {
			printf(" %d", fb);
		}
	printf("\n");



}


void
fbt_dump()
{
	int i;
	printf("fbt_dump:\n");
	printf("fbthead = %d\n", fbthead);
	printf("fbfree  = %d\n", fbfree);
	printf("       --------------\n");
	for (i = 0 ; i < SSBLKS ; i++) {
		printf("    %2d |     %2d     |\n", i, fbt[i]);
		printf("       --------------\n");
	}

}



void
fbt_verify()
{
	int bn;
	int count;

	for (count = 0, bn = fbthead; fbt[bn] >= 0; bn = fbt[bn], count++);
	count++;
	if (count == fbfree)
		printf("fbt_verify: Nomal(free = %dblks)\n", count);
	else
		printf("Error");
}




int nfiles = 0;

void
fbt_Random_banging_test()
{



	int fn = 0;
	int s = 0;
	int head = 0;
	int blks = 0;


	int judge = rand() % 100;



	if (judge > 50) {
		blks = rand() % 7 + 1;

		for (int m = 0; m < FILENO; m++) {
			if (Files[m].head == -1) {
				fn = m;
				head = fbt_alloc(blks);
				if (head == -1) {
					goto next4;
				}
				goto next1;
			}
		}
		printf("Do'nt write more!");
		goto next3;
	next1:;
		Files[fn].head = head;
		Files[fn].blks = blks;
		nfiles++;
		printf("fbt_alloc: name %d head %d blks %d %dfiles\n", Files[fn].name, Files[fn].head, Files[fn].blks, nfiles);


	}
	else {
		for (int a = 0; a < FILENO; a++) {
			if (Files[a].head != -1) {
				while (1) {
					s = rand() % FILENO ;
					if (Files[s].head > -1) {
						fn = s;
						break;
					}
				}
				fbt_free(Files[fn].head, Files[fn].blks);
				nfiles = nfiles - 1;
				printf("fbt_free: name %d head %d blks %d %dfiles\n", Files[fn].name, Files[fn].head, Files[fn].blks, nfiles);
				Files[fn].head = -1;
				Files[fn].blks = 0;

				goto next2;


			}
		}
		printf("fbt_free  don't free\n\n");
	next2:;

	}
next3:;
next4:;

	printf("Current state\n\n\n");
	for (int i = 0; i < FILENO; i++) {
		if (Files[i].head != -1)
			printf("name %4d\nhead    %d\nblks    %d\n\n", Files[i].name, Files[i].head, Files[i].blks);
	}



}







int getcmd(char* scmd, int* sparam1, int* sparam2) {

	char str[100];
	fgets(str, sizeof(str), stdin);
	char* tp;


	tp = strtok(str, " ");
	*scmd = *tp;

	tp = strtok(NULL, " ");
	if (tp != NULL) {
		*sparam1 = atoi(tp);
	}
	else
		*sparam1 = -2;



	tp = strtok(NULL, " ");
	if (tp != NULL) {
		*sparam2 = atoi(tp);
	}
	else
		*sparam2 = -2;


	if (*sparam1 == -2 && *sparam2 == -2)
		return 1;
	if (*sparam1 != -2 && *sparam2 == -2)
		return 2;
	if (*sparam1 != -2 && *sparam2 != -2)
		return 3;
	else - 1;
}


void
fbt_interactive()
{
	int bn;
	char cmd;
	int param1;
	int param2;
	int ic;

	for (;;) {
		fputs("fbt>", stdout); fflush(stdout);

		switch ((ic = getcmd(&cmd, &param1, &param2))) {
		case 0: //EOF
			goto out;
		case 1: //command only
			if (cmd == 'd')
				fbt_dump();
			else if (cmd == 'v')
				fbt_verify(); 
			else if (cmd == 'r')
				fbt_Random_banging_test();
			else
				printf("don't use [ %c ] command\n", cmd);
			break;
		case 2: //command and 1 parameter
			if (cmd == 'a') {
				bn = fbt_alloc(param1);
				printf("A %d %d\n", bn, param1);
			}
			else if (cmd == 'd')
				fbt_list(param1);
			else if (cmd == 'r') {
				for (int i = 0; i < param1; i++)
					fbt_Random_banging_test();
			}
			else {
				printf("don't use [ %c %d ] command\n", cmd, param1);
			}
			break;

		case 3: //command and 2parameter
			if (cmd == 'f') {
				fbt_free(param1, param2);
				printf("F %d\n", param1);
			}else {
				printf("don't use [ %c %d %d ] command\n", cmd, param1, param2);
			}
			break;

		case -1:
			break;


		}
	}
out:;
}


int
main()
{
	fbt_init();
#if 0
	fbt_interactive();
#else
	ssbanger(1000, fbt_alloc, fbt_free, NULL, NULL, NULL);
#endif
	fbt_dump();
	fbt_verify();
}
