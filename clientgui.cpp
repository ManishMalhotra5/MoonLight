#include "clientgui.h"



User::User(const std::string &name,QWidget * parent = nullptr)
{
    m_userName = name;
    m_userButton = new QPushButton(name.data(),parent);
}

User::~User()
{
    delete m_userButton;
}

QPushButton * User::userButton() const
{
    return m_userButton;
}

std::string User::userName() const
{
    return m_userName;
}


/*===================================================================================*/
ReqUsr::ReqUsr(const std::string &name , QWidget * parent = nullptr)
{
    m_userName = name;
    m_container = new QWidget (parent);
    m_userButton = new QPushButton(m_userName.data(),m_container);
    m_acceptButton = new QPushButton("Accept",m_container);

    hbox = new QHBoxLayout(m_container);
    hbox->addWidget(m_userButton);
    hbox->addWidget(m_acceptButton);
    hbox->setSpacing(4);
    m_container->setStyleSheet(
        "QWidget{"
        "background-color: rgb(255,255,255);"
        "height:50px;"
        "}"
        "QWidget::hover{"
        "background-color:#F5F5F5;"
        "}"
        );
    m_container->setMaximumHeight(60);
    setStyle(m_acceptButton);
}


ReqUsr::~ReqUsr()
{
    delete m_container;
    delete hbox;
}
void ReqUsr::setStyle(QPushButton * button)
{
    button->setStyleSheet("QPushButton{"
                          "border:none;"
                          "color: rgb(255, 255, 255);"
                          "font-size:15px;"
                          "height:50px;"
                          "background-color: rgb(6, 139, 107);"
                          "}"
                          "QPushButton::hover{"
                          "background-color: rgb(0, 120, 99);"
                          "}"
                          );
}


QPushButton * ReqUsr::userButton() const
{
    return m_userButton;
}

QPushButton *ReqUsr::acceptButton()const
{
    return m_acceptButton;
}

QWidget * ReqUsr::container() const
{
    return m_container;
}

std::string ReqUsr::userName() const
{
    return m_userName;
}

/*===================================================================================*/

Message::Message(std::string & msg,QWidget * parent = nullptr,bool rec = false)
{
    m_container = new QWidget(parent);
    m_msg = new QLabel(m_container);
    containerLay = new QVBoxLayout(m_container);
    m_msg->setText(msg.data());
    containerLay->addWidget(m_msg);
    m_rec = rec;
    m_container->setStyleSheet(
       " background:#fff;"
        "padding:10px;"
        "border-radius: 10px;"
        );
    m_msg->setStyleSheet(
        "color: black;"
        // "padding: 10px;"
        "font: 15px;"
        "white-space: wrap;"
        );
    // m_container->setMinimumHeight(60);


}

Message::~Message()
{
    delete m_container;
}

QWidget * Message::container()
{
    return m_container;
}
bool Message::received()
{
    return m_rec;
}

