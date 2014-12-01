/**
 * Reading routines for volumetric data
 *
 * Author: Tao Ju
 * Date: 02/16/2005
 */


#ifndef READER_H
#define READER_H

//#define CT_STRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QProgressDialog>

#include "volume.h"

class MRCReader
{
public:
	/* Initializer */
	MRCReader( char* fname )
	{
		sprintf( mrcfile, "%s", fname ) ;

		FILE* fin = fopen( fname, "rb" ) ;

		// Parse header
		fread( &totx, sizeof( int ), 1, fin ) ;
		fread( &toty, sizeof( int ), 1, fin ) ;
		fread( &totz, sizeof( int ), 1, fin ) ;

		fread( &mode, sizeof( int ), 1, fin ) ;

		fread( &offx, sizeof( int ), 1, fin ) ;
		fread( &offy, sizeof( int ), 1, fin ) ;
		fread( &offz, sizeof( int ), 1, fin ) ;
		
		fread( &dimx, sizeof( int ), 1, fin ) ;
		fread( &dimy, sizeof( int ), 1, fin ) ;
		fread( &dimz, sizeof( int ), 1, fin ) ;
		dimx ++ ;
		dimy ++ ;
		dimz ++ ;

		fread( &angsx, sizeof( float ), 1, fin ) ;
		fread( &angsy, sizeof( float ), 1, fin ) ;
		fread( &angsz, sizeof( float ), 1, fin ) ;

		fread( &anglex, sizeof( float ), 1, fin ) ;
		fread( &angley, sizeof( float ), 1, fin ) ;
		fread( &anglez, sizeof( float ), 1, fin ) ;

		fseek( fin, 12, SEEK_CUR ) ;

		fread( &dmin, sizeof( float ), 1, fin ) ;
		fread( &dmax, sizeof( float ), 1, fin ) ;
		fread( &dmean, sizeof( float ), 1, fin ) ;

		fseek( fin, 4 * 32, SEEK_CUR ) ;

		fread( &drms, sizeof( float ), 1, fin ) ;
		fclose( fin ) ;

		dimx = totx ;
		dimy = toty ;
		dimz = totz ;

		if ( mode > 2 && mode != 6 )
		{
			printf("Complex mode not supported.\n") ;
			exit(0) ;
		}
	}

	/* Read volume */
    Volume* getVolume(QProgressDialog *progress )
	{
		FILE* fin = fopen( mrcfile, "rb" ) ;
		fseek( fin, 1024, SEEK_SET ) ;

		char chard ;
		short shortd ;
        unsigned short ushortd;
		float floatd ;
		float d ;
        int count = 0;
        int unit = dimx*dimy*dimz / 100;
		
		Volume* vol = new Volume( dimx, dimy, dimz ) ;
        vol->setRatioxy(angsx / (dimx-1));
        vol->setRatioz(angsz / (dimz-1));
		for ( int i = 0 ; i < dimz ; i ++ )
			for ( int j = 0 ; j < dimy ; j ++ )
				for ( int k = 0 ; k < dimx ; k ++ )
				{
                    if(count % unit == 0)
                        progress->setValue(count/unit);
                    count++;
                    if(progress->wasCanceled())
                        return NULL;
					switch ( mode )
					{
                        case 0:
                            fread( &chard, sizeof( char ), 1, fin ) ;
                            d = (float) chard ;
                            break ;
                        case 1:
                            fread( &shortd, sizeof( short ), 1, fin ) ;
                            d = (float) shortd ;
                            break ;
                        case 2:
                            fread( &floatd, sizeof( float ), 1, fin ) ;
                            d = (float) floatd ;
                            break ;
                        case 6:
                            fread(&ushortd, sizeof(unsigned short), 1, fin);
                            d = (float) ushortd;
                            break;
                        default:
                            printf("MRC mode unsupported!\n");
                            exit(0);
                    
					}
					vol->setDataAt( k, j, i, d ) ;
				}
		fclose( fin ) ;

		return vol ;
	}

	/* Get resolution */
	void getSpacing( float& ax, float& ay, float& az )
	{
		ax = angsx / (dimx - 1);
		ay = angsy / (dimy - 1) ;
		az = angsz / (dimz - 1) ;
	}


private:

	int totx, toty, totz ;
	int offx, offy, offz ;
	int dimx, dimy, dimz ;

	float angsx, angsy, angsz ;
	float anglex, angley, anglez ;
	float dmin, dmax, dmean, drms ;
	
	int mode ;

	char mrcfile[1024] ;
};

#endif
