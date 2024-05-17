#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("MoonLight");

    auth = new Authentication(this);
    auth->show();


    QObject::connect(ui->ReqPageButton,&QPushButton::clicked,this,&MainWindow::ReqPageRenderer);

    QObject::connect(ui->SearchPageButton,&QPushButton::clicked,this,&MainWindow::SearchPageButtonClicked);
    QObject::connect(ui->SearchButton,&QPushButton::clicked,this,&MainWindow::SearchButtonClicked);
    QObject::connect(ui->BackButton,&QPushButton::clicked,this,&MainWindow::BackButtonClicked);
    QObject::connect(ui->HomeButton,&QPushButton::clicked,this,&MainWindow::BackButtonClicked);
    QObject::connect(ui->AboutButton,&QPushButton::clicked,this,&MainWindow::AboutButtonClicked);
    QObject::connect(ui->ChatPageButton,&QPushButton::clicked,this,&MainWindow::ChatPageButtonClicked);
    QObject::connect(ui->SendButton,&QPushButton::clicked,this,&MainWindow::SendButtonClicked);
    QObject::connect(auth,&Authentication::SuccessFullLogin,this,&MainWindow::WriteUserName);
    QObject::connect(this, &MainWindow::TriggerRoomUpdate, this, &MainWindow::UpdateChatRoom);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::WriteUserName()
{
    ui->AboutButton->setText(auth->client.MyName().data());
}

void MainWindow::ChatPageButtonClicked()
{
    ui->HomeOptions->setCurrentIndex(0);
    clearFrndList();
    if(auth->client.GetRoomList().empty())
    {
        // qWarning() <<"No Friends";
        return;
    }

    for(auto &i : auth->client.GetRoomList())
    {
        addFriend(i.data());
    }
}
void MainWindow::addFriend(const std::string & uid)
{

    QVBoxLayout * layout = qobject_cast<QVBoxLayout *>(ui->UserList->layout());
    auto parent = ui->UserList;
    User * newUsr = new User(uid,parent);
    FrndList.push_back(newUsr);
    layout->insertWidget(0,newUsr->userButton());
    connect(newUsr->userButton(),&QPushButton::clicked,this,&MainWindow::UserButtonClicked);

}

void MainWindow::UserButtonClicked()
{
    ui->StackedW->setCurrentIndex(1);
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());

    
    exitChat();
    
    for(auto & i : FrndList)
    {
        if(i->userButton() == clickedButton && auth->client.EnterRoom(i->userName()))
        {
            ui->UserName->setText(i->userName().data());
            exit_chat = false;
            break;
        }
    }
    
    if(chat_thread.joinable())
    {
        std::cerr << "Something went wrong !!\n";
        return;
    }
    
    chat_thread = std::thread(
      [this]()
      {
        int count = 0;
        while(!exit_chat)
        {
             std::shared_ptr<sun::ChatMessage> mesg_container = auth->client.CurrentRoomNextMessage();
             if(!mesg_container || exit_chat)	continue;
 		  	 
             emit TriggerRoomUpdate(mesg_container, count);
   			 
             count ++;
        }
      });
    
}

void MainWindow::UpdateChatRoom(std::shared_ptr<sun::ChatMessage> mesg_container, uint32_t mesg_number)
{

	QVBoxLayout * chat_lay = qobject_cast<QVBoxLayout *>(ui->Chats->layout());
   Message * container = new Message(mesg_container->message, ui->Chats, false);
   MessageList.push_back(container);
   if(mesg_container->status) // status true for incoming message
   	chat_lay->insertWidget(mesg_number,container->container(),Qt::AlignTop,Qt::AlignLeft);
   else							// status false for outgoing message
   	chat_lay->insertWidget(mesg_number,container->container(),Qt::AlignTop,Qt::AlignRight);
}

void MainWindow::exitChat()
{
	exit_chat = true;
	auth->client.ExitRoom();
	
	if(chat_thread.joinable())
    {
    	chat_thread.join();
    }

    for(auto & i : MessageList)
    {
        delete i->container();
    }
    MessageList.clear();
	//will be completed by Manish

	//Task -> clear chat_lay...
	// if chat_lay is empty, do nothing
	
}


void MainWindow::clearReqUsrList()
{
    for(auto & i: ReqFrndList)
    {
        delete i->container();
    }
    ReqFrndList.clear();
}

void MainWindow::clearFrndList()
{
    for(auto & i: FrndList)
    {
        delete i->userButton();
    }
    FrndList.clear();
}
void MainWindow::ReqPageRenderer()
{
    ui->HomeOptions->setCurrentIndex(2);
    clearReqUsrList();
    if(auth->client.GetRecvRoomRequestList().empty())
    {
        qDebug() <<" Reqused User List is empty";
        return;
    }


    for(auto & i : auth->client.GetRecvRoomRequestList())
    {
        qDebug() <<"Reqused User "<<i<<"\n";
        requsedUsr(i);
    }
}


void MainWindow::requsedUsr(const std::string & uid)
{
    QVBoxLayout * layout = qobject_cast<QVBoxLayout *>(ui->Requested->layout());
    auto parent = ui->Requested;
    ReqUsr *reqUsr = new ReqUsr(uid,parent);
    ReqFrndList.push_back(reqUsr);
    layout->insertWidget(0,reqUsr->container());
    connect(reqUsr->acceptButton(),&QPushButton::clicked,this,&MainWindow::reqAccepted);
}

void MainWindow::reqAccepted()
{
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());

    if(!clickedButton) return;
    for(auto &i : ReqFrndList)
    {
        if(i->acceptButton() == clickedButton)
        {
            auth->client.AcceptRoomRequestOf(i->userName());
            addFriend(i->userName());
            deleteReqUsr(i);
            break;
        }
    }
}


void MainWindow::deleteReqUsr(ReqUsr * usr)
{
    auto d = std::find(ReqFrndList.begin(),ReqFrndList.end(),usr);
    ReqFrndList.erase(d);

    delete usr->container();
}

void MainWindow::sendRequest()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());

    for(auto &i : auth->client.GetSendRoomRequestList())
    {
        qDebug()<<" you had sent req to "<<i<<" \n";
    }

    if(!clickedButton) return ;
    for(auto & i : AllUser)
    {
        if(i->acceptButton() == clickedButton)
        {
            auth->client.SendRoomRequestTo(i->userName());
            qDebug() <<" Request is Sent to "<<i->userName()<<" \n";
            delete i->container();
            break;
        }
    }
}


void MainWindow::showQueriedUsr(const std::string & uid,QVBoxLayout * layout)
{
    ReqUsr *usr = new ReqUsr(uid,ui->AllUsrList);
    usr->acceptButton()->setText("Send Request");
    layout->insertWidget(0,usr->container());
    AllUser.push_back(usr);
    QObject::connect(usr->acceptButton(),&QPushButton::clicked,this,&MainWindow::sendRequest);
}


void MainWindow::queryHandler()
{
    QVBoxLayout * layout = qobject_cast<QVBoxLayout *>(ui->AllUsrList->layout());
    std::string query = ui->Search->text().toStdString();

    if(query.empty())
    {
        qDebug() <<"Type Something";
        return;
    }

    if( query == auth->client.MyID())
    {
        qDebug() << "we can't add you as your friend\n";
        return;
    }

    resetAllUsrList();
    std::vector<std::string> clients = auth->client.GetQueryResultFor(query);
    if(clients.empty())
    {
        qDebug() <<"No Users";

        return;
    }
    else
    {
    for(auto &i : clients)
    {
        qDebug()<<i<<"\n";
        showQueriedUsr(i,layout);
    }
    }
    ui->Search->clear();

}

void MainWindow::resetAllUsrList()
{
    for(auto &usr : AllUser)
    {
        delete usr->container();
    }
    AllUser.clear();
}

void MainWindow::SendButtonClicked()
{
    QString msg = ui->InputBox->toPlainText();
    if (msg.isEmpty()) {
        qDebug() << "Error: Empty message";
        return;
    }
    
    std::string typed_msg = msg.toStdString();
    ui->InputBox->clear();
    auth->client.SendTextPacket(typed_msg);   
    
    /* 
    static int count = 0;
    
    QVBoxLayout * chat_lay = qobject_cast<QVBoxLayout *>(ui->Chats->layout());
    Message * container = new Message(typed_msg,ui->Chats,false);

    chat_lay->insertWidget(count,container->container(),Qt::AlignTop,Qt::AlignRight);
    count ++;
	*/
}



void MainWindow::SearchPageButtonClicked()
{
    ui->HomeOptions->setCurrentIndex(1);

}

void MainWindow::SearchButtonClicked()
{
    queryHandler();
}


void MainWindow::BackButtonClicked()
{
    exitChat();
    ui->StackedW->setCurrentIndex(0);
}

void MainWindow::AboutButtonClicked()
{

    ui->StackedW->setCurrentIndex(2);
}
