#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QLabel>
#include <fstream>
#include <opencv2/opencv.hpp>

const size_t DELIMITER_SIZE = 1;
const char DELIMITER     = '\n';
const char START_CHAR    = 0x02; // STX
const char END_CHAR      = 0x03; // ETX
// const char START_CHAR       = '*';
// const char END_CHAR         = '!';


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), data_(nullptr),
    READ_BUF_SIZE(1024*16)
{
    ui->setupUi(this);
    settings = new SettingsDialog;

    socket = new QTcpSocket(this);
    // data_  = new AdtEigen(90, 16);

    ///! initialize the buffer.
    read_buf_ = new char[READ_BUF_SIZE];
    memset(read_buf_, 0x00, READ_BUF_SIZE*sizeof(char));
    buf_top_ = read_buf_;
    buf_btm_ = read_buf_;
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
    if (data_) delete data_;
}

//void MainWindow::readyread() {
//    buf_lock_.lock();
//    size_t read = socket->read(buf_top_, BUF_END_ - buf_top_);
//    if (0 == read) return;

//    buf_top_ += read;
//    buf_lock_.unlock();
//}

void MainWindow::readyread() {
    size_t read = socket->read(buf_top_, BUF_END_ - buf_top_);
    // ui->textBrowser->append("recv: " + QString::number(read));
    if (0 == read) return;

    buf_top_ += read;
    parse();
}

inline void MainWindow::__parse_value(char* _raw) {
    tmp_r_ = (_raw[0] - '0')*10 + (_raw[1]-'0');
    tmp_c_ = (_raw[2] - '0')*10 + (_raw[3]-'0');
    tmp_v_ = (_raw[4] - '0')+ 0.1*(_raw[6]-'0')
            +0.01  *(_raw[7]-'0')+0.001  *(_raw[8] -'0')
            +0.0001*(_raw[9]-'0')+0.00001*(_raw[10]-'0');
    if ((tmp_r_ < data_->ROWS) && (tmp_c_ < data_->COLS))
        data_->update(count_, tmp_r_, tmp_c_, tmp_v_);
    else
        ;// ui->textBrowser->append("ERROR SIZE");
}

void MainWindow::parse() {
    int size = buf_top_ - buf_btm_;
    // ui->textBrowser->append("Enter: residual -- (" + QString::number(size) + "/1024)");

    const int LEN   = 11;
    char* off = buf_btm_;
    while (off != buf_top_) {
        if (DELIMITER == *off) {
            if ((off - buf_btm_) != LEN) {
                off += DELIMITER_SIZE;
                buf_btm_ = off;
                continue;
            }
            __parse_value(buf_btm_);
            off += DELIMITER_SIZE; // eat the '\n'
            buf_btm_ = off;
            // debug_str += QString::number(tmp_r_) + " " + QString::number(tmp_c_) + " " + QString::number(tmp_v_) + "\n";
            // ui->textBrowser->setText(debug_str);
        } else if (END_CHAR == *off) {
            while ((END_CHAR == *off) && (off < buf_top_)) ++off; // eat the all of the END_CHAR
            if (DELIMITER == *off) off += DELIMITER_SIZE;

            buf_btm_ = off;
            showData(count_);
            ++count_;
            // ui->textBrowser->append("add " + QString::number(count_-1));
        } else if (START_CHAR == *off) {
            while ((START_CHAR == *off) && (off < buf_top_)) ++off; // eat the all of the END_CHAR
            if (DELIMITER == *off) off += DELIMITER_SIZE;

            buf_btm_ = off;
        } else {
            ++off;
        }
    }

    if ((BUF_END_ - buf_top_) < 4*LEN) {
        // buf_lock_.lock();
        size_t _s = buf_top_ - buf_btm_;
        memmove(read_buf_, buf_btm_, _s);
        buf_btm_ = read_buf_;
        buf_top_ = buf_btm_ + _s;
        // buf_lock_.unlock();
    }
    size = buf_top_ - buf_btm_;
    // ui->textBrowser->append("Out:   parsed -- (" + QString::number(size) + "/1024)");
}

void MainWindow::showData(size_t) {
    imshow(data_->cvtCvMat());
}

void MainWindow::imshow(const cv::Mat& _img, bool auto_resize, QImage::Format format) {
    //cv::imshow("test", _img);
    //return;
    cv::Mat img = cv::Mat(_img);
    if (auto_resize)
        cv::resize(img, img, cv::Size(ui->imshow->width(), ui->imshow->height()));
    // cv::cvtColor(img, img, CV_BGR2RGB);
    QImage showImage((const uchar*)img.data, img.cols, img.rows, img.cols*img.channels(), format);
    ui->imshow->setPixmap(QPixmap::fromImage(showImage));
}

void MainWindow::connect_csr()
{
//    cv::Mat img = data_->cvtCvMat(0); // cv::imread("/home/bibei/photo.jpg");
//    std::cout << img.rows << " " << img.cols << std::endl;
//    imshow(img);

//    return;
    auto cfg = settings->settings();
    data_  = new AdtEigen(cfg.height, cfg.width);
    data_->setFile(cfg.data_file.toStdString());
    switch (socket->state()) {
    case QAbstractSocket::SocketState::UnconnectedState:
        socket->connectToHost(cfg.ip, cfg.port, QTcpSocket::ReadOnly);
        break;
    case QAbstractSocket::SocketState::ConnectingState:
        QMessageBox::about(this, "Tip", "正在连接...");
        break;
    case QAbstractSocket::SocketState::ConnectedState:
        QMessageBox::about(this, "Tip", "当前已经连接");
        break;
    default:break;
    }
}

void MainWindow::close_csr()
{
    socket->close();
    status->setText("disconnected");
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
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
        status->setText("connected");
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyread()));
        break;
    case QAbstractSocket::SocketState::BoundState:     break;
    case QAbstractSocket::SocketState::ListeningState: break;
    case QAbstractSocket::SocketState::ClosingState:
        status->setText("disconnected");
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
    // connect(this, SIGNAL(readAllFromFile()), this, SLOT(readyread()));
}
