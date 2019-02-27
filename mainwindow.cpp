#include "mainwindow.h"
#include "ui_mainwindow.h"

#define DEFAULTTHRES 40
#include <QDebug>
using namespace std;
using namespace cv;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    currentlevel = 0;
    //link ui
    ui->label_threshold->setText(QString("%1").arg(ui->slider_threshold->value()));

    //signal and slots
    connect(ui->actionOpen_Volume,SIGNAL(triggered()),this,SLOT(openvolume()));
    connect(ui->actionGenerate,SIGNAL(triggered()),this,SLOT(generate()));
    connect(ui->actionDilate2D,SIGNAL(triggered()),this,SLOT(dilate2D()));
    connect(ui->actionErode2D,SIGNAL(triggered()),this,SLOT(erode2D()));
    connect(ui->actionOpen2D,SIGNAL(triggered()),this,SLOT(open2D()));
    connect(ui->actionClose2D,SIGNAL(triggered()),this,SLOT(close2D()));
    connect(ui->actionDilate_All,SIGNAL(triggered()),this,SLOT(dilateall()));
    connect(ui->actionErode_All,SIGNAL(triggered()),this,SLOT(erodeall()));
    connect(ui->actionDilate3D,SIGNAL(triggered()),this,SLOT(dilate3D()));
    connect(ui->actionErode3D,SIGNAL(triggered()),this,SLOT(erode3D()));
    connect(ui->actionOpen3D,SIGNAL(triggered()),this,SLOT(open3D()));
    connect(ui->actionClose3D,SIGNAL(triggered()),this,SLOT(close3D()));
    connect(ui->actionClean2D,SIGNAL(triggered()),this,SLOT(clean2D()));
    connect(ui->actionClean_all,SIGNAL(triggered()),this,SLOT(clean2Dall()));
    connect(ui->actionThreshold_All,SIGNAL(triggered()),this,SLOT(thresholdall()));
    connect(ui->actionReset,SIGNAL(triggered()),this,SLOT(reset()));
    connect(ui->actionSave_Mesh,SIGNAL(triggered()),this,SLOT(savemesh()));
    connect(ui->actionSave_Volume,SIGNAL(triggered()),this,SLOT(savevolume()));
    connect(ui->actionUndo,SIGNAL(triggered()),this,SLOT(undo()));
    connect(ui->actionSmooth3D,SIGNAL(triggered()),this,SLOT(smooth()));
    connect(ui->actionSimplification,SIGNAL(triggered()),this,SLOT(simplification()));
    connect(ui->actionAuto_Generate,SIGNAL(triggered()),this,SLOT(preset_more_bone()));

    connect(ui->checkBox,SIGNAL(stateChanged(int)),SLOT(setisdrawcursor(int)));

    connect(ui->combo_frame,SIGNAL(currentIndexChanged(int)),SLOT(setlevel(int)));
    connect(ui->slider_threshold,SIGNAL(valueChanged(int)),SLOT(setthreshold(int)));
    mrcvolume = NULL;
    mrcvolume2 = NULL;
    nverts = 0;
    ntris = 0;
    minvalue = 0;
    maxvalue = 0;


    ui->checkBox->setCheckState(Qt::Unchecked);

    mesh.request_vertex_normals();
    mesh.request_face_normals();
    mesh.request_vertex_status();
    mesh.request_face_status();
}

MainWindow::~MainWindow()
{
    delete mrcvolume;
    delete mrcvolume2;
    delete ui;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(mrcvolume == NULL)
        return;
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees;
    if(currentlevel + numSteps >= 0 && currentlevel + numSteps < sizez)
    ui->combo_frame->setCurrentIndex(currentlevel + numSteps);
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(mrcvolume == NULL)
        return;
    if(event->key() == Qt::Key_Up)
    {
        if(currentlevel > 0)
            ui->combo_frame->setCurrentIndex(currentlevel - 1);
    }
    if(event->key() == Qt::Key_Down)
    {
        if(currentlevel < sizez - 1)
            ui->combo_frame->setCurrentIndex(currentlevel + 1);
    }
    event->accept();
}

void MainWindow::setlevel(int level)
{
    if(level <0 || level>sizez || !mrcvolume)
        return;
    currentlevel = level;
    ui->slider_threshold->setValue(thresholdvalue[currentlevel]);
    float planelevel = (float)level * mrcvolume->getRatioz()/mrcvolume->getRatioxy();
    ui->glwidget->drawplane(planelevel);
    showimage();
}

void MainWindow::setthreshold(int thres)
{
    ui->label_threshold->setText(QString("%1").arg(thres));
    if(thresholdvalue.size() > currentlevel)
        thresholdvalue[currentlevel] = thres;
    for(int y=0;y<sizey;y++)
    {
        for(int x=0;x<sizex;x++)
        {
            if(mrcvolume2->getDataAt(x,y,currentlevel) < thres)
                binaryimages[currentlevel].at<Vec3b>(y,x) = Vec3b(255,255,255);
            else
                binaryimages[currentlevel].at<Vec3b>(y,x) = Vec3b(0,0,0);
        }
    }
    removebackground(binaryimages[currentlevel]);
    showimage();
}

void MainWindow::showimage()
{
    if(grayimages.size() > currentlevel && binaryimages.size() > currentlevel)
    {
        QImage grayimg((uchar*)grayimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
        QPixmap pix,pixscaled,pixbinary,pixbinaryscaled;

        pix.convertFromImage(grayimg);
        pixscaled = pix.scaledToHeight(ui->img_gray->height());
        ui->img_gray->setPixmap(pixscaled);


        QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
        pixbinary.convertFromImage(binaryimg);
        pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
        ui->img_binary->setPixmap(pixbinaryscaled);
    }
}

void MainWindow::removebackground(Mat & m)
{
    //remove background
    int *queue = new int[sizex*sizey*2];
    bool *isSearched = new bool[sizex*sizey];
    bool *largest = new bool[sizex*sizey];
    int largestarea = -1;
    int head = 0,tail = 0;

    for(int i=0;i<sizex*sizey;i++)
        isSearched[i] = false;

    for(int y=0;y<sizey;y++)
    {
        for(int x=0;x<sizex;x++)
        {
            int index = y*sizex + x;
            if(m.at<Vec3b>(y,x) == Vec3b(255,255,255) && !isSearched[index])
            {
                head = 0; tail = 0;
                int curarea = 1;
                queue[head] = index;
                head++;
                isSearched[index] = true;
                while(head>tail)
                {
                    int v = queue[tail];
                    tail++;
                    int curx = v % sizex;
                    int cury = v / sizex;
                    for(int dx=-1;dx<=1;dx++)
                    {
                        for(int dy=-1;dy<=1;dy++)
                        {
                            if(dx == 0 && dy == 0)
                                continue;
                            if(dx * dy != 0)
                                continue;
                            if(curx+dx >= 0 && curx+dx < sizex && cury+dy >= 0 && cury+dy < sizey
                                    && m.at<Vec3b>(cury+dy,curx+dx) == Vec3b(255,255,255) && !isSearched[(cury+dy)*sizex + curx+dx])
                            {
                                queue[head] = (cury+dy)*sizex + curx+dx;
                                head++;
                                isSearched[(cury+dy)*sizex + curx+dx] = true;
                                curarea++;
                            }
                        }
                    }
                }
                if(curarea > largestarea)
                {
                    largestarea = curarea;
                    for(int i=0;i<sizex*sizey;i++) largest[i] = false;
                    for(int i=0;i<head;i++)
                        largest[queue[i]] = true;
                }
            }
        }
    }


    for(int i=0;i<sizex*sizey;i++)
    {
        int curx = i%sizex;
        int cury = i/sizex;
        if(largest[i])
            m.at<Vec3b>(cury,curx) = Vec3b(0,0,0);

    }

    delete[]queue;
    delete[]isSearched;
    delete[]largest;
}

void MainWindow::removesmallarea(Mat & mat, int minarea)
{
    int head=0,tail=0;
    bool *isSearched = new bool[sizex*sizey];
    int *queue = new int[sizex*sizey];
    for(int i=0;i<sizex*sizey;i++)
        isSearched[i] = false;

    for(int y=0;y<sizey;y++)
    {
        for(int x=0;x<sizex;x++)
        {
            if(mat.at<Vec3b>(y,x) == Vec3b(255,255,255) && !isSearched[y*sizex+x])
            {
                int curarea = 0;
                head = 0;tail = 0;
                queue[head++] = y*sizex+x;
                curarea++;
                isSearched[y*sizex+x] = true;
                while(tail<head)
                {
                    int curind = queue[tail++];
                    int curx = curind % sizex;
                    int cury = curind / sizex;
                    for(int dx = -1;dx<=1;dx++)
                    {
                        for(int dy=-1;dy<=1;dy++)
                        {
                            if(dx == 0 && dy == 0)
                                continue;
                            int nextx = curx + dx;
                            int nexty = cury + dy;
                            if(nextx>=0 && nextx<sizex && nexty>=0 && nexty<sizey )
                            {
                                if(!isSearched[nexty*sizex+nextx] && mat.at<Vec3b>(nexty,nextx) == Vec3b(255,255,255))
                                {
                                    int nextind = nexty * sizex + nextx;
                                    isSearched[nextind] = true;
                                    queue[head++] = nextind;
                                    curarea++;
                                }
                            }
                        }
                    }
                }
                if(curarea < minarea)
                {
                    for(int i=0;i<head;i++)
                        mat.at<Vec3b>(queue[i]/sizex,queue[i]%sizex) = Vec3b(0,0,0);
                }
            }
        }
    }

    delete[] queue;
    delete[] isSearched;
}

void MainWindow::openvolume()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Mesh File"),"/Users/yanhang/Documents/research");
    if(filename.length()>0)
    {
        if(mrcvolume != NULL)
            delete mrcvolume;
        binaryimages.clear();
        grayimages.clear();

        QProgressDialog *progress = new QProgressDialog(this);
        progress->setModal(true);
        progress->setMinimum(0);
        progress->setMaximum(100);
        progress->setLabelText("Opening Volume...");
        progress->setAutoClose(true);

        QByteArray ba = filename.toLocal8Bit();
        char *filenamechar = ba.data();
        MRCReader *reader = new MRCReader(filenamechar);


        mrcvolume = reader->getVolume(progress);
        if(mrcvolume == NULL)
            return;

        sizex = mrcvolume->getSizeX();
        sizey = mrcvolume->getSizeY();
        sizez = mrcvolume->getSizeZ();
        minvalue = mrcvolume->getMin();
        maxvalue = mrcvolume->getMax();

        mrcvolume2 = new Volume(sizex,sizey,sizez,0,0,0,mrcvolume);

        ui->slider_threshold->setMaximum(maxvalue);
        ui->slider_threshold->setMinimum(minvalue);

        thresholdvalue.resize(sizez);
        progress->setLabelText("Preprocessing...");
        progress->setRange(0,sizez);
        for(int z=0;z<sizez;z++)
        {
            progress->setValue(z);
            thresholdvalue[z] = DEFAULTTHRES;
            Mat curimg(sizey,sizex,CV_8UC3);
            Mat curbinary(sizey,sizex,CV_8UC3);
            for(int y=0;y<sizey;y++)
            {
                for(int x=0;x<sizex;x++)
                {
                    int grayscale = (int)mrcvolume->getDataAt(x,y,z)/((float)maxvalue-(float)minvalue)*255;
                    curimg.at<Vec3b>(y,x) = Vec3b(grayscale,grayscale,grayscale);
                    if(mrcvolume->getDataAt(x,y,z) < thresholdvalue[z])
                        curbinary.at<Vec3b>(y,x) = Vec3b(255,255,255);
                    else
                        curbinary.at<Vec3b>(y,x) = Vec3b(0,0,0);
                }
            }
            removebackground(curbinary);
            grayimages.push_back(curimg);
            binaryimages.push_back(curbinary);
            ui->combo_frame->addItem(QString("Level %1").arg(z));
        }
        setlevel(0);
        ui->slider_threshold->setValue(DEFAULTTHRES);
        delete progress;
    }
}

void MainWindow::generate()
{
    if(mrcvolume == NULL)
    {
        QMessageBox::critical(this,"Generate","No volume opened!");
        return;
    }
    mrcvolume->copyFromeImages(binaryimages);
    showmesh();
}

void MainWindow::showmesh()
{
    UniGrid *grid = new UniGrid(mrcvolume);
    QProgressDialog *progress = new QProgressDialog(this);
    progress->setModal(true);
    progress->setLabelText("Contouring...");
    progress->setRange(0,100);
    progress->setAutoClose(true);

    grid->genContour(0.5,progress);

    for(TriMesh::FaceIter f_it = mesh.faces_begin();f_it!=mesh.faces_end();++f_it)
        mesh.delete_face(*f_it,false);
    for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
        mesh.delete_vertex(*v_it,false);
    mesh. garbage_collection();

    grid->getMesh(mesh);

    if(mesh.n_vertices() == 0)
    {
        QMessageBox::warning(this,"No mesh generated!","No mesh generated!");
        return;
    }
    ui->glwidget->loaddata(mesh);
    delete grid;
    delete progress;
}

void MainWindow::dilate2D()
{
    if(binaryimages.size() <= currentlevel)
        return;
    bool isOK;
    int radius = QInputDialog::getInt(this,"Dilate Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(radius,radius));
    dilate(binaryimages[currentlevel],binaryimages[currentlevel],element);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::erode2D()
{
    if(binaryimages.size() <= currentlevel)
        return;
    bool isOK;
    int radius = QInputDialog::getInt(this,"Erode Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(radius,radius));
    erode(binaryimages[currentlevel],binaryimages[currentlevel],element);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::open2D()
{
    if(binaryimages.size() <= currentlevel)
        return;

    bool isOK;
    int radius = QInputDialog::getInt(this,"Open Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(radius,radius));
    erode(binaryimages[currentlevel],binaryimages[currentlevel],element);
    dilate(binaryimages[currentlevel],binaryimages[currentlevel],element);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::close2D()
{
    if(binaryimages.size() <= currentlevel)
        return;
    bool isOK;
    int radius = QInputDialog::getInt(this,"Close Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(radius,radius));
    dilate(binaryimages[currentlevel],binaryimages[currentlevel],element);
    erode(binaryimages[currentlevel],binaryimages[currentlevel],element);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::erodeall()
{
    if(binaryimages.size() <= currentlevel)
        return;

    int minlevel,maxlevel,radius;
    rangedialog *setting = new rangedialog(this);
    setting->setmax(binaryimages.size()-1);
    setting->setmin(0);
    if(setting->exec()==QDialog::Accepted)
    {
        minlevel = setting->getmin();
        maxlevel = setting->getmax();
        radius = setting->getvalue();
    }
    else
        return;

    addhistory();
    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(radius,radius));
    if(maxlevel >= binaryimages.size())
    {
        QMessageBox::critical(this,"Error","Invalid range!");
        return;
    }
    for(int i=minlevel;i<maxlevel;i++)
        erode(binaryimages[i],binaryimages[i],element);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::dilateall()
{
    if(binaryimages.size() <= currentlevel)
        return;

    int minlevel,maxlevel,radius;
    rangedialog *setting = new rangedialog(this);
    setting->setmax(binaryimages.size()-1);
    setting->setmin(0);
    if(setting->exec()==QDialog::Accepted)
    {
        minlevel = setting->getmin();
        maxlevel = setting->getmax();
        radius = setting->getvalue();
    }
    else
        return;

    addhistory();

    Mat element = getStructuringElement(MORPH_ELLIPSE,Size(radius,radius));

    if(maxlevel >= binaryimages.size())
    {
        QMessageBox::critical(this,"Error","Invalid range!");
        return;
    }

    for(int i=minlevel;i<maxlevel;i++)
        dilate(binaryimages[i],binaryimages[i],element);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::dilate3D()
{
    if(!mrcvolume->getisBinary())
    {
        QMessageBox::warning(NULL,"Dilate Operation","Generate the binary volume first!");
        return;
    }
    bool isOK;
    int radius = QInputDialog::getInt(this,"Dilate Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    mrcvolume->dilate(radius);
    mrcvolume->copyToImages(binaryimages);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

    showmesh();
}

void MainWindow::erode3D()
{
    if(!mrcvolume->getisBinary())
    {
        QMessageBox::warning(NULL,"Erode Operation","Generate the binary volume first!");
        return;
    }
    bool isOK;
    int radius = QInputDialog::getInt(this,"Erode Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    mrcvolume->erode(radius);
    mrcvolume->copyToImages(binaryimages);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

    showmesh();
}

void MainWindow::open3D()
{
    if(!mrcvolume->getisBinary())
    {
        QMessageBox::warning(NULL,"Open Operation","Generate the binary volume first!");
        return;
    }
    bool isOK;
    int radius = QInputDialog::getInt(this,"Open Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    mrcvolume->erode(radius);
    mrcvolume->dilate(radius);
    mrcvolume->copyToImages(binaryimages);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

    showmesh();
}

void MainWindow::close3D()
{

    if(!mrcvolume->getisBinary())
    {
        QMessageBox::warning(NULL,"Close Operation","Generate the binary volume first!");
        return;
    }
    bool isOK;
    int radius = QInputDialog::getInt(this,"Close Operation","Please input the radius.",1,0,1000,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    mrcvolume->dilate(radius);
    mrcvolume->erode(radius);
    mrcvolume->copyToImages(binaryimages);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

    showmesh();
}

void MainWindow::clean2D()
{
    if(binaryimages.size() <= currentlevel)
        return;
    bool isOK;
    int minarea = QInputDialog::getInt(this,"Clean Operation","Minimum area: ",2,0,2147483467,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    removesmallarea(binaryimages[currentlevel],minarea);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::clean2Dall()
{
    if(binaryimages.size() <= currentlevel)
        return;

    int minlevel,maxlevel,minarea;
    rangedialog *setting = new rangedialog(this);
    setting->setmax(binaryimages.size()-1);
    setting->setmin(0);
    if(setting->exec()==QDialog::Accepted)
    {
        minlevel = setting->getmin();
        maxlevel = setting->getmax();
        minarea = setting->getvalue();
    }
    else
        return;

    addhistory();

    clean2Dall_process(minlevel,maxlevel,minarea);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);
}

void MainWindow::clean2Dall_process(int minlevel, int maxlevel,int minarea)
{
    for(int i=minlevel;i<maxlevel;i++)
        removesmallarea(binaryimages[i],minarea);
}

void MainWindow::thresholdall()
{
    if(binaryimages.size() <= currentlevel)
        return;

    int minlevel,maxlevel,thres;
    rangedialog *setting = new rangedialog(this);
    setting->setmax(binaryimages.size()-1);
    setting->setmin(0);
    if(setting->exec()==QDialog::Accepted)
    {
        minlevel = setting->getmin();
        maxlevel = setting->getmax();
        thres = setting->getvalue();
    }
    else
        return;
    addhistory();
    thresholdall_process(minlevel,maxlevel,thres);
    ui->slider_threshold->setValue(thresholdvalue[currentlevel]);
    ui->label_threshold->setText(QString("%1").arg(thresholdvalue[currentlevel]));
    showimage();
}

void MainWindow::thresholdall_process(int minlevel, int maxlevel, int thres)
{
    for(int i=minlevel;i<maxlevel;i++)
    {
        for(int y=0;y<sizey;y++)
        {
            for(int x=0;x<sizex;x++)
            {
                if(mrcvolume2->getDataAt(x,y,i) < thres)
                    binaryimages[i].at<Vec3b>(y,x) = Vec3b(255,255,255);
                else
                    binaryimages[i].at<Vec3b>(y,x) = Vec3b(0,0,0);
            }
        }
        removebackground(binaryimages[i]);
        thresholdvalue[i] = thres;
    }
}

void MainWindow::reset()
{
    if(mrcvolume2 == NULL)
    {
        QMessageBox::critical(this,"Reset","Fatal error, cannot reset!");
        return;
    }
    delete mrcvolume;
    mrcvolume  = new Volume(sizex,sizey,sizez,0,0,0,mrcvolume2);
    binaryimages.clear();

    for(int z=0;z<sizez;z++)
    {
        thresholdvalue[z] = DEFAULTTHRES;
        Mat curbinary(sizey,sizex,CV_8UC3);
        for(int y=0;y<sizey;y++)
        {
            for(int x=0;x<sizex;x++)
            {
                if(mrcvolume->getDataAt(x,y,z) < thresholdvalue[z])
                    curbinary.at<Vec3b>(y,x) = Vec3b(255,255,255);
                else
                    curbinary.at<Vec3b>(y,x) = Vec3b(0,0,0);
            }
        }
        removebackground(curbinary);
        binaryimages.push_back(curbinary);
    }
    ui->slider_threshold->setValue(thresholdvalue[currentlevel]);
    ui->label_threshold->setText(QString("%1").arg(thresholdvalue[currentlevel]));
    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

}

void MainWindow::savemesh()
{
    if(mesh.n_vertices() == 0)
    {
        QMessageBox::critical(this,"Save Mesh","No mesh opened!");
        return;
    }
    QString filename = QFileDialog::getSaveFileName();
    if(filename.length() > 0)
    {
        OpenMesh::IO::write_mesh(mesh,filename.toStdString());
    }
}

void MainWindow::savevolume()
{
    if(mrcvolume == NULL)
    {
        QMessageBox::critical(this,"Save Volume","No volume opened!");
        return;
    }
    QString filename = QFileDialog::getSaveFileName();
    if(filename.length() > 0)
    {
        OpenMesh::IO::write_mesh(mesh,filename.toStdString());
        QByteArray ba = filename.toLocal8Bit();
        char *filenamechar = ba.data();
        mrcvolume->toMRCFile(filenamechar);
    }
}

void MainWindow::setisdrawcursor(int state)
{
    if(state == Qt::Unchecked)
        ui->glwidget->setisDrawPlane(false);
    if(state == Qt::Checked)
        ui->glwidget->setisDrawPlane(true);
}

void MainWindow::addhistory()
{
    if(binary_history.size() == UNDOSTEPS)
        binary_history.erase(binary_history.begin());
    if(mrcvolume_history.size() == UNDOSTEPS)
        mrcvolume_history.erase(mrcvolume_history.begin());
    if(mesh_history.size() == UNDOSTEPS)
        mesh_history.erase(mesh_history.begin());

    vector <Mat> cursnapshot;
    for(int i=0;i<binaryimages.size();i++)
        cursnapshot.push_back(binaryimages[i].clone());
    binary_history.push_back(cursnapshot);

    Volume *volsnapshot = NULL;
    if(mrcvolume != NULL)
        volsnapshot = new Volume(sizex,sizey,sizez,0,0,0,mrcvolume);
    mrcvolume_history.push_back(volsnapshot);

    TriMesh temp = mesh;
    mesh_history.push_back(temp);
}

void MainWindow::undo()
{
    if(binary_history.size() == 0 || mrcvolume_history.size() == 0 || mesh_history.size() == 0)
        return;
    vector <Mat> binaryback = binary_history.back();
    for(int i=0;i<binaryimages.size();i++)
        binaryimages[i] = binaryback[i].clone();
    binary_history.pop_back();

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

    Volume * mrcback = mrcvolume_history.back();
    delete mrcvolume;
    mrcvolume = NULL;
    if(mrcback != NULL)
        mrcvolume = new Volume(sizex,sizey,sizez,0,0,0,mrcback);
    delete mrcback;
    mrcvolume_history.pop_back();

    mesh = mesh_history.back();
    mesh_history.pop_back();
    if(mesh.n_vertices() > 0)
        ui->glwidget->loaddata(mesh);

}

void MainWindow::smooth_process(int iter)
{
    QProgressDialog *progress = new QProgressDialog();
    progress->setLabelText("Smoothing...");
    progress->setModal(true);
    progress->setRange(0,100);

    int unitcount = mesh.n_vertices() * 2 * iter / 100;
    int progresscount = 0;
    TriMesh mesh2;
    for(int i=0;i<iter;i++)
    {
        mesh2 = mesh;
        for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
        {
            if(progresscount % unitcount == 0)
                progress->setValue(progresscount / unitcount);
            progresscount++;
            TriMesh::Point pointacc(0.0,0.0,0.0);
            TriMesh::Point curpt = mesh.point(*v_it);
            int count = 0;
            for(TriMesh::VertexVertexIter vv_it = mesh.vv_iter(*v_it);vv_it.is_valid();++vv_it)
            {
                pointacc += mesh2.point(*vv_it);
                count++;
            }
            float lamda = 0.1;
            pointacc[0] = pointacc[0] / (double)count;
            pointacc[1] = pointacc[1] / (double)count;
            pointacc[2] = pointacc[2] / (double)count;
            curpt[0] = (1.0-lamda) * pointacc[0] + lamda*curpt[0];
            curpt[1] = (1.0-lamda) * pointacc[1] + lamda*curpt[1];
            curpt[2] = (1.0-lamda) * pointacc[2] + lamda*curpt[2];
            mesh.set_point(*v_it,curpt);
        }
        mesh2 = mesh;
        for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
        {
            if(progresscount % unitcount == 0)
                progress->setValue(progresscount / unitcount);
            progresscount++;
            TriMesh::Point pointacc(0.0,0.0,0.0);
            TriMesh::Point curpt = mesh.point(*v_it);
            int count = 0;
            for(TriMesh::VertexVertexIter vv_it = mesh.vv_iter(*v_it);vv_it.is_valid();++vv_it)
            {
                pointacc += mesh2.point(*vv_it);
                count++;
            }
            const float lamda = 1.0/(0.1-1/0.1);
            pointacc[0] = pointacc[0] / (double)count;
            pointacc[1] = pointacc[1] / (double)count;
            pointacc[2] = pointacc[2] / (double)count;
            curpt[0] = (1.0-lamda)*pointacc[0] + lamda*curpt[0];
            curpt[1] = (1.0-lamda)*pointacc[1] + lamda*curpt[1];
            curpt[2] = (1.0-lamda)*pointacc[2] + lamda*curpt[2];
            mesh.set_point(*v_it,curpt);
        }
    }
    delete progress;
}

void MainWindow::smooth()
{
    if(mesh.n_vertices() == 0)
            return;

    bool isOK;
    int iter = QInputDialog::getInt(this,"Smoothing","Iteration",2,1,30,1,&isOK);
    if(!isOK)
        return;
    addhistory();
    smooth_process(iter);

    ui->glwidget->loaddata(mesh);
}

void MainWindow::simplification()
{
    if(mesh.n_vertices() == 0)
        return;

    bool isOK;
    int targetnum = QInputDialog::getInt(this,"Simplification",QString("Target vertices(current %1): ").arg(mesh.n_vertices()),mesh.n_vertices(),1,2147483647,1,&isOK);
    if(!isOK)
        return;
    addhistory();

    mesh.release_vertex_normals();
    mesh.release_face_normals();
    mesh.release_vertex_status();
    mesh.release_face_status();


    OpenMesh::Decimater::DecimaterT <TriMesh> decimater(mesh);
    OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle hModQuadric;
    decimater.add(hModQuadric);
    decimater.module(hModQuadric).unset_max_err();
    decimater.initialize();
    decimater.decimate_to(targetnum);

    mesh.request_vertex_normals();
    mesh.request_face_normals();
    mesh.request_vertex_status();
    mesh.request_face_status();

    mesh.update_normals();
    ui->glwidget->loaddata(mesh);
}

void MainWindow::preset_more_bone()
{
    if(mrcvolume == NULL || binaryimages.size() < sizez)
        return;
    thresholdall_process(0,sizez - 1,PRESET_THRES_LESS_BONES);
    clean2Dall_process(0,sizez-1,45);

    QPixmap pixbinary,pixbinaryscaled;
    QImage binaryimg((uchar*)binaryimages[currentlevel].data,sizex,sizey,QImage::Format_RGB888);
    pixbinary.convertFromImage(binaryimg);
    pixbinaryscaled = pixbinary.scaledToHeight(ui->img_binary->height());
    ui->img_binary->setPixmap(pixbinaryscaled);

    generate();
}
