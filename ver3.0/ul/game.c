#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <time.h>

struct player_info{
	int life;
	int monster;
	int mode;
} player[2];
struct card_info{
	char ID[20];
	int atk;
	int def;
} card[3];

int max=2;
char cardID[20];

void sighandler(int signo)
{
	printf("Awaken!\n");
}



void update(int x, FILE *fp, int phase)
{
	fp = fopen("game.dat", "w+");
	fprintf(fp, "%d ", x);
	int i;
	for (i=0;i<=1; i++){
		printf("player%d: %d %d %d\n",i,player[i].life, player[i].monster, card[player[i].monster].atk);
		if (player[i].monster == -1)
			fprintf(fp,"%d -1 0 0 %d ",player[i].life, player[i].mode);
		else
			fprintf(fp,"%d %d %d %d %d ",player[i].life, player[i].monster, card[player[i].monster].atk, card[player[i].monster].def, player[i].mode);
	}
	fprintf(fp, "%d ", phase);
	fclose(fp);
}

int main(int argc, char **argv)
{

	int pFile, oflags;
	struct sigaction action, oa;
	pFile = open("/dev/mytimer", O_RDWR);
	if (pFile < 0) {
		fprintf (stderr, "mytimer module isn't loaded\n");
		return 1;
	}

	memset(&action, 0, sizeof(action));
	action.sa_handler = sighandler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGIO, &action, NULL);
	fcntl(pFile, F_SETOWN, getpid());
	oflags = fcntl(pFile, F_GETFL);
	fcntl(pFile, F_SETFL, oflags | FASYNC);
	int i = 0;
	//pause();
	char buf[100];
	FILE* data;
	data = fopen("data.txt","r");
	while(fgets(buf,sizeof(buf),data))
	{
		if (i <= max)
		{
			strcpy(card[i].ID, strtok(buf, " "));
			card[i].atk = atoi(strtok(NULL, " "));
			card[i].def = atoi(strtok(NULL, " "));
			i++;
		}
		else break;
	}	
	fclose(data);
	FILE *fp = NULL;
	int x=0;
	for (i=0;i<=1;i++)
	{
		player[i].life = 8000;
		player[i].monster = -1;
		player[i].mode = 0;
	}
	/*
	card[0].atk = 3000;
	card[1].atk = 2000;
	card[2].atk = 1000;
	card[0].def = 1000;
	card[1].def = 2000;
	card[2].def = 3000;
	strcpy(card[0].ID,"190213203224");
	strcpy(card[1].ID,"222155172252");
	strcpy(card[2].ID,"14218735211");
	*/
	while(x<=1&&player[0].life>0&&player[1].life>0)
	{
		//wait for card
		char get[10];
		printf("It's %d player's turn!\n", x);
		update(x,fp,0);
		printf("input a card to use\n");
		getID(pFile);
		for(i=0;i<=max;i++)
		{
			if(strcmp(cardID,card[i].ID)==0)
			{
				player[x].monster=i;
				break;
			}
		}
		update(x,fp,0);

		if (player[x].monster != -1)
		{
			printf("select to change status\n");
			update(x,fp,1);
			pause();
			sprintf(get,"");
			read(pFile,get,10);
			//printf(" status: %s\n",get);
			if (get[0]=='0')
				player[x].mode = 0;
			else player[x].mode = 1;
		}

		update(x,fp,2);
		//enter second phase
		//attack or not
		if (player[x].monster>=0)
		{
			if (player[x].mode == 0)
			{
				pause();
				sprintf(get,"");
				read(pFile,get,10);
				if(get[0]=='0')
				{
					if (player[abs(x-1)].monster<0)
					{
						player[abs(x-1)].life -= card[player[x].monster].atk;
					}
					else 
					{
						if (player[abs(x-1)].mode == 0)
						{
							int diff;
							diff =  card[player[x].monster].atk - card[player[abs(x-1)].monster].atk;
							if (diff<0)
							{
								player[x].life = player[x].life + diff;
								player[x].monster=-1;
								player[x].mode = 0;
							}
							else if (diff==0)
							{
								player[x].monster=-1;
								player[abs(x-1)].monster=-1;
								player[x].mode = 0;
								player[abs(x-1)].mode = 0;
							}
							else if (diff>0)
							{
								player[abs(x-1)].monster=-1;
								player[abs(x-1)].life=player[abs(x-1)].life-diff;
								player[abs(x-1)].mode = 0;
							}
						}
						else
						{
							int diff;
							diff =  card[player[x].monster].atk - card[player[abs(x-1)].monster].def;
							if (diff<0)
							{
								player[x].life = player[x].life + diff;
							}
							else if (diff>0)
							{
								player[abs(x-1)].monster = -1;
								player[abs(x-1)].mode = 0;
							}
						}
					}
				}
			}
		}
		for(i=0;i<=1;i++)
		{
			if(player[i].life<0)
				player[i].life=0;
		}
		update(x,fp,2);
		x=abs(x-1);
	}
	int win;
	if(player[0].life>0)
		win=0;
	else win=1;
	printf("game over!\nPlayer %d wins!\n",win);
	sleep(5);
	remove("game.dat");
}

void getID(int pFile)
{
	int devfile;
	struct input_event event;
    	cardID[0] = 0;
	//printf("get info0\n");
	char get[10];
	sprintf(get,"");

	devfile = open("/dev/input/event0", O_RDONLY);
	while (1)
	{	
		//printf("get info0\n");
		if (read(devfile, &event, sizeof(struct input_event)) || read(pFile,get,10))
		{
			read(pFile,get,10);
			//printf("pfile: %s", get);			
			if(get[0]=='1')
				break;
			if (event.type == 1 && event.value == 1)
			{
				if (event.code == 2)
				{
					strcat(cardID,"1");
				}
				if (event.code == 3)
				{
					strcat(cardID,"2");
				}
				if (event.code == 4)
				{
					strcat(cardID,"3");
				}
				if (event.code == 5)
				{
					strcat(cardID,"4");
				}
				if (event.code == 6)
				{
					strcat(cardID,"5");
				}
				if (event.code == 7)
				{
					strcat(cardID,"6");
				}
				if (event.code == 8)
				{
					strcat(cardID,"7");
				}
				if (event.code == 9)
				{
					strcat(cardID,"8");
				}
				if (event.code == 10)
				{
					strcat(cardID,"9");
				}
				if (event.code == 11)
				{
					strcat(cardID,"0");
				}
				if (event.code == 28)
				{
                    			break;
				}
			}
		}
	}
}
