#-------------------------------------------------
#
# Project created by QtCreator 2014-11-20T21:13:16
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MRIBone
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    Cube.cpp \
    dbh.cpp \
    TriangulationTree.cpp \
    UniGrid.cpp \
    volume.cpp \
    glwidget.cpp \
    rangedialog.cpp

HEADERS  += mainwindow.h \
    Cube.h \
    curvature.h \
    Cycle.h \
    dbh.h \
    geocommon.h \
    HashMap.h \
    LinkedList.h \
    PLYWriter.h \
    reader.h \
    TriangulationTree.h \
    UniGrid.h \
    volume.h \
    glwidget.h \
    rangedialog.h


FORMS    += mainwindow.ui \
    rangedialog.ui

LIBS += -L/usr/local/lib/ -lopencv_core -lopencv_highgui -lopencv_imgproc
LIBS += -L/usr/local/lib/OpenMesh/ -lOpenMeshCore -lOpenMeshTools

INCLUDEPATH += /usr/local/include
