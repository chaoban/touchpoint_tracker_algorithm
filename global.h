//global.h
//
//Control globals
//

typedef struct
{	
    int     TPANEL_WIDTH; // 22
    int     TPANEL_HEIGHT; // 13
    float   Touch_max; // 5.0f
    float   Touch_min; // 1.0f
    //% Finger=5  0.7-1.15 Finger=6  0.8-1.4 Finger=7 0.9-1.6 Finger=8 1.1-1.9 Finger=9  1.2-2.1  Finger=10  1.3-2.35
    float   Sigma; // 1.1f
    //0: linear 1:gaussian
    int     Modeltype; // 1 
    //0 ConstanFindecision 1 Start_End decision
    int     Find_decision;//1
    int     Finger_Size; // 5
    int	    Finger_Size_Max;//8
    int	    Finger_Size_Min;//3
    //0: NONE  1:Normalize
    int     NormalizeDecision;// 0
    int	    NormalizeMax;//10
    int     RowDataSize; // 35
    int     ChannelValidX; // 0xFFFFFFFF
    int     ChannelValidY; // 0x3FF
    int     DataCalibrateEn; // 1
    int     CalibrateLength; // 20
    int     DisableCaliLength; // 400
    //0: replace, 1: progressive
    int     UpdateInitMethod; // 0
    int     UpdateWeightingI; // 5
    int     UpdateWeightingC; // 5
    int     NormalWeighting; // 40
    float   SenseWeighting; //0.0f
    int     InitDataEnable; // 1
    int     InitMaxValueEnable; // 1
    float   NormalizeBase; // 1.0f
    int     NormalizeSizeX; // 46000
    int     NormalizeSizeY; // 7000    
    int	    InitDataSize; // 300
    //avoid power on or other noise
    int     SkipInitSize; // 100
    int     RowDeNoiseEn;// 1  
    int     MeanPeriod;// 16
    int     MeanUpdateWeightI;// 1
    //MeanUpdateWeightC = MeanPeriod - MeanUpdateWeightI
    int     MeanUpdateWeightC;// 15
    int     EnMeanDeNoise; // 1
    int     EnDiffMean; // 1
    float   InitChSenseMax; // 3.0f
    float   SenseMaxValue; // 1024.0f   
    //Data Inverse Enable
    int     DataInverseEn; // 0
    int     DataMaxValue; // 0
    //0:Gradient, 1: 2DMagnitude, 2: ML Decision
    int     TPAlgorithm; // 0
    int     tapnum; // 7
    int     tapnum_x; // 7
    int     tapnum_y; // 5
    //1:NUM_TAP_INTERPOLATE, 2:CENTER_OF_MASS;
    int     PosMethod; // 1    
    int	    InterPolate_Row;// 0
    //0: NONE, 1: Gaussian Filter, 2: SINC Filter, 3: ButterWorth Filter
    int     IntrWetMethod; // 1
    float   GaussianSD_X;// 5.0f
    float   GaussianSD_Y;// 7.0f
    float   SincDistScaler_X; // 0.8f
    float   SincDistScaler_Y; // 0.8f
    int     IntrButterOrder;//3
    float   IntrButterCutOff_X;//3.0f
    float   IntrButterCutOff_Y;//3.0f
    int     AutoAdjustEn;// 0
    int     EnIIRFilterData;// 0
    // LPF_PI_N, PI/N, N:2~24
    int     IIRCutOFF;// 0
    //0: smooth after secondIIR, 1: secondIIR after smooth
    int     IIRTEST;// 0

    //0: NONE, 1:Mean, 2:1D-DCT, 3: weighting Normalize
    int     RowProcessingMethod; // 2
    //DCT
    int     DCT_RM_NUM_X;//1
    int     DCT_RM_NUM_Y;//1    
    int     DCT_IntrScale_X;// 1
    int     DCT_IntrScale_Y;// 1
    int     DCT_ACL_NUM_X; // 10
    int     DCT_ACL_NUM_Y; // 6
    int     DCT_ACH_NUM_X; // 2
    int     DCT_ACH_NUM_Y; // 2
    int     Spatialtapnum;//3
    //0:smooth filter 1:weight filter
    int     Spatial_type;//0
    float   multiplemeanx;//1.5
    float   multiplemeany;//1.5

    //0:Ideal 1:Butterworth 2:Gaussian
    int     DCT_FILTER; // 1
    int     BUTTER_ORDER; // 3
    int     TOUCHAREA_SINGLE; // 1
    int     TOUCHAREA_MULTIPLE; // 20
    int     OneFingerSize_x; // 3	
    int     OneFingerSize_y; // 2
    float   TouchThreshold; // 5.0f    
    int     RowMeanThresholdEn; // 1
    float   TouchThreshold_1DX; // 10.0f
    float   TouchThreshold_1DY; // 2.0f
    int     DynamicThresholdEn; // 0
    //  |-             TouchCalPeriod           -|
    //  |- TouchValidLength -|- CalNewThreshold -|     
    int     TouchCalPeriod;// 30
    int     TouchValidLength;// 20
    float   LeaveThreshold_1DX; // 15.0f
    float   LeaveThreshold_1DY; // 5.0f 
    int     VarianceCheckEn;// 0
    float   VarianceThreshold_X; // 1.0f
    float   VarianceThreshold_Y; // 1.2f
    // update NewTouchThreshold = TouchThreshold * TouchModWeight
    float   TouchModWeight_X;// 1.0f
    float   TouchModWeight_Y;// 1.0f
    // update NewLeaveThreshold = LeaveThreshold * LeaveModWeight
    float   LeaveModWeight_X;// 0.8f
    float   LeaveModWeight_Y;// 0.8f
    float   GradientTouchThreshold; // 0.0f
    float   GradientPlus; // 0.0f
    float   RowDataBase; // 64.0f
    float   GhostThresholdX_t; // 0.3f
    float   GhostThresholdY_t; // 0.4f
    float   GhostThresholdX_s; // 0.3f
    float   GhostThresholdY_s; // 0.4f

    //---used to region segmentation
    int     AREA_THRESHOLD_MIN; // 1
    int     AREA_THRESHOLD_MAX; // 30
    int     AREA_1D_MIN_X; // 2
    int     AREA_1D_MIN_Y; // 1
    int     AREA_1D_MAX_X; // 20 
    int     AREA_1D_MAX_Y; // 10 
    int     MorphicEnable; // 0
    int     MorphicChoice; // 0    
    int     WeightBase; // 1
    float   BoundaryWeight;// 0.4f
    int     LearningNum;// 10
    int     LearningRangeX;// 1
    int     LearningRangeY;// 1
    int     SegRange;// 2
    int     PanelCalibrateEn;//0
    float   ChannelWeight; // 0.0f
    int     PanelCalibrateRange;// 40
    //0: Update_With_Table, 1: Update_With_Learning
    int     PanelCalibrateMethod;// 0
    int     LearningCalibrateEn;// 0
    //0: Learning from Trainer, 1: learning from input file
    int     LearningMethod;// 0
    int     PannelCheckEn;// 0
    int     CheckMessageEn;// 1
    //0: variance square sum, 1: absolute variance sum, 2: sqrt of variance square sum
    int     CheckNum;// 500	
    int     VoltageValue; // 500
    int     UsingMaxVoltageEn;// 0
    float   MaxVoltage;// 512.0f
    int     DumpLogEn;// 1
    int     TrackPeriod;// 20
    int     PatternGenEn;// 0
    //0: none, 1: single Touch, 2: dual Touch 3: Multi-Touch
    int     TouchMode;// 2
    int     MaxVTrackingEn;// 1
    int     TouchTrackingEn;// 0
    int     TouchRegionX;// 4
    int     TouchRegionY;// 2
    int     PlusCmpNum;// 1
    int     MinusCmpNum;// 3
    int     PlusTrackValue;// 1
    int     MinusTrackValue;// 1
    int     TouchRepayEn;// 1
    int     repay_y; // 0    
    float   TouchRepayThresholdX;// 0.35f 
    float   TouchRepayThresholdY;// 0.35f        
    int     RS232DataEn;// 0
    float   Kalman_ChQ;//0.1f
    float   Kalman_ChR;//4.0f
    int     Kalman_initail;//4
    float   Kalman_multiple;//10.0f
    int     dynamicREn;//0 
    int     movementframecnt;//4
    int     zero_cnt;// 2    
    int     ScatterCheckEn;// 0   
    //1: DCTData, 0: RawData
    int     WeightDataMode;// 1
    int     WeightNum;// 1
    float   ScatterThresholdX;// 0.8f 
    float   ScatterThresholdY;// 0.8f
    //0: None, 1: WeightingSmooth, 2: Kalman, 3:DynamicKalman 
    int     RawSmoothMethod;//2
    //0 1dim 1 2dim
    int     kalman_dim;//0
    // ----- TouchCalibration() ---------
    int     TouchCaliEn; // 0
    int     TouchCaliRegionX; // 10
    int     TouchCaliRegionY; // 10
    // ----- UpdateThreshold() ----------
    // 0:NONE_UPDATE 1:UPDATE_THRESHOLD_BY_FRAMEMEAN0, 2:UPDATE_THRESHOLD_BY_FRAMEMEAN1, 3:UPDATE_THRESHOLD_BY_ROWDATAMEAN, 4:By FrameMean, *ModifyUnit
    int     UpdateMethod; // 0
    float   ThresholdIntervalX; // 2.0f
    float   ThresholdIntervalY; // 2.0f
    float   ModifyUnitX; // 80.0f
    float   ModifyUnitY; // 20.0f
    //0: normal, 1: Inverse
    int     InverseXorder;// 0
    int     InverseYorder;// 0
    // Dump drawdata control
    int     DumpDrawdataEn; // 0
    // UpdateDCT_ACH
    // 0: NoneUpdate, 1: UPDATE_BY_CHANNELVARIANCE 2:UPDATE_BY_FRAMEVARIANCE 3:UPDATE_BY_FRAMEMEAN
    int     UpdateDCT_ACH_Method; // 0
    int     UpdateDCT_SamplePeriod; // 2
    float   UpdateDCT_ThresholdX; // 20.0f
    float   UpdateDCT_ThresholdY; // 20.0f
    int     UpdateDCT_ACH_NUM_X; // 2
    int     UpdateDCT_ACH_NUM_Y; // 2
    // Gamma 
    int     GammaCorrectEn;// 1
    int     GammaMethod;//0
    float   GammaCof;//0.6f
    float   GammaFactor;//1.2f
    //0: NormalizeSize, 1: MaxweightData to be the NormalizeSize
    int     GammaDataMode;//0
    // Gradient Method
    // 0: GRADIENT_BY_DCT, 1: GRADIENT_BY_ROWDIFF
    int     GradientMethod; // 0
    int     EnDCT_SHIFT;//0
    int     FrameDropEn;// 1
    //mean shift
    int     meanshiftrange;//4
    int     MeanshiftEpanechnikovEn;//1
    float   MeanshiftEpanechnikovmultiple;//2.0f
    int     EpanechnikovEn;//1
    float   Epanechnikovmultiple ;//2.0f
    int     frametype;//0  
    //0 Rawdiff 1 DCT
    int     meanshiftscatter; // 0
    int     meanshiftrepay; // 1
    float   meanshiftcoef; //99.0f
    //PreTrackerQueue
    int     PreTrackEn;// 0
    int     QueueSize;// 8
    int     PreviousFrames;// 4
    //for clustering
    int     ClusterEn; // 1
    // 0:Min Distance   1:First match
    int     ClusterMethod;// 0
    //for Min Distance
    float   MinimumDistance;// 1.0f
    int     ChangeDirectionFrmThroshold;// 2
    float   ChangeDirectionDisThroshold;// 0.8f
    //for First Match
    float   MatchDis;// 1.0f
    int     FixPointEn;// 1
    int     DeleteScatterEn;// 1
    //for scatter check
    float   ScatterStepDistance; //1.2f
    //limit
    int     LimitTPnumEn;// 1
    //Tracker
    int     TrackEn;// 1
    int     TrackKalmanEn;// 1
    int     KalmanEn;// 0
    int     EKF_EN;// 0
    float   Kalman_R;// 10.0f
    float   Kalman_Q;// 0.01f
    float   Kalman_P;// 10.0f
    float   Kalman_HV;// 3.0f
    float   Kalman_HA;// 12.0f
    float   Kalman_HW;// 0.0f
    int     smoothtapnum;// 0
    int     meansmoothEn;// 0
    //1:Queue, 2:Pivot
    int     TrackMethod;// 2
    //Queue_Tracker
    float   ScatDis;// 1.0f
    int     DropFixEn;// 0
    float   DropDis;// 1.0f
    float   CoGDis;// 2.0f
    int     DeleteGhostEn;// 1
    //set FreeSuspect = QueueSize+max_delay_frm to solve xy-delaycase
    int     FreeSuspect;// 8
    int     DeathCD;// 10
    int     Threshold_Dual2Quad;// 3
    int     Threshold_Quad2Dual;// 3
    int     DumpTrackLogEn;// 0
    //Pivot_Tracker
    int	    PivotTrackerLogEn;// 0    
    // 0:first 1:centroid_angle 2:centroid_delta 3:first_centroid_delta
    int	    PivotMethod; // 3
    float   CentroidAngle; // 30.0f
    float	CentroidDeltaRatio; // 0.2
    float   CentroidDelta; // 1
    // 0:Orig 1:Kalman 2:Integer
    int	    CentroidMethod; // 1    
    // 0:equally 1:confident
    int	    CentroidWeight; // 0
    float   CentroidKalman_R; // 256
    int     TrackAdjustMode; // 0
    float   TrackAdjustMaximumuDistance; // 2.0f
    float   TrackAdjustDither; // 0.0f
    int	    Threshold_Point0_Point1;// 3
    int	    Threshold_Point1_Dual2;// 5
    int	    Threshold_Dual2_Point4;// 0
    int	    Threshold_Dual2_Point1;// 3
    int	    Threshold_Dual2_Point0;// 3
    int	    Threshold_Dual4_Dual2;// 3
    int	    Threshold_Dual4Ghost_Dual4;// 3
    //Painter Snapshot
    int	    PainterEn; // 0
    int	    PainterMultiFrame; // 0
    int     PainterDropShortFrame; // 20
    int	    PainterPreTrack; // 0
    int	    PainterSnapshot; // 0
    int     PainterCentroid; // 0
    int	    PainterGridSize; // 32
    int	    PainterPointSize; // 8
    //for waveline
    int     waveline_en; // 1
    //for calculate position
    float   offset_max; // 0.1f
    float   offset_min; // 0.02f
    //for modify rowdiffdata
    int     modify_rowdiff_en; // 0
    float   mod_offset_x; // 0.5f
    float   mod_offset_y; // 0.5f
    int	    rowdiff_cali_en; // 0
    int	    y_diffsqrt_en; // 0
    int	    x_diffsqrt_en; // 0
    float   y_sqrt_ratio;  // 0.5f
    float   x_sqrt_ratio;  // 1.0f
	//for updata initdata 
    int     updata_init_always_en; // 0
    int     InterPrecise;// 7

   }GLOBAL;
