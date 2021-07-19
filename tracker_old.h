#ifndef _DEFINE_TRACKER
#define _DEFINE_TRACKER
#ifndef __COMMON_H__
#include "common.h"
#endif

float absf(float a);

void Meansmooth( float *TP, int ID, int Clear );

void Adaptive_tracking( float *TP, int ID, int Type, float Kalman_R );


int TPDetector( char *dataLA, int *TPnum, TouchPointInfo *TP );
// Pivot
//bool TrackerPivot( int *TPnum, TouchPointInfo *TP );
#endif
