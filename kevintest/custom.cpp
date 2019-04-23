#include "custom.h"

#include <linux/input.h>
#include <fcntl.h>

struct card {
    int atk;
    int def;
    QString cardName;
    QString imgFileName;
};

//Spots for images to be displayed
QVector<QImage> player0_monsters(3);
QVector<QImage> player1_monsters(3);

int player0_currNumMonsters;
int player1_currNumMonsters;

//Our "database" of cards, read from file
QHash<QString, struct card> card_hash;

QString currCardId;

QSocketNotifier *keyboard_notifier;
int keyboard_devfile;

Custom::Custom(QWidget *parent) : QWidget(parent)
{
    currCardId = "";
    player0_currNumMonsters = 0;
    player1_currNumMonsters = 0;

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


    keyboard_devfile = ::open("/dev/input/event0", O_RDONLY);
    if (keyboard_devfile >= 0)
    {
        keyboard_notifier = new QSocketNotifier(keyboard_devfile, QSocketNotifier::Read, this);
        connect(keyboard_notifier, SIGNAL(activated(int)), this, SLOT(handleKeyboard()));
    }

    //setFocusPolicy(Qt::StrongFocus);
    //setEnabled(false);
    //setFocusPolicy(Qt::NoFocus);
    
    //qDebug() << "Hello";

    /*QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);*/
}

void Custom::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //qDebug() << currCardId;
    
    if (currCardId.length() > 0)
    {
        
        //painter.drawText(QRectF(0, 0, 100, 100), Qt::AlignCenter, card_hash[currCardId].cardName);
        
        player0_monsters[0].load(card_hash[currCardId].imgFileName);
        player0_monsters[1].load(card_hash[currCardId].imgFileName);
        player0_monsters[2].load(card_hash[currCardId].imgFileName);

        //qDebug() << card_hash[currCardId].imgFileName;
        //qDebug() << currCardId;

        painter.drawPixmap(100, 100, 150, 150, QPixmap::fromImage(player0_monsters[0]));
        painter.drawPixmap(150, 100, 200, 200, QPixmap::fromImage(player0_monsters[1]));
        painter.drawPixmap(200, 100, 250, 250, QPixmap::fromImage(player0_monsters[2]));
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
    //qDebug() << currCardId;
    update();
}
