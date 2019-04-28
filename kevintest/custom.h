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
    void updateMonsterImage(int player, int monsterSpotIndex);
    void addMonsterId(int player, QString id);

signals:
 
public slots:
    void handleKeyboard();
    void handleButtons();
 
};
 
#endif
