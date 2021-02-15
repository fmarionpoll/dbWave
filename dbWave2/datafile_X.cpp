// CDataFileX.cpp : implementation file
//

#include "StdAfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Wrapper to generic data files
// Object subclassed for specific data files of other types
// One routine is common to all: reading data from files assuming that data
// are 16 bits numbers and that channels are interleaved

IMPLEMENT_DYNCREATE(CDataFileX, CObject)

CDataFileX::CDataFileX()
{
	m_bHeaderSize = 0;
	m_ulOffsetData = 0;
	m_ulOffsetHeader = 0;
	m_ulbytescount = 0;
	m_idType = DOCTYPE_UNKNOWN;
	m_csType = _T("UNKNOWN");
}

CDataFileX::~CDataFileX()
{
}

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

long CDataFileX::ReadAdcData(long dataIndex, long nbpoints, short* pBuffer, CWaveChanArray* pArray)
{
	// seek and read CFile
	const LONGLONG l_off = (LONGLONG(dataIndex) * sizeof(short)) + m_ulOffsetData;
	Seek(l_off, CFile::begin);
	const long l_size = Read(pBuffer, nbpoints);
	// adjust dependent parameters
	return l_size / sizeof(short);
}

int CDataFileX::CheckFileType(CFile* pfile)
{
	return DOCTYPE_UNKNOWN;
}
BOOL CDataFileX::ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray)
{
	return TRUE;
}
BOOL CDataFileX::ReadHZtags(CTagList* pHZtags)
{
	return FALSE;
}
BOOL CDataFileX::ReadVTtags(CTagList* pVTtags)
{
	return FALSE;
}

BOOL CDataFileX::InitFile()
{
	return FALSE;
}
BOOL CDataFileX::DataAppendStart()
{
	m_ulbytescount = 0; return FALSE;
}
BOOL CDataFileX::DataAppend(short* pBU, UINT uibytesLength)
{
	return FALSE;
}
BOOL CDataFileX::DataAppendStop()
{
	return FALSE;
}
BOOL CDataFileX::WriteDataInfos(CWaveFormat* pwF, CWaveChanArray* pwC)
{
	return FALSE;
}
BOOL CDataFileX::WriteHZtags(CTagList* ptags)
{
	return FALSE;
}
BOOL CDataFileX::WriteVTtags(CTagList* ptags)
{
	return FALSE;
}

int CDataFileX::isPatternPresent(char* bufRead, int lenRead, const char* bufPattern, int lenPattern) 
{
	std::string needle(bufPattern, lenPattern-1);
	std::string haystack(bufRead, lenRead);
	std::size_t n = haystack.find(needle);
	int flag = DOCTYPE_UNKNOWN;
	if (n != std::string::npos) {
		flag = m_idType;
		m_csType = bufPattern;
	}
	return flag;
}

bool CDataFileX::isOpened(CString& sz_path_name)
{
	if (m_hFile == CFile::hFileNull)
		return false;
	return (GetFileName().CompareNoCase(sz_path_name) == 0);
}

bool CDataFileX::openDataFile(CString& sz_path_name, UINT u_open_flag)
{
	bool flag = Open(sz_path_name, u_open_flag);
	if (!flag)
		Abort();
	return flag;
}

void CDataFileX::closeDataFile()
{
	if (m_hFile != CFile::hFileNull)
		Close();
}

