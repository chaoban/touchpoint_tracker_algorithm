#include "tracker.h"

struct tagCTP CTP = { STATE_P0, STATE_P0, 0, 1, 1 };

struct Globalt GlobalT =
{
    MAXLINKDP, 
    PIVOTSDP, 

    0,
    0,
    1,
    0,
    0,
    3,
};

void CalculateCentroid( int *TPnum, TouchPointInfo* TP, TouchPointInfo *centroid );

int DiffIndex( TouchPointInfo* a, TouchPointInfo* b )
{
    //
    //   0  |  1
    // -----b----->
    //   2  |  3
    //      V

    int Diff[2];
    Diff[0] = ( a->PosX > b->PosX ) ? 1 : 0;
    Diff[1] = ( a->PosY > b->PosY ) ? 2 : 0;                
    return Diff[0] + Diff[1];
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

    int idx;
    int i;
    TouchPointInfo TPTmp[4];
    TouchPointInfo CentroidTmp;

    CalculateCentroid( TPnum, TP, &CentroidTmp );

    for ( i = 0 ; i < *TPnum ; i++ )
    {
        TPTmp[i].PosX = TP[i].PosX;
        TPTmp[i].PosY = TP[i].PosY;
    }

    for ( i = 0 ; i < *TPnum ; i++ )
    {
        idx = DiffIndex( &TPTmp[i], &CentroidTmp );

        TP[idx].PosX = TPTmp[i].PosX;
        TP[idx].PosY = TPTmp[i].PosY;
    }

}

void CopyPoints( TouchPointInfo *TPDst, TouchPointInfo *TPSrc, int TPnum )
{
    int i;

    for( i = 0 ; i < TPnum ; i++ )
    {
        TPDst[i].ID		= TPSrc[i].ID;
        TPDst[i].PosX		= TPSrc[i].PosX;
        TPDst[i].PosY		= TPSrc[i].PosY;
        TPDst[i].State		= TPSrc[i].State;	
        TPDst[i].Confidence	= TPSrc[i].Confidence;	
    }
}

void EstimatePoints( int *TPnum, TouchPointInfo *TP )
{
    int right, down;

    switch( *TPnum )
    {
    case 1: 
        if ( DIRECTION_HORIZONTAL == CTP.DirectionPrev )
        {           
            if ( TP[0].PosX > CTP.CentroidD2.PosX )
            {
                TP[1].PosX = TP[0].PosX;
                TP[0].PosX -= CTP.WidthPrev;                  
            }
            else
            {
                TP[1].PosX = TP[0].PosX + CTP.WidthPrev;                 
            }

            TP[1].PosY = TP[0].PosY;
        }
        else
        {
            TP[1].PosX = TP[0].PosX;

            if ( TP[0].PosY > CTP.CentroidD2.PosY )
            {
                TP[1].PosY = TP[0].PosY;
                TP[0].PosY -= CTP.HeightPrev;         
            }
            else
            {
                TP[1].PosY = TP[0].PosY + CTP.HeightPrev;                
            }
        }

        *TPnum = 2;
        break;

    case 2:
        if ( DIRECTION_HORIZONTAL == CTP.Direction )
        {
            down = CTP.Centroid.PosY > CTP.CentroidD4.PosY;

            TP[2].PosX = TP[0].PosX;
            TP[2].PosY = down ? TP[0].PosY - CTP.HeightPrev : TP[0].PosY + CTP.HeightPrev ;

            TP[3].PosX = TP[1].PosX;
            TP[3].PosY = down ? TP[1].PosY - CTP.HeightPrev : TP[1].PosY + CTP.HeightPrev ;
        }
        else
        {            
            right = CTP.Centroid.PosX > CTP.CentroidD4.PosX;

            TP[2].PosX = right ? TP[0].PosX - CTP.WidthPrev : TP[0].PosX + CTP.WidthPrev ;
            TP[2].PosY = TP[0].PosY;

            TP[3].PosX = right ? TP[1].PosX - CTP.WidthPrev : TP[1].PosX + CTP.WidthPrev ;
            TP[3].PosY = TP[1].PosY;    
        }

        *TPnum = 4;
        SortPoints( TPnum, TP );
        break;
    }

    CTP.Width = CTP.WidthPrev;
    CTP.Height = CTP.HeightPrev;
}


int CalculateDistance( TouchPointInfo *TP0, TouchPointInfo *TP1 )
{
    int dX = TP1->PosX - TP0->PosX;
    int dY = TP1->PosY - TP0->PosY;

    return ( dX * dX + dY * dY ) / 128 ;
}


void CalculateRectangle( int *TPnum, TouchPointInfo* TP )
{
    switch( *TPnum )
    {
    case 1:
        CTP.Width   = 0;
        CTP.Height  = 0;
        break;

    case 2:
        CTP.Width   = TP[1].PosX - TP[0].PosX;
        CTP.Height  = TP[1].PosY - TP[0].PosY;
        break;

    case 4:
        CTP.Width   = TP[3].PosX - TP[0].PosX;
        CTP.Height  = TP[3].PosY - TP[0].PosY;
        break;
    }
}


void CalculateCentroid( int *TPnum, TouchPointInfo* TP, TouchPointInfo *centroid )
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
        centroid->PosX = ( TP[0].PosX + TP[3].PosX ) / 2;
        centroid->PosY = ( TP[0].PosY + TP[3].PosY ) / 2;
        break;
    }
}


int CheckLinkDistance( int i, int j )
{
    return ( GlobalT.MaximumLinkDistance > 0 &&  
        CalculateDistance( &CTP.TPSort[i], &CTP.TPTrackPrev[j] ) > GlobalT.MaximumLinkDistance );
}

int CheckDualLinkDistance( const short* map )
{
    int i;
    for( i = 0 ; i < 2 ; i++ )
    {
        if( CheckLinkDistance( map[i], i ) )
        {
            CTP.Death += i + 1;
            CTP.StateNext = STATE_P1;

            CTP.TrackIdx = 1 - i;
            CTP.PivotIdx = map[CTP.TrackIdx];
        }
    }

    if ( 3 == CTP.Death )
    {
        CTP.StateNext = STATE_P0;
    }

    return CTP.Death;
}

int FindPivot( int *TPnum, TouchPointInfo *TP )
{
    int i;
    int mini = 0x7fffffff, dist;
    int idx = -1;

    for( i = 0 ; i < *TPnum ; ++i )
    {	
        dist = CalculateDistance( &TP[i], &CTP.Pivot );   
        if ( dist < mini )
        {
            mini = dist;
            idx = i;
        }
    }

    return idx;
}


void TrackMapping( int* TPnum )
{
    int i;
    TouchPointInfo* TPDst;
    TouchPointInfo* TPSrc;
    const short *map = NULL;

    // TID mapping
    for( i = 0 ; i < 2 ; i++ )
    {
        if ( CTP.Birth & ( i + 1 ) )
        {
            CTP.TID[i]	= CTP.TIDNext;
            CTP.TIDNext = 3 - CTP.TIDNext;
        }
    }

    // Determine touch points state 
    switch ( CTP.StateNext)
    {
    case STATE_P0:
        *TPnum = 0;
        break;

    case STATE_P1:
        *TPnum = 1;
        break;

    case STATE_D2:        
        map = D2_ID[CTP.Subtype];
        *TPnum = 2;
        break;

    case STATE_D4:
        map = D4_ID[CTP.Subtype-4];
        *TPnum = 2;
        break;
    } // end of IDs

    for ( i = 0 ; i < *TPnum ; i++ )
    {
        TPDst = &CTP.TPTrack[i];
        TPSrc = ( STATE_P1 == CTP.StateNext ) ? &CTP.TPSort[CTP.PivotIdx] : &CTP.TPSort[map[i]];

        TPDst->PosX = TPSrc->PosX;
        TPDst->PosY = TPSrc->PosY;
        TPDst->ID   = ( STATE_P1 == CTP.StateNext ) ? CTP.TID[CTP.TrackIdx] : CTP.TID[i];
        TPDst->State = POINT_MOVE;
    }

    // Birth
    for( i = 0 ; i < 2 ; i++ )
    {
        if ( CTP.Birth & ( i + 1 ) )
        {
            CTP.TPTrack[i].State = POINT_BIRTH;	
        }
    }

    // Death
    CTP.TPnumDead   = 0;
    for( i = 0 ; i < 2 ; i++ )
    {
        if ( CTP.Death & ( i + 1 ) )
        {
            TPDst	= &CTP.TPDead[CTP.TPnumDead];
            TPSrc	= &CTP.TPTrackPrev[i];

            TPDst->PosX  = TPSrc->PosX;
            TPDst->PosY  = TPSrc->PosY;
            TPDst->ID    = TPSrc->ID;
            TPDst->State = POINT_DEATH;

            CTP.TIDNext = TPSrc->ID;
            CTP.TPnumDead ++;
        }
    }

    if ( 1 == CTP.Death && 1 != CTP.Birth ) 
    {
        CTP.TID[0]  = CTP.TID[1];
    }

}

void TrackerUpdate(void)
{
    ////////////////////////////////////////////////////////////////////////////
    // Save current frame 
    ////////////////////////////////////////////////////////////////////////////
    CTP.State           = CTP.StateNext;
    CTP.DirectionPrev   = CTP.Direction;
    CTP.WidthPrev       = CTP.Width;
    CTP.HeightPrev      = CTP.Height;

    CTP.CentroidPrev	= CTP.Centroid;
    CTP.CentroidKalmanPrev	= CTP.CentroidKalman;
}

int Tracker( int *TPnum, TouchPointInfo *TP )
{         
    int i;
    int idx;

    if( *TPnum > 0 )
    {
        // Sort points       
        CopyPoints( CTP.TPSort, TP, *TPnum ); 
        CalculateCentroid( TPnum, CTP.TPSort, &CTP.Centroid );
        ClonePoint( CTP.CentroidKalman, CTP.Centroid );

        SortPoints( TPnum, CTP.TPSort );     
        CalculateRectangle( TPnum, CTP.TPSort );

        if ( 4 == *TPnum )
        {
            ClonePoint( CTP.CentroidD4, CTP.Centroid )
        }

        if ( 2 == *TPnum )
        {
            ClonePoint( CTP.CentroidD2, CTP.Centroid )

                CTP.Direction = ( CTP.TPSort[0].PosX == CTP.TPSort[1].PosX ) ? DIRECTION_VERTICAL : DIRECTION_HORIZONTAL;
        }

    }
    else if ( STATE_P0 == CTP.State )
    {             
        return;
    }

    CTP.PivotIdx = 0;
    CTP.TrackIdx = 0;
    CTP.Birth = 0;
    CTP.Death = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////            
    // Track FSM
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    switch( CTP.State )
    {
    case STATE_P0:
        switch( *TPnum )
        {
        case 0:
            CTP.Age = 0;
            break;
        case 1:
            if ( CTP.Age < GlobalT.Threshold_Point0_Point1 )
            {
                CTP.Age ++;
            }
            else
            {
                ClonePoint( CTP.TPFirst, CTP.TPSort[0] );

                CTP.Age = 0;
                CTP.Birth = 1;
                CTP.StateNext = STATE_P1;
            }
            break;
        case 2:        
            if ( CTP.Age < GlobalT.Threshold_Point0_Dual2 )
            {
                CTP.Age ++;
            }
            else
            {
                if ( CTP.TPSort[0].Confidence < CTP.TPSort[1].Confidence )
                {
                    ClonePoint( CTP.TPSort[0], CTP.TPSort[1] );
                }

                ClonePoint( CTP.TPFirst, CTP.TPSort[0] );

                CTP.Age = 0;
                CTP.Birth = 1;
                CTP.StateNext = STATE_P1;                    
            }
            break;
        case 4:
            // For DTM test, map to NE4
            CTP.Subtype = SUBTYPE_NE4;

            if ( GlobalT.Confidence_Point0_Dual4 == 1 )
            {
                for( i = 0 ; i < 4 ; i++ )
                {
                    if ( CTP.TPSort[i].Confidence == 2 )
                    {
                        CTP.Subtype = i + 4;

                        ClonePoint( CTP.TPFirst, CTP.TPSort[i] );                        
                    }
                }                
            }

            CTP.Age = 0;
            CTP.Birth = 3;
            CTP.StateNext = STATE_D4;
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
            CTP.Age = 0;
            CTP.Death = 1;
            CTP.StateNext = STATE_P0;
            break;

        case 1:
            if ( CheckLinkDistance( 0, 0 ) )
            {
                CTP.Birth = 1;
                CTP.Death = 1;
            }
            CTP.Age = 0;
            break;

        case 2:
            if ( CTP.Age < GlobalT.Threshold_Point1_Dual2 )
            {
                CTP.PivotIdx = FindPivot( TPnum, CTP.TPSort );  

                if ( CheckLinkDistance( CTP.PivotIdx, 0 ) )
                {
                    CTP.PivotIdx = 1 - CTP.PivotIdx;

                    CTP.Birth = 1;
                    CTP.Death = 1;   
                }
                
                CTP.Age ++;
            }
            else                     
            {  
                CTP.PivotIdx = FindPivot( TPnum, CTP.TPSort );

                if ( CheckLinkDistance( CTP.PivotIdx, 0 ) )
                {
                    CTP.PivotIdx = 1 - CTP.PivotIdx;

                    CTP.Birth = 1;
                    CTP.Death = 1;   
                }
                else
                {
                    CTP.Subtype = CTP.Direction * 2 + CTP.PivotIdx;

                    CTP.Birth = 2;
                    CTP.StateNext = STATE_D2;                    
                }

                CTP.Age = 0;
            }
            break;

        case 4:
            CTP.PivotIdx = FindPivot( TPnum, CTP.TPSort );
            CTP.Subtype = CTP.PivotIdx + 4;

            CTP.Age = 0;
            CTP.Birth = 2;
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
            if ( CTP.Age <  GlobalT.Threshold_Dual2_Point0 )
            {
                CTP.Age ++;                               
            }
            else
            {
                CTP.Age = 0;
                CTP.Death = 3;                            
                CTP.StateNext = STATE_P0;
            }
            break;

        case 1:
            if ( CTP.Age < GlobalT.Threshold_Dual2_Point1 )
            {
                EstimatePoints( TPnum, CTP.TPSort );   
                CTP.Age ++;                               
            }
            else
            {
                ClonePoint( CTP.Pivot, CTP.TPSort[0] );                    
                CTP.TrackIdx = FindPivot( &CTP.TPnumTrackPrev, CTP.TPTrackPrev );  

                CTP.Age = 0;
                CTP.Death = 2 - CTP.TrackIdx;                            
                CTP.StateNext = STATE_P1;
            }
            break;

        case 2:
            // from vertical to horizontal
            if ( CTP.Direction != CTP.DirectionPrev )
            {
                CTP.PivotIdx = FindPivot( TPnum, CTP.TPSort );  
                CTP.Subtype = CTP.Direction * 2 + CTP.PivotIdx;
            }

            CheckDualLinkDistance( D2_ID[CTP.Subtype] );

            CTP.Age = 0;

            /*
            for ( i = 0 ; i < 2 ; i++ )
            {
                if ( CheckLinkDistance( D2_ID[CTP.Subtype][i], i ) )
                {
                    CTP.Death += i + 1;
                    CTP.StateNext = STATE_P1;

                    CTP.TrackIdx = 1 - i;
                    CTP.PivotIdx = D2_ID[CTP.Subtype][CTP.TrackIdx];
                }
            }

            if ( 3 == CTP.Death )
            {
                CTP.StateNext = STATE_P0;
            }
            */

            break;

        case 4:
            idx = DiffIndex ( &CTP.CentroidKalman, &CTP.CentroidKalmanPrev );   

            CTP.Age = 0;
            CTP.Subtype = D2_D4_TABLE[CTP.Subtype][idx];          
            CTP.StateNext = STATE_D4;  
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
            CTP.Age = 0;
            CTP.Death = 3;                        
            CTP.StateNext = STATE_P0;
            break;

        case 1:
            ClonePoint( CTP.Pivot, CTP.TPSort[0] );                    
            CTP.TrackIdx = FindPivot( &CTP.TPnumTrackPrev, CTP.TPTrackPrev );  

            if ( CheckLinkDistance( 0, CTP.TrackIdx ) )
            {
                CTP.Death = 3;   
                CTP.StateNext = STATE_P0;
            }
            else
            {
                CTP.Death = 2 - CTP.TrackIdx;   
                CTP.StateNext = STATE_P1;
            }

            CTP.Age = 0;
            break;

        case 2:
            if ( CTP.Age < GlobalT.Threshold_Dual4_Dual2 )
            {
                EstimatePoints( TPnum, CTP.TPSort );

                if( !CheckDualLinkDistance( D4_ID[CTP.Subtype-4] ) )
                    CTP.Age ++;                                                
            }
            else                                                                      
            {
                CTP.Age = 0;
                CTP.Subtype = D4_D2_TABLE[CTP.Subtype - 4][CTP.Direction];
                CTP.StateNext = STATE_D2;
            }
            break;

        case 4:  
            CheckDualLinkDistance( D4_ID[CTP.Subtype-4] );
            /*
            for( i = 0 ; i < 2 ; i++ )
            {
                if( CheckLinkDistance( D4_ID[CTP.Subtype-4][i], i ) )
                {
                    CTP.Death += i + 1;
                    CTP.StateNext = STATE_P1;

                    CTP.TrackIdx = 1 - i;
                    CTP.PivotIdx = D4_ID[CTP.Subtype-4][CTP.TrackIdx];
                }
            }

            if ( 3 == CTP.Death )
            {
                CTP.StateNext = STATE_P0;
            }
            */
            CTP.Age = 0;
            break;
        }
        break;

    } // end of State           

    // 
    TrackMapping( TPnum );

    if ( *TPnum > 0 )
    {
        ClonePoint( CTP.Pivot, CTP.TPTrack[0] );

        CTP.TPnumTrackPrev = *TPnum;
        CopyPoints( CTP.TPTrackPrev, CTP.TPTrack, *TPnum );
    }

    if ( GlobalT.PivotStationaryDistance > 0 )
    {
        if ( CalculateDistance( &CTP.TPFirst, &CTP.TPTrack[0] ) < GlobalT.PivotStationaryDistance )
        {
            ClonePoint( CTP.TPTrack[0], CTP.TPFirst );
        }
    }

    CopyPoints( &CTP.TPTrack[(*TPnum)], CTP.TPDead, CTP.TPnumDead );
    *TPnum += CTP.TPnumDead;

    CopyPoints( TP, CTP.TPTrack, *TPnum );
    /*
    ////////////////////////////////////////////////////////////////////////////
    // Save current frame 
    ////////////////////////////////////////////////////////////////////////////
    CTP.State           = CTP.StateNext;
    CTP.DirectionPrev   = CTP.Direction;
    CTP.WidthPrev       = CTP.Width;
    CTP.HeightPrev      = CTP.Height;

    ClonePoint( CTP.CentroidPrev, CTP.Centroid );
    ClonePoint( CTP.CentroidKalmanPrev, CTP.CentroidKalman );
    */
#ifdef __DRIVER 
    for( i = 0 ; i < *TPnum ; i++ )
    {
        if ( POINT_BIRTH == TP[i].State )
        {
            CTP.DTID[TP[i].ID-1] = CTP.DTIDNext;
            CTP.DTIDNext = ( CTP.DTIDNext == 255 ) ? 1 : CTP.DTIDNext + 1;
        }

        TP[i].ID = CTP.DTID[TP[i].ID-1];
    }
#endif
    return TOUCH_DETECTED;
}



