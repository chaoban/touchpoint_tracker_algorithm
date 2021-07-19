//#include "common.h"

//Interface

// extern  int TPDetector( char *dataLA, int *TPnum, TouchPointInfo *TP ); // jiunhau 981022
extern int Tracker( int *TPnum, TouchPointInfo *TP );

//extern 'C' int TPDetector( char *dataLA, int *TPnum, TouchPointInfo *TP );

void UpdateGlobal( GLOBAL regGlobal );
void DataCalibration( int TPnum );

void TPMethod_Gradient( int *TPnum, TouchPointInfo *TP );
int Segmentation_1D( int ch );
int Gradient1D( int ch, int segcnt );
//
int RawDataPreProcessing( char *LA );
float GetDataMean( int size, float *data );
float Get_1D_DCT_Gain( int u, int size, float *data );
float GetInv1DDCT( int u, int size, float *data );
//TT   void RowDataProcessing();
//TT   void Get2DMagnitude();
//TT   int Get2DBinaryArea();
//TT   
//TT   void RowProcMethod_None();
//TT   void RowProcMethod_DCT();
//TT   void RowProcMethod_Mean();
//TT   void RowProcMethod_Weighting();

void MorphologicalProcessing( int choice, 
				     		  int element_width, int element_height, int* element,
							  int mark_position_x, int mark_position_y, int* result_image );
void binary_dilation( int element_width, int element_height, int* element,
					  int mark_position_x, int mark_position_y, int* result_image );
void binary_erosion( int element_width, int element_height, int* element,
					 int mark_position_x, int mark_position_y, int* result_image );
//TT   int Segmentation_InnerBoundaryTracing();//return region_number

void CenterOfMass( int cnt );

void Num_Tap_Interpolate( int cnt );
//TT   void GlobalSetting();//set all global for 2D algorithm
void TPMethod_2DMagnitude( int* TPnum, TouchPointInfo* TP );
//TODO: Progressive Update RowData
//TODO: incremental algorithm for calculating position
//TODO: Morphological Processing, Noise reduction
//TODO: Search touch region
//TODO: Multi-touch distinguish

void Get2DGradient( int w );
//TT   void DoCalibration();
int TouchCalibration(int TPnum, TouchPointInfo *TP, int sizex, int sizey);
float PSNRCalculation( float maxValue, float variance );
int LearningProgram( float fx, float fy, char* dataLA );
int PosComparsion( float* refTP );
float MSECalculation( int method, int size, float* data );
float VarianceCalculation( int method, int size, float* data );

void RecordRowdataVariation(char* dataLA,int TPnum);
//TT   int* OpenGlobalFile();
//TT   void ReadLearningTable();
//TT   void DumpLearningTable();
//TT   void DumpWetTable();
void INTRAlg_SINC( int ch, int w, float ds );
void INTRAlg_Gaussian( int ch, int w, float sd );
void INTRAlg_ButterWorth( int ch, int order, int length, float Cutoff );
float GenRawData( int ch, char* LA );
void GenRawDiffData( int ch, float rawdata );
float GetDataStandardDeviation( int size, float mean, float* data );
//TT   int TouchCheck();
float PointConfidenceCheck( int ch, int TPcnt );
short GetRawData( char* LA );
float Num_Tap_Interpolate_1D( int ch, int mark, int* step, float* variance, int i, int tap_num, int chSize, int scale , int *dct_shift);
float CenterOfMass_1D( SegmentInfo Seg, int ch, int* step, float* variance, int scale );
void DCT_BUTTERWORTH( int ch, float* gain, int order );
void DCT_GAUSSIAN( int ch, float* gain );
void DCT_CUTOFF( int ch, float* gain );
int SegmentRepayCheck( int ch, int num );
float GetWeighting( int frame, int ch, int mark );
void ScatterCheck( int ch, int num );
void tracking_kalman( int XY_decision, int cont, int ID, float Kalman_R );
void Adaptive_tracking( float *TP, int ID, int Type, float Kalman_R );
void Kalman_Channel_filter( float *St, int Channel_ID, float Kalman_ChR, float Kalman_ChQ );
void ChVarianceCalculate( int bClear, int ch, float *St, float *VarR );
void Meansmooth( float *TP, int ID, int Clear );
void Movement_Route_variance( float *TP, int ID ,float *var, float clear );
void TouchValidRegion( int ch, int num );
void ThresholdAdjust( int iCnt, int cntSize, int CaliLength, int state );
//TT   void Open_Overlap_table();
//TT   int UpdateThreshold();
void panel_check(float *RowData, 
				 float *SDIFF_X_MAX, float *SDIFF_Y_MAX, float *SMEAN_X_MAX, float *SMEAN_Y_MAX,
				 int   *CNT,		 int   *TMINCNT,     float *TMAX,		 float *TMIN,    
				 float *TMEAN,		 int   *TMINDATACNT, float *TMINDATA);

int DumpInfo(int TPnum);
int GenDrawdata( int TPnum, TouchPointInfo *TPInfo);
int GenDrawdataPreTrack( int TPnum, TouchPointInfo *TPInfo);
int UpdateDCT_ACH(int TPnum, int TchCntX, int TchCntY, int Overlap, TouchPointInfo *TPInfo);
void LocalHistogramEqualization( float *inData, float *outData, int blockSize, float factor );

//bool Tracker( int *TPnum, TouchPointInfo *TP );
//bool TrackerQ( int *TPnum, TouchPointInfo *TP );
//bool TrackerPivot( int *TPnum, TouchPointInfo *TP );
int PanelCheck( char* LA );

