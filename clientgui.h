#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>


class User
{
public:
    User(const std::string &,QWidget *);
    ~User();
private:
    QPushButton * m_userButton;
    std::string m_userName;
public:
    QPushButton * userButton() const;
    std::string userName() const;

};

class ReqUsr
{
public:
    ReqUsr(const std::string &,QWidget *);
    ~ReqUsr();
private:
    QPushButton * m_acceptButton;
    QWidget * m_container;
    QPushButton * m_userButton;
    std::string m_userName;
    QHBoxLayout *hbox;

public:
    QPushButton * acceptButton() const;
    QWidget * container() const;
    QPushButton * userButton() const;
    std::string userName() const;
    void setStyle(QPushButton *);
};


class Message
{
public:
    Message(std::string &,QWidget *,bool);
    ~Message();
public:
    QWidget * container();
    QLabel *msg();
    bool received();
private:
    QWidget * m_container;
    QLabel * m_msg;
    bool m_rec;
    QVBoxLayout * containerLay;

};


#endif // CLIENTGUI_H
