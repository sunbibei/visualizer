#include "mainwindow.h"
#include <QApplication>
#include <Eigen/Dense>
#include <iostream>
#include "adt_eigen.h"

#include <opencv2/opencv.hpp>
#include <QDateTime>

#ifdef USE_QT_3D
#include "scenemodifier.h"
#include <Qt3DRender/qcamera.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcameralens.h>

#include <Qt3DInput/QInputAspect>

#include <Qt3DExtras/qtorusmesh.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/qpointlight.h>

#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DExtras/qforwardrenderer.h>

#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qfirstpersoncameracontroller.h>
#include <Qt3DExtras/qt3dwindow.h>
#endif
int main(int argc, char *argv[])
{
//    size_t ROWS = 8;
//    size_t COLS = 16;
//    Eigen::MatrixXd curr;
//    curr.resize(ROWS, COLS);
//    curr << 1, 2, 3, 2, 4, 2, 6, 32, 43, 12, 43, 65, 65,  12, 423, 43,
//            3, 2, 3, 4, 5, 6, 12,43, 56, 23, 23, 76, 45, 344, 544, 54,
//            3, 5, 3, 6, 7, 2, 12,12, 32, 54, 34, 23, 43, 545, 453, 23,
//            1, 2, 5, 6, 7, 2, 1, 32, 43, 54, 23, 23, 68, 454, 234, 76,
//            1, 2, 3, 2, 4, 2, 2, 43, 66, 76, 34, 76, 76, 345, 346, 23,
//            3, 2, 3, 4, 5, 6, 2, 54, 65, 23, 54, 23, 65, 767, 566, 23,
//            3, 5, 3, 6, 7, 2, 3, 54, 34, 23, 12, 23, 34, 545, 656, 56,
//            1, 2, 5, 6, 7, 2, 7, 45, 78, 67, 56, 87, 45, 767, 454, 45;


//    double mean = curr.mean();
//    double sqMean = curr.array().square().mean();
//    double var  = sqMean - mean*mean;
//    std::cout << "mean: " << mean << std::endl;
//    std::cout << "var:  " << var  << std::endl;

//    double thr  = mean + 5*sqrt(var);
////    Eigen::VectorXd r_corrs(20);
////    Eigen::VectorXd c_corrs(20);
//    std::vector<size_t> r_corrs;
//    r_corrs.reserve(32);
//    size_t mean_r = 0;
//    std::vector<size_t> c_corrs;
//    c_corrs.reserve(32);
//    size_t mean_c = 0;
//    std::cout << "ready into loop, thr=" << thr << std::endl;
//    for (size_t _r = 0; _r < ROWS; ++_r) {
//        const auto& rows = curr.row(_r);
//        for (size_t _c = 0; _c < COLS; ++_c) {
//            std::cout << rows(_c) << " ";
//            if (rows(_c) > thr) {
//                r_corrs.push_back(_r);
//                c_corrs.push_back(_c);
//                mean_r += _r;
//                mean_c += _c;
//            }
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "out the loop, " << r_corrs.size() << ", " << c_corrs.size() << std::endl;
//    if (!r_corrs.empty()) mean_r /= r_corrs.size();
//    if (!c_corrs.empty()) mean_c /= c_corrs.size();
//    std::cout << "center: (" << mean_r << ", " << mean_c << ")" << std::endl;
//    // curr.

//    return 0;
//    cv::namedWindow("test", CV_WINDOW_NORMAL);
//    cv::setOpenGlDrawCallback("test", on_opengl, nullptr);
//    for (int i = 0; i < 10; ++i) {
//        cv::updateWindow("test");
//        cv::waitKey(1000);
//    }
//    return 0;
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

//    cv::Mat img = cv::imread("E:/Workspaces/Visualizer/images/IconsLand_038.png");
//    cv::resize(img, img, cv::Size(128, 128));
//    cv::imwrite("E:/Workspaces/Visualizer/images/stop_new.png", img);

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
#ifdef USE_QT_3D
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    QWidget *container = QWidget::createWindowContainer(view);
    QSize screenSize = view->screen()->size();
    QSize widgetSize = w.getView()->size();
    container->setMinimumSize( widgetSize );
    container->setMaximumSize(screenSize);

    QHBoxLayout *hLayout = new QHBoxLayout(w.getView());
    hLayout->addWidget(container, 1);
    // hLayout->addLayout(vLayout);

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    view->registerAspect(input);

    // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

    // Camera
    Qt3DRender::QCamera *cameraEntity = view->camera();

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 20.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(cameraEntity->position());
    lightEntity->addComponent(lightTransform);

    // For camera controls
    Qt3DExtras::QFirstPersonCameraController *camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(cameraEntity);

    // Scenemodifier
    SceneModifier *modifier = new SceneModifier(rootEntity);

    // Set root object of the scene
    view->setRootEntity(rootEntity);
#endif
    w.show();
    // view->resize(screenSize);
    return a.exec();
}
