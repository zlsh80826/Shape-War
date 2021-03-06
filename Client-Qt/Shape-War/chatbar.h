#ifndef CHATBAR_H
#define CHATBAR_H
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPainter>
#include <QPair>
#include <QString>
#include <QTextEdit>
#include <QTimer>
#include <QVector>
#include <QWebSocket>
#include <boardcastboard.h>

class ChatBar : public QLineEdit {
    Q_OBJECT
public:
    ChatBar(QString partUrl, QWidget *parent = 0);
    void startChat();
    void sendTextToServer();
    void setParentWidth();
    void setName(QString);

protected:
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;

private:
    QUrl *chat_url;
    QWebSocket chat_webSocket;
    QTimer *upTimer;
    QTimer *downTimer;
    QTimer *clearTimer;
    QString name;
    BoardcastBoard *broadcast;
    QVector<QString> broadcastContent;
    int posY;
    const int chatBarHeight = 30;
    const int maxPosY = 0;
    const int minPosY = -40;
    const int broadcastHeight = 120;

private slots:
    void onConnected();
    void onTextMessageReceived(QString message);
    void clearTimeoutContent();
    void down();
    void up();
};

#endif // CHATBAR_H
