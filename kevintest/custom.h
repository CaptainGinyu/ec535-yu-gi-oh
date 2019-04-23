#ifndef CUSTOM_H
#define CUSTOM_H
 
#include <QtGui>
#include <QWidget>
 
class Custom : public QWidget
{
    Q_OBJECT
public:
    Custom(QWidget *parent = 0);
 
protected:
    void paintEvent(QPaintEvent *event);

signals:
 
public slots:
    void handleKeyboard();
 
};
 
#endif
