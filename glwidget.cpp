/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"

#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <iostream>
#include <math.h>
#include <time.h>
#include <QKeyEvent>
#include <eigen3/Eigen/Eigen>

#define BUFFER_OFFSET(i) ((uchar*)NULL + (i))
#define max(a,b) a>b?a:b

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    xRot = 0;
    yRot = 0;
    zRot = 0;
    xTranslate = 0.0;
    yTranslate = 0.0;
    zTranslate = -10.0;
    scale = 1.0;
    xmin = 999; ymin = 999; zmin = 999;
    xmax = -999; zmax = -999, zmax = -999;
    isRotate = true;
    isTranslate = false;
    isScale = false;
    modelscale = 100.0;
    planelevel = 20.0;
    isDrawplane = false;
    facenum = 0;
}

GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteBuffers(bufferObjectNum,bo);
    //SafeRelease(vIndex);
    //SafeRelease(vPosition);
    //SafeRelease(vNormal);
}

void GLWidget::SafeRelease(void *ptr)
{
    if (ptr != NULL)
        delete[] ptr;
}


void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::initializeGL()
{
    GLfloat light_position[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
    GLfloat light2_position[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
    GLfloat light3_position[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

    GLfloat light_diffuse[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat light_ambient[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

    GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_diffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_specular[4] = { 0.2f,0.2f, 0.2f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);

    glLightfv(GL_LIGHT1, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT1,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT1,GL_SPECULAR,light_specular);

    glLightfv(GL_LIGHT2, GL_POSITION, light3_position);
    glLightfv(GL_LIGHT2,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT2,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT2,GL_SPECULAR,light_specular);


    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mat_specular);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_DEPTH_TEST);

    //glEnable(GL_NORMALIZE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(xTranslate,yTranslate,zTranslate);
    glScalef(scale,scale,scale);
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);

    GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_diffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_specular[4] = { 0.2f,0.2f, 0.2f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mat_specular);

    clock_t start,end;
    start = clock();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bo[index]);
    glDrawElements(GL_TRIANGLES,facenum * 3,GL_UNSIGNED_INT,BUFFER_OFFSET(0));

    //draw plane
    if(isDrawplane)
    {
        float zlevel = (planelevel- (zmin+zmax)/2) / modelscale;

        glEnable(GL_COLOR_MATERIAL);
        glBegin(GL_QUADS);
        glColor3f(0.0,0.3,0.0);
        glNormal3f(0.0,0.0,1.0);
        glVertex3f(-1.0,-1.0,zlevel);
        glVertex3f(-1.0,1.0,zlevel);
        glVertex3f(1.0,1.0,zlevel);
        glVertex3f(1.0,-1.0,zlevel);
        glEnd();
        glDisable(GL_COLOR_MATERIAL);
    }
    end = clock();

    glFlush();
}

void GLWidget::drawplane(float level)
{
    planelevel = level;
    updateGL();
}

void GLWidget::setisDrawPlane(bool d)
{
    isDrawplane = d;
    updateGL();
}

void GLWidget::reset()
{
    glDeleteBuffers(bufferObjectNum,bo);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    facenum = 0;
}

void GLWidget::loaddata(int nverts, int ntris, float *verts, int *tris)
{   
    reset();
    resizeGL(this->width(),this->height());

    glGenBuffers(bufferObjectNum,bo);
    qDebug() << "Loading data...";
    //face
    facenum = ntris;

    qDebug() << "Binding index...";

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bo[index]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,3 * facenum*sizeof(GLuint),(GLuint*)tris,GL_STATIC_DRAW);

    qDebug() << "Binding position...";
    xmin = 999; ymin=999; zmin=999; xmax=-999; ymax=-999; zmax=-999;
    for(int i=0;i<nverts;i++)
    {
        xmin = verts[i*3] < xmin ? verts[i*3] : xmin;
        xmax = verts[i*3] > xmax ? verts[i*3] : xmax;
        ymin = verts[i*3+1] < ymin ? verts[i*3+1] : ymin;
        ymax = verts[i*3+1] > ymax ? verts[i*3+1] : ymax;
        zmin = verts[i*3+2] < zmin ? verts[i*3+2] : zmin;
        zmax = verts[i*3+2] > zmax ? verts[i*3+2] : zmax;
    }
    float scalex = xmax - xmin;
    float scaley = ymax - ymin;
    float scale = max(scalex,scaley) / 2;
    for(int i=0;i<nverts;i++)
    {
        verts[i*3] = (verts[i*3] - (xmin+xmax)/2) / scale;
        verts[i*3 + 1] = (verts[i*3 + 1] - (ymin+ymax)/2) / scale;
        verts[i*3 + 2] = (verts[i*3 + 2] - (zmin+zmax)/2) / scale;
    }
    //vertex
    glBindBuffer(GL_ARRAY_BUFFER,bo[vertex]);
    glBufferData(GL_ARRAY_BUFFER,3*nverts*sizeof(GLfloat),(GLfloat*)verts,GL_STATIC_DRAW);
    glVertexPointer(3,GL_FLOAT,0,BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);

    //compute vertex normal
    std::vector <Eigen::Vector3f> vnormal(nverts);
    for(int i=0;i<nverts;i++)
        vnormal[i] = Eigen::Vector3f::Zero();
    for(int i=0;i<ntris;i++)
    {
        Eigen::Vector3f v1(verts[tris[i*3]*3],verts[tris[i*3]*3+1],verts[tris[i*3]]*3+2);
        Eigen::Vector3f v2(verts[tris[i*3+1]*3],verts[tris[i*3+1]*3+1],verts[tris[i*3+1]]*3+2);
        Eigen::Vector3f v3(verts[tris[i*3+2]*3],verts[tris[i*3+2]*3+1],verts[tris[i*3+2]]*3+2);
        Eigen::Vector3f diff1 = v2 - v1;
        Eigen::Vector3f diff2 = v3 - v2;
        Eigen::Vector3f curnormal = diff1.cross(diff2);
        curnormal.normalize();
        vnormal[tris[i*3]] += curnormal;
        vnormal[tris[i*3+1]] += curnormal;
        vnormal[tris[i*3+2]] += curnormal;
    }
    for(int i=0;i<nverts;i++)
    {
        Eigen::Vector3f curdir(verts[i*3],verts[i*3+1],verts[i*3+2]);
        if(vnormal[i].dot(curdir) < 0)
            vnormal[i] = -1*vnormal[i];
        vnormal[i].normalize();
    }

    qDebug() << "Binding normal...";
    //normal
    glBindBuffer(GL_ARRAY_BUFFER,bo[normal]);
    glBufferData(GL_ARRAY_BUFFER,3*nverts*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    GLfloat *glnormal = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
    for(int i=0;i<nverts;i++)
    {
        glnormal[i * 3 + 0] = (GLfloat)vnormal[i][0];
        glnormal[i * 3 + 1] = (GLfloat)vnormal[i][1];
        glnormal[i * 3 + 2] = (GLfloat)vnormal[i][2];
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glNormalPointer(GL_FLOAT,0,BUFFER_OFFSET(0));
    glEnableClientState(GL_NORMAL_ARRAY);
    qDebug() <<"Upload complete!";
    updateGL();
}

void GLWidget::loaddata(TriMesh mesh)
{
    reset();

    resizeGL(this->width(),this->height());
;
    xmin = 999; ymin=999; zmin=999; xmax=-999; ymax=-999; zmax=-999;
    for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
    {
        TriMesh::Point curpt = mesh.point(*v_it);
        xmin = curpt[0]<xmin?curpt[0]:xmin;
        xmax = curpt[0]>xmax?curpt[0]:xmax;
        ymin = curpt[1]<ymin?curpt[1]:ymin;
        ymax = curpt[1]>ymax?curpt[1]:ymax;
        zmin = curpt[2]<zmin?curpt[2]:zmin;
        zmax = curpt[2]>zmax?curpt[2]:zmax;
    }
    float scalex = xmax - xmin;
    float scaley = ymax - ymin;
    modelscale = max(scalex,scaley) / 2;


    for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
    {
        TriMesh::Point curpt = mesh.point(*v_it);
        curpt[0] = (curpt[0] - (xmin+xmax)/2) / modelscale;
        curpt[1] = (curpt[1] - (ymin+ymax)/2) / modelscale;
        curpt[2] = (curpt[2] - (zmin+zmax)/2) / modelscale;
        mesh.set_point(*v_it,curpt);
    }

    glGenBuffers(bufferObjectNum,bo);

    //face
    facenum = mesh.n_faces();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bo[index]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,3 * facenum*sizeof(GLuint),NULL,GL_STATIC_DRAW);
    GLuint *glindex = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,GL_WRITE_ONLY);
    int ind = 0;
    for(TriMesh::FaceIter f_it = mesh.faces_begin();f_it!=mesh.faces_end();++f_it)
    {
        for(TriMesh::FaceVertexIter fv_it = mesh.fv_iter(f_it);fv_it.is_valid();++fv_it)
        {
            glindex[ind] = (GLuint)fv_it->idx();
            ind++;
        }
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    //vertex
    ind = 0;
    glBindBuffer(GL_ARRAY_BUFFER,bo[vertex]);
    glBufferData(GL_ARRAY_BUFFER,3*mesh.n_vertices()*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    GLfloat *glvertex = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
    for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
    {
        TriMesh::Point curpt = mesh.point(*v_it);
        glvertex[ind * 3 + 0] = (GLfloat)curpt[0];
        glvertex[ind * 3 + 1] = (GLfloat)curpt[1];
        glvertex[ind * 3 + 2] = (GLfloat)curpt[2];
        ind++;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glVertexPointer(3,GL_FLOAT,0,BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);

    //normal
    ind = 0;
    glBindBuffer(GL_ARRAY_BUFFER,bo[normal]);
    glBufferData(GL_ARRAY_BUFFER,3*mesh.n_vertices()*sizeof(GLfloat),NULL,GL_STATIC_DRAW);
    GLfloat *glnormal = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
    for(TriMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end();++v_it)
    {
        TriMesh::Normal curnormal = mesh.normal(*v_it);
        glnormal[ind * 3 + 0] = (GLfloat)curnormal[0];
        glnormal[ind * 3 + 1] = (GLfloat)curnormal[1];
        glnormal[ind * 3 + 2] = (GLfloat)curnormal[2];
        ind++;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glNormalPointer(GL_FLOAT,0,BUFFER_OFFSET(0));
    glEnableClientState(GL_NORMAL_ARRAY);

    updateGL();
}

void GLWidget::resizeGL(int width, int height)
{
    glMatrixMode(GL_VIEWPORT);
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 20.0);

    updateGL();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    if(isRotate)
    {
        if (event->buttons() & Qt::LeftButton) {
            setXRotation(xRot + 8 * dy);
            setYRotation(yRot + 8 * dx);
        } else if (event->buttons() & Qt::RightButton) {
            setXRotation(xRot + 8 * dy);
            setZRotation(zRot + 8 * dx);
        }
    }
    if(isTranslate)
    {
        xTranslate+=((float)dx+0.0)/100;
        yTranslate-=((float)dy+0.0)/100;
        updateGL();
    }
    if(isScale)
    {
        scale += ((float)dy+0.0)/100.0;
        updateGL();
    }

    lastPos = event->pos();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        isRotate = false;
        isTranslate = true;
        isScale = false;
    }
    if(event->key() == Qt::Key_Shift)
    {
        isRotate = false;
        isTranslate = false;
        isScale = true;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control || event->key() == Qt::Key_Shift)
    {
        isRotate = true;
        isTranslate = false;
        isScale = false;
    }
}

void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}
