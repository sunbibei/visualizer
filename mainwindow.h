#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>
#include <QString>
#include <QDataStream>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
#include <mutex>

#include <fstream>

#include "adt_eigen.h"
#include "qcustomplot.h"
#include "scenemodifier.h"

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


#ifdef USE_QT_3D
    QWidget* getView();
    void setmodifier(SceneModifier* ptr);
#endif
private:
    void initActionsConnections();

    void showData(size_t);
    void imshow(const cv::Mat&, bool auto_resize = true, QImage::Format = QImage::Format_Grayscale8);
    void __parse_value(char* _raw);

signals:
    void readAllFromFile();

private slots:
    void readyread();
    void plotCenter(size_t *pc = nullptr);

    void parse();

    void connect_csr();
    void close_csr();
    void start_stop();
    void start_recv();
    void start_recv_mutl();
    void clear();

    void about();

    void handle_state(QAbstractSocket::SocketState);

private:
    // size_t parseFromData(const char *, size_t, size_t&, size_t&, double&, bool);
    void initPlot();
    void plotData(const QVector<double>& _keys, const QVector<QVector<double>>& _vals, bool is_clear=false);

    void csrClosed();
    void csrConnected();
private:
    Ui::MainWindow *ui;

    enum {
        G_X = 0,
        G_Y,
        N_G,
    };
    QCustomPlot*    plot[N_G];
    QCPGraph*       graphs[N_G];
    QTimer*         plot_timer_;
    QVector<double> keys_;
    QVector<QVector<double>> vals_;

    std::ofstream   center_ofd_;

    QString         vals_view_;

    cv::Mat         img_;
    int             exp_;
    enum {
        C_I = -1,
        C_S = 0,
        C_F,
        C_E,
    } state_data_;

    bool            dis_start;
    enum {
        NO_CONNECT = -1,
        SINGLE = 0,
        CONTINUE,
        STOP_RECV,
        N_RECV,
    } recv_state_;

    QTcpSocket*     socket;
    QLabel*         status;
    SettingsDialog* settings;
#ifdef USE_QT_3D
    SceneModifier *modifier;
#endif
    AdtEigen*   data_;

    QFuture<void>     parse_thread_;
    bool              thread_alive_;
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
