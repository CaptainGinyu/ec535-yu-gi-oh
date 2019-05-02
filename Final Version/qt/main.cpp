#include <QtGui>
#include <QPalette>
#include <QWSServer>
#include "custom.h"

int main(int argc, char **argv){
  QApplication app(argc, argv);

  Custom custom;
  QPalette p = custom.palette();
  p.setColor(QPalette::Window, Qt::white);
  custom.setPalette(p);
  custom.setAutoFillBackground(true);
  QWSServer::setCursorVisible( false );

  custom.showFullScreen();
  return app.exec();
}
