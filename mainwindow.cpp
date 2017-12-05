#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QLabel>
#include <fstream>
#include <opencv2/opencv.hpp>

const size_t DELIMITER_SIZE = 2;
const char DELIMITER[]      = "\r\n";
// const char START_CHAR    = 0x02; // STX
// const char END_CHAR      = 0x03; // ETX
const char START_CHAR       = '*';
const char END_CHAR         = '!';


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    READ_BUF_SIZE(1024)
{
    ui->setupUi(this);
    settings = new SettingsDialog;

    socket = new QTcpSocket(this);
    data_  = new AdtEigen(90, 16);

    ///! initialize the buffer.
    read_buf_ = new char[READ_BUF_SIZE];
    memset(read_buf_, 0x00, READ_BUF_SIZE*sizeof(char));
    buf_btm_ = buf_top_ = read_buf_;
    BUF_END_ = read_buf_ + READ_BUF_SIZE;

    count_ = tmp_r_ = tmp_c_ = 0;
    tmp_v_ = 0;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    status = new QLabel;
    ui->statusBar->addWidget(status);

    initActionsConnections();
}

MainWindow::~MainWindow()
{
    delete[] read_buf_;
    BUF_END_ = read_buf_ = buf_btm_ = buf_top_ = nullptr;
    delete ui;
    delete settings;
}

void MainWindow::readyread() {
    buf_lock_.lock();
    size_t read = socket->read(buf_top_, BUF_END_ - buf_top_);
    if (0 == read) return;

    buf_top_ += read;
    buf_lock_.unlock();
}

inline void __parse_value(char* _raw, size_t count, AdtEigen* data) {
    size_t _r = (_raw[0] - '0')*10 + (_raw[1]-'0');
    size_t _c = (_raw[2] - '0')*10 + (_raw[3]-'0');
    double _v = (_raw[4] - '0')+ 0.1*(_raw[6]-'0')
            +0.01  *(_raw[7]-'0')+0.001  *(_raw[8] -'0')
            +0.0001*(_raw[9]-'0')+0.00001*(_raw[10]-'0');
    data->update(count, _r, _c, _v);
}

void MainWindow::parse() {
    const int LEN   = 11;
    char* off = buf_btm_;
    while (off != buf_top_) {
        if ((DELIMITER[0] == *off) && (DELIMITER[1] == *(off+1))) {
            if ((off - buf_btm_) != LEN) {
                off += DELIMITER_SIZE;
                buf_btm_ = off;
                continue;
            }
            __parse_value(buf_btm_, count_, data_);
            off += DELIMITER_SIZE; // eat the '\r\n'
            buf_btm_ = off;
            // show += QString::number(tmp_r_) + " " + QString::number(tmp_c_) + " " + QString::number(tmp_v_) + "\n";
        } else if (END_CHAR == *off) {
            while ((END_CHAR == *off) && (off < buf_top_)) ++off; // eat the all of the END_CHAR
            if ((DELIMITER[0] == *off) && (DELIMITER[1] == *(off+1))) off += DELIMITER_SIZE;

            buf_btm_ = off;
        } else if (START_CHAR == *off) {
            while ((START_CHAR == *off) && (off < buf_top_)) ++off; // eat the all of the END_CHAR
            if ((DELIMITER[0] == *off) && (DELIMITER[1] == *(off+1))) off += DELIMITER_SIZE;

            buf_btm_ = off;
            ++count_;
        } else {
            ++off;
        }
    }

    if ((BUF_END_ - buf_top_) < 4*LEN) {
        buf_lock_.lock();
        size_t _s = buf_top_ - buf_btm_;
        memmove(read_buf_, buf_btm_, _s);
        buf_btm_ = read_buf_;
        buf_top_ = buf_btm_ + _s;
        buf_lock_.unlock();
    }
    // ui->textBrowser->setText(show);
}

void MainWindow::imshow(const cv::Mat& _img, bool auto_resize, QImage::Format format) {
    cv::Mat img = cv::Mat(_img);
    if (auto_resize)
        cv::resize(img, img, cv::Size(ui->imshow->width(), ui->imshow->height()));
    // cv::cvtColor(img, img, CV_BGR2RGB);
    QImage showImage((const uchar*)img.data, img.cols, img.rows, img.cols*img.channels(), format);
    ui->imshow->setPixmap(QPixmap::fromImage(showImage));
}

void MainWindow::connect_csr()
{
    cv::Mat img = data_->cvtCvMat(0); // cv::imread("/home/bibei/photo.jpg");
    std::cout << img.rows << " " << img.cols << std::endl;
    imshow(img);

    return;
    switch (socket->state()) {
    case QAbstractSocket::SocketState::UnconnectedState:
        socket->connectToHost("10.10.100.254", 8899, QTcpSocket::ReadOnly);
        break;
    case QAbstractSocket::SocketState::ConnectingState:
        QMessageBox::about(this, "Tip", "connecting...");
        break;
    case QAbstractSocket::SocketState::ConnectedState:
        QMessageBox::about(this, "Tip", "connected...");
        break;
    default:break;
    }
}

void MainWindow::close_csr()
{
    socket->close();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

void MainWindow::handle_state(QAbstractSocket::SocketState _s) {
    switch (_s) {
    case QAbstractSocket::SocketState::UnconnectedState: break;
    case QAbstractSocket::SocketState::HostLookupState:  break;
    case QAbstractSocket::SocketState::ConnectingState:
        status->setText("connecting...");
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        break;
    case QAbstractSocket::SocketState::ConnectedState:
        status->setText("connected...");
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        break;
    case QAbstractSocket::SocketState::BoundState:     break;
    case QAbstractSocket::SocketState::ListeningState: break;
    case QAbstractSocket::SocketState::ClosingState:
        status->setText("disconnected...");
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        break;
    default:break;
    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect,    &QAction::triggered, this,     &MainWindow::connect_csr);
    connect(ui->actionDisconnect, &QAction::triggered, this,     &MainWindow::close_csr);
    connect(ui->actionQuit,       &QAction::triggered, this,     &MainWindow::close);
    connect(ui->actionConfigure,  &QAction::triggered, settings, &MainWindow::show);
    // connect(ui->actionClear,      &QAction::triggered, console,  &Console::clear);
    // connect(ui->actionAbout,      &QAction::triggered, this,     &MainWindow::about);
    connect(ui->actionAboutQt,    &QAction::triggered, qApp,     &QApplication::aboutQt);

    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handle_state(QAbstractSocket::SocketState)));
    connect(this->socket, SIGNAL(readyRead()), this, SLOT(readyread()));

    connect(this, SIGNAL(readAllFromFile()), this, SLOT(readyread()));
}
