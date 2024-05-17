#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "authentication.h"
#include "clientgui.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void reqAccepted();
    void ReqPageRenderer();
    void ChatPageButtonClicked();
    void SearchPageButtonClicked();
    void sendRequest();
    void SearchButtonClicked();
    void BackButtonClicked();
    void UserButtonClicked();
    void AboutButtonClicked();
    void SendButtonClicked();
    void WriteUserName();
    void UpdateChatRoom(std::shared_ptr<sun::ChatMessage>, uint32_t );

    
signals:
     void TriggerRoomUpdate(std::shared_ptr<sun::ChatMessage>, uint32_t );

private:
    Ui::MainWindow *ui;
    Authentication *auth;
    std::list<User *> FrndList;
    std::list<ReqUsr*> ReqFrndList;
    std::list<ReqUsr *> AllUser;
    std::list<Message *> MessageList;
    QVBoxLayout *userListLay;
    QVBoxLayout *reqUserListLay;
    QVBoxLayout *allUserLay;
    
    //for chat
    std::atomic<bool> exit_chat = true;
    std::thread chat_thread;

public:
    void addFriend(const std::string &);
    void handleReq(const std::string &);
    void requsedUsr(const std::string &);
    void deleteReqUsr(ReqUsr*);
    void queryHandler();
    void resetAllUsrList();
    void showQueriedUsr(const std::string &,QVBoxLayout *);
    void clearReqUsrList();
    void clearFrndList();
    void renderMessages();
    void exitChat();

};
#endif // MAINWINDOW_H
