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


struct player_info{
	int life;
	int monster;
	int mode;
} player[2];
struct card_info{
	char ID[20];
	int atk;
} card[3];

int max=2;
char cardID[20];

void sighandler(int signo)
{
	printf("Awaken!\n");
}



void update(int x, FILE *fp)
{
	fp = fopen("game.dat", "w+");
	fprintf(fp, "%d ", x);
	int i;
	for (i=0;i<=1; i++){
		printf("player%d: %d %d %d\n",i,player[i].life, player[i].monster, card[player[i].monster].atk);
		fprintf(fp,"%d %d %d ",player[i].life, player[i].monster, card[player[i].monster].atk);}
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

	//pause();

	FILE *fp = NULL;
	int x=0;
	int i=0;
	for (i=0;i<=1;i++)
	{
		player[i].life=8000;
		player[i].monster=-1;
	}
	card[0].atk=3000;
	card[1].atk=2000;
	card[2].atk=1000;

	strcpy(card[0].ID,"190213203224");
	strcpy(card[1].ID,"222155172252");
	strcpy(card[2].ID,"14218735211");

	int cardnum;
	while(x<=1&&player[0].life>0&&player[1].life>0)
	{
		//wait for card
		printf("It's %d player's turn!\n", x);
		update(x,fp);
		printf("input a card to use\n");
		pause();
		char get[10];
		sprintf(get,"");
		read(pFile,get,10);
		printf("%s\n",get);
		if(get[0]=='0')
		{
			getID();
			
			for(i=0;i<=max;i++)
			{
				if(strcmp(cardID,card[i].ID)==0)
				{
					player[x].monster=i;
					break;
				}
			}
		}

		update(x,fp);
		//enter second phase
		//attack or not
		if (player[x].monster>=0)
		{
			pause();
			sprintf(get,"");
			read(pFile,get,10);
			if(get[0]=='0')
			{
				int diff;
				if(player[abs(x-1)].monster<0)
					diff = card[player[x].monster].atk;
				else diff =  card[player[x].monster].atk - card[player[abs(x-1)].monster].atk;
				if (diff<0)
				{
					player[x].life = player[x].life + diff;
					player[x].monster=-1;
				}
				else if (diff==0)
				{
					player[x].monster=-1;
					player[abs(x-1)].monster=-1;
				}
				else if (diff>0)
				{
					player[abs(x-1)].monster=-1;
					player[abs(x-1)].life=player[abs(x-1)].life-diff;
				}
			}
		}
		for(i=0;i<=1;i++)
		{
			if(player[i].life<0)
				player[i].life=0;
		}
		update(x,fp);
		x=abs(x-1);
	}
	int win;
	if(player[0].life>0)
		win=0;
	else win=1;
	printf("game over!\nPlayer %d wins!\n",win);
}

void getID()
{
	int devfile;
	struct input_event event;
    cardID[0] = 0;

	devfile = open("/dev/input/event0", O_RDONLY);
	while (1)
	{
		if (read(devfile, &event, sizeof(struct input_event)))
		{
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
