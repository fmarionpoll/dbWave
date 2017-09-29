// CDataFileX.cpp : implementation file
//

#include "stdafx.h"
#include "dataheader_Atlab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Wrapper to generic data files
// Object subclassed for specific data files of other types
// One routine is common to all: reading data from files assuming that data
// are 16 bits numbers and that channels are interleaved

/////////////////////////////////////////////////////////////////////////////
// CDataFileX

IMPLEMENT_DYNCREATE(CDataFileX, CObject)

CDataFileX::CDataFileX()
{
	m_bHeaderSize = 0;
	m_ulOffsetData = 0;
	m_ulOffsetHeader = 0;
	m_idType=DOCTYPE_UNKNOWN;
	m_csType= _T("UNKNOWN");
}


CDataFileX::~CDataFileX()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDataFileX diagnostics

#ifdef _DEBUG
void CDataFileX::AssertValid() const
{
	CObject::AssertValid();
}

void CDataFileX::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "headersize = " << m_bHeaderSize;
	dc << "header_offset = " << m_ulOffsetHeader;
	dc << "data_offset = " << m_ulOffsetData;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CDataFileX commands

long CDataFileX::ReadData(long dataIndex, long nbpoints, short* pBuffer)
{
	// seek and read CFile
	LONGLONG lOff = (dataIndex * sizeof(short)) + m_ulOffsetData;
	Seek(lOff, CFile::begin);
	long lSize = Read(pBuffer, nbpoints);
	// adjust dependent parameters
	return lSize / sizeof(short);
}

////////////////////////////////////////////////////////////////////////
// base class implementation of virtual functions

BOOL CDataFileX::CheckFileType(CFile* pfile, int bSignatureOffset)
	{return DOCTYPE_UNKNOWN;}
BOOL CDataFileX::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
	{return TRUE;}
BOOL CDataFileX::ReadHZtags(CTagList* pHZtags) 
	{return FALSE;}
BOOL CDataFileX::ReadVTtags(CTagList* pVTtags) 
	{return FALSE;}

// I/O operations to be updated by Save

BOOL CDataFileX::InitFile() 
	{return FALSE;}
BOOL CDataFileX::DataAppendStart() 
	{m_ulbytescount = 0; return FALSE;}
BOOL CDataFileX::DataAppend(short* pBU, UINT uibytesLength) 
	{return FALSE; }
BOOL CDataFileX::DataAppendStop() 
	{return FALSE;}
BOOL CDataFileX::WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC) 
	{return FALSE;}
BOOL CDataFileX::WriteHZtags(CTagList* ptags)
	{return FALSE;}
BOOL CDataFileX::WriteVTtags(CTagList* ptags) 
	{return FALSE;}

CString	CDataFileX::GetFileType() 
{ 
	CString cstype;
	cstype = CA2T(m_csType);
	return cstype; 
}