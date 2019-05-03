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
const int MAX_MONSTERS = 2;

struct card {
    int atk;
    int def;
    QString cardName;
    QString imgFileName;
};

//Empty image to show for empty spot where there is no monster
QImage emptyImage;

//Spots for images to be displayed
QVector<QImage> player0_monsterImages(MAX_MONSTERS);
QVector<QImage> player1_monsterImages(MAX_MONSTERS);

//RFID numbers of each players' monsters
QVector<QString> player0_monsterIds(MAX_MONSTERS);
QVector<QString> player1_monsterIds(MAX_MONSTERS);

//Modes of each of the players' monsters (true for attack, false for defense)
QVector<bool> player0_monsterModes(MAX_MONSTERS);
QVector<bool> player1_monsterModes(MAX_MONSTERS);

//Number of monsters each player currently has summoned
int player0_currNumMonsters;
int player1_currNumMonsters;

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

Custom::Custom(QWidget *parent) : QWidget(parent)
{
    currCardId = "";
    player0_currNumMonsters = 0;
    player1_currNumMonsters = 0;

    currPlayer = 0;
    currState = WELCOME_STATE;

    emptyImage = QPixmap(IMAGE_WIDTH, IMAGE_HEIGHT).toImage();

    QFile card_info_file("card_info.dat");
    QString card_info_line;
    QStringList thingsFromFile;

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
                struct card currCard = {thingsFromFile[2].toInt(), thingsFromFile[3].toInt(), thingsFromFile[1], thingsFromFile[4]};
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

void Custom::updateMonsterImage(int player, int monsterSpotIndex)
{
    if (player == 0)
    {
        if (player0_monsterIds[monsterSpotIndex].length() > 0)
        {
            if (card_hash.find(player0_monsterIds[monsterSpotIndex]) != card_hash.end())
            {
                player0_monsterImages[monsterSpotIndex].load(card_hash[player0_monsterIds[monsterSpotIndex]].imgFileName);
            }
            else
            {
                player0_monsterImages[monsterSpotIndex] = emptyImage;
                player0_monsterImages[monsterSpotIndex].fill(Qt::color1);
            }
        }
        else
        {
            player0_monsterImages[monsterSpotIndex] = emptyImage;
            player0_monsterImages[monsterSpotIndex].fill(Qt::color1);
        }
    }
    if (player == 1)
    {
        if (player1_monsterIds[monsterSpotIndex].length() > 0)
        {
            if (card_hash.find(player1_monsterIds[monsterSpotIndex]) != card_hash.end())
            {
                player1_monsterImages[monsterSpotIndex].load(card_hash[player1_monsterIds[monsterSpotIndex]].imgFileName);
            }
            else
            {
                player1_monsterImages[monsterSpotIndex] = emptyImage;
                player1_monsterImages[monsterSpotIndex].fill(Qt::color1);
            }
        }
        else
        {
            player1_monsterImages[monsterSpotIndex] = emptyImage;
            player1_monsterImages[monsterSpotIndex].fill(Qt::color1);
        }
    }
}

void Custom::addMonsterId(int player, QString id)
{
    if (id.length() > 0)
    {
        if (player == 0)
        {
            if (player0_currNumMonsters < MAX_MONSTERS)
            {
                player0_monsterIds[player0_currNumMonsters] = id;
                player0_currNumMonsters++;
            }
        }
        if (player == 1)
        {
            if (player1_currNumMonsters < MAX_MONSTERS)
            {
                player1_monsterIds[player1_currNumMonsters] = id;
                player1_currNumMonsters++;
            }
        }
    }
}

void Custom::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (currState == WELCOME_STATE)
    {
        qDebug() << "In Welcome State";
        painter.drawText(QRectF(0, 0, 100, 100), "Press Button 0 to start game");
        //In function for checking button press, check for button press and if button is pressed
        //Buttons: Button 0: Start game and go to main phase of player 0, Button 1: Quit
    }
    if ((currState == MAIN_PHASE) || (currState == MODE_SELECT))
    {
        if (currState == MAIN_PHASE)
        {
            qDebug() << "In Main phase";
        }
        if (currState == MODE_SELECT)
        {
            qDebug() << "In Mode select";
        }
        //for each monster spot, we first check to see what monster each player has, then draw each monster
        for (int i = 0; i < MAX_MONSTERS; i++)
        {
            updateMonsterImage(0, i);

            updateMonsterImage(1, i);

            painter.drawPixmap((100 * i) + 10, 150, IMAGE_WIDTH, IMAGE_HEIGHT, QPixmap::fromImage(player0_monsterImages[i]));
        }

        //In function for checking button press, check for button press and if button is pressed
        //Buttons: Button 0: Go to battle phase, Button 1-2: Change position of monster 1-2
    }

    if (currState == BATTLE_PHASE)
    {
        for (int i = 0; i < MAX_MONSTERS; i++)
        {
            painter.drawPixmap((100 * i) + 10, 150, IMAGE_WIDTH, IMAGE_HEIGHT, QPixmap::fromImage(player0_monsterImages[i]));
        }
        //In function for checking button press, check for button press and if button is pressed
        //Buttons: Button 0-2: Select attack target, Button 3: Switch player and go to main phase
    }
    if (currState == GAME_OVER)
    {
        painter.drawText(QRectF(0, 0, 100, 100), "Game over!");
        //In function for checking button press, check for button press and if button is pressed
        //Buttons: Button 0: Start new game, Button 1: Quit
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

    if (currState == MAIN_PHASE)
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
}

void Custom::handleKeyboard()
{
    struct input_event event;

    currCardId = "";

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

    if (currState == MAIN_PHASE)
    {
        //if a card with a non-zero length ID was read, we add a monster with the given card ID if there is enough room
        if (currCardId.length() > 0)
        {
            addMonsterId(currPlayer, currCardId);
            currState = MODE_SELECT;
            update();
        }
    }    
}
