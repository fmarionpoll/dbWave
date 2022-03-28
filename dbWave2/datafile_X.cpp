#include "StdAfx.h"
#include "datafile_X.h"

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
	UINT m_u_open_flag = modeReadWrite | shareDenyNone | typeBinary;
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
	const LONGLONG l_off = (static_cast<LONGLONG>(dataIndex) * sizeof(short)) + m_ulOffsetData;
	Seek(l_off, begin);
	const long l_size = Read(pBuffer, nbpoints);
	// adjust dependent parameters
	return l_size / sizeof(short);
}

int CDataFileX::CheckFileType(CString& cs_filename)
{
	return DOCTYPE_UNKNOWN;
}

BOOL CDataFileX::ReadDataInfos(CWaveBuf* pBuf)
{
	return TRUE;
}

BOOL CDataFileX::ReadHZtags(TagList* pHZtags)
{
	return FALSE;
}

BOOL CDataFileX::ReadVTtags(TagList* pVTtags)
{
	return FALSE;
}

BOOL CDataFileX::InitFile()
{
	return FALSE;
}

BOOL CDataFileX::DataAppendStart()
{
	m_ulbytescount = 0;
	return FALSE;
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

BOOL CDataFileX::WriteHZtags(TagList* ptags)
{
	return FALSE;
}

BOOL CDataFileX::WriteVTtags(TagList* ptags)
{
	return FALSE;
}

int CDataFileX::isPatternPresent(char* bufRead, int lenRead, const char* bufPattern, int lenPattern)
{
	std::string needle(bufPattern, lenPattern - 1);
	std::string haystack(bufRead, lenRead);
	std::size_t n = haystack.find(needle);
	int flag = DOCTYPE_UNKNOWN;
	if (n != std::string::npos)
	{
		flag = m_idType;
		m_csType = bufPattern;
	}
	return flag;
}

bool CDataFileX::OpenDataFile(CString& sz_path_name, UINT u_open_flag)
{
	CFileException error{};
	bool flag = Open(sz_path_name, u_open_flag, &error);
	if (flag == 0)
		Abort();
	return flag;
}

void CDataFileX::CloseDataFile()
{
	if (m_hFile != hFileNull)
		Close();
}
