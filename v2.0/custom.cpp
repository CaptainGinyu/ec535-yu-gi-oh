#include "custom.h"

QString message;
//QImage imageUsr0;
//QImage imageUsr1;
QImage image0;
QImage image1;
QImage image2;
QImage helloScreen;
QString activeUsr, card0, card1, card0Att, card1Att, usr0Life, usr1Life, card0Def, Mode0,card1Def,Mode1;
QFile gameFile("game.dat");
qint8 getFile;

Custom::Custom(QWidget *parent) : QWidget(parent)
{
    getFile = 0;
    message = "";
    activeUsr = "";
    card0 = "";
    card1 = "";
    card0Att = "";
    card1Att = "";
    usr0Life = "";
    usr1Life = "";
    helloScreen.load("blue-eyes.png");
    image0.load("blue-eyes.png");
    image1.load("dark-magician.png");
    image2.load("kuriboh.png");
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);
}

void Custom::paintEvent(QPaintEvent *event)
{ 
    getFile = 0;
    if (gameFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream reading(&gameFile);
        message = reading.readLine();
        QTextStream messageStream(&message);
        messageStream >> activeUsr >> usr0Life >> card0 >> card0Att >> card0Def >> Mode0 >> usr1Life >> card1 >> card1Att >> card1Def >> Mode1;
        gameFile.close();
        getFile = 1;
    }

    QPainter painter(this);

    if(getFile == 0)
    {
        //painter.drawPixmap(100, 100, 150, 150, QPixmap::fromImage(helloScreen));
        painter.drawText(QRectF(0, 0, 480, 272), Qt::AlignCenter, tr("Welcome to Yu_Gi_Oh\nLoad game to start!"));        
    }    
    else{
        painter.translate(240, 136);
        painter.rotate(90);
        // Start painting the left side.
        if (QString::compare(usr0Life, "0", Qt::CaseSensitive) == 0)
        {
            painter.drawText(QRectF(-136, 0, 272, 240), Qt::AlignCenter, tr("User 0 Lost"));
           }
        else {
            painter.drawText(QRectF(-136, 0, 90, 120), Qt::AlignBottom | Qt::AlignHCenter, "Life\n"+usr0Life);
            if(QString::compare(activeUsr, "0", Qt::CaseSensitive) == 0){
                        painter.drawText(QRectF(-136, 120, 90, 120), Qt::AlignTop | Qt::AlignHCenter, "Your turn");
            }
            if (QString::compare(card0, "-1", Qt::CaseSensitive) != 0){
                if(QString::compare(Mode0, "0", Qt::CaseSensitive) == 0){
                    //painter.drawText(QRectF(-31, 165, 75, 60), Qt::AlignCenter, "Attack\n"+card0Att);
                    painter.drawText(QRectF(-31, 165, 150, 60), Qt::AlignCenter, "Attack\n"+card0Att);
                }
                else{
                    //painter.drawText(QRectF(-31, 165, 75, 60), Qt::AlignCenter, "Defence\n"+card0Att);
                    painter.drawText(QRectF(-31, 165, 150, 60), Qt::AlignCenter, "Defence\n"+card0Def);
                }
                //painter.drawText(QRectF(44, 165, 75, 60), Qt::AlignCenter, "Status\n"+card0Att);
                if(QString::compare(card0, "0", Qt::CaseSensitive) == 0){
                    painter.drawPixmap(-35, 15, 150, 150, QPixmap::fromImage(image0));
                }
                else if(QString::compare(card0, "1", Qt::CaseSensitive) == 0){
                    painter.drawPixmap(-35, 15, 150, 150, QPixmap::fromImage(image1));
                }
                else if(QString::compare(card0, "2", Qt::CaseSensitive) == 0){
                    painter.drawPixmap(-35, 15, 150, 150, QPixmap::fromImage(image2));
                }
            }
        }

    painter.rotate(180);
    // Start painting the right side.
        if (QString::compare(usr1Life, "0", Qt::CaseSensitive) == 0)
        {
            painter.drawText(QRectF(-136, 0, 272, 240), Qt::AlignCenter, tr("User 1 Lost"));
        }
        else{
            painter.drawText(QRectF(-136, 0, 90, 120), Qt::AlignBottom | Qt::AlignHCenter, "Life\n"+usr1Life);
            if(QString::compare(activeUsr, "1", Qt::CaseSensitive) == 0){
                painter.drawText(QRectF(-136, 120, 90, 120), Qt::AlignTop | Qt::AlignHCenter, "Your turn");
            }
            if (QString::compare(card1, "-1", Qt::CaseSensitive) != 0){
                if(QString::compare(Mode1, "0", Qt::CaseSensitive) == 0){
                    //painter.drawText(QRectF(-31, 165, 75, 60), Qt::AlignCenter, "Attack\n"+card1Att);
                    painter.drawText(QRectF(-31, 165, 150, 60), Qt::AlignCenter, "Attack\n"+card1Att);
                }
                else{
                    //painter.drawText(QRectF(-31, 165, 75, 60), Qt::AlignCenter, "Defence\n"+card1Att);
                    painter.drawText(QRectF(-31, 165, 150, 60), Qt::AlignCenter, "Defence\n"+card1Def);
                }
                //painter.drawText(QRectF(44, 165, 75, 60), Qt::AlignCenter, "Status\n"+card1Att);
                if(QString::compare(card1, "0", Qt::CaseSensitive) == 0){
                    painter.drawPixmap(-35, 15, 150, 150, QPixmap::fromImage(image0));
                }
                else if(QString::compare(card1, "1", Qt::CaseSensitive) == 0){
                    painter.drawPixmap(-35, 15, 150, 150, QPixmap::fromImage(image1));
                }
                else if(QString::compare(card1, "2", Qt::CaseSensitive) == 0){
                    painter.drawPixmap(-35, 15, 150, 150, QPixmap::fromImage(image2));
                }
            }
        }
    }
}
