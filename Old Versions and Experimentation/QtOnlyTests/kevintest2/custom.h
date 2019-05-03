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
    void updateMonster(int player, QString id);
    void battleMonsters();
    void setupGame();

signals:
 
public slots:
    void handleKeyboard();
    void handleButtons();
 
};
 
#endif
