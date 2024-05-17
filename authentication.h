#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include "client.hpp"
#include <QDialog>

namespace Ui {
class Authentication;
}

class Authentication : public QDialog
{
    Q_OBJECT

public:
    explicit Authentication(QWidget *parent = nullptr);
    ~Authentication();

private slots:
    void on_SignUpbutton_clicked();

    void on_LoginButton_2_clicked();

    void on_LoginButton_clicked();

    void on_pushButton_clicked();

signals:
    void SuccessFullLogin();

private:
    Ui::Authentication *ui;
public:
    sun::client client;

};

#endif // AUTHENTICATION_H
