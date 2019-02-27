#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define _USE_MATH_DEFINES
#define UNDOSTEPS 5
#define PRESET_THRES_MORE_BONES 25
#define PRESET_THRES_LESS_BONES 40

#include <QMainWindow>
#include <QLabel>
#include <QTextEdit>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QInputDialog>
#include <QPixmap>
#include <QImage>
#include <QRgb>
#include <vector>
#include "volume.h"
#include "UniGrid.h"
#include "reader.h"
#include "rangedialog.h"

#include "opencv2/opencv.hpp"

#include "OpenMesh/Core/IO/MeshIO.hh"
#include "OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh"
#include "OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh"
#include "OpenMesh/Tools/Decimater/DecimaterT.hh"
#include "OpenMesh/Tools/Decimater/ModQuadricT.hh"


typedef OpenMesh::TriMesh_ArrayKernelT<> TriMesh;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void removebackground(cv:: Mat&);
    void removesmallarea(cv::Mat&,int);
public slots:
    void openvolume();
    void showimage();

    void showmesh();
    void setlevel(int);
    void setthreshold(int);
    void dilate2D();
    void erode2D();
    void open2D();
    void close2D();
    void erodeall();
    void dilateall();

    void dilate3D();
    void erode3D();
    void open3D();
    void close3D();

    void clean2D();
    void clean2Dall();
    void clean2Dall_process(int,int,int);

    void thresholdall();
    void thresholdall_process(int,int,int);

    void reset();
    void addhistory();

    void savemesh();
    void savevolume();

    void setisdrawcursor(int);

    void preset_more_bone();

    void smooth();
    void smooth_process(int);
    void simplification();

    void undo();

    void generate();
private:
    Ui::MainWindow *ui;

    //data
    Volume *mrcvolume;
    Volume *mrcvolume2;

    int nverts;
    int ntris;

    int sizex;
    int sizey;
    int sizez;
    int minvalue;
    int maxvalue;
    int currentlevel;

    bool isShowBone;
    TriMesh mesh;
    //parameter
    std::vector <int> thresholdvalue;
    std::vector <cv::Mat> grayimages;
    std::vector <cv::Mat> binaryimages;

    std::vector < std::vector < cv::Mat > > binary_history;
    std::vector <Volume *> mrcvolume_history;
    std::vector <TriMesh> mesh_history;

protected:
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
