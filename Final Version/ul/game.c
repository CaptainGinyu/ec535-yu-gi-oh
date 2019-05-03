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

//create structs that store battle information
struct player_info{
	int life;
	int monster;
	int mode;
	int survive;
} player[2];
struct card_info{
	char ID[20];
	int atk;
	int def;
} card[33];

int max=32;        //how many cards in the database
char cardID[20];

//wake up signal callback
void sighandler(int signo)
{
	printf("Awaken!\n");
}

//write battle information to file that allows QT to read
void update(int x, FILE *fp, int phase)
{
	fp = fopen("game.dat", "w+");
	fprintf(fp, "%d ", x);
	int i;
	for (i=0;i<=1; i++){
		printf("player%d: %d %d %d\n",i,player[i].life, player[i].monster, card[player[i].monster].atk);
		if (player[i].monster == -1)
			fprintf(fp,"%d -1 0 0 %d 0 ",player[i].life, player[i].mode);
		else
			fprintf(fp,"%d %d %d %d %d %d ",player[i].life, player[i].monster, card[player[i].monster].atk, card[player[i].monster].def, player[i].mode, player[i].survive);
	}
	fprintf(fp, "%d ", phase);
	fclose(fp);
}

int main(int argc, char **argv)
{
	//set up signal hanlder
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
	char buf[100];
	//read the file that stores card information
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

	//initialize the player data
	for (i=0;i<=1;i++)
	{
		player[i].life = 8000;
		player[i].monster = -1;
		player[i].mode = 0;
		player[i].survive = 0;
	}

	//main game logic starts
	while(x<=1&&player[0].life>0&&player[1].life>0)
	{
		//wait for card
		char get[10];
		printf("It's %d player's turn!\n", x);
		update(x,fp,0);
		printf("input a card to use\n");
		
		//wait for card scan or button pressed
		getID(pFile);

		//check if the card number in the database and grab the card number
		for(i=0;i<=max;i++)
		{
			if(strcmp(cardID,card[i].ID)==0)
			{
				player[x].monster=i;
				player[x].survive=0;
				break;
			}
		}
		update(x,fp,0);

		//entering phase choosing phase, first check if the player has a monster
		if (player[x].monster != -1)
		{
			printf("select to change status\n");
			update(x,fp,1);
			//wait for button signal
			pause();
			sprintf(get,"");
			//check which button is pressed
			read(pFile,get,10);
			//printf(" status: %s\n",get);
			if (get[0]=='0')
				player[x].mode = 0;
			else player[x].mode = 1;
		}

		update(x,fp,2);
		//entering battle phase
		//check if player has a monster
		if (player[x].monster>=0)
		{
			//check if the monster is in attack mode
			if (player[x].mode == 0)
			{
				//wait for button signal to decide whether to attack or not
				pause();
				sprintf(get,"");
				read(pFile,get,10);
				//attack ordered, start damage calculation
				if(get[0]=='0')
				{
					//check the if the opposite player has a monster, if not it is a direct attack
					if (player[abs(x-1)].monster<0)
					{
						player[abs(x-1)].life -= card[player[x].monster].atk;
					}
					else 
					{
						//opposite player has a monster, check its mode
						//if in attack mode
						if (player[abs(x-1)].mode == 0)
						{
							int diff;
							diff =  card[player[x].monster].atk - card[player[abs(x-1)].monster].atk;
							if (diff<0)
							{
								player[x].life = player[x].life + diff;
								player[x].monster=-1;
								player[x].mode = 0;
								player[x].survive = 0;
							}
							else if (diff==0)
							{
								player[x].monster=-1;
								player[abs(x-1)].monster=-1;
								player[x].mode = 0;
								player[abs(x-1)].mode = 0;
								player[x].survive = 0;
								player[abs(x-1)].survive = 0;
							}
							else if (diff>0)
							{
								player[abs(x-1)].monster=-1;
								player[abs(x-1)].life=player[abs(x-1)].life-diff;
								player[abs(x-1)].mode = 0;
								player[abs(x-1)].survive = 0;
							}
						}
						//if in defend mode
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
								player[abs(x-1)].survive = 0;
								player[abs(x-1)].mode = 0;
							}
						}
					}
				}
			}
		}
		//after-turn check, check for winning, monster turn check, etc.
		for(i=0;i<=1;i++)
		{
			if (player[i].life<0)
				player[i].life=0;
			if (player[i].monster != -1)
				player[i].survive++;
			if (player[i].survive >= 4)
				player[i].monster = -1;
		}
		update(x,fp,2);
		x=abs(x-1);
	}

	//one of the player has lost, point out and end the game
	int win;
	if(player[0].life>0)
		win=0;
	else win=1;
	printf("game over!\nPlayer %d wins!\n",win);
	sleep(5);
	remove("game.dat");
}

//scan card or button
void getID(int pFile)
{
	int devfile;
	struct input_event event;
    	cardID[0] = 0;
	//printf("get info0\n");
	char get[10];
	sprintf(get,"");

	//open the keyboard kernel where the card ID comes
	devfile = open("/dev/input/event0", O_RDONLY);
	while (1)
	{	
		//see if a card is tapped ot a button is pressed
		if (read(devfile, &event, sizeof(struct input_event)) || read(pFile,get,10))
		{
			read(pFile,get,10);
			//button pressed, meaning no card to use	
			if(get[0]=='1')
				break;
			//card scanned, start getting card ID
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
