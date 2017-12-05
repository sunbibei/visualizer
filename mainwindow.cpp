#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fstream"

#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    data_  = new AdtEigen(90, 90);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connected()
{
//    QMessageBox::about(this, "TIP", "CONNECTED");
   ui->status->setText("连接成功");
   ui->btn_connect->setText("Disconnect");
   connect(this->socket, SIGNAL(readyRead()), this, SLOT(readyread()));
}

QString out;
std::ofstream ofd("/Users/bibei/data.txt");
void MainWindow::readyread()
{
    QByteArray arr = socket->readAll();
    ofd << QString(arr).toStdString();
    this->ui->textBrowser->setText(QString(arr));
    return;
    int p0 = 0;
    int p1 = 0;
    size_t _r = 0;
    size_t _c = 0;
    double _v = 0;
    while (p1 < arr.size()) {
        if ('\n' == arr[p1]) {
            if (p1 - p0 != 11) {
                ++p1;
                p0 = p1;
                continue;
            }
            _r = (arr[p0] - '0')*10 + (arr[p0+1]-'0');
            p0 += 2;
            _c = (arr[p0] - '0')*10 + (arr[p0+1]-'0');
            p0 += 2;
            _v = (arr[p0] - '0') + 0.1*(arr[p0+2]-'0')
                    +0.01*(arr[p0+3]-'0')+0.001*(arr[p0+4]-'0')
                    +0.0001*(arr[p0+5]-'0')+0.00001*(arr[p0+6]-'0');
            p0 += 8;
            out += (QString::number(_r) + " " + QString::number(_c) + " " + QString::number(_v) + "\n");
        }
        ++p1;
    }

    this->ui->textBrowser->setText(out);
}

void MainWindow::parseFromData(double _raw, size_t& _r, size_t& _c, double& _v) {
    _r = _raw / 1000;
    //size_t tmp = _raw%1000;
    //_c = tmp/10;
    //_v = _raw%10;
}

void MainWindow::on_btn_connect_clicked()
{
    this->socket->connectToHost("10.10.100.254", 8899,QTcpSocket::ReadOnly);
    connect(this->socket, SIGNAL(connected()), this, SLOT(connected()));
}

void MainWindow::on_pushButton_clicked()
{
    socket->close();
    ofd.close();
}
