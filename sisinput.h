/*++
    Copyright (c) Silicon Integrated Systems Corporation

    Module Name:
        sisinput.h

    Abstract:
        Contains SiS specific definitions.

    Environment:
        Kernel mode

--*/

#ifndef _SISINPUT_H
#define _SISINPUT_H

// OS Define
#define __linux__	1
#define LINUX   1


//#include "pch.h"
#ifndef INTERNAL
#define INTERNAL
#endif
#ifndef EXTERNAL
#define EXTERNAL
#endif

#ifdef DBG
#ifndef DEBUG
  #define DEBUG
#endif
#endif


//#include <wdm.h>  //LINUX
#include "wtrace.h"
#include "memmgr.h"
//#include "common.h"
#define __UpdateToTouchMode 1


//
// Key definitions
//


//
// Constants
//
#define HPEN_POOL_TAG                   'nepH'

// OEM input raw data
#ifdef __UpdateToTouchMode
#define OEM_MT_MAX_COUNT        2
#endif // __UpdateToTouchMode

#define NUM_INPUT_SWAP          1
#define SIZE_RAW_INPUT_BUFF     128
#define SIZE_INPUT_SYNC_BUFF    ((SIZE_RAW_INPUT_BUFF) * 3)

#define VAL_CMDTYPE2                0xA002
#define VAL_CMDTYPE7                0xA007
#define VAL_CMDTYPE9                0xA009

// bStatus bit values
#define INSTATUS_PEN_TIP_DOWN           0x01
#define INSTATUS_SIDESW_DOWN            0x02
#define INSTATUS_INVERTED               0x04
#define INSTATUS_ERASER_DOWN            0x08
#define INSTATUS_RESERVED               0x18
#define INSTATUS_IN_RANGE               0x20
#define INSTATUS_CONTROL_DATA           0x40
#define INSTATUS_SYNC                   0x80
#define INWDATA_SYNC                    0x8080
#define INBDATA_SYNC                    0x80
#ifdef __UpdateToTouchMode
#define FINGER_STATUS                   0x01 // finger down (either touch or mouse)
#define RANGE_STATUS                    0x02 // inrange
#define RANGE_FINGER_STATUS             0x03 // finger down (range + finge)
#endif // __UpdateToTouchMode


//
// Macros
//


//
// Type Definitions
//

#ifndef LINUX
#pragma warning(disable:4201) // nameless struct/union
#endif


#ifdef LINUX

//common type
#ifdef FALSE
#undef FALSE
#endif
#define FALSE                           (1 == 0)

#ifdef TRUE
#undef TRUE
#endif
#define TRUE                            (1 == 1)

#ifndef NULL
#define NULL                            (void *) 0
#endif

typedef unsigned char		UCHAR;
typedef unsigned char*		PUCHAR;
typedef unsigned short		USHORT;
typedef unsigned short*		PUSHORT;
typedef unsigned int		ULONG;
typedef unsigned int*		PULONG;
typedef unsigned int	    UINT32;
typedef _Bool               BOOLEAN;
typedef PUCHAR              PIRP;

typedef enum _NTSTATUS {
    STATUS_SUCCESS      = 0x00000000L,
    STATUS_TIMEOUT      = 0x00000102L,
    STATUS_DATA_ERROR   = 0xC000003EL
} NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//#define MEMSET(addr, size, value)           
//#define MEMMOVE(dst, src, size)             
//#define MEMCPY(dst, src, size)              
//#define MEMMALLOC(size)     1   
//#define MEMFREE(addr)

#define __inout
#define __in
#define __out
#define VOID void

//typedef struct
//{
//    int     ID;
//    float   PosX;
//    float   PosY;
//    int     State;
//    float   DelX;
//    float   DelY;
//	float   Confidence;
//}TouchPointInfo;
//
//enum TouchPointState
//{
//    POINT_BIRTH,    // 0
//    POINT_DEATH,    // 1
//    POINT_MOVE,     // 2
//    POINT_TAP,      // 3
//    NOTOUCH         // 4
//};
#endif //#ifdef LINUX


// Command info
typedef struct _CMD2_INFO
{
	ULONG                 Length;
} CMD2_INFO, *PCMD2_INFO;

typedef struct _CMD7_INFO
{
    ULONG                 Precise;
    ULONG                 PointCount;
} CMD7_INFO, *PCMD7_INFO;

typedef struct _CMD9_INFO
{
    ULONG                 FrameId;
    ULONG                 XCount;
    ULONG                 YCount;
    BOOLEAN               UseCrc;
    BOOLEAN               UseConfidence;

    ULONG                 Precise;
    ULONG                 PointCount;
} CMD9_INFO, *PCMD9_INFO;

typedef struct _COMMAND_INFO
{
	ULONG                CmdType;
    union 
    {
        CMD2_INFO         Cmd2Info;
        CMD7_INFO         Cmd7Info;
        CMD9_INFO         Cmd9Info;
    };

    PUCHAR                FrameDataBuff;
    ULONG                 FrameDataLen;
} COMMAND_INFO, *PCOMMAND_INFO;

// Parser info
typedef struct _RAW_PARSER
{
    BOOLEAN               ReadMoreBytes;

    PUCHAR                CurrentHead;
    PUCHAR                NextHead;

    UCHAR                 FrameId;
} RAW_PARSER, *PRAW_PARSER;

// raw data descript
typedef struct _CMD2_PARAM
{
	USHORT                Length;
} CMD2_PARAM, *PCMD2_PARAM;

typedef struct _CMD7_PARAM
{
    UCHAR                 Precise;
    UCHAR                 PointCount;
} CMD7_PARAM, *PCMD7_PARAM;

typedef struct _CMD9_PARAM
{
    UCHAR                 FrameId;
    UCHAR                 DataType;
} CMD9_PARAM, *PCMD9_PARAM;

typedef struct _RAW_COMMAND
{
	USHORT                CmdType;
    union 
    {
        CMD2_PARAM         Cmd2Param;
        CMD7_PARAM         Cmd7Param;
        CMD9_PARAM         Cmd9Param;
    };
} RAW_COMMAND, *PRAW_COMMAND;

typedef struct _CMD2_DATA
{
    USHORT                SenseData;
} CMD2_DATA, *PCMD2_DATA;

typedef struct _CMD7_DATA
{
	USHORT                PosX;
	USHORT                PosY;
	ULONG                 Confidence;
} CMD7_DATA, *PCMD7_DATA;

typedef struct _CMD9_DATA
{
	USHORT                Pos;
} CMD9_DATA, *PCMD9_DATA;

typedef struct _CMD9_DATA_EX
{
	USHORT                Pos;
	USHORT                Confidence;
} CMD9_DATA_EX, *PCMD9_DATA_EX;

typedef struct _RAW_DATA
{
    UCHAR            Head[4];
    RAW_COMMAND      RawCmd;
	union 
    {
        CMD2_DATA    Cmd2Data;
        CMD7_DATA    Cmd7Data;
        CMD9_DATA    Cmd9Data;
        CMD9_DATA_EX Cmd9DataEx;
    };
} RAW_DATA, *PRAW_DATA;

// input buffer
typedef struct _INPUT_BUFFER
{
    ULONG                 Stamp;
    PIRP                  Irp;
    UCHAR                 RawInput[SIZE_RAW_INPUT_BUFF];    // input raw data buffer
    ULONG                 BytesToRead;
} INPUT_BUFFER, *PINPUT_BUFFER;

typedef struct _INPUT_BUFFERS
{
    ULONG                 Stamp;
    INPUT_BUFFER          InputBuffer[NUM_INPUT_SWAP];
} INPUT_BUFFERS, *PINPUT_BUFFERS;

typedef struct _RESYNC_BUFFER
{
    ULONG                 Stamp;
    UCHAR                 ResyncData[SIZE_INPUT_SYNC_BUFF]; // resync data buffer
    ULONG                 BytesInBuff;                      // number of bytes in the resync buffer
} RESYNC_BUFFER, *PRESYNC_BUFFER;

#ifndef __UpdateToTouchMode
typedef struct _OEM_INPUT_REPORT
{
    union
    {
        struct
        {
            UCHAR  bStatus;
            USHORT wXData;
            USHORT wYData;
            USHORT wPressureData;
            UCHAR  bXTiltData;
            UCHAR  bYTiltData;
        } InputReport;
        UCHAR RawInput[9];
    };
} OEM_INPUT_REPORT, *POEM_INPUT_REPORT;
#else // __UpdateToTouchMode
typedef struct _OEM_REPORT_TOUCH
{
    UCHAR  bStatus;
    UCHAR  ContactId;
    USHORT wXData;
    USHORT wYData;            
} OEM_REPORT_TOUCH, *POEM_REPORT_TOUCH;

typedef struct _OEM_INPUT_REPORT
{
    union
    {
        struct
        {
            OEM_REPORT_TOUCH TouchData[OEM_MT_MAX_COUNT];
            UCHAR  ActualCount;
        } InputReport;
        UCHAR RawInput[6 * OEM_MT_MAX_COUNT + 1];
    };
} OEM_INPUT_REPORT, *POEM_INPUT_REPORT;

typedef struct _HID_FEATURE_REPORT
{
    UCHAR       ReportID;
    UCHAR       InputMode;
    UCHAR       DeviceIndex;
} HID_FEATURE_REPORT, *PHID_FEATURE_REPORT;

typedef struct _HID_MAX_COUNT_REPORT
{
    UCHAR      ReportID;
    UCHAR      MaxCount;
}HID_MAX_COUNT_REPORT, *PHID_MAX_COUNT_REPORT;
#endif // __UpdateToTouchMode

typedef struct _HID_INPUT_REPORT
{
    UCHAR            ReportID;
    OEM_INPUT_REPORT Report;
#ifdef _TIMESTAMP_
    LARGE_INTEGER TimeStamp;
#endif
} HID_INPUT_REPORT, *PHID_INPUT_REPORT;


//
// Function prototypes
//

//sisinput.c

NTSTATUS INTERNAL
ProcessResyncBuffer(
    __inout PRESYNC_BUFFER      ResyncBuffer,
    __inout PRAW_PARSER         RawParser,
    __out   PHID_INPUT_REPORT   HidReport,
    __in    BOOLEAN             OnlyValid
    );

NTSTATUS INTERNAL
ProcessInputData(
    __in    PINPUT_BUFFERS      InputBuffers,
    __out   PRESYNC_BUFFER      ResyncBuffer,
    __out   PRAW_PARSER         RawParser,
    __in    PIRP                Irp,
    __out   PHID_INPUT_REPORT   HidReport,
    __in    BOOLEAN             OnlyValid
    );

// sisinbuf.c

ULONG INTERNAL
OemGetResyncRest(
    __in     PRESYNC_BUFFER ResyncBuffer,
    __in     PINPUT_BUFFERS InputBuffers
    );

PUCHAR INTERNAL
OemSetNextInputBuff(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp,
    __in     ULONG          ReadLength
    );

NTSTATUS INTERNAL
OemUpdateInputReadLen(
    __in     PINPUT_BUFFERS InputBuffers,
    __in     PIRP           Irp,
    __in     ULONG          BytesToRead
    );


//
// Global Data Declarations
//


#endif  //ifndef _SISINPUT_H
