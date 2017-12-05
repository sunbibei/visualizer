#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTcpSocket>
#include <QString>
#include <QDataStream>
#include <QByteArray>

#include "adt_eigen.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void connected();
    void readyread();

    void on_btn_connect_clicked();
    void on_pushButton_clicked();

private:
    void parseFromData(double, size_t&, size_t&, double&);

private:
    Ui::MainWindow *ui;

    QTcpSocket* socket;
    AdtEigen*   data_;
    std::string str_buf_;
};

#endif // MAINWINDOW_H
