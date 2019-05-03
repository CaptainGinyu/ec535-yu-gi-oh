#include <QtGui>
#include <QPalette>
#include <QWSServer>
#include "custom.h"

int main(int argc, char **argv){
    QApplication app(argc, argv);

    QWSServer *server = QWSServer::instance();
    if(server)
    {
        server->setCursorVisible(false);
    }

    Custom custom;
    QPalette p = custom.palette();
    p.setColor(QPalette::Window, Qt::white);
    custom.setPalette(p);
    custom.setAutoFillBackground(true);
    custom.showFullScreen();

    return app.exec();
}
