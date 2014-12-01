#ifndef CURVATURE_H
#define CURVATURE_H

const float epsilon = 0.9f ;

/************************************************************************/
/* Class for a tri-variate function                                     */
/************************************************************************/
class TriFunction
{

public:

	TriFunction(){};

	// Evaluation 
	virtual float eval( float x, float y, float z ) = 0 ;

	virtual void grad( float x, float y, float z, float d[3] ) = 0 ;
	virtual void hessian( float x, float y, float z, float dd[3][3] ) = 0 ;

	// Derivatives
	void gradN ( float x, float y, float z, float d[3] )
	{
		d[0] = ( eval( x + epsilon, y, z ) - eval( x - epsilon, y, z ) ) / ( 2 * epsilon );
		d[1] = ( eval( x, y + epsilon, z ) - eval( x, y - epsilon, z ) ) / ( 2 * epsilon );
		d[2] = ( eval( x, y, z + epsilon ) - eval( x, y, z - epsilon ) ) / ( 2 * epsilon );
	}

	// Second derivatives
	void hessianN( float x, float y, float z, float dd[3][3] )
	{
		float d1[3], d2[3] ;

		grad( x + epsilon, y, z, d1 ) ;
		grad( x - epsilon, y, z, d2 ) ;
		dd[0][0] = ( d1[0] - d2[0] ) / ( 2 * epsilon ) ;
		dd[1][0] = ( d1[1] - d2[1] ) / ( 2 * epsilon );
		dd[2][0] = ( d1[2] - d2[2] ) / ( 2 * epsilon );

		grad( x, y + epsilon, z, d1 ) ;
		grad( x, y - epsilon, z, d2 ) ;
		dd[0][1] = ( d1[0] - d2[0] ) / ( 2 * epsilon ) ;
		dd[1][1] = ( d1[1] - d2[1] ) / ( 2 * epsilon );
		dd[2][1] = ( d1[2] - d2[2] ) / ( 2 * epsilon );

		grad( x, y, z + epsilon, d1 ) ;
		grad( x, y, z - epsilon, d2 ) ;
		dd[0][2] = ( d1[0] - d2[0] ) / ( 2 * epsilon ) ;
		dd[1][2] = ( d1[1] - d2[1] ) / ( 2 * epsilon );
		dd[2][2] = ( d1[2] - d2[2] ) / ( 2 * epsilon );
	}

};

class LinearQuadQuadFunction : public TriFunction
{
	float v[2][3][3] ;

	// Helper functions: x defined in [0,1]
	float linear( float a, float b, float x )
	{
		return (1-x) * a + x * b ;
	}
	// Helper functions: x defined in [-1,1]
	float quadApprox( float a, float b, float c, float x )
	{
		float xx = ( x + 1 ) / 2 ;
		return linear( linear( a, b, xx ), linear( b, c, xx ), xx ) ;
	}
	float quadInterp( float a, float b, float c, float x )
	{
		return x * (x-1) * a / 2 + 
			(x+1) * (1-x) * b +
			(x+1) * x * c / 2 ; 
	}

public:

	// x varies last while z varies first
	LinearQuadQuadFunction( float p[2][3][3] )
	{
		for ( int i = 0 ; i < 2 ; i ++ )
			for( int j = 0 ; j < 3 ; j ++ )
				for ( int k = 0 ; k < 3 ; k ++ )
				{
					v[i][j][k] = p[i][j][k] ;

//					printf("%f,",v[i][j][k]) ;
				}

//		printf("point: (%f %f %f)\n", 0.1,0.2,0.3) ;
//		printf("Value: %f\n", eval(0.1,0.2,0.3) ) ;
//		exit(0) ;
	}

	// Assume that the function is linear in x and quadratic in y,z
	// Assume that x is in [0,1] and y,z are in [-1,1]
	float eval( float x, float y, float z )
	{
		float vx[2] ;

		for ( int i = 0 ; i < 2 ; i ++ )
		{
			float vy[3] ;
			for ( int j = 0 ; j < 3 ; j ++ )
			{
				// vy[j] = quadApprox( v[i][j][0], v[i][j][1], v[i][j][2], z ) ;
				vy[j] = quadInterp( v[i][j][0], v[i][j][1], v[i][j][2], z ) ;
//				printf("vy[%d]: %f (%f %f %f %f)", j, vy[j], v[i][j][0], v[i][j][1], v[i][j][2], z) ;
			}

			// vx[i] = quadApprox( vy[0], vy[1], vy[2], y ) ;
			vx[i] = quadInterp( vy[0], vy[1], vy[2], y ) ;
//			printf("\nvx[%d]: %f\n",i,vx[i]) ;
		}

		return linear( vx[0], vx[1], x ) ;
	}

	/* Assuming y and z are all zero, we can do this exactly */
	void grad ( float x, float y, float z, float d[3] )
	{
		d[0] = v[1][1][1] - v[0][1][1] ;
		d[1] = ( (v[0][2][1] - v[0][0][1]) * ( 1 - x ) + ( v[1][2][1] - v[1][0][1] ) * x ) / 2 ;
		d[2] = ( (v[0][1][2] - v[0][1][0]) * ( 1 - x ) + ( v[1][1][2] - v[1][1][0] ) * x ) / 2 ;
	}


	/* Assuming y and z are all zero, we can do this exactly */
	void hessian( float x, float y, float z, float dd[3][3] )
	{
		dd[0][0] = 0 ;
		dd[0][1] = ( v[0][0][1] - v[0][2][1] + v[1][2][1] - v[1][0][1] ) / 2 ;
		dd[0][2] = ( v[0][1][0] - v[0][1][2] + v[1][1][2] - v[1][1][0] ) / 2 ;

		dd[1][0] = dd[0][1] ;
		dd[1][1] = ( v[0][0][1] + v[0][2][1] - 2 * v[0][1][1] ) * ( 1- x ) + ( v[1][0][1] + v[1][2][1] - 2 * v[1][1][1] ) * x ;
		dd[1][2] = ( ( v[0][0][0] + v[0][2][2] - v[0][2][0] - v[0][0][2] ) * ( 1 - x ) + ( v[1][0][0] + v[1][2][2] - v[1][2][0] - v[1][0][2] ) * x ) / 4 ;

		dd[2][0] = dd[0][2] ;
		dd[2][1] = dd[1][2] ;
		dd[2][2] = ( v[0][1][0] + v[0][1][2] - 2 * v[0][1][1] ) * ( 1- x ) + ( v[1][1][0] + v[1][1][2] - 2 * v[1][1][1] ) * x ;
	}

};

/************************************************************************/
/* Class for curvature                                                  */
/************************************************************************/
class Curvature
{
public:

	Curvature(){} ;

	// Entry function
	static void getCurvatures( TriFunction *f, float x, float y, float z, float& curvG, float& curvM, float& curvP1, float& curvP2 )
	{

		int i, j ;

		// First, compute gradient and hessian
		float g[3], h[3][3] ;
		f->grad( x, y, z, g ) ;
		f->hessian( x, y, z, h ) ;

		// Compute co-hessian
		float ch[3][3] =
		{
			{ h[1][1]*h[2][2] - h[1][2]*h[2][1], h[1][2]*h[2][0] - h[1][0]*h[2][2], h[1][0]*h[2][1] - h[1][1]*h[2][0] },
			{ h[2][1]*h[0][2] - h[2][2]*h[0][1], h[2][2]*h[0][0] - h[2][0]*h[0][2], h[2][0]*h[0][1] - h[2][1]*h[0][0] },
			{ h[0][1]*h[1][2] - h[0][2]*h[1][1], h[0][2]*h[1][0] - h[0][0]*h[1][2], h[0][0]*h[1][1] - h[0][1]*h[1][0] }
		};
		
		// Compute Gaussian and mean curvature
		float numer1 = 0, numer2 = 0, g2 = 0, tr = 0 ;
		for ( i = 0 ; i < 3 ; i ++ )
		{
			for ( j = 0 ; j < 3 ; j ++ )
			{
				numer1 += g[i] * g[j] * h[i][j] ;
				numer2 += g[i] * g[j] * ch[i][j] ;
			}
			g2 += g[i] * g[i] ;
			tr += h[i][i] ;
		}
		
		curvG = numer2 / (g2 * g2) ;
		curvM = ( numer1 - g2 * tr ) / ( 2 * g2 * (float) sqrt( g2 ) ) ;

		// Compute principal curvatures
		float desc =  curvM * curvM - curvG ;
		if ( desc < 0 )
		{
			// printf("Negative descriminant! %f \n", desc) ;
			desc = 0 ;
		}
		else
		{
			// printf("Positive descriminant! %f \n", desc) ;
		}
		desc = (float) sqrt( desc ) ;
		curvP1 = curvM + desc ;
		curvP2 = curvM - desc ;

/*
		printf("point: (%f %f %f)\n", x, y, z) ;
		printf("Value: %f\n", f->eval(x, y, z) ) ;

		
		printf("gradient: %f %f %f\n", g[0], g[1], g[2]) ;
		printf("hessian: (%f %f %f) (%f %f %f) (%f %f %f)\n", h[0][0], h[0][1], h[0][2], h[1][0], h[1][1], h[1][2], h[2][0], h[2][1], h[2][2]) ;

		printf("Gaussian curvature: %f\n", curvG) ;
		printf("Mean curvature: %f\n", curvM) ;

		printf("Principal curvatures: %f %f\n", curvP1, curvP2) ;

		exit(0) ;
*/
	}
};























#endif