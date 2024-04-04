#-------------------------------------------------
#
# Project created by QtCreator 2017-03-05T12:30:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencvtest
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


SOURCES +=\
        main.cpp \
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += C:\Users\Korisnik\Desktop\ETF\opencv\build\include

LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_core490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_highgui490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_imgcodecs490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_imgproc490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_features2d490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_calib3d490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_video490.dll
LIBS += C:\Users\Korisnik\Desktop\ETF\opencv-build\bin\libopencv_videoio490.dll

# more correct variant, how set includepath and libs for mingw
# add system variable: OPENCV_SDK_DIR=D:/opencv/opencv-build/install
# read http://doc.qt.io/qt-5/qmake-variable-reference.html#libs

#INCLUDEPATH += $$(OPENCV_SDK_DIR)/include

#LIBS += -L$$(OPENCV_SDK_DIR)/x86/mingw/lib \
#        -lopencv_core320        \
#        -lopencv_highgui320     \
#        -lopencv_imgcodecs320   \
#        -lopencv_imgproc320     \
#        -lopencv_features2d320  \
#        -lopencv_calib3d320

RESOURCES += \
    resurs.qrc
