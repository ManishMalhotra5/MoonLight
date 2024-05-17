#include "authentication.h"
#include "ui_authentication.h"

Authentication::Authentication(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Authentication)
{
    ui->setupUi(this);
    this->setWindowTitle("Login");
    client.connect("192.168.43.1",60000);
}

Authentication::~Authentication()
{
    delete ui;
}

void Authentication::on_SignUpbutton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);

}


void Authentication::on_LoginButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

}


void Authentication::on_LoginButton_clicked()
{
    if(client.isLogged())
    {
        client.Logout();
    }
    std::string uid = ui->uid->text().toStdString();
    std::string pass = ui->pass->text().toStdString();
    qDebug() <<"User Id "<<uid<<" password "<<pass<<"\n";
    client.SendLoginPacket(uid,pass);

    ui->Warning->setText("wait!! logging you in");

    if(!client.isLogged())
    {
        ui->Warning->setText("Login failed ! please retry!");

    }
    else
    {
        ui->Warning->setText("Logged Successfully!");
        emit SuccessFullLogin();

    }
}


void Authentication::on_pushButton_clicked()
{
    std::string userName = ui->UserName->text().toStdString();
    std::string userId = ui->UserId->text().toStdString();
    std::string userPass = ui->UserPass->text().toStdString();
    qDebug() <<"User Name "<<userName<<"User Id "<<userId<<" password "<<userPass<<"\n";
    client.SendSignUpPacket(userId,userName,userPass);
    if(!client.isLogged())
    {
        ui->label->setText("Not Signed In");
    }
    else
    {
        ui->label->setText(" Signed In");
         emit SuccessFullLogin();
    }

}

