#-------------------------------------------------
#
# Project created by QtCreator 2017-12-03T19:55:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Visualizer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    adt.cpp \
    adt_eigen.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    adt.h \
    adt_eigen.h \
    settingsdialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

QT += network

win32 {

}

macx {
QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -mmacosx-version-min=10.12
QMAKE_LFLAGS += -mmacosx-version-min=10.12

INCLUDEPATH += /usr/local/include \
/usr/local/include/eigen3

LIBS += -L/usr/local/lib \
-ltinyxml \
-lopencv_core \
-lopencv_highgui \
-lopencv_imgproc \
-lopencv_video \
-lopencv_objdetect \
-lopencv_videoio \
-lopencv_imgcodecs \
-lopencv_ml

}

linux {
INCLUDEPATH += /usr/include \
/usr/include/eigen3 \
/opt/ros/kinetic/include/opencv-3.2.0-dev

LIBS += -L/usr/lib -L/opt/ros/kinetic/lib \
-ltinyxml \
-lopencv_core3 \
-lopencv_highgui3 \
-lopencv_imgproc3 \
-lopencv_video3 \
-lopencv_objdetect3 \
-lopencv_videoio3 \
-lopencv_imgcodecs3 \
-lopencv_ml3 \
-lopencv_features2d3
}

RESOURCES += \
    visualizer.qrc
