#define __DRIVER

#include "tracker.h"
#include "memmgr.h"
#ifndef __DRIVER
#include "Painter.hpp"
#endif

#include "tracking_kalman.c"

#include "AdjustTable.h"

/* Shorthand for type of comparison functions.  */
#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) (__const void *, __const void *);

# ifdef	__USE_GNU
typedef __compar_fn_t comparison_fn_t;
# endif
#endif

extern double sqrt(double);
extern double log(double);
extern double atan(double);
extern float  sqrtf(float);
//extern void qsort (void *b, size_t n, size_t s, __compar_fn_t cmp);
void _quicksort (void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp);
// STATE
#define STATE_P0 0
#define STATE_P1 1
#define STATE_D2 2
#define STATE_D4 3
#define STATE_D4G 4
// GESTURE_STATE
#define GESTURE_STATE_PIVOT 0
#define GESTURE_STATE_MULTI_ARCS 1
// SUBTYPE
// D2 subtype
#define SUBTYPE_S2 0
#define SUBTYPE_N2 1
#define SUBTYPE_E2 2
#define SUBTYPE_W2 3 
// D4 subtype
#define SUBTYPE_SE4 4
#define SUBTYPE_SW4 5
#define SUBTYPE_NE4 6
#define SUBTYPE_NW4 7 
// D4G subtype
#define SUBTYPE_SSW4 8
#define SUBTYPE_SSE4 9
#define SUBTYPE_NNW4 10
#define SUBTYPE_NNE4 11
#define SUBTYPE_ENE4 12
#define SUBTYPE_ESE4 13
#define SUBTYPE_WNW4 14
#define SUBTYPE_WSW4 15
#define SUBTYPE_ERR  9999
//
#define DIRECTION_VERTICAL 0
#define DIRECTION_HORIZONTAL 1

#ifdef __DRIVER
#define assert
#endif 

#define ASSERT_SUBTYPE_D2  //TT    assert( CTP.Subtype >= 0 && CTP.Subtype <= 3 );
#define ASSERT_SUBTYPE_D4  //TT    assert( CTP.Subtype >= 4 && CTP.Subtype <= 7 );
#define ASSERT_SUBTYPE_D4G //TT    assert( CTP.Subtype >= 8 && CTP.Subtype <= 15 );

#ifndef __DRIVER
Painter PainterPre, PainterPost, PainterAdjust;
extern char PatternName[128];
extern float NewRawData[FRAMENUM][AXISNUM][MAXSIZE];
#endif

GLOBAL Global;
float DynaKalR;

#ifdef __DRIVER
float
absf(float a)
{
    a = a > 0 ? a : (-a);

    return a;
}
#endif

TouchPointInfo Centroid, CentroidPrev, CentroidD2, CentroidTrack, CentroidKalman, CentroidKalmanPrev;
TouchPointInfo Pivot, PivotPrev;
TouchPointInfo TPSort[MAXTOUCHNUM], TPTrack[MAXTOUCHNUM], TPTrackPrev[MAXTOUCHNUM];
TouchPointInfo TPAdjust[16][MAXTOUCHNUM];
TouchPointInfo FirstTouch;

static int initial = 1;

struct Frame
{
    int TPnum;
    TouchPointInfo TP[4];
} ;

// Circular Queue
#define CQSIZE 128
struct 
{
    int cp;
    struct Frame frame[CQSIZE]; 
} InQueue, OutQueue;

typedef struct 
{
    // STATE
    int State;
    int StateNext;
    // GESTURE_STATE
    int GState;
    int GStateNext;
    // SUBTYPE
    int Subtype;
    int SubtypeD2;
    //
    int Direction;
    int DirectionPrev;
    int DirectionD2;
    //
    int Birth;
    int Death;
    int Age;
    //
    int TPnumPrev;
    int TPnumTrackPrev;

    // Adjust Table
    int Adjust;
    int AdjustTotal;

    // Track ID map
    int TID[2];
    int TIDNext;
    float Radius;
    float RadiusPrev;
    float RadiusD2;

    float Width;
    float WidthPrev;
    float Height;
    float HeightPrev;

    //
    int Initial;
    int Frame;
    int Snapshot;

#ifndef __DRIVER
    FILE *log;
#endif

} CTrackerPivot;
CTrackerPivot CTP;

//TT   #ifdef __DRIVER
//TT   int _cdecl SortPointsCallback ( const void* a_, const void* b_ )
//TT   #else
int SortPointsCallback ( const void* a_, const void* b_ )
//TT   #endif
{    
    TouchPointInfo *a = (TouchPointInfo *) a_, *b = (TouchPointInfo *)b_;

    if ( a->PosY > b->PosY )
        return 1;        
    else if ( a->PosY < b->PosY )
        return -1;

    if ( a->PosX > b->PosX )
        return 1;
    else if ( a->PosX < b->PosX )
        return -1;

    return 0;
}

void constructor(void)
{        
    CTP.Birth       = 0;
    CTP.Death       = 0;

    CTP.Age         = 0;
    CTP.Frame       = 1;
    CTP.Snapshot    = 1;

    CTP.State       = STATE_P0;
    CTP.StateNext   = STATE_P0;

    CTP.GState      = GESTURE_STATE_PIVOT;
    CTP.GStateNext  = GESTURE_STATE_PIVOT;

    CTP.Subtype     = SUBTYPE_ERR;

    CTP.TIDNext     = 1;    
    CTP.Radius      = 0;
    CTP.Initial     = 1;

    CTP.Adjust      = -1;
    CTP.AdjustTotal = 0;

#ifndef __DRIVER
    CTP.log   	= NULL;
#endif

    InQueue.cp      = 0;
    OutQueue.cp     = 0;
}

void destructor(void)
{
#ifndef __DRIVER
    if( CTP.log )
        fclose( CTP.log );
#endif
}


// Uniform random number generator
float RandomUniform(void)
{
    static int iseed = 1;

    const int ia=16807,ic=2147483647,iq=127773,ir=2836;
    int il,ih,it;    

    ih = iseed/iq;
    il = iseed%iq;
    it = ia*il-ir*ih;
    iseed = ( it > 0 ) ? it : ic+it;    
    return iseed/(float)ic;
}

// Gaussian random number generator
float RandomGaussian (const float mean, const float sigma)
{
    float x, y, r2;

    do
    {
        /* choose x,y in uniform square (-1,-1) to (+1,+1) */

        x = -1 + 2 * RandomUniform();
        y = -1 + 2 * RandomUniform();

        /* see if it is in the unit circle */
        r2 = x * x + y * y;
    }
    while (r2 > 1.0 || r2 == 0);

    /* Box-Muller transform */
    return mean + sigma * y * (float)sqrt((-2.0) * log(r2) / r2);
}

void SortPoints( int *TPnum, TouchPointInfo *TP )
{
    // Sort points in Z-order
    //
    // 0->1        
    //   /        
    //  /
    // 2->3
    //
    //qsort( TP, *TPnum, sizeof(TouchPointInfo), SortPointsCallback );
    _quicksort( TP, *TPnum, sizeof(TouchPointInfo), SortPointsCallback );
}

void EstimatePoints( int *TPnum, TouchPointInfo *TP )
{
    if ( *TPnum == 1 )  // 1 -> 2
    {   
        TP[0].PosX = Centroid.PosX - CTP.WidthPrev / 2;
        TP[0].PosY = Centroid.PosY - CTP.HeightPrev / 2;
        TP[1].PosX = Centroid.PosX + CTP.WidthPrev / 2;
        TP[1].PosY = Centroid.PosY + CTP.HeightPrev / 2;

        *TPnum = 2;       
    }
    else                // 2 -> 4
    {
        TP[0].PosX = Centroid.PosX - CTP.WidthPrev / 2;
        TP[0].PosY = Centroid.PosY - CTP.HeightPrev / 2;

        TP[1].PosX = Centroid.PosX + CTP.WidthPrev / 2;
        TP[1].PosY = Centroid.PosY + CTP.HeightPrev / 2;

        TP[2].PosX = Centroid.PosX + CTP.WidthPrev / 2;
        TP[2].PosY = Centroid.PosY - CTP.HeightPrev / 2;

        TP[3].PosX = Centroid.PosX - CTP.WidthPrev / 2;
        TP[3].PosY = Centroid.PosY + CTP.HeightPrev / 2;

        *TPnum = 4;
    } 

    CTP.Width = CTP.WidthPrev;
    CTP.Height = CTP.HeightPrev;

    SortPoints( TPnum, TP );                        
}

int FindPivot( int *TPnum, TouchPointInfo *TP )
{
    int i;
	float dist[MAXTOUCHNUM];
    float mini = 10000;
    int idx = 999;

    for ( i = 0 ; i < *TPnum ; ++i )
    {
        float dX = TP[i].PosX - Pivot.PosX, dY = TP[i].PosY - Pivot.PosY;
        dist[i] = dX * dX + dY * dY;
    }

    for ( i = 0 ; i < *TPnum ; ++i )
    {	
        if ( dist[i] < mini )
        {
            mini = dist[i];
            idx = i;
        }
    }

    return idx;
}

float CalculateAngle( TouchPointInfo *TP )
{
    float dX = TP[3].PosX - TP[0].PosX;
    float dY = TP[3].PosY - TP[0].PosY;
    return ( float ) (atan( dY / dX ) / 3.1415926f * 180);
}

float CalculateRadius( int *TPnum, TouchPointInfo *TP )
{
    float dX = 0;
    float dY = 0;

    switch( *TPnum )
    {
    case 1:
        dX = 0;
        dY = 0;
        break;
    case 2:
        dX = TP[1].PosX - TP[0].PosX;
        dY = TP[1].PosY - TP[0].PosY;
        break;
    case 4:
        dX = TP[3].PosX - TP[0].PosX;
        dY = TP[3].PosY - TP[0].PosY;
        break;
    }

    return sqrtf( dX * dX + dY * dY );
}

float CalculateDistance( TouchPointInfo *TP0, TouchPointInfo *TP1 )
{
    float dX = TP1->PosX - TP0->PosX;
    float dY = TP1->PosY - TP0->PosY;

    return sqrtf( dX * dX + dY * dY );
}

int DiffIndex( TouchPointInfo a, TouchPointInfo b )
{
    int Diff[2];
    Diff[0] = ( a.PosX > b.PosX ) ? 1 : 0;
    Diff[1] = ( a.PosY > b.PosY ) ? 1 : 0;                
    return Diff[0] + Diff[1]*2;
}

void KalmanFilter( int *TPnum, TouchPointInfo *TP  )
{
    int i;
	float CurXY[2];
    DynaKalR = Global.Kalman_R;

    for ( i = 0 ; i < *TPnum ; i++ )
    {
        CurXY[0] = TP[i].PosX;
        CurXY[1] = TP[i].PosY;

        if( 0 == Global.meansmoothEn )
            Adaptive_tracking( CurXY, TP[i].ID, TP[i].State , DynaKalR );
        else if( 1== Global.meansmoothEn )
        {
            Meansmooth( CurXY, TP[i].ID, TP[i].State );
            Adaptive_tracking( CurXY, TP[i].ID, TP[i].State , DynaKalR );
        }

        TP[i].PosX = CurXY[0];
        TP[i].PosY = CurXY[1];         
    }
}

float Round( float x )
{
	float y = (float)(int)(x + 0.5f);
    return y;
}

void CalculateCentroid( int *TPnum, TouchPointInfo* TP, TouchPointInfo *centroid )
{        
#ifndef __DRIVER
	int sX = 0, eX = 0, sY = 0, eY = 0, x = 0, y = 0;
	float ratio[42], sum = 0;
#endif

    // Calculate centroid 
    if ( 2 == Global.CentroidMethod )
    {
        switch( *TPnum )
        {
        case 1:
            centroid->PosX = Round( TP[0].PosX );
            centroid->PosY = Round( TP[0].PosY );
            break;

        case 2:
            centroid->PosX = ( Round( TP[0].PosX ) + Round( TP[1].PosX ) ) / 2;
            centroid->PosY = ( Round( TP[0].PosY ) + Round( TP[1].PosY ) ) / 2;
            break;

        case 4:
            centroid->PosX = ( Round( TP[0].PosX ) + Round( TP[1].PosX ) + Round( TP[2].PosX ) + Round( TP[3].PosX ) ) / 4;
            centroid->PosY = ( Round( TP[0].PosY ) + Round( TP[1].PosY ) + Round( TP[2].PosY ) + Round( TP[3].PosY ) ) / 4;
            break;
        }                    
    }
    else
    {
        if ( 1 == Global.CentroidWeight )
        {
#ifndef __DRIVER
            
            switch( *TPnum )
            {
            case 1:
                centroid->PosX = TP[0].PosX;
                centroid->PosY = TP[0].PosY;
                return;
                break;

            case 2:
                sX = (int)Round(TP[0].PosX) - 3;
                eX = (int)Round(TP[1].PosX) + 3;
                sY = (int)Round(TP[0].PosY) - 3;
                eY = (int)Round(TP[1].PosY) + 3;
                break;

            case 4:
                sX = (int)Round(TP[0].PosX) - 1;
                eX = (int)Round(TP[3].PosX) + 1;
                sY = (int)Round(TP[0].PosY) - 1;
                eY = (int)Round(TP[3].PosY) + 1;
                break;
            }

            if ( sX < 0 ) sX = 0;
            if ( eX >= Global.TPANEL_WIDTH ) eX = Global.TPANEL_WIDTH - 1;
            if ( sY < 0 ) sY = 0;
            if ( eY >= Global.TPANEL_HEIGHT ) eY = Global.TPANEL_HEIGHT - 1;

            centroid->PosX = 0;
            centroid->PosY = 0;
            for ( x = sX ; x <= eX ; x++ )
                sum += NewRawData[1][0][x];

            for ( x = sX ; x <= eX ; x++ )
                ratio[x] =  NewRawData[1][0][x] / sum;

            for ( x = sX ; x <= eX ; x++ )
            {
                centroid->PosX += x * ratio[x];                                        
            }

            sum = 0;
            for ( y = sY ; y <= eY ; y++ )
                sum += NewRawData[1][1][y];

            for ( y = sY ; y <= eY ; y++ )
                ratio[y] =  NewRawData[1][1][y] / sum;

            for ( y = sY ; y <= eY ; y++ )
            {
                centroid->PosY += y * ratio[y];                                        
            }
#endif
        }
        else
        {
            switch( *TPnum )
            {
            case 1:
                centroid->PosX = TP[0].PosX;
                centroid->PosY = TP[0].PosY;
                break;

            case 2:
                centroid->PosX = ( TP[0].PosX + TP[1].PosX ) / 2;
                centroid->PosY = ( TP[0].PosY + TP[1].PosY ) / 2;
                break;

            case 4:
                centroid->PosX = ( TP[0].PosX + TP[1].PosX + TP[2].PosX + TP[3].PosX ) / 4;
                centroid->PosY = ( TP[0].PosY + TP[1].PosY + TP[2].PosY + TP[3].PosY ) / 4;
                break;

            case 6:
                centroid->PosX = ( TP[0].PosX + TP[1].PosX + TP[2].PosX + TP[3].PosX + TP[4].PosX + TP[5].PosX ) / 6;
                centroid->PosY = ( TP[0].PosY + TP[1].PosY + TP[2].PosY + TP[3].PosY + TP[4].PosY + TP[5].PosY ) / 6;
                break;
            }                    
        }
    }
}

#ifndef __DRIVER
void PrintTouchPointInfo( int *TPnum, TouchPointInfo* TP, bool state )
{
    static const char* TouchPointState_str[] = 
    { 
        "BIRTH",    // 0
        "DEATH",    // 1
        "MOVE",     // 2
        "UNKNOWN"
    };

    for ( int i = 0 ; i < *TPnum ; i++ )           
    {
        int idx = TP[i].State;
        if( idx < 0 || idx > 2 )
            idx = 3;
        fprintf( CTP.log, "    %d %3.4f %3.4f\t%s\n", TP[i].ID, TP[i].PosX, TP[i].PosY, (state) ? TouchPointState_str[ idx ] : "" );
    }
}

void InitialLog()
{
	char filename[128];
    if( 1 == Global.PivotTrackerLogEn )
    {
        if( CTP.log == NULL )
        {                                
            sprintf( filename, "%s_track.log", PatternName );
            CTP.log = fopen( filename, "w" );            

            if ( 1 == Global.PainterEn && 0 == Global.PainterMultiFrame )
            {
                if ( 1 == Global.PainterPreTrack )
                {
                    sprintf( filename, "%s_snapshot_pretrack", PatternName );
                    PainterPre.Open(filename);
                }
                sprintf( filename, "%s_snapshot", PatternName );
                PainterPost.Open(filename);

                if ( 1 == Global.TrackAdjustMode )
                {
                    sprintf( filename, "%s_snapshot_adjust", PatternName );
                    PainterAdjust.Open(filename);
                }
            }
        }
    }

    CTP.Initial = 0;
}

void PreLog( int *TPnum, TouchPointInfo* TP )
{
    fprintf( CTP.log, "Frame %d : Input TPnum = %d\n", CTP.Frame, *TPnum);
    PrintTouchPointInfo( TPnum, TP, false );
    fflush( CTP.log );

    if ( 1 == Global.PainterPreTrack )
    {
        PainterPre.Plot( TPnum, TP );
    }
}

void PostLog( int *TPnum, TouchPointInfo* TP )
{
    static const char* State_str[] = { "P0", "P1", "D2", "D4", "D4G" };
    static const char* Subtype_str[] = { "S2", "N2", "E2", "W2", "SE4", "SW4", "NE4", "NW4", "SSW4", "SSE4", "NNW4", "NNE4", "ENE4", "ESE4", "ENE4", "ESE4" };
	char filename[128];
	int TPnumTemp = 1;
	
    fprintf( CTP.log, "=== %s => %s", State_str[CTP.State], State_str[CTP.StateNext] );    
    if ( STATE_P0 != CTP.State && STATE_P1 != CTP.State )
        fprintf( CTP.log, ", %s ===\n" , Subtype_str[CTP.Subtype] );
    else
        fprintf( CTP.log, "===\n" );

    PrintTouchPointInfo( TPnum, TP, true );

    if ( STATE_D4G == CTP.State )
    {   
        fprintf( CTP.log, "      CentroidD2      %2.4f, %2.4f\n", CentroidD2.PosX, CentroidD2.PosY );
        fprintf( CTP.log, "      CentroidKalman  %2.4f, %2.4f\n", CentroidKalman.PosX, CentroidKalman.PosY );
        fprintf( CTP.log, "      Age             %d\n", CTP.Age );
    }

    fprintf( CTP.log, "\n" );
    fflush( CTP.log );

    if ( 1 == Global.PainterEn && 1 == Global.PainterMultiFrame )
    {
        if ( CTP.StateNext != CTP.State )
        {
            if ( STATE_P0 == CTP.State )
            {
                if ( 1 == Global.PainterPreTrack )
                {
                    sprintf( filename, "%s_snapshot_frame_%d_pretrack", PatternName, CTP.Frame );
                    PainterPre.Open(filename);
                }
                sprintf( filename, "%s_snapshot_frame_%d", PatternName, CTP.Frame );
                PainterPost.Open(filename);

                printf( "Snapshot #%d @ frame %d\n", CTP.Snapshot, CTP.Frame );
            }
            else if ( STATE_P0 == CTP.StateNext )
            {
                if ( 1 == Global.PainterPreTrack )
                {
                    PainterPre.Close();
                }

                if ( !PainterPost.Close() )
                {
                    printf( "Snapshot #%d drop\n", CTP.Snapshot, CTP.Frame );
                }
                else
                {
                    CTP.Snapshot ++;
                }
            }
        }
    }

    PainterPost.Plot( TPnum, TP );

    switch ( CTP.State )
    {
    case STATE_D2:
        CentroidKalman.ID = 7;            
        break;
    case STATE_D4:
        CentroidKalman.ID = 5;            
        break;
    case STATE_D4G:
        CentroidKalman.ID = 6;            
        break;
    default:
        CentroidKalman.ID = 7;    
        break;
    }

    if ( *TPnum > 1 && 1 == Global.PainterCentroid )
    {      
        PainterPost.Plot( &TPnumTemp, &CentroidKalman );

        if ( 1 == Global.PainterPreTrack )
            PainterPre.Plot( &TPnumTemp, &CentroidKalman );
    }
}

void LoadAdjustTable()
{
    // draw grid point 
    int one = 1;
	int first = 1;
    TouchPointInfo TP_tmp;
	FILE* f = fopen( "adjust.txt", "r" );
	TouchPointInfo *tp ;
	
    TP_tmp.ID = 9;
    for ( TP_tmp.PosX = 0 ; TP_tmp.PosX < Global.TPANEL_WIDTH ; TP_tmp.PosX += 1 )
    {
        for ( TP_tmp.PosY = 0 ; TP_tmp.PosY < Global.TPANEL_HEIGHT ; TP_tmp.PosY += 1 )
        {
            //PainterPost.Plot( &one, &TP_tmp );
            PainterAdjust.Plot( &one, &TP_tmp );
        }
    }

    // load & draw adjust points
    

    CTP.AdjustTotal = 0;
    
    tp = TPAdjust[0];
    while( EOF != fscanf( f, "%f %f", &tp->PosX, &tp->PosY ) )
    {   
        if ( first )
        {                       
            tp->ID = 11;
            first = 0;
        }                    
        else                                    
            tp->ID = 10;

        //PainterPost.Plot( &one, tp );
        PainterAdjust.Plot( &one, tp );

        // change to next adjust table
        if ( -1 == tp->PosX && -1 == tp-> PosY )
        {
            CTP.AdjustTotal ++;

            tp = TPAdjust[CTP.AdjustTotal];                    

            first = 1;
        }
        else
        {
            tp++;
        }
    }
}
#endif

void AdjustMode( int *TPnum, TouchPointInfo *TP )
{
    int i, j;
	float mini = 9999;
    int idx = -1;
	float dist;
	TouchPointInfo *tp ;

    for ( j = 0 ; j < *TPnum ; j++ )        
    {
        tp = TPAdjust[CTP.Adjust];

        for( i = 0 ; -1 != tp->PosX && -1 != tp->PosY  ; i++, tp++ )    
        {
            dist = CalculateDistance( &TP[j], tp );

            if ( dist < Global.TrackAdjustMaximumuDistance && dist < mini )
            {
                mini = dist;
                idx = i;
            }
        }

        if ( idx != -1 )
        {
            TP[j].PosX = TPAdjust[CTP.Adjust][idx].PosX;
            TP[j].PosY = TPAdjust[CTP.Adjust][idx].PosY;

            if ( 0.0f != Global.TrackAdjustDither )
            {
                TP[j].PosX = RandomGaussian( TPTrack[j].PosX, Global.TrackAdjustDither );
                TP[j].PosY = RandomGaussian( TPTrack[j].PosY, Global.TrackAdjustDither );
            }

#ifndef __DRIVER	
            if( 1 == Global.PivotTrackerLogEn )
            {
                fprintf( CTP.log, "-> adjust\n", CTP.Adjust ); 
                int one = 1;
                PrintTouchPointInfo( &one, &TPTrack[j], true );
            }
#endif
        }
    }
}

void TrackMapping( int* TPnum )
{
	int i;

    static const int D2_ID[4][2] = 
    {                              
        {0, 1},
        {1, 0},
        {0, 1},
        {1, 0},
    };

    static const int D4_ID[4][2] =
    {
        {0, 3}, 
        {1, 2}, 
        {2, 1}, 
        {3, 0},
    };

    static const int D4G_ID[8][2] = 
    {
        {1, 3},
        {0, 2},
        {3, 1},
        {2, 0},
        {2, 3},
        {0, 1},
        {3, 2}, 
        {1, 0},
    };


    // TID mapping
    switch ( CTP.Birth )
    {
    case 1:
        CTP.TID[ 0 ] = CTP.TIDNext;
        CTP.TIDNext = 3 - CTP.TIDNext;
        break;
    case 2:
        CTP.TID[ 1 ] = CTP.TIDNext;
        CTP.TIDNext = 3 - CTP.TIDNext;
        break;
    case 3:
        CTP.TID[ 0 ] = CTP.TIDNext;
        CTP.TIDNext = 3 - CTP.TIDNext;
        CTP.TID[ 1 ] = CTP.TIDNext;
        CTP.TIDNext = 3 - CTP.TIDNext;
        break;
    }

    // Determine touch points state 
    switch ( CTP.StateNext)
    {
    case STATE_P0:
        *TPnum = 0;
        break;

    case STATE_P1:
        TPTrack[0] = TPSort[0];
        TPTrack[0].ID = CTP.TID[0];
        TPTrack[0].State = POINT_MOVE;
        *TPnum = 1;
        break;

    case STATE_D2:        
        ASSERT_SUBTYPE_D2;
        for ( i = 0 ; i < 2 ; i++ )
        {
            TPTrack[i] = TPSort[D2_ID[CTP.Subtype][i]];
            TPTrack[i].ID = CTP.TID[i];
            TPTrack[i].State = POINT_MOVE;
        }
        //*TPnum = 2;
        break;

    case STATE_D4G:
        ASSERT_SUBTYPE_D4G;
        for ( i = 0 ; i < 2 ; i++ )
        {
            TPTrack[i] = TPSort[D4G_ID[CTP.Subtype-8][i]];
            TPTrack[i].ID = CTP.TID[i];
            TPTrack[i].State = POINT_MOVE;
        }
        *TPnum = 2;
        break;

    case STATE_D4:
        ASSERT_SUBTYPE_D4;
        for ( i = 0 ; i < 2 ; i++ )
        {             
            TPTrack[i] = TPSort[D4_ID[CTP.Subtype-4][i]];
            TPTrack[i].ID = CTP.TID[i];
            TPTrack[i].State = POINT_MOVE;
        }
        *TPnum = 2;
        break;
    } // end of IDs

    // Birth
    switch ( CTP.Birth )
    {
    case 1:
        TPTrack[ 0 ].State = POINT_BIRTH;
        break;
    case 2:
        TPTrack[ 1 ].State = POINT_BIRTH;
        break;
    case 3:
        TPTrack[ 0 ].State = POINT_BIRTH;
        TPTrack[ 1 ].State = POINT_BIRTH;
        break;
    }

    // Death
    switch ( CTP.Death )
    {
    case 1:
        TPTrack[0] = ( POINT_DEATH == TPTrackPrev[0].State ) ? TPTrackPrev[1] : TPTrackPrev[0];
        TPTrack[0].State = POINT_DEATH;
        TPTrack[0].ID = CTP.TID[0];

        CTP.TIDNext = CTP.TID[0];

        CTP.TID[0] = CTP.TID[1];
        CTP.TID[1] = 0;
        (*TPnum) ++;
        break;

    case 2:
        TPTrack[1] = TPTrackPrev[1];
        TPTrack[1].State = POINT_DEATH;
        TPTrack[1].ID = CTP.TID[1];

        CTP.TIDNext = CTP.TID[1];

        CTP.TID[1] = 0;
        (*TPnum) ++;
        break;

    case 3:
        TPTrack[0] = TPTrackPrev[0];
        TPTrack[0].State = POINT_DEATH;
        TPTrack[0].ID = CTP.TID[0];

        TPTrack[1] = TPTrackPrev[1];
        TPTrack[1].State = POINT_DEATH;
        TPTrack[1].ID = CTP.TID[1];

        CTP.TIDNext = 1;

        CTP.TID[0] = CTP.TID[1] = 0;
        (*TPnum) += 2;
        break;		
    }        
}


void Step( int *TPnum, TouchPointInfo *TP )
{
	int i;
    int idx;
    float theta = 0;
    int result = 0;
	float dx, dy, dist;
	int TPnum_tmp ;
	TouchPointInfo *tp;
	TouchPointInfo CentroidTemp;

    static const int D2_D4_TABLE[4][4] = 
    {
        {SUBTYPE_SW4, SUBTYPE_SE4, SUBTYPE_SW4, SUBTYPE_SE4}, 
        {SUBTYPE_NW4, SUBTYPE_NE4, SUBTYPE_NW4, SUBTYPE_NE4},
        {SUBTYPE_NE4, SUBTYPE_NE4, SUBTYPE_SE4, SUBTYPE_SE4},
        {SUBTYPE_NW4, SUBTYPE_NW4, SUBTYPE_SW4, SUBTYPE_SW4}
    };

    static const int D2_D4G_TABLE[4][4] = 
    {
        {SUBTYPE_SSW4, SUBTYPE_SSE4, SUBTYPE_SSW4, SUBTYPE_SSE4}, 
        {SUBTYPE_NNW4, SUBTYPE_NNE4, SUBTYPE_NNW4, SUBTYPE_NNE4},
        {SUBTYPE_ENE4, SUBTYPE_ENE4, SUBTYPE_ESE4, SUBTYPE_ESE4},
        {SUBTYPE_WNW4, SUBTYPE_WNW4, SUBTYPE_WSW4, SUBTYPE_WSW4}
    };

    static const int D4_D2_TABLE[4][2] =
    {
        {SUBTYPE_S2, SUBTYPE_E2},
        {SUBTYPE_S2, SUBTYPE_W2},
        {SUBTYPE_N2, SUBTYPE_E2},
        {SUBTYPE_N2, SUBTYPE_W2}
    };

    static const int D4G_D2_TABLE[8][2] =
    {
        {SUBTYPE_S2, SUBTYPE_W2},
        {SUBTYPE_S2, SUBTYPE_E2},
        {SUBTYPE_N2, SUBTYPE_W2},
        {SUBTYPE_N2, SUBTYPE_E2},
        {SUBTYPE_N2, SUBTYPE_E2},
        {SUBTYPE_S2, SUBTYPE_E2},
        {SUBTYPE_N2, SUBTYPE_W2},
        {SUBTYPE_S2, SUBTYPE_W2},
    };

    static const int D4G_D4_TABLE[2][4] = 
    {
        {SUBTYPE_SW4, SUBTYPE_SE4, SUBTYPE_NW4, SUBTYPE_NE4}, 
        {SUBTYPE_NE4, SUBTYPE_NW4, SUBTYPE_SE4, SUBTYPE_SW4}, 
    };


    static const int D2_D4_SWAP[4][4] = 
    {
        {0, 0, 1, 1},
        {1, 1, 0, 0}, 
        {0, 1, 0, 1},
        {1, 0, 1, 0},
    };

#ifndef __DRIVER
    if ( CTP.Initial )
    {
        InitialLog();

        if ( 1 == Global.TrackAdjustMode )
            LoadAdjustTable();

        CTP.Initial = 0;
    }

    if( 1 == Global.PivotTrackerLogEn )
        PreLog( TPnum, TP );
#else    
    if ( CTP.Initial )
    {     
        if ( 1 == Global.TrackAdjustMode )
        {
            CTP.AdjustTotal = 0;
            tp = TPAdjust[0];

            for ( i = 0 ; i < sizeof(AdjustTable)/sizeof(float) ; i += 2 )
            {
                tp->PosX = AdjustTable[i];
                tp->PosY = AdjustTable[i+1];

                // change to next adjust table
                if ( -1 == tp->PosX && -1 == tp-> PosY )
                {
                    CTP.AdjustTotal ++;

                    tp = TPAdjust[CTP.AdjustTotal];                    
                }
                else
                {
                    tp++;
                }
            }            
        }

        CTP.Initial = 0;
    }
#endif

    // Input Queue
    InQueue.frame[InQueue.cp].TPnum = *TPnum;
    MEMCOPY( InQueue.frame[InQueue.cp].TP, TP, sizeof(TouchPointInfo) * (*TPnum) );
    InQueue.cp = ( InQueue.cp + 1 ) % CQSIZE;

    if( *TPnum > 0 )
    {
        // Sort points
        MEMCOPY( TPSort, TP, sizeof(TouchPointInfo) * (*TPnum) );
        SortPoints( TPnum, TPSort );     

        switch( *TPnum )
        {
        case 1:
            CTP.Width = 0;
            CTP.Height = 0;
            break;

        case 2:
            CTP.Width = TP[1].PosX - TP[0].PosX;
            CTP.Height = TP[1].PosY - TP[0].PosY;
            break;

        case 4:
            CTP.Width = TP[3].PosX - TP[0].PosX;
            CTP.Height = TP[3].PosY - TP[0].PosY;
            break;
        }

        if ( 2 == *TPnum )                            
            CTP.Direction = ( TPSort[0].PosX == TPSort[1].PosX ) ? DIRECTION_VERTICAL : DIRECTION_HORIZONTAL;

        CalculateCentroid( TPnum, TPSort, &Centroid );
        CTP.Radius = CalculateRadius( TPnum, TPSort );

        if ( 1 == Global.CentroidMethod )
        {
            if ( *TPnum > 1 )
            {
                if ( 1 == CTP.TPnumPrev )
                {
                    CentroidKalman.PosX = Centroid.PosX;
                    CentroidKalman.PosY = Centroid.PosY;
                    CentroidKalman.State = POINT_BIRTH;
                    CentroidKalman.ID = 3;

                    TPnum_tmp = 1;
                    KalmanFilter( &TPnum_tmp, &CentroidKalman );                        
                }
                else
                {
                    CentroidKalman.PosX = Centroid.PosX;
                    CentroidKalman.PosY = Centroid.PosY;
                    CentroidKalman.State = POINT_MOVE;
                    CentroidKalman.ID = 3;

                    TPnum_tmp = 1;
                    KalmanFilter( &TPnum_tmp, &CentroidKalman );                        
                }
            }
        }

        CTP.TPnumPrev = *TPnum;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////            
    // Track FSM
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    switch( CTP.State )
    {
    case STATE_P0:
        switch( *TPnum )
        {
        case 0:
            CTP.Birth = 0;
            CTP.Death = 0;
            CTP.Age = 0;
            CTP.StateNext = STATE_P0;
            break;
        case 1:
            if ( CTP.Age < Global.Threshold_Point0_Point1 )
            {
                CTP.Birth = 0;
                CTP.Death = 0;
                CTP.Age = CTP.Age + 1;
                CTP.StateNext = STATE_P0;
            }
            else
            {
                FirstTouch = TPSort[0];

                if ( 1 == Global.TrackAdjustMode )
                {                                                        
                    float mini = 9999;
                    int idx = -1;

                    // search initial adjust point
                    for ( i = 0 ; i < CTP.AdjustTotal ; i++ )
                    {
                        float dist = CalculateDistance( &TPAdjust[i][0], &FirstTouch );

                        if ( dist < Global.TrackAdjustMaximumuDistance && dist < mini )
                        {
                            mini = dist;
                            idx = i;
                        }
                    }

                    if ( -1 != idx )
                    {
                        CTP.Adjust = idx;

#ifndef __DRIVER	
                        if( 1 == Global.PivotTrackerLogEn )
                        {
                            fprintf( CTP.log, "Select Adjust Table #%d\n", CTP.Adjust ); 
                        }
#endif
                    }
                }

                CTP.Birth = 1;
                CTP.Death = 0;                        
                CTP.Age = 0;
                CTP.StateNext = STATE_P1;
            }
            break;
        case 2:                                    
            FirstTouch = TPSort[0];
            CTP.Adjust = -1;

            CTP.Birth = 1;
            CTP.Death = 0;  
            CTP.Age = 0;
            CTP.StateNext = STATE_P1;                    
            break;
        case 4:
            CTP.Birth = 0;
            CTP.Death = 0;
            CTP.Age = 0;
            CTP.StateNext = STATE_P0;
            break;
        }
        break;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 1 Point            
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    case STATE_P1:
        switch( *TPnum )
        {
        case 0:
            CTP.Birth = 0;
            CTP.Death = 1;
            CTP.Age = 0;
            CTP.StateNext = STATE_P0;
            break;
        case 1:
            CTP.Birth = 0;
            CTP.Death = 0;
            CTP.Age = 0;
            CTP.StateNext = STATE_P1;
            break;
        case 2:
            if ( CTP.Age < Global.Threshold_Point1_Dual2 )
            {
                idx = FindPivot( TPnum, TPSort );                    
                TPSort[0] = TPSort[idx];
                *TPnum = 1;

                CTP.Birth = 0;
                CTP.Death = 0;                            
                CTP.Age = CTP.Age + 1;
                CTP.StateNext = STATE_P1;
            }
            else                     
            {	
                idx = FindPivot( TPnum, TPSort );
                CTP.Subtype = CTP.Direction * 2 + idx;
                ASSERT_SUBTYPE_D2;

                CTP.Birth = 2;
                CTP.Death = 0;
                CTP.Age = 0;
                CTP.StateNext = STATE_D2;                    
            }
            break;
        case 4:
            idx = FindPivot( TPnum, TPSort );
            CTP.Subtype = idx + 4;
            ASSERT_SUBTYPE_D4;

            CTP.Birth = 2;
            CTP.Death = 0;                        
            CTP.Age = 0;
            CTP.StateNext = STATE_D4;                    
            break;
        }
        break;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 2 Points            
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    case STATE_D2:
        switch( *TPnum )
        {
        case 0:
            if ( CTP.Age <  Global.Threshold_Dual2_Point0 )
            {
                CTP.Birth = 0;
                CTP.Death = 0;                            
                CTP.Age = CTP.Age + 1;                               
                CTP.StateNext = STATE_D2;
            }
            else
            {
                CTP.Birth = 0;
                CTP.Death = 3;                            
                CTP.Age = 0;                        
                CTP. StateNext = STATE_P0;
            }
            break;

        case 1:
            if ( CTP.Age < Global.Threshold_Dual2_Point1 )
            {
                Centroid = CentroidKalman;                        
                EstimatePoints( TPnum, TPSort );   

                CTP.Birth = 0;
                CTP.Death = 0;
                CTP.Age = CTP.Age + 1;                               
                CTP.StateNext = STATE_D2;
            }
            else
            {
                Pivot = TPSort[0];                    
                idx = FindPivot( &CTP.TPnumTrackPrev, TPTrackPrev );  

                CTP.Birth = 0;
                CTP.Death = 2 - idx;                            
                CTP.Age = 0;                        
                CTP.StateNext = STATE_P1;
            }
            break;

        case 2:
            // from vertical to horizontal
            if ( CTP.Direction != CTP.DirectionPrev )
            {
                idx = FindPivot( TPnum, TPSort );  
                CTP.Subtype = CTP.Direction * 2 + idx;
                ASSERT_SUBTYPE_D2;
            }

            CTP.Birth = 0;
            CTP.Death = 0;                        
            CTP.Age = 0;
            CTP.StateNext = STATE_D2;
            break;

        case 4:
            if ( CTP.Age < Global.Threshold_Dual2_Point4 )
            {
                // Drop 2 points                    
                MEMCOPY( TPSort, TPTrackPrev, sizeof(TouchPointInfo) * (CTP.TPnumTrackPrev) );
                SortPoints( &CTP.TPnumTrackPrev, TPSort );  
                dx = CentroidKalman.PosX - CentroidKalmanPrev.PosX;
                dy = CentroidKalman.PosY - CentroidKalmanPrev.PosY;
                TPSort[0].PosX += dx;
                TPSort[1].PosX += dx;
                TPSort[0].PosY += dy;
                TPSort[1].PosY += dy;
                *TPnum = 2;

                CTP.Birth = 0;
                CTP.Death = 0;                        
                CTP.Age = CTP.Age + 1;
                CTP.StateNext = STATE_D2;                    
            }
            else
            {
                switch ( Global.PivotMethod )
                {
                case 0:
                    CTP.GState = GESTURE_STATE_PIVOT;
                    break;

                case 1:
                case 2:
                    CTP.GState = GESTURE_STATE_MULTI_ARCS;
                    break;

                case 3:
                    {
                        idx = FindPivot( TPnum, TPSort );                       

                        if ( CalculateDistance( &TPSort[idx], &FirstTouch ) > 1 )
                            CTP.GState = GESTURE_STATE_MULTI_ARCS;
                        else
                            CTP.GState = GESTURE_STATE_PIVOT;
                    }
                } // end of Global.PivotMethod

                switch ( CTP.GState )
                {
                case GESTURE_STATE_PIVOT:
                    if ( 1 == Global.CentroidMethod )
                        idx = DiffIndex ( CentroidKalman, CentroidKalmanPrev );   
                    else
                        idx = DiffIndex( Centroid, CentroidPrev );

                    ASSERT_SUBTYPE_D2;
                    CTP.Subtype = D2_D4_TABLE[CTP.Subtype][idx];          
                    ASSERT_SUBTYPE_D4;

                    CTP.Birth = 0;
                    CTP.Death = 0;                        
                    CTP.Age = 0;
                    CTP.StateNext = STATE_D4;  
                    break;

                case GESTURE_STATE_MULTI_ARCS: 
                    CentroidD2  = ( 1 == Global.CentroidMethod ) ? CentroidKalmanPrev : CentroidPrev ; 
                    CTP.DirectionD2 = CTP.Direction;
                    CTP.SubtypeD2   = CTP.Subtype;
                    CTP.RadiusD2    = CTP.RadiusPrev;

                    idx = DiffIndex( Centroid, CentroidD2 );  

                    ASSERT_SUBTYPE_D2;
                    CTP.Subtype = D2_D4G_TABLE[CTP.Subtype][idx];
                    ASSERT_SUBTYPE_D4G;

                    CTP.Birth = 0;
                    CTP.Death = 0;                        
                    CTP.Age = 0;
                    CTP.StateNext = STATE_D4G;
                    break;
                } // end of CTP.GState
            }
            break;
        }
        break;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 4 Points with ghost mapping
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    case STATE_D4G:
        switch( *TPnum )
        {
        case 1:
            Pivot = TPSort[0];                    
            idx = FindPivot( &CTP.TPnumTrackPrev, TPTrackPrev );  

            CTP.Birth = 0;
            CTP.Death = 2 - idx;                            
            CTP.Age = 0;                        
            CTP.StateNext = STATE_P1;
            break;

        case 2:                                
            if ( CTP.Age < Global.Threshold_Dual4_Dual2 )
            {
                Centroid = CentroidKalman;                        
                EstimatePoints( TPnum, TPSort );

                CTP.Birth = 0;
                CTP.Death = 0;                            
                CTP.Age = CTP.Age + 1;                                                
                CTP.StateNext = STATE_D4G;
            }
            else                                                                      
            {                    
                ASSERT_SUBTYPE_D4G;
                CTP.Subtype = D4G_D2_TABLE[CTP.Subtype - 8][CTP.Direction];
                ASSERT_SUBTYPE_D2;

                CTP.Birth = 0;
                CTP.Death = 0;                            
                CTP.Age = 0;
                CTP.StateNext = STATE_D2;
            }
            break;  

        case 4:
            switch( Global.PivotMethod )
            {
            case 1:
                theta = CalculateAngle( TPSort );

                if ( DIRECTION_VERTICAL == CTP.DirectionD2 )                    
                    theta = 90 - theta;

                result = theta > Global.CentroidAngle;
                break;

            case 2:
            case 3:
                {                
                    CentroidTemp = ( 1 == Global.CentroidMethod ) ? CentroidKalman : CentroidPrev;

                    dist = absf( ( DIRECTION_VERTICAL == CTP.DirectionD2 ) ? CentroidTemp.PosY - CentroidD2.PosY : CentroidTemp.PosX - CentroidD2.PosX );

                    if ( Global.CentroidDeltaRatio > 0 )
                        result = dist > ( CTP.RadiusD2 * Global.CentroidDeltaRatio );
                    else
                        result = dist > Global.CentroidDelta ;
                }
                break;
            }

            CTP.Age = ( result ) ? CTP.Age + 1 : 0;

            if ( CTP.Age > Global.Threshold_Dual4Ghost_Dual4 )
            {                    
                idx = DiffIndex ( ( 1 == Global.CentroidMethod ) ? CentroidKalman : CentroidPrev, CentroidD2 );   

                ASSERT_SUBTYPE_D4G;
                CTP.Subtype = D4G_D4_TABLE[CTP.DirectionD2][idx];
                ASSERT_SUBTYPE_D4;

                // Swap ID if D2 and D4 not match
                ASSERT_SUBTYPE_D4;
                if ( D2_D4_SWAP[CTP.SubtypeD2][CTP.Subtype-4] )
                {
                    int temp = CTP.TID[0];
                    CTP.TID[0] = CTP.TID[1];
                    CTP.TID[1] = temp;
                }

                CTP.Birth = 0;
                CTP.Death = 0;                        
                CTP.Age = 0;
                CTP.StateNext = STATE_D4;  
            }
            else
            {                                                        
                CTP.Birth = 0;
                CTP.Death = 0;                                        
                CTP.StateNext = STATE_D4G;
            }			    
            break;
        }
        break;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 4 Points            
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    case STATE_D4:
        switch( *TPnum )
        {
        case 0:
            CTP.Birth = 0;
            CTP.Death = 3;                        
            CTP.Age = 0;                        
            CTP.StateNext = STATE_P0;
            break;

        case 1:
            Pivot = TPSort[0];		
            idx = FindPivot( &CTP.TPnumTrackPrev, TPTrackPrev );  

            CTP.Birth = 0;
            CTP.Death = 2 - idx;   
            CTP.Age = 0;                        
            CTP.StateNext = STATE_P1;
            break;

        case 2:
            if ( CTP.Age < Global.Threshold_Dual4_Dual2 )
            {
                Centroid = CentroidKalman;                        
                EstimatePoints( TPnum, TPSort );

                CTP.Birth = 0;
                CTP.Death = 0;                            
                CTP.Age = CTP.Age + 1;                                                
                CTP.StateNext = STATE_D4;
            }
            else                                                                      
            {
                ASSERT_SUBTYPE_D4;
                CTP.Subtype = D4_D2_TABLE[CTP.Subtype - 4][CTP.Direction];
                ASSERT_SUBTYPE_D2;

                CTP.Birth = 0;
                CTP.Death = 0;                            
                CTP.Age = 0;
                CTP.StateNext = STATE_D2;
            }
            break;

        case 4:
            CTP.Birth = 0;                        
            CTP.Death = 0;
            CTP.Age = 0;
            CTP.StateNext = STATE_D4;  
            break;
        }
        break;

    } // end of State       

    // 
    TrackMapping( TPnum );

    // Adjust Mode
    if ( 1 == Global.TrackAdjustMode && -1 != CTP.Adjust )
        AdjustMode( TPnum, TPTrack );

    // Kalman filter
    if (1 == Global.TrackKalmanEn)
        KalmanFilter( TPnum, TPTrack );

    MEMCOPY( TP, TPTrack, sizeof(TouchPointInfo) * (*TPnum) );

#ifndef __DRIVER	
    if( 1 == Global.PivotTrackerLogEn )   
        PostLog( TPnum, TP );
#endif	

    ////////////////////////////////////////////////////////////////////////////
    // Save current frame 
    ////////////////////////////////////////////////////////////////////////////
    CTP.State           = CTP.StateNext;
    CentroidPrev		= Centroid;
    CentroidKalmanPrev	= CentroidKalman;
    CTP.DirectionPrev   = CTP.Direction;
    CTP.RadiusPrev      = CTP.Radius;
    CTP.WidthPrev       = CTP.Width;
    CTP.HeightPrev      = CTP.Height;

    if ( *TPnum > 0 )
    {
        PivotPrev = Pivot;
        Pivot = TPTrack[0];

        CTP.TPnumTrackPrev = *TPnum;
        MEMCOPY( TPTrackPrev, TPTrack, sizeof(TouchPointInfo) * (*TPnum) );
    }

    // Output Queue
    OutQueue.frame[OutQueue.cp].TPnum = *TPnum;
    MEMCOPY( OutQueue.frame[OutQueue.cp].TP, TP, sizeof(TouchPointInfo) * (*TPnum) );
    OutQueue.cp = ( OutQueue.cp + 1 ) % CQSIZE;

    CTP.Frame++ ;
}

void TrackerPivot( int *TPnum, TouchPointInfo *TP )
{         
    if ( initial )
    {
        constructor();		
        initial = 0;
    }
    else
    {
        Step( TPnum, TP );		
    }

    return;
}

int TPDetector( char *dataLA, int *TPnum, TouchPointInfo *TP )
{
    dataLA = NULL;
    TrackerPivot(TPnum, TP);
    return TOUCH_DETECTED;
}
