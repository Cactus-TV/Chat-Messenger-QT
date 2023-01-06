#ifndef EXTRA_H
#define EXTRA_H
#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>

class QExLabel : public QLabel {
    Q_OBJECT
public:
    explicit QExLabel(QWidget* parent = 0)
        : QLabel(parent){};
signals:
    void clicked(QLabel* sender);
protected:
    void mouseReleaseEvent(QMouseEvent* e)
    {
        if(e->button() == Qt::RightButton)
        {
            // Нажата правая кнопка
            emit clicked((QLabel*) this);
        }
    }
};

class QExPushButton : public QPushButton {
    Q_OBJECT
public:
    explicit QExPushButton(const QString &text, QWidget* parent = 0)
        : QPushButton(parent){};
signals:
    void clicked_left();
    void clicked_right();
protected:
    void mouseReleaseEvent(QMouseEvent* e)
    {
        if (e->button() == Qt::LeftButton)
        {
            // Нажата левая кнопка
            emit clicked_left();
        }
        else if (e->button() == Qt::RightButton)
        {
            // Нажата правая кнопка
            emit clicked_right();
        }
    }
};

#endif // EXTRA_H
