#include "mainwindow.h"
#include <QApplication>
#include <Eigen/Dense>
#include <iostream>
#include "adt_eigen.h"

#include <opencv/cv.hpp>
#include <QDateTime>

int main(int argc, char *argv[])
{
//    QDateTime current_date_time = QDateTime::currentDateTime();
//    QString current_date = current_date_time.toString("yyyy-MM-dd.hh-mm");
//    std::cout << current_date.toStdString() << std::endl;
//    return 0;
//    AdtEigen test(10, 10);

//    auto& m = test.data_ref(0);
//    for (int r = 0; r < test.COLS; ++r) {
//        for (int c = 0; c < test.ROWS; ++c) {
//            test.update(0, r, c, r%10 + (c*0.01));
//        }
//    }

//    cv::Mat img = test.cvtCvMat(0);
//    cv::namedWindow("window", CV_WINDOW_NORMAL);
//    cv::imshow("window",img);
//    cv::waitKey(0);
//    return 0;

//    test.save(0);
//    Eigen::MatrixXd m_r;
//    std::cout << test.load(0, m_r) << std::endl;
//    std::cout << m_r << std::endl;
//    return 0;

 //   cv::Mat img = cv::imread("/Users/bibei/Downloads/IMG_0015.JPG");
//    cv::namedWindow("window", CV_WINDOW_NORMAL);
//    cv::imshow("window",img);
//    cv::waitKey(0);
//    return 0;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
