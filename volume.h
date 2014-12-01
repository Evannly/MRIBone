/**
 * Volumetric data definition
 *
 * Author: Tao Ju
 * Date: 02/16/2005
 */


#ifndef VOLUME_H
#define VOLUME_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <queue>

#include <QMessageBox>
#include <QProgressDialog>

#include "dbh.h"
#include <opencv2/opencv.hpp>
using namespace std;
typedef struct _point3D{
    _point3D(int dx,int dy,int dz){x=dx;y=dy;z=dz;}
    _point3D(){}
    int x;
    int y;
    int z;
}point3D;

class Volume
{
public:
	/* Initialization */
    Volume(int x,int y,int z);
    Volume(int x,int y,int z,float val);
    Volume( int x, int y, int z, int offx, int offy, int offz, Volume* vol );
    /* Destruction */
    ~Volume( )
    {
        delete data ;
        delete binarydata;
    }
	
	/* Statistics function */
	int getSizeX( )
	{
		return sizex ;
	}
	int getSizeY( )
	{
		return sizey ;
	}
	int getSizeZ( )
	{
		return sizez ;
	}

	void setSpacing( float sx, float sy, float sz )
	{
		spcx = sx ;
		spcy = sy ;
		spcz = sz ;
	}

	float getSpacingX( )
	{
		return spcx ;
	}
	float getSpacingY( )
	{
		return spcy ;
	}
	float getSpacingZ( )
	{
		return spcz ;
	}
    float getRatioxy()
    {
        return ratioxy;
    }
    void setRatioxy(float d)
    {
        ratioxy = d;
    }
    void setRatioz(float d)
    {
        ratioz = d;
    }
    float getRatioz()
    {
        return ratioz;
    }
    bool getisBinary()
    {
        return isBinary;
    }
    bool getisNormalized()
    {
        return isNormalized;
    }
    /* Set data at a pixel */
    void setDataAt( int x, int y, int z, float d )
    {
        data[ x * sizey * sizez + y * sizez + z ] = d ;
    }
    
    void setDataAt(int ind,float d)
    {
        data[ind] = d;
    }
    
    
    /* Get data at a single voxel */
    float getDataAt( int x, int y, int z )
    {
        return data[ x * sizey * sizez + y * sizez + z ] ;
    }
    float getDataAt( int index )
    {
        return data[ index ] ;
    }
    bool getBinaryAt(int index)
    {
        return binarydata[index];
    }
    int coordtoind(int x,int y,int z)
    {
        return x*sizey*sizez + y*sizez + z;
    }
    void indtocoord(int &x,int &y,int &z,int ind)
    {
        x = ind/sizey/sizez;
        ind -= x*sizey*sizez;
        y = ind/sizez;
        ind -= y*sizez;
        z = ind;
    }
    bool isInside(int x,int y,int z)
    {
        if(x<sizex && x>=0 && y<sizey && y>=0 && z<sizez && z>=0)
            return true;
        else
            return false;
    }

    float getMin();
    
    float getMax();


	/**
	 * Normalize to a given range 
	 */
    void threshold( float thres );
    void thresholdSliced(std::vector<int>thres);
    void copyFromeImages(std::vector<cv::Mat>binary);
    void copyToImages(std::vector<cv::Mat>&binary);
    void invert();
    void getLargestComponent(bool, int, int, int, int,int,int);
    void getLargestComponentSliced(bool,int,int,int,int,int,int);
    
    void erode(int r);
    void erodeSlice(int,int,int);
    void dilate(int r);
    void dilateSlice(int,int,int);
    
    void normalize( float min, float max );
    void normalize( float min, float max, float thresh, float ithresh );
    void normalizeSliced();
    
    void subtract(Volume *a);

	/* Write to file */
    void toMRCFile( char* fname );
    
private:

	/* Sizes */
	int sizex, sizey, sizez ;

	/* Data array */
	float * data ;
    bool *binarydata;
	/* Spacing */
	float spcx, spcy, spcz ;
    float ratioxy;
    float ratioz;
    bool isBinary;
    bool isNormalized;
};


#endif
