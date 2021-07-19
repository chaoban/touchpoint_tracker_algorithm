/*++
    Copyright (c) Silicon Integrated Systems Corporation

    Module Name:
        sistrans.c


    Abstract:
        Contains specific functions that translate SiS touch raw data to report.

    Environment:
        Kernel mode

--*/
//#include <unistd.h>
#include "sisinput.h"
//#include <math-emu/soft-fp.h>
//#include "tp_system.h"
//#include "common.h"  //LINUX
#include "tracker.h"
#define MODULE_ID                       MODULE_SISTRANS
#define MSGMDL_ID                       MSGMDL_SISTRANS


//
// Key definitions
//
//#define __OutputSimTest 1


//
// Constants
//
#define NUM_TPINFO_DEFAULT          20

//
// Macro
//
#define GET_ALG_PRECISE(precise) (0x1 << precise)
#define DRIVERINFO printk(KERN_INFO "[sistrans] %s : %d\n", __func__, __LINE__)

//
// Type definitions
//


//
// Extern function prototype.
//
int TPDetector( char *dataLA, int *TPnum, TouchPointInfo *TP );  //LINUX

//
// Local function prototype.
//


//
// Global variable definitions
//


//
// Translate(Normalize) functions
//

/*++
    @doc    INTERNAL

    @func   NTSTATUS | OemPreprocessFrameData | Process big and little endian in frame data.

    @parm   IN ULONG | TpNum | Command type.
    @parm   IN PUCHAR | FrameData | Points to the frame data in raw data.
    @parm   IN ULONG | FrameLength | The frame data length.
    @parm   OUT PUCHAR | SenseData | Points to the processed output data. 

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_DATA_ERROR.
--*/
NTSTATUS INTERNAL
OemPreprocessFrameData(
    __in        ULONG CmdType,
    __in        PUCHAR FrameData,
    __in        ULONG FrameLength,
    __out       PUCHAR SenseData
    )
{
    NTSTATUS status = STATUS_SUCCESS;
#define SIZE_MAX_CHANG   4
    UCHAR TempBuff[SIZE_MAX_CHANG] = {0};
    ULONG i;

    TEnter(Func, ("(CmdType=%d, FrameData=%p, FrameLength=%p, SenseData=%p)", 
        CmdType, FrameData, FrameLength, SenseData));

    switch (CmdType)
    {
        case VAL_CMDTYPE2:
            MTInfo(MSGFLTR_INPUTDATA, ("CMD2 DATA:"));
            PrintMatrix(MSGFLTR_INPUTDATA, FrameData, FrameLength, 16);
            for (i = 0; i < FrameLength; i += SIZE_MAX_CHANG)
            {
                TempBuff[0] = FrameData[i + 3];
                TempBuff[1] = FrameData[i + 2];
                TempBuff[2] = FrameData[i + 1];
                TempBuff[3] = FrameData[i];
                MEMCOPY(&SenseData[i], TempBuff, SIZE_MAX_CHANG);
            }
            break;
        case VAL_CMDTYPE7:
            if (FrameData != SenseData)
            {
                MEMCOPY(SenseData, FrameData, FrameLength);
            }
            break;
        case VAL_CMDTYPE9:
            for (i = 0; i < FrameLength; i += 2)
            {
                TempBuff[0]= FrameData[i];
                SenseData[i] = FrameData[i + 1];
                SenseData[i + 1] = TempBuff[0];
            }
            break;
        default:
            status = STATUS_DATA_ERROR;
            TErr(("No command type %4x.", CmdType));

            TExit(Func, ("=%x", status));
            return status;
    }

    TExit(Func, ("=%x", status));
    return status;
}       //OemPreprocessFrameData

/*++
    @doc    INTERNAL

    @func   NTSTATUS | OemProcessTpConfidence | Process touch point confidence.

    @parm   IN PCOMMAND_INFO | CommandInfo | Points to the command information through parser.
    @parm   IN ULONG | TpNum | Algorithm output touch point number.
    @parm   OUT TouchPointInfo * | TpInfo | Points to the Algorithm output touch point information.

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_DATA_ERROR.
--*/
NTSTATUS INTERNAL
OemProcessTpConfidence(
    __in        PCOMMAND_INFO CommandInfo,
    __in        ULONG TpNum,
    __inout     TouchPointInfo *TpInfo
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i, j;
    TouchPointInfo TpInfoTemp;

    TEnter(Func, ("(CommandInfo=%p, TpNum=%d, TpInfo=%p)", CommandInfo, TpNum, TpInfo));

    switch (CommandInfo->CmdType)
    {
        case VAL_CMDTYPE2:
            break;
        case VAL_CMDTYPE9:
            if (!CommandInfo->Cmd9Info.UseConfidence)   break;
        case VAL_CMDTYPE7:
            for (i = 0; i < TpNum; i++)
            {
                for (j = i; j < TpNum; j++)
                {
                    if (TpInfo[i].Confidence < TpInfo[j].Confidence)
                    {
                       MEMCOPY(&TpInfoTemp, &TpInfo[i], sizeof(TouchPointInfo));
                       MEMCOPY(&TpInfo[i], &TpInfo[j], sizeof(TouchPointInfo));
                       MEMCOPY(&TpInfo[j], &TpInfoTemp, sizeof(TouchPointInfo));
                    }
                }
            }
            break;
        default:
            status = STATUS_DATA_ERROR;
            TErr(("No command type %4x.", CommandInfo->CmdType));

            TExit(Func, ("=%x", status));
            return status;
    }

    TExit(Func, ("=%x", status));
    return status;
}       //OemProcessTpConfidence

/*++
    @doc    INTERNAL

    @func   NTSTATUS | OemTransToAlgInput | Transfer raw data to algorithm input.

    @parm   IN PUCHAR | FrameData | Points to the frame data in raw data.
    @parm   IN PCOMMAND_INFO | CommandInfo | Points to the command information through parser.
    @parm   OUT PUCHAR | SenseData | Points to the processed output data. (Only use for Command 0xA002 sense data)
    @parm   OUT PULONG | TpNum | Points to the algorithm output touch point number.
    @parm   OUT TouchPointInfo * | TpInfo | Points to the Algorithm output touch point information.

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_DATA_ERROR.
--*/
NTSTATUS INTERNAL
OemTransToAlgInput(
    __in        PUCHAR FrameData,
    __in        PCOMMAND_INFO CommandInfo,
    __out       PUCHAR SenseData,
    __out       PULONG pTpNum,
    __inout     TouchPointInfo **pTpInfo
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    TouchPointInfo *TpInfo = *pTpInfo; // default size alloc
    ULONG TpNum = 0;
    ULONG i, j, TpIndex;
    USHORT PosX, PosY;
	PCMD7_DATA Cmd7Data;
    PCMD9_DATA Cmd9Data = NULL;
    PCMD9_DATA_EX Cmd9DataEx = NULL;

    TEnter(Func, ("(FrameData=%p, CommandInfo=%p, SenseData=%p, TpNum=%d, TpInfo=%p)", 
        FrameData, CommandInfo, SenseData, *pTpNum, *pTpInfo));

    if (TpInfo == NULL)
    {
        status = STATUS_DATA_ERROR;
        TErr(("TpInfo buffer is null."));

        TExit(Func, ("=%x(TpNum=%d, TpInfo=%p)", status, *pTpNum, *pTpInfo));
        return status;
    }

    status = OemPreprocessFrameData(CommandInfo->CmdType, FrameData, CommandInfo->FrameDataLen, SenseData);
    if (!NT_SUCCESS(status))
    {
        TExit(Func, ("=%x(TpNum=%d, TpInfo=%p)", status, *pTpNum, *pTpInfo));
        return status;
    }

    FrameData = SenseData;
    switch (CommandInfo->CmdType)
    {
        case VAL_CMDTYPE2:
            break;
        case VAL_CMDTYPE7:
            *pTpNum = TpNum = CommandInfo->Cmd7Info.PointCount;
            if (TpNum > NUM_TPINFO_DEFAULT)
            {
                MEMFREE(TpInfo);
                TpInfo = MEMMALLOCNONPAGED(TpNum * sizeof(TouchPointInfo));
                *pTpInfo = TpInfo;
                if (TpInfo == NULL)
                {
                    status = STATUS_DATA_ERROR;
                    TErr(("Not allocate %ds TpInfo buffer.", TpNum));

                    TExit(Func, ("=%x(TpNum=%d, TpInfo=%p)", status, *pTpNum, *pTpInfo));
                    return status;
                }
            }

            Cmd7Data = (PCMD7_DATA)FrameData;
            for (i = 0; i < TpNum; i++)
            {
                TpInfo[i].ID = i;
                TpInfo[i].State = POINT_BIRTH;
#if 0
                TpInfo[i].DelX = 0;
                TpInfo[i].DelY = 0;
                TpInfo[i].PosX = (float)((float)(Cmd7Data[i].PosX) / (float)GET_ALG_PRECISE(CommandInfo->Cmd7Info.Precise)); // todo remove float
                TpInfo[i].PosY = (float)((float)(Cmd7Data[i].PosY) / (float)GET_ALG_PRECISE(CommandInfo->Cmd7Info.Precise));
                TpInfo[i].Confidence = (float)((float)(Cmd7Data[i].Confidence) / (float)GET_ALG_PRECISE(CommandInfo->Cmd7Info.Precise));
#endif
            }
            break;
        case VAL_CMDTYPE9:
            *pTpNum = TpNum = CommandInfo->Cmd9Info.PointCount;
            if (TpNum > NUM_TPINFO_DEFAULT)
            {
                MEMFREE(TpInfo);
                TpInfo = MEMMALLOCNONPAGED(TpNum * sizeof(TouchPointInfo));
                *pTpInfo = TpInfo;
                if (TpInfo == NULL)
                {
                    status = STATUS_DATA_ERROR;
                    TErr(("Error: Not allocate %ds TpInfo buffer.", TpNum));

                    TExit(Func, ("=%x(TpNum=%d, TpInfo=%p)", status, *pTpNum, *pTpInfo));
                    return status;
                }
            }

            if (CommandInfo->Cmd9Info.UseConfidence)
            {
                Cmd9DataEx = (PCMD9_DATA_EX)FrameData;
            }
            else
            {
                Cmd9Data = (PCMD9_DATA)FrameData;
            }
#define GET_CMD9_DATA_POS(i) ((CommandInfo->Cmd9Info.UseConfidence) ? (Cmd9DataEx[i].Pos) : (Cmd9Data[i].Pos))
            TpIndex = 0;
            for (i = 0; i < CommandInfo->Cmd9Info.XCount; i++)
            {
                for (j = 0; j < CommandInfo->Cmd9Info.YCount; j++)
                {
                    if (TpIndex >= TpNum)
                    {
                        TWarn(("TpIndex %d > TpInfo support %d.", TpIndex, TpNum));
                        break;
                    }
                    TpInfo[TpIndex].ID = TpIndex;
                    TpInfo[TpIndex].State = POINT_BIRTH;
#if 0
                    TpInfo[TpIndex].DelX = (float)0;
                    TpInfo[TpIndex].DelY = (float)0;
#endif
                    PosX = GET_CMD9_DATA_POS(i);
                    PosY = GET_CMD9_DATA_POS(j + CommandInfo->Cmd9Info.XCount);
                    TpInfo[TpIndex].PosX = (float)((float)(PosX & 0xFFFF) / (float)GET_ALG_PRECISE(CommandInfo->Cmd9Info.Precise)); // todo remove 0xffff
                    TpInfo[TpIndex].PosY = (float)((float)(PosY & 0xFFFF) / (float)GET_ALG_PRECISE(CommandInfo->Cmd9Info.Precise));
                    if (CommandInfo->Cmd9Info.UseConfidence)
                    {
                        TpInfo[TpIndex].Confidence = 
                            (float)((float)((float)(Cmd9DataEx[i].Confidence) / (float)GET_ALG_PRECISE(CommandInfo->Cmd9Info.Precise)) * 
                            (float)((float)(Cmd9DataEx[j + CommandInfo->Cmd9Info.XCount].Confidence) / GET_ALG_PRECISE(CommandInfo->Cmd9Info.Precise)));
                    }
                    TpIndex++;
                }
            }
            break;
        default:
            status = STATUS_DATA_ERROR;
            TErr(("No command type %4x.", CommandInfo->CmdType));

            TExit(Func, ("=%x(TpNum=%d, TpInfo=%p)", status, *pTpNum, *pTpInfo));
            return status;
    }

    status = OemProcessTpConfidence(CommandInfo, TpNum, TpInfo);

    TExit(Func, ("=%x(TpNum=%d, TpInfo=%p)", status, *pTpNum, *pTpInfo));
    return status;
}       //OemTransAlgInput

/*++
    @doc    INTERNAL

    @func   NTSTATUS | OemTransToReport | Transfer Algorithm output to HID report.

    @parm   IN ULONG | TpNum | Algorithm output touch point number.
    @parm   IN TouchPointInfo * | TpInfo | Points to the Algorithm output touch point information.
    @parm   OUT POEM_INPUT_REPORT | InData | Points to the input data packet.

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_DATA_ERROR.
--*/
NTSTATUS INTERNAL
OemTransToReport(
    __in   ULONG TpNum,
    __in   TouchPointInfo *TpInfo,
    __out  POEM_INPUT_REPORT InData
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR bStatus = 0;
    USHORT PosX = 0, PosY = 0;
    USHORT XSensingLineNum, YSensingLineNum;
#ifdef __UpdateToTouchMode
    ULONG i;
    UCHAR TouchId = 0;
    float PosXFloat, PosYFloat;
#endif // __UpdateToTouchMode

    TEnter(Func, ("(TpNum=%d, TpInfo=%p, InData=%p)", TpNum, TpInfo, InData));

#ifndef __UpdateToTouchMode
    UNREFERENCED_PARAMETER(TpNum);
#endif // __UpdateToTouchMode

    XSensingLineNum = 27; // 22, 26, 27  // todo constant
    YSensingLineNum = 15; // 13, 14, 15

#ifndef __UpdateToTouchMode
    if ((TpInfo[0].State == 0) || (TpInfo[0].State == 2)) // POINT_BIRTH(0), POINT_MOVE(2) // todo constant
    {
        bStatus = (INSTATUS_IN_RANGE | INSTATUS_PEN_TIP_DOWN);
    }
    else // POINT_DEATH(1), POINT_TAP(3), NOTOUCH(4)
    {
        bStatus = (INSTATUS_IN_RANGE);
    }
    PosX = (USHORT)((TpInfo[0].PosX * 200) / XSensingLineNum); // todo constant
    PosY = (USHORT)((TpInfo[0].PosY * 200) / YSensingLineNum);

    InData->InputReport.bStatus = bStatus;
    InData->InputReport.wXData = PosX;
    InData->InputReport.wYData = PosY;
    InData->InputReport.wPressureData = 0; //30
    InData->InputReport.bXTiltData = 0;
    InData->InputReport.bYTiltData = 0;

    TInfo(("Status=%x,x=%x(%d),y=%x(%d),Pressure=%x(%d),XTilt=%x(%d),YTilt=%x(%d)",
           InData->InputReport.bStatus, InData->InputReport.wXData,
           InData->InputReport.wXData, InData->InputReport.wYData,
           InData->InputReport.wYData, InData->InputReport.wPressureData,
           InData->InputReport.wPressureData, InData->InputReport.bXTiltData,
           InData->InputReport.bXTiltData, InData->InputReport.bYTiltData,
           InData->InputReport.bYTiltData));
#else // __UpdateToTouchMode
    if (TpNum > OEM_MT_MAX_COUNT)
    {
        TWarn(("TpNum %d > report support %d.", TpNum, OEM_MT_MAX_COUNT));
        TpNum = OEM_MT_MAX_COUNT;
    }
    for (i = 0; i < TpNum; i++)
    {
        if ((TpInfo[i].State == 0) || (TpInfo[i].State == 2)) // POINT_BIRTH(0), POINT_MOVE(2) // todo constant
        {
            bStatus = RANGE_FINGER_STATUS;
        }
        else // POINT_DEATH(1), POINT_TAP(3), NOTOUCH(4)
        {
            bStatus =  0x0;
        }
        PosXFloat = TpInfo[i].PosX * 4095; // todo constant
        PosYFloat = TpInfo[i].PosY * 4095;
        PosX = (USHORT)(PosXFloat / XSensingLineNum); // todo remove temp
        PosY = (USHORT)(PosYFloat / YSensingLineNum);
        TouchId = (UCHAR)TpInfo[i].ID;

        InData->InputReport.TouchData[i].bStatus = bStatus;
        InData->InputReport.TouchData[i].wXData = PosX;
        InData->InputReport.TouchData[i].wYData = PosY;
        InData->InputReport.TouchData[i].ContactId = TouchId;

        TInfo(("Status=%x,x=%d,y=%d,ID=%d",
               InData->InputReport.TouchData[i].bStatus, InData->InputReport.TouchData[i].wXData,
               InData->InputReport.TouchData[i].wYData, InData->InputReport.TouchData[i].ContactId));
    }
    InData->InputReport.ActualCount = (UCHAR)TpNum;

    TInfo(("Point count=%d", InData->InputReport.ActualCount));
#endif // __UpdateToTouchMode

    TExit(Func, ("=%x", status));
    return status;
}       //OemTransToReport

/*++
    @doc    INTERNAL

    @func   NTSTATUS | OemNormalizeInputData | Normalize the input data.

    @parm   IN PCOMMAND_INFO | CommandInfo | Points to the raw data command info.
    @parm   IN OUT POEM_INPUT_REPORT | InData | Points to the input data packet.

    @rvalue SUCCESS | Returns STATUS_SUCCESS.
    @rvalue FAILURE | Returns STATUS_DATA_ERROR.
--*/
NTSTATUS INTERNAL
OemNormalizeInputData(
    __in     PCOMMAND_INFO     CommandInfo,
    __inout  POEM_INPUT_REPORT InData
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    TouchPointInfo *TpInfo = NULL;
    ULONG TpNum = 0;
    PUCHAR FrameData = CommandInfo->FrameDataBuff;
    DRIVERINFO;
    TEnter(Func, ("(CommandInfo=%p, InData=%p, FrameData=%p)", CommandInfo, InData, FrameData));

    if (FrameData == NULL)
    {
        status = STATUS_DATA_ERROR;
        TErr(("The frame data start address fail: %x.", FrameData));

        TExit(Func, ("=%x", status));
        return status;
    }

    TpInfo = MEMMALLOCNONPAGED(NUM_TPINFO_DEFAULT * sizeof(TouchPointInfo));
    if (TpInfo == NULL)
    {
        status = STATUS_DATA_ERROR;
        TErr(("Not allocate %ds TpInfo buffer.", NUM_TPINFO_DEFAULT));

        TExit(Func, ("=%x", status));
        return status;
    }

    status = OemTransToAlgInput(FrameData, CommandInfo, FrameData, &TpNum, &TpInfo);
    if (!NT_SUCCESS(status))
    {
        if (TpInfo)
        {
            MEMFREE(TpInfo);
        }

        TExit(Func, ("=%x", status));
        return status;
    }

    switch (CommandInfo->CmdType)
    {
        case VAL_CMDTYPE2:
            status = Tracker( (int *)&TpNum, TpInfo ); // jiunhau 981022
            //status = TPDetector((char *)FrameData, (int *)&TpNum, TpInfo); //LINUX
            break;
        case VAL_CMDTYPE7:
#ifdef __TWO_TOUCH_TRACKER
            if (TpNum > 4)
            {
                TpNum = 4;
            }
#endif // __TWO_TOUCH_TRACKER
//		    status = TPDetector(NULL, (int *)&TpNum, TpInfo); //LINUX
            status = Tracker( (int *)&TpNum, TpInfo );
            break;
        case VAL_CMDTYPE9:
#ifdef __TWO_TOUCH_TRACKER
            if (TpNum > 4)
            {
                TpNum = 4;
            }
#endif // __TWO_TOUCH_TRACKER
//            status = TPDetector(NULL, (int *)&TpNum, TpInfo); //LINUX
            status = Tracker( (int *)&TpNum, TpInfo );
            break;
        default:
            status = STATUS_DATA_ERROR;
            TErr(("No command type %4x.", CommandInfo->CmdType));
            if (TpInfo)
            {
                MEMFREE(TpInfo);
            }

            TExit(Func, ("=%x", status));
            return status;
    }

    if (TpNum > 0)
    {
        status = OemTransToReport(TpNum, TpInfo, InData);
    }
    else
    {
        status = STATUS_DATA_ERROR;
    }

    if (TpInfo)
    {
        MEMFREE(TpInfo);
    }

//    PoSetSystemState(ES_USER_PRESENT); // todo check

    TExit(Func, ("=%x", status));
    return status;
}       //OemNormalizeInputData
