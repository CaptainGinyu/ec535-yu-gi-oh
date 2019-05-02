#include "custom.h"
#include "QPen"

// Announce the useful global variables that will be needed in the Qt part.
QString message;				// A temp str to store the information read from the game file
QImage imageusr0;					// The image of the first user
QImage imageusr1;					// The image of the second user
QImage helloScreen;                             // Image that will be shown in the hello screen
QString activeUsr, card0, card1, card0Att, card1Att, usr0Life, usr1Life, card0Def, Mode0,card1Def,Mode1, status, survive0, survive1; // Useful strings to print on the screen
QFile gameFile("game.dat");			// game file that will be containing the game result after every operation.
qint8 getFile;                                  // A flag to show whether a file has been read or not.
QStringList statusList;
qint8 statusInt;
QStringList imageList;
qint8 imageUsrInt0;
qint8 imageUsrInt1;

Custom::Custom(QWidget *parent) : QWidget(parent)
{
    // Set up all default variables
    getFile = 0;
    message = "";
    activeUsr = "";
    card0 = "";
    card1 = "";
    card0Att = "";
    card1Att = "";
    usr0Life = "";
    usr1Life = "";
    statusList << "Tap Card" << "Sel Mode" << " Battle ";

    // Temp variables to store the info read from file.
    QString temp0,temp1,temp2,cardFile;

    // Read the file
    QFile inputFile(QString("data.txt"));
    inputFile.open(QIODevice::ReadOnly);
    if (!inputFile.isOpen())
        return;
    QTextStream stream(&inputFile);
    QString line = stream.readLine();
    while (!line.isNull()) {
        QTextStream cardInfoStream(&line);
        // Read oen line of the file
        cardInfoStream >> temp0 >> temp1 >> temp2 >> cardFile;
        // Store it to the image list.
	cardFile = "data/"+cardFile;
        imageList << cardFile;
        line = stream.readLine();
    };

    // Load all useful images into the Qt module
    helloScreen.load("helloScreen.png");

    // Use a timer to call for the update screen function every 100ms
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);
}

void Custom::paintEvent(QPaintEvent *event)
{
    // Only when the file is 'there' then we go into the function of printing with all info of the game
    // Otherwise only helloScreen will be shown.
    getFile = 0;
    if (gameFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream reading(&gameFile);
        message = reading.readLine();
        QTextStream messageStream(&message);
        messageStream >> activeUsr >> usr0Life >> card0 >> card0Att >> card0Def >> Mode0 >> survive0 >> usr1Life >> card1 >> card1Att >> card1Def >> Mode1 >> survive1 >> status;
        statusInt = status.toInt();
        imageUsrInt0 = card0.toInt();
        imageUsrInt1 = card1.toInt();
        gameFile.close();
        getFile = 1;
    }

    // Set up a painter
    QPainter painter(this);
    painter.setBrush(Qt::NoBrush);
    QPen penOrd;
    QPen penAround;
    penAround.setWidth(1);
    penAround.setColor(Qt::red);
    QPen penStatus;
    penStatus.setColor(Qt::blue);
    penAround.setWidth(2);
        
    // If no file was found, show hello screen, else shows the battle screen
    if(getFile == 0){
        painter.drawText(QRectF(0, 136, 480, 136), Qt::AlignCenter, tr("Welcome to Yu_Gi_Oh\nLoad game to start!"));
        painter.drawPixmap(0, 0, 480, 136, QPixmap::fromImage(helloScreen));        
    }    
    else{
        // Set up the original point of the painter to the central of the screen
        painter.translate(240, 136);
        // Rotate the painter to paint on one side
        painter.rotate(90);
        // Start painting the left side.

        // If someone's life falls to 0, print the lost screen, else shows the life point and other information
        if (QString::compare(usr0Life, "0", Qt::CaseSensitive) == 0){
            painter.drawText(QRectF(-136, 0, 272, 240), Qt::AlignCenter, tr("User 0 Lost"));
        }
        else {
            // Shows life in the left upper part.
            painter.drawText(QRectF(-136, 0, 90, 240), Qt::AlignCenter, "Life\n"+usr0Life+"\n\n"+survive0);
            for(int i = 0; i < 3 ; i++)
                painter.drawText(QRectF(i*88-132, 0, 90, 15), Qt::AlignHCenter, statusList.at(i));
            if(QString::compare(activeUsr, "0", Qt::CaseSensitive) == 0){
                // Shows the status of the user
                painter.setPen(penAround);
                painter.drawRect(QRectF(-133, 0, 266, 237));
                painter.setPen(penStatus);
                painter.drawRect(QRectF(statusInt*88-132, 0, 90, 15));
                painter.setPen(penOrd);
            }
            if (QString::compare(card0, "-1", Qt::CaseSensitive) != 0){
                // Shows the status of the monster
		painter.drawText(QRectF(-35, 165, 73, 30), Qt::AlignLeft, "Attack\n"+card0Att);
		painter.drawText(QRectF(38, 165, 73, 30), Qt::AlignRight, "Defence\n"+card0Def);
                if(QString::compare(Mode0, "0", Qt::CaseSensitive) == 0){
		    painter.drawText(QRectF(-35, 195, 146, 45), Qt::AlignCenter, "Attack\n");
                }
                else{
		    painter.drawText(QRectF(-35, 195, 146, 45), Qt::AlignCenter, "Defence\n");
                }
                // Shows the card image on the screen
                imageusr0.load(imageList.at(imageUsrInt0));
                painter.drawPixmap(-35, 17, 146, 146, QPixmap::fromImage(imageusr0));
            }
        }
        
        // Rotate the painter to paint on the other side
        painter.rotate(180);
        // Start painting the right side.
        if (QString::compare(usr1Life, "0", Qt::CaseSensitive) == 0){
            painter.drawText(QRectF(-136, 0, 272, 240), Qt::AlignCenter, tr("User 1 Lost"));
        }
        else{
            painter.drawText(QRectF(-136, 0, 90, 240), Qt::AlignCenter, "Life\n"+usr1Life+"\n\n"+survive1);
            for(int i = 0; i < 3 ; i++)
                painter.drawText(QRectF(i*88-132, 0, 90, 15), Qt::AlignHCenter, statusList.at(i));
            if(QString::compare(activeUsr, "1", Qt::CaseSensitive) == 0){
                painter.setPen(penAround);
                painter.drawRect(QRectF(-133, 0, 266, 237));
                painter.setPen(penStatus);
                painter.drawRect(QRectF(statusInt*88-132, 0, 90, 15));
                painter.setPen(penOrd);
            }
            if (QString::compare(card1, "-1", Qt::CaseSensitive) != 0){
		painter.drawText(QRectF(-35, 165, 73, 30), Qt::AlignLeft, "Attack\n"+card1Att);
		painter.drawText(QRectF(38, 165, 73, 30), Qt::AlignRight, "Defence\n"+card1Def);
                if(QString::compare(Mode1, "0", Qt::CaseSensitive) == 0){
		    painter.drawText(QRectF(-35, 195, 146, 45), Qt::AlignCenter, "Attack\n");
                }
                else{
		    painter.drawText(QRectF(-35, 195, 146, 45), Qt::AlignCenter, "Defence\n");
                }
                imageusr1.load(imageList.at(imageUsrInt1));
                painter.drawPixmap(-35, 17, 146, 146, QPixmap::fromImage(imageusr1));
            }
        }
    }
}

