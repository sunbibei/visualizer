#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>
#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <mutex>

#include "adt_eigen.h"

namespace Ui {
class MainWindow;
}

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void initActionsConnections();
    void imshow(const cv::Mat&, bool auto_resize = true, QImage::Format = QImage::Format_Grayscale8);


signals:
    void readAllFromFile();

private slots:
    void readyread();

    void parse();

    void connect_csr();
    void close_csr();

    void about();

    void handle_state(QAbstractSocket::SocketState);

private:
    // size_t parseFromData(const char *, size_t, size_t&, size_t&, double&, bool);

private:
    Ui::MainWindow *ui;

    QTcpSocket*     socket;
    QLabel*         status;
    SettingsDialog* settings;


    AdtEigen*   data_;

    // The buffer for read
    std::mutex        buf_lock_;
    const size_t      READ_BUF_SIZE;
    char*             read_buf_;
    char*             buf_btm_;
    char*             buf_top_;
    const char*       BUF_END_;

    size_t            count_;
    size_t            tmp_r_;
    size_t            tmp_c_;
    double            tmp_v_;
};

#endif // MAINWINDOW_H
