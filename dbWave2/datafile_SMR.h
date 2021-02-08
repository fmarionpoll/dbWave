#pragma once
#include "datafile_X.h"
//
//const long SON_NUMFILECOMMENTS = 5;
//const long SON_COMMENTSZ = 79;
//const long SON_CHANCOMSZ = 71;
//const long SON_UNITSZ = 5;
//const long SON_TITLESZ = 9;
//
//Type TMarkBytes
//	acCode(0 To 3) As Byte
//EndType
//
//Type TMarker
//	mark As Long
//	mvals As TMarkBytes
//EndType
//
//Type TSONTimeDate
//	ucHun As Byte
//	ucSec As Byte
//	ucMin As Byte
//	ucHour As Byte
//	ucDay As Byte
//	ucMon As Byte
//	wYear As Integer;
//End Type
//
//Type TSONCreator
//	acID(0 To 7) As Byte
//End Type
//
//Enum TDataKind
//	ChanOff = 0
//	Adc
//	EventFall
//	EventRise
//	EventBoth
//	Marker
//	AdcMark
//	RealMark
//	TextMark
//	RealWave
//End Enum
//
//Type TFilterMask
//	amask As Opaque
//	lFlags As Long
//End Type
//
//const long SON_FALLLAYERS = -1;
//const long SON_FALLITEMS = -1;
//const long SON_FCLEAR = 0;
//const long SON_FSET = 1;
//const long SON_FINVERT = 2;
//const long SON_FREAD = -1;
//const long SON_FMASK_ANDMODE = 0;
//const long SON_FMASK_ORMODE = 0x02000000;
//
//#define LENCOPYRIGHT 10
//struct Header {};
//typedef struct SMR_Type: Header 
//{
//	short			systemID = 0;
//	char			copyright[LENCOPYRIGHT] = { 0 };	// space for "(C) CED 877
//	TSONCreator		creator;							/* optional application identifier */
//	WORD			usPerTime;							/* microsecs per time unit */
//	WORD			timePerADC;							/* time units per ADC interrupt */
//	short			fileState;							/* condition of the file */
//	TDOF			firstData;							/* offset to first data block */
//	short			channels;							/* maximum number of channels */
//	WORD			chanSize;							/* memory size to hold chans */
//	WORD			extraData;							/* No of bytes of extra data in file */
//	WORD			bufferSz;							/* Not used on disk; bufferP in bytes */
//	WORD			osFormat;							/* 0x0101 for Mac, or 0x0000 for PC */
//	TSTime			maxFTime;							/* max time in the data file */
//	double			dTimeBase;							/* time scale factor, normally 1.0e-6 */
//	TSONTimeDate	timeDate;							/* time that corresponds to tick 0 */
//	char			pad0[3];							/* align next item to 4 bytes */
//	TDOF			LUTable;							/* 0, or the TDOF to a saved lut */ 
//	char			pad[44];							/* padding for the future */
//	TFileComment	fileComment;						/* what user thinks of it so far */
//} SMR_HEADER;
//
//typedef struct CFS_Type : Header
//{
//	char	signature[8] = { 0 };	// "CEDFILE "(+ !=1, "=2 ...)
//	char	File_name[14] = { 0 };	// Zero-terminated
//	long	file_size = 0;			// in byte
//	char	creation_time[8] = { 0 };
//	char	creation_date[8] = { 0 };
//	short	channels_in_DS = 0;
//	short	number_channel_variables = 0;
//	short	number_DS_variables = 0;
//	short	byte_size_file_header = 0;
//	short	byte_size_DS_header = 0;
//	long	last_data_section_header_offset = 0;
//	WORD	number_of_DS = 0;
//	WORD	disk_blocksize_rounding = 1;		// 1=none
//	char	File_comment[74] = { 0 };
//	long	pointer_table_offset = 0;
//	char	reserved_space[40] = { 0 };
//} CFS_HEADER;
//


constexpr auto LENCOPYRIGHT = 10;

class CDataFileSMR :
    public CDataFileX
{
public:
	CDataFileSMR();
	DECLARE_DYNCREATE(CDataFileSMR)

	// Operations
public:
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray);
	BOOL CheckFileType(CFile* file);

	// Implementation
public:
	virtual ~CDataFileSMR();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStringA	m_csFiledesc;						// ASCII chain w. file version
	
	// Generated message map functions
protected:
};

