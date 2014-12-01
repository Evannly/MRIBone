#include "UniGrid.h"

void UniGrid::init()
{
    // building tables
    ttrees = new TriangulationTree* [7];
    ttrees[4] = new TriangulationTree( "/Users/yanhang/Documents/research/contour/pretriang_4.txt", 4 );
    ttrees[5] = new TriangulationTree( "/Users/yanhang/Documents/research/contour/pretriang_5.txt", 5 );
    ttrees[6] = new TriangulationTree( "/Users/yanhang/Documents/research/contour/pretriang_6.txt", 6 );
    cube = new Cube( "/Users/yanhang/Documents/research/contour/cycle8.txt" ) ;
    
    totalverts = 0 ;
    totaltris = 0 ;
    
    root = NULL;
    verts = NULL;
    tris = NULL;
    
    for ( int i = 0 ; i < 3 ; i ++ )
    {
        offset[i] = new float[ dim[i] ] ;
        offset[i][0] = 0 ;	// Assuming origin is at 0,0,0
        for ( int j = 1 ; j < dim[i] ; j ++ )
        {
            offset[i][j] = offset[i][j-1] + dis[i] ;
        }
    }
    
}

UniGrid::~UniGrid( )
{
    for ( int i = 4 ; i < 7 ; i ++ )
    {
        delete ttrees[i] ;
    }
    delete [] ttrees ;
    delete cube ;
    
    if (data != NULL)
    {
        for ( int i = 0 ; i < dim[2] ; i ++ )
        {
            for ( int j = 0 ; j < dim[1] ; j ++ )
            {
                delete [] data[i][j] ;
            }
            delete [] data[i];
        }
        delete [] data;
    }
    
    delete offset[0] ;
    delete offset[1] ;
    delete offset[2] ;
    
    if ( root != NULL )
    {
        clearTree( root ) ;
    }
    if (verts != NULL)
        delete [] verts;
    if (tris != NULL)
        delete [] tris ;
}


// Constructor: from an existing Volume object
UniGrid::UniGrid( Volume* vol )
{
    // Read in dimensions
    dim[0] = vol->getSizeX() ;
    dim[1] = vol->getSizeY() ;
    dim[2] = vol->getSizeZ() ;
    dis[0] = vol->getSpacingX() ;
    dis[1] = vol->getSpacingY() ;
    dis[2] = vol->getSpacingZ() ;
    d1 = dim[ 0 ] ;
    d2 = dim[ 1 ] * dim[ 0 ] ;
    
    ratioxy = vol->getRatioxy();
    ratioz = vol->getRatioz();
    // Read in data
    //int size = dim[ 0 ] * dim[ 1 ] * dim[ 2 ] ;
    data = new float** [ dim[2] ] ;
    for ( int a = 0 ; a < dim[2] ; a ++ )
    {
        data[a] = new float * [ dim[1] ] ;
        for ( int b = 0 ; b < dim[1] ; b ++ )
        {
            data[a][b] = new float [ dim[0] ] ;
        }
    }
    int ct = 0 ;
    for ( int k = 0 ; k < dim[2] ; k ++ )
        for ( int j = 0 ; j < dim[1] ; j ++ )
            for ( int i = 0 ; i < dim[0] ; i ++ )
            {
                // data[ ct ] = (float) ( vol->getDataAt( i, j, k ) );
                data[ k ][ j ][ i ] = (float) ( vol->getDataAt( i, j, k ) );
                ct ++ ;
            }
    init() ;
}

void UniGrid::buildTree( )
{
    int dimen = 1 ;
    while ( dimen < dim[0] || dimen < dim[1] || dimen < dim[2] )
    {
        dimen <<= 1 ;
    }
    
    printf("Building the octree. Dimension: %d...", dimen);
    this->root = buildTree( 0, 0, 0, dimen ) ;
    printf("Done.\n") ;
}

UniTreeNode* UniGrid::buildTree( int x, int y, int z, int len )
{
    UniTreeNode* node = new UniTreeNode ;
    node->chd = NULL ;
    node->min = 100000 ;
    node->max = -100000 ;
    
    if ( x >= dim[0] - 1 || y >= dim[1] - 1 || z >= dim[2] - 1 )
    {
        return node ;
    }
    
    if ( len == 1 )
    {
        for ( int cc = 0 ; cc < 8 ; cc ++ )
        {
            float temp =  getData( x + coordmap[ cc ][ 0 ], y + coordmap[ cc ][ 1 ], z + coordmap[ cc ][ 2 ] ) ;
            if ( temp < node->min )
            {
                node->min = temp ;
            }
            if ( temp > node->max )
            {
                node->max = temp ;
            }
        }
    }
    else
    {
        int ct = 0 ;
        len /= 2 ;
        UniTreeNode* temp ;
        if ( len > 1 )
        {
            node->chd = new UniTreeNode* [8] ;
        }
        for ( int i = 0 ; i < 2 ; i ++ )
            for ( int j = 0 ; j < 2 ; j ++ )
                for ( int k = 0 ; k < 2 ; k ++ )
                {
                    temp = buildTree( x + len * i, y + len * j, z + len * k, len ) ;
                    if ( temp->min < node->min )
                    {
                        node->min = temp->min ;
                    }
                    if ( temp->max > node->max )
                    {
                        node->max = temp->max ;
                    }
                    if ( len > 1 )
                    {
                        node->chd[ct] = temp ;
                    }
                    else
                    {
                        clearTree(temp) ;
                    }
                    ct ++ ;
                }
    }
    return node ;
}

void UniGrid::clearTree( UniTreeNode* node )
{
    if ( node->chd != NULL )
    {
        for ( int i = 0 ; i < 8 ; i ++ )
        {
            clearTree( node->chd[i] ) ;
        }
        delete [] node->chd ;
    }
    delete node ;
}

void UniGrid::genContourTree( UniTreeNode* node, int x, int y, int z, int len, float iso, int isConvex, LinkedList<Point3f>* vertlist, LinkedList<Triangle3i>* trianglist, HashMap* myhash, Status* status )
{
    if ( node->min > iso || node->max < iso )
    {
        return ;
    }
    
    clock_t start = clock() ;
    if ( len == 2 )
    {
        for ( int i = 0 ; i < 2 ; i ++ )
            for ( int j = 0 ; j < 2 ; j ++ )
                for ( int k = 0 ; k < 2 ; k ++ )
                {
                    // if ( x + i < dim[0] - 1 && y + j < dim[1] - 1 && z + k < dim[2] - 1 )
                    {
                        genContourCell( iso, isConvex, x + i, y + j, z + k, vertlist, trianglist, myhash, status ) ;
                    }
                }
        meshTime += ( clock() - start ) ;
    }
    else
    {
        int ct = 0 ;
        len /= 2 ;
        for ( int i = 0 ; i < 2 ; i ++ )
            for ( int j = 0 ; j < 2 ; j ++ )
                for ( int k = 0 ; k < 2 ; k ++ )
                {
                    if(!node->chd[ct])
                        continue;
                    genContourTree( node->chd[ct], x + len * i, y + len * j, z + len * k, len,
                                   iso, isConvex, vertlist, trianglist, myhash, status ) ;
                    ct ++ ;
                }
        searchTime += ( clock() - start ) ;
    }
}

/// Contouring
void UniGrid::genContour(float iso , QProgressDialog *progress)
{
    genContour( iso, 1 ,progress) ;
}

inline float UniGrid::getData( int i, int j, int k )
{
    // return data[ ( i ) + ( j ) * d1 + ( k ) * d2 ] ;
    return data[k][j][i] ;
}

void UniGrid::genContour( float iso, int isConvex, QProgressDialog *progress )
{
    clock_t start, finish ;
    start = clock();
    
    // Initialization
    if ( totalverts != 0 )
    {
        delete verts ;
    }
    if ( totaltris != 0 )
    {
        delete tris ;
    }
    
    LinkedList<Triangle3i>* trianglist = new LinkedList<Triangle3i> ( ) ;
    LinkedList<Point3f>* vertlist = new LinkedList<Point3f> ( ) ;
    HashMap* myhash = new HashMap( ) ;
    this->totaltris = 0 ;
    this->totalverts = 0 ;
    Status* status = new Status ;
    status->num_cells = 0 ;
    status->num_cycles = 0 ;
    status->total_cyclen = 0 ;
    status->total_tests = 0 ;
    status->total_tris = 0 ;
				
    // Contouring
    if ( root == NULL )
    {
        // Un-optimized, uniform grid
        int cnt = 0;
        int unitcount =  dim[ 0 ]*dim[ 1 ]*dim[ 2 ] / 100;
        for ( int i = 0 ; i < dim[ 0 ] - 1 ; i ++ )
            for ( int j = 0 ; j < dim[ 1 ] - 1 ; j ++ )
                for ( int k = 0 ; k < dim[ 2 ] - 1 ; k ++ )
                {
                    if(cnt % unitcount == 0)
                        progress->setValue(cnt / unitcount);
                    cnt ++ ;
                    genContourCell( iso, isConvex, i, j, k, vertlist, trianglist, myhash, status ) ;
                }
        putchar( 13 ) ;
    }
    else
    {
        // Octree-optimized
        int dimen = 1 ;
        while ( dimen < dim[0] || dimen < dim[1] || dimen < dim[2] )
        {
            dimen <<= 1 ;
        }
        
        meshTime = 0 ;
        searchTime = 0 ;
        genContourTree( root, 0, 0, 0, dimen, iso, isConvex, vertlist, trianglist, myhash, status ) ;
        
        printf("Meshing time: %f\n", (float)meshTime/(float)CLOCKS_PER_SEC) ;
        printf("Searching time: %f\n", (float)searchTime/(float)CLOCKS_PER_SEC) ;
    }
    
    // Finally, flatten out to a list
    this->totaltris = trianglist->getLength();
    this->verts = new float[ 3 * this->totalverts ] ;
    this->tris = new int[ 3 * this->totaltris ] ;
    printf("Total verices: %d, total triangles: %d\n", totalverts, totaltris ) ;
    Point3f p;
    Triangle3i t ;
    int i;
    for ( i = 0 ; i < totalverts ; i ++ )
    {
        p = vertlist->getFirst();
        verts[3 * i + 0] = p.x ;
        verts[3 * i + 1] = p.y ;
        verts[3 * i + 2] = p.z * (ratioz / ratioxy) ;
        vertlist->rotateLeft();
    }
    for ( i = 0 ; i < totaltris ; i ++ )
    {
        t = trianglist->getFirst() ;
        tris[3 * i + 0] = t.v1 ;
        tris[3 * i + 1] = t.v2 ;
        tris[3 * i + 2] = t.v3 ;
        trianglist->rotateLeft() ;
    }
    
    finish = clock();
    
    delete vertlist ;
    delete trianglist ;
    delete myhash ;
    delete status ;
}

inline void UniGrid::genContourCell( float iso, int isConvex, int i, int j, int k, LinkedList<Point3f>* vertlist, LinkedList<Triangle3i>* trianglist, HashMap* myhash, Status* status )
{
    float cell[ 8 ] ;
    unsigned char mask = 0  ;
    
    for ( int cc = 0 ; cc < 8 ; cc ++ )
    {
        cell[ cc ] =  getData( i + coordmap[ cc ][ 0 ], j + coordmap[ cc ][ 1 ], k + coordmap[ cc ][ 2 ] ) ;
        mask |= ( ( cell[ cc ] > iso ? 1 : 0 ) << ( 7 - cc ) ) ;
    }
    
    
    // Quick test
    if ( mask == 0 || mask == 255 )
    {
        return ;
    }
    
    //int vind;
    float inters[ 12 ][ 3 ];
    int intersind[ 12 ];
    
    // Get intersections on edges
    for ( int ei = 0 ; ei < 12 ; ei ++ )
    {
        int e1 = vertmap[ ei ][ 0 ] ;
        int e2 = vertmap[ ei ][ 1 ] ;
        int s1 = (( mask >> ( 7 - e1 ) ) & 1 ) ;
        int s2 = (( mask >> ( 7 - e2 ) ) & 1 ) ;
        if ( s1 ^ s2 )
        {
            int x = 2 * i + coordmap[vertmap[ei][0]][0] + coordmap[vertmap[ei][1]][0] ;
            int y = 2 * j + coordmap[vertmap[ei][0]][1] + coordmap[vertmap[ei][1]][1] ;
            int z = 2 * k + coordmap[vertmap[ei][0]][2] + coordmap[vertmap[ei][1]][2] ;
            if ( myhash->find(x,y,z, intersind[ei], inters[ei][0], inters[ei][1], inters[ei][2]) == 0 )
            {
                float v1 = cell[ e1 ] ;
                float v2 = cell[ e2 ] ;
                float a = (v1 - iso) / ( v1 - v2 ) ;
                
                float coord[3] ;
                coord[ dirmap[ ei ] ] = a ;
                coord[ dirmap1[ ei ] ] = dismap1[ ei ] ;
                coord[ dirmap2[ ei ] ] = dismap2[ ei ] ;
                
                inters[ ei ][0] = offset[0][i] +  coord[0] * ( offset[0][i+1] - offset[0][i] ) ;
                inters[ ei ][1] = offset[1][j] +  coord[1] * ( offset[1][j+1] - offset[1][j] ) ;
                inters[ ei ][2] = offset[2][k] +  coord[2] * ( offset[2][k+1] - offset[2][k] ) ;
                
                
                intersind[ei] = totalverts ;
                totalverts ++ ;
                Point3f pt;
                pt.x = inters[ ei ][0];
                pt.y = inters[ ei ][1];
                pt.z = inters[ ei ][2];
                vertlist->add( pt ) ;
                
                myhash->insert(x,y,z, intersind[ei], inters[ei][0], inters[ei][1], inters[ei][2] ) ;
            }
        }
    }
    
    // Contouring
    // First, get the cycles
    LinkedList<Cycle*> *cycleList = cube->getCycle( mask ) ;
    
    
    // Now, for each cycle, generate a vertex list and give it to triangulator
    float vert[12][3] ;
    int vertind[12] ;
    Cycle* cyc ;
    int numcyc = cycleList->getLength(), cyclen ;
    LinkedList<Triangle3i> *templist = NULL ;
    status->num_cells ++ ;
    
    for ( int ii = 0 ; ii < numcyc ; ii ++ )
    {
        cyc = cycleList->getFirst() ;
        cyclen = cyc->getLength() ;
        for ( int jj = 0 ; jj < cyclen ; jj ++ )
        {
            vert[jj][0] = inters[cyc->getEdge(jj)][0] ;
            vert[jj][1] = inters[cyc->getEdge(jj)][1] ;
            vert[jj][2] = inters[cyc->getEdge(jj)][2] ;
            vertind[jj] = intersind[cyc->getEdge(jj)] ;
        }
        
        // perform statistics
        status->num_cycles ++ ;
        status->total_cyclen += cyclen ;
        status->total_tris += cyclen - 2 ;
        
        if ( isConvex )
        {
            if ( cyclen == 3 )
            {
                Triangle3i triang;
#ifdef INSIDE_CONVEX
                triang.v1 = vertind[0] ;
                triang.v2 = vertind[2] ;
                triang.v3 = vertind[1] ;
#else
                triang.v1 = vertind[0] ;
                triang.v2 = vertind[1] ;
                triang.v3 = vertind[2] ;
#endif
                trianglist->add( triang ) ;
            }
            else if ( cyclen > 3 && cyclen < 7 )
            {
                status->total_tests += ttrees[cyclen]->triangulate( vert, vertind, templist ) ;
                trianglist->append( templist ) ;
                delete templist ;
            }
        }
        else
        {
            // Imitate Marching cubes
            for ( int kk = 0 ; kk < cyclen - 2 ; kk ++ )
            {
                Triangle3i triang;
                triang.v1 = vertind[0] ;
                triang.v2 = vertind[kk+1] ;
                triang.v3 = vertind[kk+2] ;
                trianglist->add( triang ) ;
            }
        }
        
        cycleList->rotateLeft();
    }
}


void UniGrid::writePLY( char* fname )
{
    FILE * fout = fopen ( fname, "wb" ) ;
    PLYWriter * writer = new PLYWriter( ) ;
    
    writer->writeHeader( fout, totalverts, totaltris ) ;
    
    int i ;
    for ( i = 0 ; i < totalverts ; i ++ )
    {
        float vt[3]= {verts[i*3], verts[i*3+1], verts[i*3+2]};
        writer->writeVertex( fout, vt ) ;
    }
    
    for ( i = 0 ; i < totaltris ; i ++ )
    {
        int fc[3] = { tris[i*3], tris[i*3+1], tris[i*3+2] } ;
        writer->writeFace( fout, 3, fc ) ;
    }
    
    fclose( fout ) ;
}

void UniGrid::smooth( int iters )
{
    int i,j,k,c;
    float epsilon = 0.0000001f;
    
    int* val = new int[ this->totalverts ] ;
    float* nverts = new float[ this->totalverts * 3 ] ;
    int* mask = new int[ this->totalverts ] ;
    for ( i = 0 ; i < this->totalverts ; i ++ )
    {
        // Test if point lies on the slice (z) plane
        float z = verts[ 3 * i + 2 ] ;
        if ( fabs( z - ((int)( z / dis[2] )) * dis[2]) < epsilon || fabs( z - ((int)( z / dis[2] )+1) * dis[2]) < epsilon )
        {
            mask[ i ] = 1 ;
        }
        else
        {
            mask[ i ] = 0 ;
        }
    }
    
    
    float wts[3] = { .5f, .25f, .25f } ;
    
    for ( int iter = 0 ; iter < iters ; iter ++ )
    {
        printf("Smoothing iteration %d\n", iter) ;
        
        //printf("Initialization\n") ;
        for ( i = 0 ; i < this->totalverts ; i ++ )
        {
            val[ i ] = 0 ;
            nverts[ 3 * i ] = 0 ;
            nverts[ 3 * i + 1 ] = 0 ;
            nverts[ 3 * i + 2 ] = 0 ;
        }
        
        for ( i = 0 ; i < this->totaltris ; i ++ )
        {
            // Look at each vertex
            for ( j = 0 ; j < 3 ; j ++ )
            {
                int vind = this->tris[ 3 * i + j ];
                float temp[3] ={ 0, 0, 0 };
                for ( k = j ; k < j + 3 ; k ++ )
                {
                    for ( c = 0 ; c < 3 ; c ++ )
                    {
                        int nind = this->tris[ 3 * i + ( k % 3 ) ];
                        temp[ c ] += wts[ k - j ] * this->verts[ nind * 3 + c ] ;
                    }
                }
                
                val[ vind ] ++ ;
                for ( c = 0 ; c < 3 ; c ++ )
                {
                    nverts[ 3 * vind + c ] += temp[ c ] ;
                }
            }
        }
        
        for ( i = 0 ; i < this->totalverts ; i ++ )
        {
            if ( mask[ i ] == 0 )
            {
                // arbitrary move
                for ( c = 0 ; c < 3 ; c ++ )
                {
                    verts[ i * 3 + c ] = nverts[ i * 3 + c ] / val[ i ] ;
                }
            }
            else
            {
                // in-plane move
                for ( c = 0 ; c < 3 ; c ++ )
                {
                    verts[ i * 3 + c ] = nverts[ i * 3 + c ] / val[ i ] ;
                }
            }
        }
    }
    
    delete [] mask;
    delete [] nverts;
    delete [] val;
}

void UniGrid::getMesh( int& numverts, int& numtris, float*& vertices, int*& triangles )
{
    numverts = totalverts ;
    numtris = totaltris ;
    
    vertices = new float[ 3 * numverts ] ;
    triangles = new int[ 3 * numtris ] ;
    
    int i ;
    for ( i = 0 ; i < 3 * numverts ; i ++ )
    {
        vertices[ i ] = verts[ i ] ; 
    }
    for ( i = 0 ; i < 3 * numtris ; i ++ )
    {
        triangles[ i ] = tris[ i ] ;
    }
} 

void UniGrid::getMesh(TriMesh &mesh)
{   
    TriMesh::VertexHandle *vhandle = new TriMesh::VertexHandle[totalverts];
    std::vector <TriMesh::VertexHandle> fhandle;
    for(int i=0;i<totalverts;i++)
       vhandle[i] = mesh.add_vertex(TriMesh::Point(verts[i*3],verts[i*3+1],verts[i*3+2]));
    for(int i=0;i<totaltris;i++)
    {
        fhandle.push_back(vhandle[tris[i*3]]);
        fhandle.push_back(vhandle[tris[i*3+1]]);
        fhandle.push_back(vhandle[tris[i*3+2]]);
        mesh.add_face(fhandle);
        fhandle.clear();
    }
    mesh.update_normals();

    delete [] vhandle;
}
