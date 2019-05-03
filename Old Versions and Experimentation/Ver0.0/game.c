#include <stdio.h>
#include <stdlib.h>
struct player_info{
	int life;
	int monster;
} player[2];
struct card_info{
	//int ID;
	int atk;
	//int def;
} card[3];
int main()
{
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
	int ca, op;
	while(x<=1&&player[0].life>0&&player[1].life>0)
	{
		//wait for card
		printf("It's %d player's turn!\n", x);
		printf("input a card to use\n");
		scanf("%d",&ca);
		//wait for button
		if(ca>=0&&ca<3)
			player[x].monster=ca;
		fp = fopen("game.dat", "w+");
		fprintf(fp, "%d ", x);
		for (i=0;i<=1; i++){
			printf("player%d: %d %d %d\n",i,player[i].life, player[i].monster, card[player[i].monster].atk);
			fprintf(fp,"%d %d %d ",player[i].life, player[i].monster, card[player[i].monster].atk);}
		fclose(fp);
		//enter second phase
		//attack or not
		if (player[x].monster>=0)
		{
			scanf("%d",&op);
			if(op==1)
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
		fp = fopen("game.dat", "w+");
		fprintf(fp, "%d ", x);
		for (i=0;i<=1; i++){
			printf("player%d: %d %d %d\n",i,player[i].life, player[i].monster, card[player[i].monster].atk);
			fprintf(fp,"%d %d %d ",player[i].life, player[i].monster, card[player[i].monster].atk);}
		fclose(fp);
		x=abs(x-1);
	}
	int win;
	if(player[0].life>0)
		win=0;
	else win=1;
	printf("game over!\nPlayer %d wins!\n",win);
}