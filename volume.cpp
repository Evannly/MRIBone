#include "volume.h"
using namespace cv;
Volume::Volume( int x, int y, int z )
{
    sizex = x ;
    sizey = y ;
    sizez = z ;
    
    spcx = 1 ;
    spcy = 1 ;
    spcz = 1 ;
    
    data = new float [ x * y * z ] ;
    for ( int i = 0 ; i < x * y * z ; i ++ )
        data[ i ] = 0 ;
    binarydata = new bool [x * y * z];
    isBinary = false;
    isNormalized = false;
}

Volume::Volume( int x, int y, int z, float val )
{
    sizex = x ;
    sizey = y ;
    sizez = z ;
    
    spcx = 1 ;
    spcy = 1 ;
    spcz = 1 ;
    
    data = new float [ x * y * z ] ;
    binarydata = new bool [x*y*z];
    for ( int i = 0 ; i < x * y * z ; i ++ )
    {
        data[ i ] = val ;
    }
};

Volume::Volume( int x, int y, int z, int offx, int offy, int offz, Volume* vol )
{
    sizex = x ;
    sizey = y ;
    sizez = z ;
    
    spcx = vol->getSpacingX() ;
    spcy = vol->getSpacingY() ;
    spcz = vol->getSpacingZ() ;
    
    data = new float [ x * y * z ] ;
    binarydata = new bool[x * y * z];
    
    int ct = 0 ;
    for ( int i = offx ; i < x + offx; i ++ )
        for ( int j = offy ; j < y + offy; j ++ )
            for ( int k = offz ; k < z + offz; k ++ )
            {
                data[ ct ] = vol->getDataAt( i, j, k ) ;
                binarydata[ct] = vol->getBinaryAt(vol->coordtoind(i, j, k));
                ct ++ ;
            }
    isBinary = vol->getisBinary();
    isNormalized = vol->getisNormalized();
}

float Volume::getMin()
{
    int size = sizex * sizey * sizez ;
    float rvalue = data[0] ;
    for ( int i = 1 ; i < size ; i ++ )
    {
        if ( rvalue > data[ i ] )
        {
            rvalue = data[ i ] ;
        }
    }
    return rvalue ;
}

float Volume::getMax()
{
    int size = sizex * sizey * sizez ;
    float rvalue = data[0] ;
    for ( int i = 1 ; i < size ; i ++ )
    {
        if ( rvalue < data[ i ] )
        {
            rvalue = data[ i ] ;
        }
    }
    return rvalue ;
}

void Volume::threshold( float thres )
{
    int size = sizex * sizey * sizez ;
    for ( int i = 0 ; i < size ; i ++ )
    {
        binarydata[i] = data[i] > thres ? true : false;
        data[ i ] = data[ i ] > thres ? 1.0 : 0.0 ;
    }
    isBinary = true;
}

void Volume::thresholdSliced(std::vector<int> thres)
{
    for(int z=0;z<sizez;z++)
    {
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
            {
                if(getDataAt(x,y,z) > thres[z])
                {
                    setDataAt(x,y,z,1.0);
                    binarydata[coordtoind(x,y,z)] = true;
                }else
                {
                    setDataAt(x,y,z,0.0);
                    binarydata[coordtoind(x,y,z)] = false;
                }
            }
        }
    }
    isBinary = true;
}

void Volume::copyFromeImages(std::vector<Mat> binary)
{
    for(int z=0;z<sizez;z++)
    {
        for(int y=0;y<sizey;y++)
        {
            for(int x=0;x<sizex;x++)
            {
                if(binary[z].at<Vec3b>(y,x) == Vec3b(255,255,255))
                {
                    setDataAt(x,y,z,1.0);
                    binarydata[coordtoind(x,y,z)] = true;
                }
                else
                {
                    setDataAt(x,y,z,0.0);
                    binarydata[coordtoind(x,y,z)] = false;
                }
            }
        }
    }
    isBinary = true;
}

void Volume::copyToImages(std::vector<Mat> &binary)
{
    if(!isBinary)
        return;
    for(int z=0;z<sizez;z++)
    {
        for(int y=0;y<sizey;y++)
        {
            for(int x=0;x<sizex;x++)
            {
                if(getDataAt(x,y,z) == 1.0)
                {
                    binary[z].at<Vec3b>(y,x) = Vec3b(255,255,255);
                }
                else
                {
                   binary[z].at<Vec3b>(y,x) = Vec3b(0,0,0);
                }
            }
        }
    }
    isBinary = true;
}

void Volume::getLargestComponent(bool conn, int leftborder = 0, int rightborder = 0, int frontborder = 0, int backborder = 0,int upborder = 0,int bottomborder = 0)
{
    //conn = true: 26 connectivity
    printf("Searching for largest component...\n");
    if(!isBinary)
    {
        printf("Threshold the volume first.");
        return;
    }
    
    bool *largestcom = new bool[sizex*sizey*sizez];
    int *queue = new int[sizex*sizey*sizez*2];
    bool *isSearched = new bool[sizex*sizey*sizez];
    memset(isSearched,0, sizex*sizey*sizez*sizeof(bool));
    
    int head=0,tail=0;
    int largestsize = -1;
    
    for(int i=0;i<sizex*sizey*sizez;i++)
    {
        int pixx,pixy,pixz;
        indtocoord(pixx, pixy, pixz, i);
        if(pixx < leftborder || pixx > sizex - rightborder || pixy < frontborder || pixy > sizey - backborder || pixz < upborder || pixz > sizez - bottomborder)
            continue;
        if(data[i] > 0 && !isSearched[i])
        {
            int cursize = 0;
            head = -1; tail = 0;
            queue[++head] = i;
            isSearched[i] = true;
            while (tail<=head)
            {
                int curind = queue[tail];
                tail++;
                int curx,cury,curz;
                indtocoord(curx, cury, curz,curind);
                
                //26 connectivity
                for(int dx=-1;dx<=1;++dx)
                {
                    for(int dy=-1;dy<=1;++dy)
                    {
                        for(int dz=-1;dz<=1;++dz)
                        {
                            if(dx==0&&dy==0&&dz==0)
                                continue;
                            if(!conn)
                            {
                                if(dx+dy+dz != -1 || dx+dy+dz != 1)
                                    continue;
                            }
                            if(isInside(curx+dx, cury+dy, curz+dz))
                            {
                                if(data[coordtoind(curx+dx, cury+dy, curz+dz)]>0 && !isSearched[coordtoind(curx+dx, cury+dy, curz+dz)])
                                {
                                    queue[++head] = coordtoind(curx+dx, cury+dy, curz+dz);
                                    isSearched[coordtoind(curx+dx, cury+dy, curz+dz)] = true;
                                    cursize++;
                                }
                            }
                        }
                    }
                }
            }
            if(cursize > largestsize)
            {
                memset(largestcom, 0, sizex*sizey*sizez*sizeof(bool));
                largestsize = cursize;
                for(int j=0;j<=head;j++)
                    largestcom[queue[j]] = true;
            }
        }
    }
    printf("\nSize of largest component: %d\n",largestsize);
    
    for(int i=0;i<sizex*sizey*sizez;i++)
    {
        if(largestcom[i] == true)
        {
            data[i] = 1.0;
            binarydata[i] = true;
        }
        else
        {
            data[i] = 0.0;
            binarydata[i] = false;
        }
    }
    for(int i=0;i<sizex*sizey*sizez;i++)
    {
        int pixx,pixy,pixz;
        indtocoord(pixx, pixy, pixz, i);
        if(pixx < leftborder || pixx > sizex - rightborder || pixy < frontborder || pixy > sizey - backborder || pixz < upborder || pixz > sizez - bottomborder)
        {
            data[i] = 1.0;
            binarydata[i] = true;
        }
    }
    delete largestcom;
    delete isSearched;
    delete queue;
    
}

void Volume::getLargestComponentSliced(bool conn, int leftborder = 0, int rightborder = 0, int frontborder = 0, int backborder = 0,int upborder = 0,int bottomborder = 0)
{
    //conn = true: 8 connectivity
    printf("Get largest component...\n");
    if(!isBinary)
    {
        printf("Threshold the volume first.");
        return;
    }
    int *queue = new int[sizex * sizey * 2];
    bool *largest = new bool[sizex * sizey];
    bool *isSearched = new bool[sizex * sizey];
    int largestsize;
    
    for(int z = upborder; z<sizez - bottomborder; z++)
    {
        largestsize = -1;
        memset(largest, 0, sizex*sizey*sizeof(bool));
        memset(isSearched, 0, sizex*sizey*sizeof(bool));
        for(int x = leftborder; x<sizex - rightborder; x++)
        {
            for(int y=frontborder; y<sizey - backborder; y++)
            {
                int ind = coordtoind(x, y, z);
                if(binarydata[ind] && !isSearched[x*sizey + y])
                {
                    int cursize = 1;
                    int head = -1, tail = 0;
                    queue[++head] = x*sizey + y;
                    isSearched[x*sizey + y] = true;
                    while (head>=tail)
                    {
                        int curind = queue[tail];
                        tail++;
                        int curx,cury;
                        curx = curind / sizey;
                        cury = curind % sizey;
                        for(int dx=-1;dx<=1;dx++)
                        {
                            for(int dy=-1;dy<=1;dy++)
                            {
                                if(dx == 0 && dy == 0)
                                    continue;
                                if(!conn)   //4 connectivity
                                {
                                    if(dx+dy != 1 && dx+dy!= -1)
                                        continue;
                                }
                                if(curx+dx>=leftborder&&curx+dx<sizex-rightborder&&cury+dy>=frontborder&&cury+dy<sizey-backborder)
                                {
                                    if(!isSearched[(curx+dx)*sizey + cury+dy] && binarydata[coordtoind(curx+dx, cury+dy, z)])
                                    {
                                        isSearched[(curx+dx)*sizey + cury+dy] = true;
                                        queue[++head] = (curx+dx)*sizey + cury+dy;
                                        cursize++;
                                    }
                                }
                            }
                        }
                    }
                    if(cursize > largestsize)
                    {
                        memset(largest, 0, sizex*sizey*sizeof(bool));
                        for(int i=0;i<=head;i++)
                            largest[queue[i]] = true;
                        largestsize = cursize;
                    }
                }
            }
        }
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
            {
                if(largest[x*sizey + y])
                {
                    data[coordtoind(x, y, z)] = 1.0;
                    binarydata[coordtoind(x, y, z)] = true;
                }
                else
                {
                    data[coordtoind(x, y, z)] = 0.0;
                    binarydata[coordtoind(x, y, z)] = false;
                }
            }
        }
    }
    for(int x = 0;x<sizex;x++)
    {
        for(int y=0;y<sizey;y++)
        {
            for(int z=0;z<sizez;z++)
            {
                if(x<=leftborder || x>=sizex-rightborder || y<=frontborder || y>=sizey-backborder || z<=upborder || z>=sizez-bottomborder)
                {
                    data[coordtoind(x, y, z)] = 1.0;
                    binarydata[coordtoind(x, y, z)] = true;
                }
            }
        }
    }
    delete queue;
    delete largest;
    delete isSearched;
}

void Volume::subtract(Volume *a)
{
    if(a->getSizeX()!=sizex || a->getSizeY()!=sizey || a->getSizeZ()!=sizez)
    {
        printf("Two sizes mush agree\n");
        return;
    }
    if(!a->getisBinary() || !isBinary)
    {
        printf("Threshold the volume first.\n");
        return;
    }
    for(int i=0;i<sizex*sizey*sizez;i++)
    {
        if(a->getBinaryAt(i) > 0.0)
        {
            data[i] = 0.0;
            binarydata[i] = false;
        }
    }
}

void Volume::invert()
{
    if(!isBinary)
    {
        printf("Threshold the volume first!\n");
        return;
    }
    for(int i=0;i<sizex*sizey*sizez;i++)
    {
        data[i] = 1-data[i];
        binarydata[i] = !binarydata[i];
    }
}

void Volume::erode(int r)
{
    if(!isBinary)
        return;
    std::vector<point3D>structure;
    for(int x=-1*r+1;x<r;x++)
    {
        for(int y=-1*r+1;y<r;y++)
        {
            for(int z=-1*r+1;z<r;z++)
            {
                if(x * x + y * y + z * z <= r * r)
                    structure.push_back(point3D(x,y,z));
            }
        }
    }
    for(int x=0;x<sizex;x++)
    {
        for(int y=0;y<sizey;y++)
        {
            for(int z=0;z<sizez;z++)
            {
                int index = x*sizey*sizez + y*sizez + z;
                if(binarydata[index])
                {
                    float vox = 1;
                    for(int i=0;i<(int)structure.size();i++)
                    {
                        int curx = x + structure[i].x;
                        int cury = y + structure[i].y;
                        int curz = z + structure[i].z;
                        if(curx >= 0 && curx <sizex && cury >=0 && cury < sizey && curz >= 0 && curz < sizez)
                        {
                            if(!binarydata[curx * sizey * sizez + cury * sizez + curz])
                                vox = 0;
                        }
                    }
                    data[index] = vox;
                }
            }
        }
    }
    printf("done!\n");
}

void Volume::erodeSlice(int r,int upborder = 0,int bottomborder = 0)
{
    printf("Erode on each slice\n");
    for(int z=upborder;z<sizez-bottomborder;z++)
    {
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
            {
                if(!binarydata[coordtoind(x, y, z)])
                    continue;
                bool vox = true;
                for(int dx=-1*r+1;dx<=r-1;dx++)
                {
                    for(int dy=-1*r+1;dy<=r-1;dy++)
                    {
                        if(dx*dx+dy*dy>r*r)
                            continue;
                        if(x+dx>=0&&x+dx<sizex&&y+dy>=0&&y+dy<sizey)
                        {
                            if(!binarydata[coordtoind(x+dx, y+dy, z)])
                            {
                                vox = false;
                                break;
                            }
                        }
                    }
                    if(!vox)
                        break;
                }
                if(!vox)
                    data[coordtoind(x, y, z)] = 0.0;
            }
        }
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
                binarydata[coordtoind(x, y, z)] = data[coordtoind(x, y, z)] > 0.0 ? true:false;
        }
    }
}

void Volume::dilateSlice(int r,int upborder = 0,int bottomborder = 0)
{
    for(int z=upborder;z<sizez-bottomborder;z++)
    {
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
            {
                if(!binarydata[coordtoind(x, y, z)])
                    continue;
                for(int dx=-r;dx<=r;dx++)
                {
                    for(int dy=-r;dy<=r;dy++)
                    {
                        if(dx*dx+dy*dy>r*r)
                            continue;
                        if(x+dx>=0&&x+dx<sizex&&y+dy>=0&&y+dy<sizey)
                            data[coordtoind(x+dx, y+dy, z)] = 1.0;
                    }
                }
            }
        }
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
                binarydata[coordtoind(x, y, z)] = data[coordtoind(x, y, z)] > 0.0 ? true:false;
        }
    }
}
void Volume::dilate(int r)
{
    if(!isBinary)
        return;
    std::vector<point3D>structure;
    for(int x=-1*r;x<=r;x++)
    {
        for(int y=-1*r;y<=r;y++)
        {
            for(int z=-1*r;z<=r;z++)
            {
                if(x * x + y * y + z * z <= r * r)
                    structure.push_back(point3D(x,y,z));
            }
        }
    }
    for(int x=0;x<sizex;x++)
    {
        for(int y=0;y<sizey;y++)
        {
            for(int z=0;z<sizez;z++)
            {
                int index = x*sizey*sizez + y*sizez + z;
                if(binarydata[index])
                {
                    for(int i=0;i<(int)structure.size();i++)
                    {
                        int curx = x + structure[i].x;
                        int cury = y + structure[i].y;
                        int curz = z + structure[i].z;
                        if(curx >= 0 && curx <sizex && cury >=0 && cury < sizey && curz >= 0 && curz < sizez)
                            data[curx * sizey * sizez + cury * sizez + curz] = 1;
                    }
                }
            }
        }
    }
    printf("done!\n");

}
void Volume::normalize( float min, float max )
{
    float imin = getMin() ;
    float imax = getMax() ;
    float irange = imax - imin ;
    float range = max - min ;
    
    int size = sizex * sizey * sizez ;
    for ( int i = 0 ; i < size ; i ++ )
    {
        data[ i ] = (( data[ i ] - imin ) / irange) * range + min ;
    }
}

void Volume::normalizeSliced()
{
    for(int z=0;z<sizez;z++)
    {
        float min = 9999, max = -1;
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
            {
                min = getDataAt(x, y, z) < min ? getDataAt(x, y, z) : min;
                max = getDataAt(x, y, z) > max ? getDataAt(x, y, z) : max;
            }
        }
        for(int x=0;x<sizex;x++)
        {
            for(int y=0;y<sizey;y++)
                setDataAt(x, y, z, (getDataAt(x, y, z)-min)*(255.0/max));
        }
    }
    isNormalized = true;
}
void Volume::normalize( float min, float max, float thresh, float ithresh )
{
    float imin = getMin() ;
    float imax = getMax() ;
    float irange1 = ithresh - imin ;
    float irange2 = imax - ithresh ;
    float range1 = thresh - min;
    float range2 = max - thresh ;
    
    int size = sizex * sizey * sizez ;
    for ( int i = 0 ; i < size ; i ++ )
    {
        if ( data[ i ] < ithresh )
        {
            data[ i ] = (( data[ i ] - imin ) / irange1) * range1 + min ;
        }
        else
        {
            data[ i ] = max - (( imax - data[ i ] ) / irange2) * range2 ;
        }
    }
}

void Volume::toMRCFile( char* fname )
{
    FILE* fout = fopen( fname, "wb" ) ;
    
    // Write header
    fwrite( &sizex, sizeof( int ), 1, fout ) ;
    fwrite( &sizey, sizeof( int ), 1, fout ) ;
    fwrite( &sizez, sizeof( int ), 1, fout ) ;
    
    int mode = 2 ;
    fwrite( &mode, sizeof ( int ), 1, fout ) ;
    
    int off[3] = {0,0,0} ;
    int intv[3] = { sizex - 1, sizey - 1, sizez - 1 } ;
    fwrite( off, sizeof( int ), 3, fout ) ;
    fwrite( intv, sizeof( int ), 3, fout ) ;
    
    float cella[3] = {2,2,2} ;
    float cellb[3] = {90,90,90} ;
    fwrite( cella, sizeof( float ), 3, fout ) ;
    fwrite( cellb, sizeof( float ), 3, fout ) ;
    
    int cols[3] = {1,2,3} ;
    fwrite( cols, sizeof( int ), 3, fout ) ;
    
    float dmin = 100000, dmax = -100000 ;
    for ( int i = 0 ; i < sizex * sizey * sizez ; i ++ )
    {
        if ( data[ i ] < dmin )
        {
            dmin = data[ i ] ;
        }
        if ( data[i] > dmax )
        {
            dmax = data[ i ] ;
        }
    }
    float ds[3] = {static_cast<float>(dmin), static_cast<float>(dmax), 0} ;
    fwrite( ds, sizeof( float ), 3, fout ) ;
    
    int zero = 0 ;
    for (int i = 22 ; i < 256 ; i ++ )
    {
        fwrite( &zero, sizeof( int ), 1, fout ) ;
    }
    
    // Write contents
    for ( int z = 0 ; z < sizez ; z ++ )
        for ( int y = 0 ; y < sizey ; y ++ )
            for ( int x = 0 ; x < sizex ; x ++ )
            {
                float d = (float)getDataAt(x,y,z) ;
                fwrite( &d, sizeof( float ), 1, fout ) ;
            }
    
    fclose( fout ) ;
}
