#ifndef CHATBAR_H
#define CHATBAR_H
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QTextEdit>
#include <QString>
#include <QTimer>
#include <QWebSocket>
#include <QVector>
#include <QPair>
#include <QPainter>

class ChatBar : public QLineEdit {
    Q_OBJECT
public:
    ChatBar(QString partUrl, QWidget *parent = 0);
    void startChat();
    void sendTextToServer();
    void setParentWidth();
    void setName(QString);

protected:
    void focusOutEvent(QFocusEvent *event);
    void focusInEvent(QFocusEvent *event);

private:
    QUrl *chat_url;
    QWebSocket chat_webSocket;
    QTimer *upTimer;
    QTimer *downTimer;
    QString name;
    QTextEdit boardcast;
    QVector< QPair<QString* , QString* >* > chatQueue;
    int posY;
    const int chatBarHeight = 30;
    const int maxPosY = 0;
    const int minPosY = -40;

private slots:
    void onConnected();
    void onTextMessageReceived(QString message);
    void down();
    void up();
};

#endif // CHATBAR_H
