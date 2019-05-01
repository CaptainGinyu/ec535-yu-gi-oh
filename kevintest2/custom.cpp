#include "custom.h"

#include <linux/input.h>
#include <fcntl.h>

const int WELCOME_STATE = 0;
const int MAIN_PHASE = 1;
const int BATTLE_PHASE = 2;
const int GAME_OVER = 3;
const int MODE_SELECT = 4;

const int IMAGE_WIDTH = 100;
const int IMAGE_HEIGHT = 100;

const int STARTING_LIFE_POINTS = 8000;

struct card {
    int atk;
    int def;
    QString cardId;
    QString imgFileName;
};

QVector<struct card> player_monsterInfo(2);
QVector<bool> player_hasMonster(2);
QVector<QImage> player_monsterImages(2);
QVector<bool> player_monsterModes(2);
QVector<int> player_lifePoints(2);

//Our "database" of cards, read from file
QHash<QString, struct card> card_hash;

//Current ID of card scanned by RFID
QString currCardId;

//Keeps track of RFID reader input
QSocketNotifier *keyboard_notifier;
int keyboard_devfile;

//Keeps track of GPIO button input
QSocketNotifier *buttons_notifier;
int buttons_devfile;

int currPlayer;
int currState;
int winner;

Custom::Custom(QWidget *parent) : QWidget(parent)
{
    QFile card_info_file("card_info.dat");
    QString card_info_line;
    QStringList thingsFromFile;

    setupGame();

    //Open the file
    if (card_info_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream reading(&card_info_file);        

        //Read until the end
        while (!reading.atEnd())
        {
            //Read a line
            card_info_line = reading.readLine();

            //Split by comma
            thingsFromFile = card_info_line.split(",");

            //Make sure we get 5 things from the line we read
            if (thingsFromFile.count() == 5)
            {
                //Add info from the line we read into our database
                struct card currCard = {thingsFromFile[2].toInt(), thingsFromFile[3].toInt(), thingsFromFile[0], thingsFromFile[4]};
                card_hash.insert(thingsFromFile[0], currCard);
            } 
        }
        
        //Close the file
        card_info_file.close();
    }

    //Keeping track of our RFID input
    keyboard_devfile = ::open("/dev/input/event0", O_RDONLY);
    if (keyboard_devfile >= 0)
    {
        keyboard_notifier = new QSocketNotifier(keyboard_devfile, QSocketNotifier::Read, this);
        connect(keyboard_notifier, SIGNAL(activated(int)), this, SLOT(handleKeyboard()));
    }

    //Keeping track of our button input
    buttons_devfile = ::open("/dev/yugiohkernel", O_RDONLY);
    if (buttons_devfile >= 0)
    {
        buttons_notifier = new QSocketNotifier(buttons_devfile, QSocketNotifier::Read, this);
        connect(buttons_notifier, SIGNAL(activated(int)), this, SLOT(handleButtons()));
    }
}

void Custom::setupGame()
{
    currCardId = "";
    currPlayer = 0;
    currState = WELCOME_STATE;
    player_lifePoints[0] = STARTING_LIFE_POINTS;
    player_lifePoints[1] = STARTING_LIFE_POINTS;
    player_hasMonster[0] = false;
    player_hasMonster[1] = false;
    player_monsterModes[0] = false;
    player_monsterModes[1] = false;
    winner = -1;
}

void Custom::updateMonster(int player, QString id)
{
    if ((player != 0) && (player != 1))
    {
        return;
    }
    if (id.length() > 0)
    {
        if (card_hash.find(id) != card_hash.end())
        {
            player_monsterInfo[player] = card_hash[id];
            player_monsterImages[player].load(player_monsterInfo[player].imgFileName);
            player_hasMonster[player] = true;
        }
    }
}

void Custom::battleMonsters()
{
    if (!player_hasMonster[currPlayer])
    {
        currState = MAIN_PHASE;
        currPlayer = !currPlayer;
    }

    //Current player can only attack if their current monster is in attack mode
    if (!player_monsterModes[currPlayer])
    {
        //If the opponent has no monster
        if (!player_hasMonster[!currPlayer])
        {
            //Opponent takes damage equal to attack of current player's monster
            player_lifePoints[!currPlayer] -= player_monsterInfo[currPlayer].atk;
            if (player_lifePoints[!currPlayer] <= 0)
            {
                //Go to game over screen, current player wins
                currState = GAME_OVER;
                winner = currPlayer;
            }
            else
            {
                currState = MAIN_PHASE;
                currPlayer = !currPlayer;
            } 
        }
        //Cases for when oppponent's monster is in attack mode
        if (!player_monsterModes[!currPlayer])
        {
            //If current player's monster's attack is greater than opponent's monster's attack
            if (player_monsterInfo[currPlayer].atk > player_monsterInfo[!currPlayer].atk)
            {
                //Destroy the opponent's monster, opponent takes damage
                player_lifePoints[!currPlayer] -= player_monsterInfo[currPlayer].atk - player_monsterInfo[!currPlayer].atk;
                player_hasMonster[!currPlayer] = false;
                if (player_lifePoints[!currPlayer] <= 0)
                {
                    //Go to game over screen, current player wins
                    currState = GAME_OVER;
                    winner = currPlayer;
                }
                else
                {
                    currState = MAIN_PHASE;
                    currPlayer = !currPlayer;
                }                
            }
            //If current player's monster's attack is less than opponent's monster's attack
            if (player_monsterInfo[currPlayer].atk < player_monsterInfo[!currPlayer].atk)
            {
                //Destroy the current player's monster, current player takes damage
                player_lifePoints[currPlayer] -= player_monsterInfo[!currPlayer].atk - player_monsterInfo[currPlayer].atk;
                player_hasMonster[currPlayer] = false;
                if (player_lifePoints[currPlayer] <= 0)
                {
                    //Go to game over screen, opponent wins
                    currState = GAME_OVER;
                    winner = !currPlayer;
                }
                else
                {
                    currState = MAIN_PHASE;
                    currPlayer = !currPlayer;
                }
            }
            //If current player's monster's attack is equal to opponent's monster's attack
            if (player_monsterInfo[currPlayer].atk == player_monsterInfo[!currPlayer].atk)
            {
                //Destroy both players' monsters, nobody takes damage
                player_hasMonster[currPlayer] = false;
                player_hasMonster[!currPlayer] = false;
                currState = MAIN_PHASE;
                currPlayer = !currPlayer;
            }
        }
        //Cases for when opponent's monster is in defense mode
        else
        {
            //If current player's monster's attack is greater than opponent's monster's defense
            if (player_monsterInfo[currPlayer].atk > player_monsterInfo[!currPlayer].def)
            {
                //Destroy the opponent's monster, no damage
                player_hasMonster[!currPlayer] = false;
                currState = MAIN_PHASE;
                currPlayer = !currPlayer;
            }
            //If current player's monster's attack is less than opponent's monster's defense
            if (player_monsterInfo[currPlayer].atk < player_monsterInfo[!currPlayer].def)
            {
                //No monsters destroyed, current player takes damage
                player_lifePoints[currPlayer] -= player_monsterInfo[!currPlayer].def - player_monsterInfo[currPlayer].atk;
                if (player_lifePoints[currPlayer] <= 0)
                {
                    //Go to game over screen, opponent wins
                    currState = GAME_OVER;
                    winner = !currPlayer;
                }
                else
                {
                    currState = MAIN_PHASE;
                    currPlayer = !currPlayer;
                }
            }
            //If current player's monster's attack is equal opponent's monster's defense
            if (player_monsterInfo[currPlayer].atk == player_monsterInfo[!currPlayer].def)
            {
                //No monsters destroyed, nobody takes damage
                currState = MAIN_PHASE;
                currPlayer = !currPlayer;
            }
        }
    }
    //If in defense mode, go to the other player's turn
    else
    {
        currState = MAIN_PHASE;
        currPlayer = !currPlayer;
    }
    update();
}

void Custom::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    qDebug() << "Current Player:";
    qDebug() << currPlayer;

    if (currState == WELCOME_STATE)
    {
        qDebug() << "In Welcome State";
        painter.drawText(QRectF(0, 0, 100, 100), "Press Button 0 to start game");
    }
    if ((currState == MAIN_PHASE) || (currState == MODE_SELECT) || (currState == BATTLE_PHASE))
    {
        if (currState == MAIN_PHASE)
        {
            qDebug() << "In Main phase";
            painter.drawText(QRectF(100, 0, 200, 200), "Main Phase");
            if (player_hasMonster[currPlayer])
            {
                currState = MODE_SELECT;
            }
        }
        if (currState == MODE_SELECT)
        {
            qDebug() << "In Mode select";
            painter.drawText(QRectF(100, 0, 200, 200), "Mode Select");
        }
        if (currState == BATTLE_PHASE)
        {
            qDebug() << "In Battle phase";
            painter.drawText(QRectF(100, 0, 200, 200), "Battle Phase");
        }
        
        QString lifeMessage = "Player 0 LP: " + QString::number(player_lifePoints[0]) + ", Player 1 LP: " + QString::number(player_lifePoints[1]);
        painter.drawText(QRectF(0, 0, 100, 100), lifeMessage);
        if (player_hasMonster[0])
        {
            painter.drawPixmap(110, 150, IMAGE_WIDTH, IMAGE_HEIGHT, QPixmap::fromImage(player_monsterImages[0]));
            if (player_monsterModes[0])
            {
                painter.drawText(QRectF(200, 150, 300, 250), "Def: " + QString::number(player_monsterInfo[0].def));
            }
            else
            {
                painter.drawText(QRectF(200, 150, 300, 250), "Atk: " + QString::number(player_monsterInfo[0].atk));
            }
        }
        if (player_hasMonster[1])
        {
            painter.drawPixmap(110, 50, IMAGE_WIDTH, IMAGE_HEIGHT, QPixmap::fromImage(player_monsterImages[1]));
            if (player_monsterModes[1])
            {
                painter.drawText(QRectF(200, 50, 300, 150), "Def: " + QString::number(player_monsterInfo[1].def));
            }
            else
            {
                painter.drawText(QRectF(200, 50, 300, 150), "Atk: " + QString::number(player_monsterInfo[1].atk));
            }
        }
    }
    if (currState == GAME_OVER)
    {
        //qDebug() << "Game over";
        //qDebug() << player_lifePoints[0];
        //qDebug() << player_lifePoints[1];
        QString gameOverMessage = "Game over! Winner is Player " + QString::number(winner);
        painter.drawText(QRectF(0, 0, 100, 100), gameOverMessage);
    }
}

void Custom::handleButtons()
{
    if (currState == WELCOME_STATE)
    {
        char buttonInfo[4];

        if (read(buttons_devfile, &buttonInfo, sizeof(char) * 4))
        {
            if (buttonInfo[0] == '1')
            {
                currState = MAIN_PHASE;
                update();
            }
        }
    }

    else if (currState == MAIN_PHASE)
    {
        char buttonInfo[4];

        if (read(buttons_devfile, &buttonInfo, sizeof(char) * 4))
        {
            
            if (buttonInfo[0] == '1')
            {
                currState = MODE_SELECT;
                update();
            }
            if (buttonInfo[1] == '1')
            {
                currState = BATTLE_PHASE;
                update();
            }
        }
    }

    else if (currState == MODE_SELECT)
    {
        char buttonInfo[4];

        if (read(buttons_devfile, &buttonInfo, sizeof(char) * 4))
        {
            
            if (buttonInfo[0] == '1')
            {
                //Select defense and switch to other player's turn
                player_monsterModes[currPlayer] = true;
                currState = MAIN_PHASE;
                currPlayer = !currPlayer;
                update();
            }
            if (buttonInfo[1] == '1')
            {
                //Select attack and go to battle phase
                player_monsterModes[currPlayer] = false;
                currState = BATTLE_PHASE;
                update();
            }            
        }
    }

    else if (currState == BATTLE_PHASE)
    {
        char buttonInfo[4];

        if (read(buttons_devfile, &buttonInfo, sizeof(char) * 4))
        {
            
            if (buttonInfo[0] == '1')
            {
                //Attack
                battleMonsters();
            }
            if (buttonInfo[1] == '1')
            {
                //No attack
                currState = MAIN_PHASE;
                currPlayer = !currPlayer;
                update();
            }
        }
    }

    else if (currState == GAME_OVER)
    {
        char buttonInfo[4];

        if (read(buttons_devfile, &buttonInfo, sizeof(char) * 4))
        {
            
            if (buttonInfo[0] == '1')
            {
                //Restart game
                setupGame();
                update();
            }
        }
    }
}

void Custom::handleKeyboard()
{
    struct input_event event;

    currCardId = "";

    //If the current player already has a monster, we don't bother reading the card
    if (player_hasMonster[currPlayer])
    {
        return;
    }

    //Only read cards during main phase
    if (currState != MAIN_PHASE)
    {
        return;
    }

    while (1)
    {
	    if (read(keyboard_devfile, &event, sizeof(struct input_event)))
	    {
		    if (event.type == 1 && event.value == 1)
		    {
			    if (event.code == 2)
			    {
                    currCardId += "1";
			    }
			    if (event.code == 3)
			    {
                    currCardId += "2";
			    }
			    if (event.code == 4)
			    {
                    currCardId += "3";
			    }
			    if (event.code == 5)
			    {
                    currCardId += "4";
			    }
			    if (event.code == 6)
			    {
                    currCardId += "5";
			    }
			    if (event.code == 7)
			    {
                    currCardId += "6";
			    }
			    if (event.code == 8)
			    {
                    currCardId += "7";
			    }
			    if (event.code == 9)
			    {
                    currCardId += "8";
			    }
			    if (event.code == 10)
			    {
                    currCardId += "9";
			    }
			    if (event.code == 11)
			    {
                    currCardId += "0";
			    }
			    if (event.code == 28)
			    {
                    break;
			    }
		    }
	    }
    }

    //if a card with a non-zero length ID was read, we add a monster with the given card ID
    //then we go to mode select
    if (currCardId.length() > 0)
    {
        updateMonster(currPlayer, currCardId);
        currState = MODE_SELECT;
        update();
    }   
}
