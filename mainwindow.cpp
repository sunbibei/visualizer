#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <chrono>

#include <QLabel>

#include <fstream>
#include <opencv2/opencv.hpp>

#include <stdio.h>

const size_t DELIMITER_SIZE = 1;
const char DELIMITER     = '\n';
const char START_CHAR    = 0x02; // STX
const char END_CHAR      = 0x03; // ETX
// const char START_CHAR       = '*';
// const char END_CHAR         = '!';

//FILE* fp = NULL; //open("tmp.txt");
//FILE* fp1 = NULL;

std::chrono::high_resolution_clock::time_point t0_;
std::chrono::high_resolution_clock::time_point t1_;


#ifdef USE_QT_3D
void MainWindow::setmodifier(SceneModifier* ptr) {
    modifier = ptr;
}


QWidget* MainWindow::getView() {
    return ui->container;
}
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), data_(nullptr),
    READ_BUF_SIZE(1024*1024), recv_state_(NO_CONNECT),
    state_data_(C_I), thread_alive_(false)
{
    ui->setupUi(this);
    settings  = new SettingsDialog;

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
    ui->actionStart->setEnabled(false);
    ui->actionStart_Multi->setEnabled(false);
    ui->actionStop->setEnabled(false);

    status = new QLabel;
    ui->statusBar->addWidget(status);

    ui->maxValue->setText("4");
    // ui->threshold->setText("5");

    dis_start = false;

    initPlot();
    initActionsConnections();

    exp_ = 3;

//    fp = fopen("tmp.txt", "w");
//    fp1 = fopen("tmp1.txt", "w");
}

MainWindow::~MainWindow()
{
//    if (QAbstractSocket::SocketState::ConnectedState == socket->state()) {
//        if (CONTINUE == recv_state_) start_stop();
//        close_csr();
//    }
    start_stop();
    close_csr();
    delete[] read_buf_;
    BUF_END_ = read_buf_ = buf_btm_ = buf_top_ = nullptr;
    delete ui;
    delete settings;
    if (data_) delete data_;

//    fclose(fp);
//    fclose(fp1);
}

//void MainWindow::readyread() {
//    buf_lock_.lock();
//    size_t read = socket->read(buf_top_, BUF_END_ - buf_top_);
//    if (0 == read) return;

//    buf_top_ += read;
//    buf_lock_.unlock();
//}

void MainWindow::readyread() {
    if (!dis_start) {
        dis_start = true;
        status->setText("starting to receive data...");
    }
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
    if ((tmp_r_ < data_->ROWS) && (tmp_c_ < data_->COLS)) {
        data_->update(count_, tmp_r_, tmp_c_, tmp_v_);
    } else
        ;// ui->textBrowser->append("ERROR SIZE");
}

void MainWindow::parse() {
    // int size = buf_top_ - buf_btm_;
    // ui->textBrowser->append("Enter: residual -- (" + QString::number(size) + "/1024)");

//    char* off1 = buf_btm_;
//    while (off1 != buf_top_) {
//        fprintf(fp, "0x%02X ", *off1);
//        fprintf(fp1, "%c ", *off1);
//        ++off1;
//    }
//    fprintf(fp, "\r\n");
//    fprintf(fp1, "\r\n");
//    buf_btm_ = buf_top_;
//    if ((BUF_END_ - buf_top_) < 44) {
//        // buf_lock_.lock();
//        size_t _s = buf_top_ - buf_btm_;
//        memmove(read_buf_, buf_btm_, _s);
//        buf_btm_ = read_buf_;
//        buf_top_ = buf_btm_ + _s;
//        // buf_lock_.unlock();
//    }
//    size = buf_top_ - buf_btm_;
//    return;
    const int LEN   = 11;
//    while (thread_alive_) {
        char* off = buf_btm_;
        while (off != buf_top_) {
            if (DELIMITER == *off) {
                if ((off - buf_btm_) != LEN) {
                    off += DELIMITER_SIZE;
                    buf_btm_ = off;
                    continue;
                }
                state_data_ = C_F;
                __parse_value(buf_btm_);
                off += DELIMITER_SIZE; // eat the '\n'
                buf_btm_ = off;
                // debug_str += QString::number(tmp_r_) + " " + QString::number(tmp_c_) + " " + QString::number(tmp_v_) + "\n";
                // ui->textBrowser->setText(debug_str);
            } else if (END_CHAR == *off) {
                while ((END_CHAR == *off) && (off < buf_top_)) ++off; // eat the all of the END_CHAR
                if (DELIMITER == *off) off += DELIMITER_SIZE;

                if (C_F != state_data_) continue;

//                t1_ = std::chrono::high_resolution_clock::now();
//                int64_t span = std::chrono::duration_cast<std::chrono::milliseconds>
//                        (t1_ - t0_).count();
//                ui->lineEdit_3->setText(QString::number(span));
                buf_btm_ = off;
                if (settings->settings().is_xml) {
                    if (SINGLE == recv_state_)
                        data_->saveOnce();
                   else
                        data_->save();

                } else {
                    if (SINGLE == recv_state_)
                        data_->saveOnceCSV();
                    else
                        data_->saveCSV();
                }

                // showData(count_);
                // vals_view_ = "";
                size_t c[2] = {0};
                bool is_cvt = false;
                double max   = -1;
                double thres = 5;
                if (!ui->maxValue->text().isEmpty()) {
                    max = ui->maxValue->text().toDouble(&is_cvt);
                    if (!is_cvt) ui->maxValue->setText("ERROR VALUE");
                    else max = -1;
                }
//                if (!ui->threshold->text().isEmpty()) {
//                    thres = ui->threshold->text().toDouble(&is_cvt);
//                    if (!is_cvt) ui->threshold->setText("ERROR VALUE");
//                    else thres = -1;
//                }

                bool is_ok = data_->whole_calc(img_, c[0], c[1], max, thres, exp_);
                if (SINGLE == recv_state_) {
                    // data_->clear();
                    // count_ = 0;
                    // data_->getCenter(c[0], c[1]);
                    if (is_ok) {
                        vals_view_ = "Center: (" + QString::number(c[0]) + ", " + QString::number(c[1]) + ")";
                    } else  {
                        vals_view_ = "Center  Error!";
                    }
                } else {
                    if ((0 == count_ % 10) && (is_ok)) plotCenter(c);
                    vals_view_ = "frame: " + QString::number(count_);
                    if (is_ok) {
                        vals_view_ += ", center: (" + QString::number(c[0]) + ", " + QString::number(c[1]) + ")";
                    } else {
                        vals_view_ += ", Center  Error!";
                    }
                }

                std::string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
                if (is_ok) {
                    ui->center_x->setText(QString::number(c[0]));
                    ui->center_y->setText(QString::number(c[1]));
                    center_ofd_ << time << "," << c[0] << "," << c[1] << ",";
                } else  {
                    ui->center_x->setText("");
                    ui->center_y->setText("");
                    center_ofd_ << time << ",-,-,";
                }
                center_ofd_ << (ui->model->text().isEmpty() ? "-" : ui->model->text().toStdString()) << ",";
                center_ofd_ << (ui->distance->text().isEmpty() ? "-" : ui->distance->text().toStdString()) << ",";
                center_ofd_ << (ui->alpha1->text().isEmpty() ? "-" : ui->alpha1->text().toStdString()) << ",";
                center_ofd_ << (ui->alpha2->text().isEmpty() ? "-" : ui->alpha2->text().toStdString()) << ",";
                center_ofd_ << (ui->maxValue->text().isEmpty() ? "-" : ui->maxValue->text().toStdString()) << ",";
                center_ofd_ << (ui->diameter->text().isEmpty() ? "-" : ui->diameter->text().toStdString()) << std::endl;

                status->setText(vals_view_ + " | " + QString::fromStdString(data_->getCurrentFileName()));
                imshow(img_, c[0], c[1]);
                ++count_;
                data_->clear();
//                t0_ = std::chrono::high_resolution_clock::now();
//                span = std::chrono::duration_cast<std::chrono::milliseconds>
//                                    (t0_ - t1_).count();
//                ui->lineEdit_4->setText(QString::number(span));
                // std::cout << "add " << count_;
                // ui->textBrowser->append("add " + QString::number(count_-1));
            } else if (START_CHAR == *off) {
                while ((START_CHAR == *off) && (off < buf_top_)) ++off; // eat the all of the END_CHAR
                if (DELIMITER == *off) off += DELIMITER_SIZE;
                state_data_ = C_S;
                buf_btm_ = off;

                // t0_ = std::chrono::high_resolution_clock::now();
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
        // size = buf_top_ - buf_btm_;
        // ui->textBrowser->append("Out:   parsed -- (" + QString::number(size) + "/1024)");
    //}
}

void MainWindow::showData(size_t) {
    // imshow(data_->cvtCvMat());
}

void MainWindow::imshow(const cv::Mat& _img, size_t c_x, size_t c_y, bool auto_resize, QImage::Format format) {
    //cv::imshow("test", _img);
    //return;
    cv::Mat img = _img.t();
    cv::flip(img, img, 0);

    if (auto_resize)
        cv::resize(img, img, cv::Size(ui->imshow->width(), ui->imshow->height()));
    // cv::cvtColor(img, img, CV_BGR2RGB);

    cv::putText(img, "0", cv::Point(20, img.rows - 20), 7, 1, cv::Scalar(255, 255, 255), 1);
    QImage showImage((const uchar*)img.data, img.cols, img.rows, img.cols*img.channels(), format);
    ui->imshow->setPixmap(QPixmap::fromImage(showImage));
}

int count = 0;
void MainWindow::connect_csr()
{
//    QVector<double> keys;
//    QVector<QVector<double>> vals;
//    vals.resize(N_G);
//    for (double t = 0; t < 3.14; t += 0.01) {
//        keys.push_back(t);
//        vals[0].push_back(sin(t)+0.1*count);
//        vals[1].push_back(cos(t)-0.1*count);
//    }
//    ++count;
//    plotData(keys, vals, true);
//    return;

//    showData(0);
//    cv::Mat img = data_->cvtCvMat(0); // cv::imread("/home/bibei/photo.jpg");
//    std::cout << img.rows << " " << img.cols << std::endl;
//    imshow(img);
//    return;
//    auto cfg = settings->settings();
//    data_  = new AdtEigen(cfg.height, cfg.width, cfg.min_val, cfg.max_val);
//    data_->setFile(cfg.data_path.toStdString());
//    img_ = cv::Mat::zeros(3*data_->ROWS, 12*data_->COLS, CV_8UC1);
//    data_->loadCSV("data.csv");
//    // data_->print();

//    size_t x, y;
//    data_->whole_calc(img_, x, y);
//    imshow(img_);
//    ui->statusBar->showMessage(QString("Center: (") + QString::number(x) + ", " + QString::number(y) + ")");
//    return;

    auto cfg = settings->settings();
    data_  = new AdtEigen(cfg.height, cfg.width, cfg.min_val, cfg.max_val);
    data_->setFile(cfg.data_path.toStdString());
    img_ = cv::Mat::zeros(exp_*2*data_->ROWS/2, exp_*2*2*data_->COLS, CV_8UC1);
    // showData(0);
//    cv::Mat img = data_->cvtCvMat(0); // cv::imread("/home/bibei/photo.jpg");
//    std::cout << img.rows << " " << img.cols << std::endl;
//    imshow(img);
    // return;
    // data_->loadCSV("/Users/bibei/Downloads/jg/20171225/test.csv");
    // data_->print();
//    size_t c[2] = {0};
//    bool is_cvt = false;
//    double max   = -1;
//    double thres = -1;
//    if (!ui->maxValue->text().isEmpty()) {
//        max = ui->maxValue->text().toDouble(&is_cvt);
//        if (!is_cvt) ui->maxValue->setText("ERROR VALUE");
//        else max = -1;
//    }
//    if (!ui->threshold->text().isEmpty()) {
//        thres = ui->threshold->text().toDouble(&is_cvt);
//        if (!is_cvt) ui->threshold->setText("ERROR VALUE");
//        else thres = -1;
//    }

//    bool is_ok = data_->whole_calc(img_, c[0], c[1], max, thres);
//    if (SINGLE == recv_state_) {
//        data_->clear();
//        count_ = 0;
//        // data_->getCenter(c[0], c[1]);
//        if (is_ok) {
//            vals_view_ = "Center: (" + QString::number(c[0]) + ", " + QString::number(c[1]) + ")";
//        } else  {
//            vals_view_ = "Center  Error!";
//        }
//    } else {
//        if (0 == count_ % 10) plotCenter(c);
//        vals_view_ = "frame: " + QString::number(count_);
//        if (is_ok) {
//            vals_view_ += ", center: (" + QString::number(c[0]) + ", " + QString::number(c[1]) + ")";
//        } else {
//            vals_view_ += ", Center  Error!";
//        }
//    }
//    if (is_ok) {
//        ui->center_x->setText(QString::number(c[0]));
//        ui->center_y->setText(QString::number(c[1]));
//    } else  {
//        vals_view_ = "Center  Error!";
//        ui->center_x->setText("");
//        ui->center_y->setText("");
//    }

//    status->setText(vals_view_ + " | " + QString::fromStdString(data_->getCurrentFileName()));
//    imshow(img_);
//    ++count_;
//    return;

    switch (socket->state()) {
    case QAbstractSocket::SocketState::UnconnectedState:
        socket->connectToHost(cfg.ip, cfg.port, QTcpSocket::ReadWrite);
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
        csrConnected();
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyread()));
        break;
    case QAbstractSocket::SocketState::BoundState:     break;
    case QAbstractSocket::SocketState::ListeningState: break;
    case QAbstractSocket::SocketState::ClosingState:
        status->setText("disconnected");
        csrClosed();
        break;
    default:break;
    }
}

void MainWindow::csrClosed() {
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionStart->setEnabled(false);
    ui->actionStart_Multi->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionConfigure->setEnabled(true);

    data_->clear();

    if (center_ofd_.is_open()) center_ofd_.close();
//    thread_alive_ = false;
//    parse_thread_.waitForFinished();
}

void MainWindow::csrConnected() {
    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
    ui->actionStart->setEnabled(true);
    ui->actionStart_Multi->setEnabled(true);
    ui->actionStop->setEnabled(true);
    ui->actionConfigure->setEnabled(false);

    data_->clear();

    std::string _path = settings->settings().data_path.toStdString();
    center_ofd_.open(_path + "/center.csv");

//    thread_alive_ = true;
//    parse_thread_ = QtConcurrent::run(this, &MainWindow::parse);
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect,    &QAction::triggered, this,     &MainWindow::connect_csr);
    connect(ui->actionDisconnect, &QAction::triggered, this,     &MainWindow::close_csr);
    connect(ui->actionStart,      &QAction::triggered, this,     &MainWindow::start_recv);
    connect(ui->actionStart_Multi,&QAction::triggered, this,     &MainWindow::start_recv_mutl);
    connect(ui->actionStop,       &QAction::triggered, this,     &MainWindow::start_stop);
    connect(ui->actionQuit,       &QAction::triggered, this,     &MainWindow::close);
    connect(ui->actionConfigure,  &QAction::triggered, settings, &MainWindow::show);
    connect(ui->actionClear,      &QAction::triggered, this,     &MainWindow::clear);
    // connect(ui->actionAbout,      &QAction::triggered, this,     &MainWindow::about);
    connect(ui->actionAboutQt,    &QAction::triggered, qApp,     &QApplication::aboutQt);

    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handle_state(QAbstractSocket::SocketState)));
    // connect(plot_timer_, SIGNAL(timeout()), this, SLOT(plotCenter()));
    // connect(this, SIGNAL(readAllFromFile()), this, SLOT(readyread()));
}

void MainWindow::start_recv() {
    if (QAbstractSocket::SocketState::ConnectedState == socket->state()) {
        QByteArray once(1, 0x0F);
        dis_start = false;
        socket->write(once);
        recv_state_ = SINGLE;
    } else {
        ui->actionStart->setEnabled(false);
    }
}

void MainWindow::start_recv_mutl() {
    if (QAbstractSocket::SocketState::ConnectedState == socket->state()) {
        QByteArray cont(1, 0xFF);
        dis_start = false;
        socket->write(cont);
        recv_state_ = CONTINUE;
        ui->actionStart->setEnabled(false);
        ui->actionStart_Multi->setEnabled(false);
        // plot_timer_->start(1000); // plot per 1s
    } else {
        ui->actionStart_Multi->setEnabled(false);
        // if (plot_timer_->isActive()) plot_timer_->stop();
    }
}

void MainWindow::start_stop() {
    if (QAbstractSocket::SocketState::ConnectedState == socket->state()) {
        QByteArray stop(1, 0x00);
        socket->write(stop);
        recv_state_ = STOP_RECV;
        ui->actionStart->setEnabled(true);
        ui->actionStart_Multi->setEnabled(true);
        // if (plot_timer_->isActive()) plot_timer_->stop();
    } else {
        ui->actionStop->setEnabled(false);
    }
}


void MainWindow::initPlot() {
    plot[G_X] = ui->plotx;
    plot[G_Y] = ui->ploty;
    // plot_timer_ = new QTimer(this);
    plot[G_X]->xAxis->setRange(0, 24);
    plot[G_X]->yAxis->setRange(0, 44);

    plot[G_Y]->xAxis->setRange(0, 24);
    plot[G_Y]->yAxis->setRange(0, 32);

    keys_.reserve(60*24);
    vals_.resize(N_G);
    // create graph and assign data to it:
    for (int i = G_X; i < N_G; ++i) {
        plot[i]->legend->setVisible(true);
        plot[i]->legend->setFont(QFont("Helvetica",9));
        graphs[i] = plot[i]->addGraph();
        vals_[i].reserve(60*24);
    }

    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    pen.setColor(Qt::red);
    graphs[G_X]->setPen(pen);
    graphs[G_X]->setName("x");

    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    pen.setColor(Qt::blue);
    graphs[G_Y]->setPen(pen);
    graphs[G_Y]->setName("y");
}

void MainWindow::plotData(const QVector<double>& _keys, const QVector<QVector<double>>& _vals, bool is_clear) {
    for (int i = 0; i < _vals.size(); ++i) {
        if (is_clear) graphs[i]->setData(_keys, _vals[i]);
        else graphs[i]->addData(_keys, _vals[i]);
        plot[i]->replot();
    }
}

void MainWindow::plotCenter(size_t* pc) {
    size_t c[2] = {0};
    if (nullptr != pc) {
        c[0] = pc[0];
        c[1] = pc[1];
    } else
        data_->getCenter(c[0], c[1]);

    QTime current_time =QTime::currentTime();
    double key   = current_time.hour() + current_time.minute()/60.0;
    for (int i = 0; i < N_G; ++i) {
        graphs[i]->addData(key, c[i]);
        plot[i]->replot();
    }

    keys_.push_back(key);
    vals_[G_X].push_back(c[0]);
    vals_[G_Y].push_back(c[1]);
}

void MainWindow::clear() {
    QVector<double> keys;
    QVector<double> vals;
    for (int i = G_X; i < N_G; ++i) {
        graphs[i]->setData(keys, vals);
    }

    plot[G_X]->replot();
    plot[G_Y]->replot();
}
