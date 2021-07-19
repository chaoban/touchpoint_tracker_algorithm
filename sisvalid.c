/*++
    Copyright (c) Silicon Integrated Systems Corporation

    Module Name:
        sisindata.c

    Abstract:
        Contains specific functions that process(parse) SiS touch raw data.

    Environment:
        Kernel mode

--*/

#include "sisinput.h"

#define MODULE_ID                       MODULE_SISVALID
#define MSGMDL_ID                       MSGMDL_SISVALID


//
// Key definitions
//


//
// Constants
//
#define NUMBYTE_RAWDATA_HEAD        4
#define NUMBYTE_RAWDATA_CMDTYPE     2
#define NUMBYTE_RAWDATA_CMDPARAM    2
#define NUMBYTE_RAWDATA_CMD         ((NUMBYTE_RAWDATA_CMDTYPE) + (NUMBYTE_RAWDATA_CMDPARAM))

#define NUM_BYTE_BIT                8


//
// Macros
//


//
// Type definitions
//


//
// Extern function prototype.
//
VOID INTERNAL
PrintResync(
    __in     PRESYNC_BUFFER ResyncBuffer
    );


//
// Local function prototype.
//


//
// Global variable definitions
//
//BOOLEAN UseSimpleHead = TRUE;
BOOLEAN UseSimpleHead = FALSE;


//
// parser(validate) functions
//
static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

/*++
    @doc    INTERNAL

    @func   BOOLEAN | crc16_ccitt | Count CRC value.

    @parm   IN PUCHAR | buf | Points to the parser buffer data start address.
    @parm   IN UINT32 | len | data length.

    @rvalue VALUE | Returns CRC VALUE.
--*/
USHORT INTERNAL 
crc16_ccitt(
    __in PUCHAR buf, 
    __in UINT32 len
    )
{
	register UINT32 counter;
	register USHORT crc = 0;

    TEnter(Func, ("(buf=%p, len=%d)", buf, len));

	for (counter = 0; counter < len; counter++)
    {
		crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0xFF];
    }

    TExit(Func, ("=%x", crc));
	return crc;
}       //crc16_ccitt

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsHeadValid | Check raw data header valid.

    @parm   IN PUCHAR | pbStart | Points to the parser buffer data start address.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsHeadValid(
	__in PUCHAR pbStart
	)
{
    TEnter(Func, ("(pbStart=%p)", pbStart));

    if( (pbStart[0] == 'P') &&
	    (pbStart[1] == '8') &&
		(pbStart[2] == '1') &&
		(pbStart[3] == '0') )
	{
        TExit(Func, ("=%x", TRUE));
	    return TRUE;
	}
	else
	{
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
}       //OemIsHeadValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsCmdTypeValid | Check command type valid.

    @parm   IN PUCHAR | pbStart | Points to the parser buffer data start address.
    @parm   OUT PCOMMAND_INFO | PRAW_COMMAND | Points to the raw command info that put the raw command data.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsCmdTypeValid(
	__in PUCHAR pbStart,
    __out PRAW_COMMAND RawCommand
    )
{
    PUCHAR pb = pbStart;
    USHORT CmdType = (pb[1] | (pb[0] << (sizeof(pb[1]) * NUM_BYTE_BIT)));

    TEnter(Func, ("(pbStart=%p, RawCommand=%p)", pbStart, RawCommand));

    if (UseSimpleHead)
    {
        RawCommand->CmdType = VAL_CMDTYPE9;
        TExit(Func, ("=%x", TRUE));
        return TRUE;
    }

    switch (CmdType)
    {
        case VAL_CMDTYPE2:
        case VAL_CMDTYPE7:
        case VAL_CMDTYPE9:
                RawCommand->CmdType = CmdType;
                TExit(Func, ("=%x", TRUE));
                return TRUE;
        default:
            TErr(("No command type %4x.", CmdType));
            break;
    }

    TExit(Func, ("=%x", FALSE));
    return FALSE;
}       //OemIsCmdTypeValid

#define GET_CMD9_PARAM_X_CNT(c) (c & 0x3)
#define GET_CMD9_PARAM_Y_CNT(c) ((c & (0x3 << 2)) >> 2)
#define GET_CMD9_PARAM_CRC_FLAG(c) ((c & (0x1 << 4)) >> 4)
#define GET_CMD9_PARAM_CONFIDENCE_FLAG(c) ((c & (0x1 << 5)) >> 5)
#define GET_CMD9_PARAM_DUMMY(c) ((c & (0x3 << 6)) >> 6)

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsCmd9DataTypeValid | Check command 9 parameter data type valid.

    @parm   IN UCHAR | DataType | data type for command 9 parameter.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsCmd9DataTypeValid(
	__in UCHAR DataType
    )
{
    UCHAR XCount, YCount, CrcFlag, ConfidenceFlag, Dummy;

    TEnter(Func, ("(DataType=%x)", DataType));

    XCount = GET_CMD9_PARAM_X_CNT(DataType);
    YCount = GET_CMD9_PARAM_Y_CNT(DataType);
    CrcFlag = GET_CMD9_PARAM_CRC_FLAG(DataType);
    ConfidenceFlag = GET_CMD9_PARAM_CONFIDENCE_FLAG(DataType);
    Dummy = GET_CMD9_PARAM_DUMMY(DataType);

    if (((XCount == 0) && (YCount != 0)) ||
        ((XCount != 0) && (YCount == 0)) ||
        (UseSimpleHead && !CrcFlag) ||      // Smbus path must have crc check
        (Dummy))
    {
        TExit(Func, ("=%x", FALSE));
        return FALSE;
    }

    TExit(Func, ("=%x", TRUE));
    return TRUE;
}       //OemIsCmd9DataTypeValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsCmd9FrameIdValid | Check command 9 parameter frame ID valid.

    @parm   IN UCHAR | CurrFrameId | Current frame ID for command 9 parameter.
    @parm   IN UCHAR | PreFrameId | Preview frame ID for command 9 parameter.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsCmd9FrameIdValid(
	__in UCHAR CurrFrameId,
	__in UCHAR PreFrameId
    )
{
    TEnter(Func, ("(CurrFrameId=%d, PreFrameId=%d)", CurrFrameId, PreFrameId));

    PreFrameId += 4; // todo constant
    if ((PreFrameId - CurrFrameId) & ~(4 - 1))
    {
        TExit(Func, ("=%x", FALSE));
        return FALSE;
    }

    TExit(Func, ("=%x", TRUE));
    return TRUE;
}       //OemIsCmd9FrameIdValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsCmdParamValid | Check command parameter valid.

    @parm   IN PUCHAR | pbStart | Points to the parser buffer data start address.
    @parm   INOUT PCOMMAND_INFO | PRAW_COMMAND | Points to the raw command info that put the raw command data.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsCmdParamValid(
	__in PUCHAR pbStart,
    __inout PRAW_COMMAND RawCommand
    )
{
    PUCHAR pb = pbStart;

    TEnter(Func, ("(pbStart=%p, RawCommand=%p)", pbStart, RawCommand));

    switch (RawCommand->CmdType)
    {
        case VAL_CMDTYPE2:
            if (pb[0] != 0x00)
            {
                TErr(("It's not a vaild length %2x %2x.", pb[0], pb[1]));
                TExit(Func, ("=%x", FALSE));
                return FALSE;
            }
            RawCommand->Cmd2Param.Length = (pb[1] | (pb[0] << (sizeof(pb[1]) * NUM_BYTE_BIT)));
            break;
        case VAL_CMDTYPE7:
            RawCommand->Cmd7Param.Precise = pb[0];
            RawCommand->Cmd7Param.PointCount = pb[1];
            break;
        case VAL_CMDTYPE9:
            if (!OemIsCmd9DataTypeValid(pb[1]))
            {
                TErr(("It's not a vaild data type %2x.", pb[1]));
                TExit(Func, ("=%x", FALSE));
                return FALSE;
            }
            if (UseSimpleHead)
            {
                if (!OemIsCmd9FrameIdValid(pb[0], RawCommand->Cmd9Param.FrameId)) // RawCommand is input for frame ID
                {
                    TErr(("It's not a vaild frame ID %2x.", pb[0]));
                    TExit(Func, ("=%x", FALSE));
                    return FALSE;
                }
            }
            RawCommand->Cmd9Param.FrameId = pb[0];
            RawCommand->Cmd9Param.DataType = pb[1];
            break;
        default:
            TErr(("No command type %2x %2x.", pb[0], pb[1]));
            TExit(Func, ("=%x", FALSE));
            return FALSE;
    }

    TExit(Func, ("=%x", TRUE));
    return TRUE;
}       //OemIsCmdParamValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemGetCmd9FrameDataLen | Get frame data length from command data for command 9.

    @parm   IN PCOMMAND_INFO | PRAW_COMMAND | Points to the raw command info that put the raw command data.

    @rvalue VALUE | Returns frame data length for command 9.
--*/
ULONG INTERNAL
OemGetCmd9FrameDataLen(
	__in UCHAR DataType
    )
{
    UCHAR Length, XCount, YCount, CrcFlag, ConfidenceFlag;

    TEnter(Func, ("(DataType=%x)", DataType));

    XCount = GET_CMD9_PARAM_X_CNT(DataType);
    YCount = GET_CMD9_PARAM_Y_CNT(DataType);
    CrcFlag = GET_CMD9_PARAM_CRC_FLAG(DataType);
    ConfidenceFlag = GET_CMD9_PARAM_CONFIDENCE_FLAG(DataType);

    Length = (((XCount + YCount) * (1 + ConfidenceFlag)) + CrcFlag) * sizeof(CMD9_DATA);

    TExit(Func, ("=%d", Length));
    return Length;
}       //OemGetCmd9FrameDataLen

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemGetCmdFrameDataLen | Get frame data length from command data.

    @parm   IN PCOMMAND_INFO | PRAW_COMMAND | Points to the raw command info that put the raw command data.

    @rvalue VALUE | Returns frame data length.
--*/
ULONG INTERNAL
OemGetCmdFrameDataLen(
    __in PRAW_COMMAND RawCommand
    )
{
	ULONG FrameDataLength = 0;

    TEnter(Func, ("(RawCommand=%p)", RawCommand));

    switch (RawCommand->CmdType)
    {
        case VAL_CMDTYPE2:
            FrameDataLength = (RawCommand->Cmd2Param.Length & 0xFFFF) * (sizeof(CMD2_DATA) * 2); // todo remove 0xffff
            break;
        case VAL_CMDTYPE7:
            FrameDataLength = (RawCommand->Cmd7Param.PointCount & 0xFF) * sizeof(CMD7_DATA); // todo remove 0xff
            break;
        case VAL_CMDTYPE9:
            FrameDataLength = OemGetCmd9FrameDataLen(RawCommand->Cmd9Param.DataType);
            break;
        default:
            TErr(("No command type %4x.", RawCommand->CmdType));
            break;
    }

    TExit(Func, ("=%d", FrameDataLength));
    return FrameDataLength;
}       //OemGetCmdFrameDataLen

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsFrameDataValid | Check frame buffer data valid for command 9.

    @parm   IN PUCHAR | pbStart | Points to the parser buffer data start address.
    @parm   IN UCHAR | DataType | Data type for command 9.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsCmd9FrameDataValid(
	__in PUCHAR pbStart,
    __in UCHAR DataType
    )
{
    PUCHAR pb = pbStart;
	USHORT RawCrc = 0, CheckCrc = 0;
	ULONG FrameDataLength = 0;

    TEnter(Func, ("(pbStart=%p, DataType=%x)", pbStart, DataType));

    if (GET_CMD9_PARAM_X_CNT(DataType) &&
        GET_CMD9_PARAM_Y_CNT(DataType) &&
        GET_CMD9_PARAM_CRC_FLAG(DataType))
    {
        FrameDataLength = OemGetCmd9FrameDataLen(DataType);
        RawCrc = pb[FrameDataLength + 1] | pb[FrameDataLength] << (sizeof(pb[0]) * NUM_BYTE_BIT);
        CheckCrc = crc16_ccitt(pb, FrameDataLength);
        if (CheckCrc != RawCrc)
        {
            TErr(("CRC check fail. Raw: %x, Check: %x.", RawCrc, CheckCrc));
            TExit(Func, ("=%x", FALSE));
            return FALSE;
        }
    }

    TExit(Func, ("=%x", TRUE));
	return TRUE;
}       //OemIsCmd9FrameDataValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsFrameDataValid | Check frame buffer data valid.

    @parm   IN PUCHAR | pbStart | Points to the parser buffer data start address.
    @parm   OUT PCOMMAND_INFO | PRAW_COMMAND | Points to the raw command info that put the raw command data.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsFrameDataValid(
	__in PUCHAR pbStart,
    __in PRAW_COMMAND RawCommand
    )
{
    PUCHAR pb = pbStart;

    TEnter(Func, ("(pbStart=%p, RawCommand=%p)", pbStart, RawCommand));

    switch (RawCommand->CmdType)
    {
        case VAL_CMDTYPE2:
        case VAL_CMDTYPE7:
            break;
        case VAL_CMDTYPE9:
            if (!OemIsCmd9FrameDataValid(pb - NUMBYTE_RAWDATA_CMDPARAM, RawCommand->Cmd9Param.DataType)) // todo note
            {
                TExit(Func, ("=%x", FALSE));
                return FALSE;
            }
            break;
        default:
            TErr(("No command type %4x.", RawCommand->CmdType));
            break;
    }

    TExit(Func, ("=%x", TRUE));
	return TRUE;
}       //OemIsFrameDataValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsFrameValid | Fill command info from raw command data.

    @parm   IN PCOMMAND_INFO | PRAW_COMMAND | Points to the raw command info that put the raw command data.
    @parm   IN PUCHAR | FrameDataBuff | Points to the buffer frame data start address.
    @parm   OUT PCOMMAND_INFO | CommandInfo | Points to the command info that put the parser command result.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemFillCommandInfo(
    __in    PRAW_COMMAND RawCommand,
    __in    PUCHAR FrameDataBuff,
    __out   PCOMMAND_INFO CommandInfo
    )
{
    TEnter(Func, ("(RawCommand=%p, FrameDataBuff=%p, CommandInfo=%p)", 
        RawCommand, FrameDataBuff, CommandInfo));

	CommandInfo->CmdType = RawCommand->CmdType;
    switch (RawCommand->CmdType)
    {
        case VAL_CMDTYPE2:
            CommandInfo->Cmd2Info.Length = RawCommand->Cmd2Param.Length;
            break;
        case VAL_CMDTYPE7:
            CommandInfo->Cmd7Info.Precise = RawCommand->Cmd7Param.Precise;
            CommandInfo->Cmd7Info.PointCount = RawCommand->Cmd7Param.PointCount;
            break;
        case VAL_CMDTYPE9:
            CommandInfo->Cmd9Info.FrameId = RawCommand->Cmd9Param.FrameId;
            CommandInfo->Cmd9Info.XCount = GET_CMD9_PARAM_X_CNT(RawCommand->Cmd9Param.DataType);
            CommandInfo->Cmd9Info.YCount = GET_CMD9_PARAM_Y_CNT(RawCommand->Cmd9Param.DataType);
            CommandInfo->Cmd9Info.UseCrc = GET_CMD9_PARAM_CRC_FLAG(RawCommand->Cmd9Param.DataType) ;
            CommandInfo->Cmd9Info.UseConfidence = GET_CMD9_PARAM_CONFIDENCE_FLAG(RawCommand->Cmd9Param.DataType);

#define VAL_CMD9_PRECISE_DEFAULT    7
            CommandInfo->Cmd9Info.Precise = VAL_CMD9_PRECISE_DEFAULT;
            CommandInfo->Cmd9Info.PointCount = CommandInfo->Cmd9Info.XCount * CommandInfo->Cmd9Info.YCount;
            break;
        default:
            TErr(("No command type %4x.", RawCommand->CmdType));
            break;
    }

    CommandInfo->FrameDataBuff = FrameDataBuff;
    CommandInfo->FrameDataLen = OemGetCmdFrameDataLen(RawCommand);

    TExit(Func, ("=%x", TRUE));
	return TRUE;
}       //OemFillCommandInfo

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsFrameValid | Check if the data is valid frame.

    @parm   IN PUCHAR | pbStart | Points to the buffer start address.
    @parm   IN PUCHAR | pbEnd | Points to the buffer data end address.
    @parm   OUT PRAW_PARSER | RawParser | Points to the raw parser info that put parser status.
    @parm   OUT PCOMMAND_INFO | CommandInfo | Points to the command info that put the parser command result.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsFrameValid(
	__in PUCHAR pbStart,
	__in PUCHAR pbEnd,
    __out PRAW_PARSER RawParser,
    __out PCOMMAND_INFO CommandInfo
    )
{
    PUCHAR pb = pbStart;
	RAW_COMMAND RawCommand = {0}, RawCommandTemp = {0};
    PUCHAR FrameDataBuff = NULL;
	ULONG FrameDataLength = 0;

    TEnter(Func, ("(pbStart=%p, pbEnd=%p, RawParser=%p, CommandInfo=%p)", 
        pbStart, pbEnd, RawParser, CommandInfo));

    // init parser info
    RawParser->ReadMoreBytes = FALSE;
    RawParser->CurrentHead = NULL;
    RawParser->NextHead = NULL;

#define CHECK_RAWDATA_LEN(readbytes) (pb > (pbEnd - readbytes))
	if (CHECK_RAWDATA_LEN(NUMBYTE_RAWDATA_HEAD))
	{
	    RawParser->ReadMoreBytes = TRUE;
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
	if (!OemIsHeadValid(pb))
	{
	    if (!UseSimpleHead)
	    {
            TExit(Func, ("=%x", FALSE));
	        return FALSE;
	    }
	}
	else
	{
        pb += NUMBYTE_RAWDATA_HEAD;
	}

	if (CHECK_RAWDATA_LEN(NUMBYTE_RAWDATA_CMDTYPE))
	{
	    RawParser->ReadMoreBytes = TRUE;
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
	if (!OemIsCmdTypeValid(pb, &RawCommand))
	{
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}

    if (UseSimpleHead)
    {
        RawCommand.Cmd9Param.FrameId = RawParser->FrameId; // Smbus need check frame ID in param valid
    }
    else // Smbus path needn't validate command type
    {
        pb += NUMBYTE_RAWDATA_CMDTYPE;
    }

	if (CHECK_RAWDATA_LEN(NUMBYTE_RAWDATA_CMDPARAM))
	{
	    RawParser->ReadMoreBytes = TRUE;
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
	if (!OemIsCmdParamValid(pb, &RawCommand))
	{
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
	pb += NUMBYTE_RAWDATA_CMDPARAM;

    FrameDataBuff = pb;
    FrameDataLength = OemGetCmdFrameDataLen (&RawCommand);
	if (CHECK_RAWDATA_LEN(FrameDataLength))
	{
	    if (((pbEnd - pbStart) == SIZE_INPUT_SYNC_BUFF) || 
	        ((pb + FrameDataLength) > (pbStart + SIZE_INPUT_SYNC_BUFF))) // Note: pbStart maybe the Resync buffer start address or raw input buffer
	    {
            TErr(("Frame necessary length %d exceed the buffer size  %d.", FrameDataLength, SIZE_INPUT_SYNC_BUFF));
	    }
	    else
	    {
    	    RawParser->ReadMoreBytes = TRUE;
    	}
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
	if (!OemIsFrameDataValid(pb, &RawCommand))
	{
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
    pb += FrameDataLength;

	if (CHECK_RAWDATA_LEN(NUMBYTE_RAWDATA_HEAD))
	{
	    RawParser->ReadMoreBytes = TRUE;
        TExit(Func, ("=%x", FALSE));
	    return FALSE;
	}
    if (!OemIsHeadValid(pb))
    {
	    if (UseSimpleHead)
	    {
        	MEMCOPY(&RawCommandTemp, &RawCommand, sizeof(RAW_COMMAND));
        	if (!OemIsCmdParamValid(pb, &RawCommandTemp))
        	{
                TExit(Func, ("=%x", FALSE));
        	    return FALSE;
        	}
	    }
	    else
	    {
            TExit(Func, ("=%x", FALSE));
            return FALSE;
        }
    }

    // fill parser info
    RawParser->CurrentHead = pbStart;
    RawParser->NextHead = pb;
    if (UseSimpleHead)
    {
        RawParser->FrameId = RawCommand.Cmd9Param.FrameId;
    }

	// fill command info
    OemFillCommandInfo(&RawCommand, FrameDataBuff, CommandInfo);

    TExit(Func, ("=%x", TRUE));
	return TRUE;
}       //OemIsFrameValid

/*++
    @doc    INTERNAL

    @func   BOOLEAN | OemIsResyncDataValid | Check if the data in the resync
            buffer is valid.

    @parm   IN OUT PRESYNC_BUFFER | ResyncBuffer | Points to the resync buffer.
    @parm   OUT PRAW_PARSER | RawParser | Points to the raw data parser status.
    @parm   OUT PCOMMAND_INFO | CommandInfo | Points to the raw data command info.

    @rvalue SUCCESS | Returns TRUE.
    @rvalue FAILURE | Returns FALSE.
--*/
BOOLEAN INTERNAL
OemIsResyncDataValid(
    __inout PRESYNC_BUFFER ResyncBuffer,
    __out PRAW_PARSER      RawParser,
    __out PCOMMAND_INFO    CommandInfo
    )
{
    BOOLEAN rc;
    PUCHAR pbStart = (PUCHAR)ResyncBuffer->ResyncData;
    PUCHAR pbEnd = (PUCHAR)ResyncBuffer->ResyncData + ResyncBuffer->BytesInBuff;

    TEnter(Func, ("(ResyncBuffer=%p,RawParser=%p,CommandInfo=%p)", ResyncBuffer, RawParser, CommandInfo));

    rc = (BOOLEAN)OemIsFrameValid(pbStart, pbEnd, RawParser, CommandInfo);
    if ((rc == FALSE) && (!RawParser->ReadMoreBytes))
    {
        PUCHAR pb = pbStart;

        while (pb < pbEnd)
        {
            pb++;
            if (pb > (pbEnd - NUMBYTE_RAWDATA_HEAD))
            {
                ResyncBuffer->BytesInBuff = (ULONG)(pbEnd - pb);
                MEMMOVE(pbStart, pb, ResyncBuffer->BytesInBuff);
                RawParser->ReadMoreBytes = TRUE;

                MTInfo(MSGFLTR_INPUTDATA, ("(OemIsResyncDataValid) 1."));
                PrintResync(ResyncBuffer);
                break;
            }
            else if (OemIsHeadValid(pb))
            {
                ResyncBuffer->BytesInBuff = (ULONG)(pbEnd - pb);
                MEMMOVE(pbStart, pb, ResyncBuffer->BytesInBuff);

                MTInfo(MSGFLTR_INPUTDATA, ("(OemIsResyncDataValid) 2."));
                PrintResync(ResyncBuffer);
                break;
            }
            else if (UseSimpleHead)
            {
                if (OemIsCmd9DataTypeValid(pb[1])) // only valid data type // todo reset frame ID
                {
                    ResyncBuffer->BytesInBuff = (ULONG)(pbEnd - pb);
                    MEMMOVE(pbStart, pb, ResyncBuffer->BytesInBuff);

                    MTInfo(MSGFLTR_INPUTDATA, ("(OemIsResyncDataValid) 3."));
                    PrintResync(ResyncBuffer);
                    break;
                }
            }
        } // while

        if (pb >= pbEnd)
        {
            ResyncBuffer->BytesInBuff = 0;
        }
	}

    TExit(Func, ("=%x(BytesInBuff=%d)", rc, ResyncBuffer->BytesInBuff));
    return rc;
}       //OemIsResyncDataValid

//
// Get parser status functions (for extern)
//

/*++
    @doc    INTERNAL

    @func   ULONG | OemGetParserReadStatus | Get parser read more bytes status.

    @parm   IN PRAW_PARSER | RawParser | Points to the raw data parser status.

    @rvalue TRUE | Returns parser need to read more bytes.
    @rvalue FALSE | Returns parser needn't read more bytes.
--*/
ULONG INTERNAL
OemGetParserReadStatus(
    __in     PRAW_PARSER RawParser
    )
{
    TEnter(Func, ("(RawParser=%p)", RawParser));

    TExit(Func, ("=%d", RawParser->ReadMoreBytes));
    return RawParser->ReadMoreBytes;
}       //OemGetParserReadStatus

/*++
    @doc    INTERNAL

    @func   ULONG | OemGetProcessedRawDataLen | Get processed raw data length.

    @parm   IN PRAW_PARSER | RawParser | Points to the raw data parser status.

    @rvalue Value | Returns processed raw data length.
--*/
ULONG INTERNAL
OemGetProcessedRawDataLen(
    __in     PRAW_PARSER RawParser
    )
{
    ULONG Length = 0;

    TEnter(Func, ("(RawParser=%p)", RawParser));

    if (!RawParser->ReadMoreBytes)
    {
        Length = RawParser->NextHead - RawParser->CurrentHead;
    }

    TExit(Func, ("=%d", Length));
    return Length;
}       //OemGetProcessedRawDataLen
