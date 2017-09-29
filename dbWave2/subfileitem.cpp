// subfileitem.cpp

#include "stdafx.h"
#include "subfileitem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////
// class CSubfileItem

IMPLEMENT_DYNCREATE(CSubfileItem, CObject);

// Construction / Destruction
CSubfileItem::CSubfileItem()
{
	m_rec.wCode=STRUCT_JUMP;		// Subfile type identifier
	SetLabel("JUMP");	
	m_rec.ucEncoding=UL_MODE;		// Encoding mode of the subfile (offset=UL)
	m_rec.itemnb= 0;				// item number
	m_rec.ulOffset=0;				// Pointer to the beginning of the subfile
	m_rec.ulLength=0;				// Length of the subfile
}

CSubfileItem::CSubfileItem(unsigned char ucCode, char* szLabel, ULONGLONG ulOffset, ULONGLONG ulLength, unsigned char ucEncoding, int itemnb)
{
	m_rec.wCode = ucCode;			// data, acqdef, acqchan, ..., zero if end
	SetLabel(szLabel);	
	m_rec.ulOffset = ulOffset;		// offset within file
	m_rec.ulLength = ulLength;		// length of the subfile
	m_rec.ucEncoding = ucEncoding;	// type: normal(default), compressed (future implement.)
	m_rec.itemnb = itemnb;
}

CSubfileItem::~CSubfileItem()
{
}

// long Write(CDataFile& datafile)
// write the structure in the binary file datafile
// assume that file pointer is correctly set

ULONGLONG CSubfileItem::Write(CFile*  pfile)
{
	ULONGLONG lFirst = pfile->GetPosition();	// current file position

	pfile->Write(&m_rec.szLabel, (UINT) LABEL_LEN+1);
	m_rec.ucEncoding = UL_MODE;
	pfile->Write(&m_rec.ucEncoding, (UINT) 1);

	pfile->Write(&m_rec.wCode,		(UINT) sizeof(WORD));
	pfile->Write(&m_rec.ulOffset,	(UINT) sizeof(ULONGLONG));
	pfile->Write(&m_rec.ulLength,	(UINT) sizeof(ULONGLONG));
	pfile->Write(&m_rec.itemnb,		(UINT) sizeof(int));
	
	ULONGLONG lLast = pfile->GetPosition();		// file position after writing
	return (lLast - lFirst);					// return length of data written
}

// void Read(CDataFile datafile)
// read the structure in the binary file datafile
// assume file pointer correctly set

void CSubfileItem::Read(CFile* pfile)
{
	pfile->Read(&m_rec.szLabel, (UINT) LABEL_LEN+1);
	UINT ucEncoding = 0;
	pfile->Read(&ucEncoding, (UINT) 1);

	if (UL_MODE == ucEncoding)
	{
		pfile->Read(&m_rec.wCode,		(UINT) sizeof(WORD));
		pfile->Read(&m_rec.ulOffset,	(UINT) sizeof(ULONGLONG));
		pfile->Read(&m_rec.ulLength,	(UINT) sizeof(ULONGLONG));
		pfile->Read(&m_rec.itemnb,		(UINT) sizeof(int));	
	}
	else
	{
		ASSERT(NORMAL_MODE == ucEncoding);
		pfile->Read(&m_rec.wCode, (UINT) sizeof(WORD));
		long lValue;
		pfile->Read(&lValue, (UINT) sizeof(long));
		m_rec.ulOffset = lValue;
		pfile->Read(&lValue, (UINT) sizeof(long));
		m_rec.ulLength = lValue;
		pfile->Read(&m_rec.itemnb, (UINT) sizeof(int));	
	}
}

void CSubfileItem::SetLabel(char* pszLabel)
{
	char* plab = &m_rec.szLabel[0];
	int i=0;
	for (i=0; i< LABEL_LEN; i++, plab++, pszLabel++)
	{
		if (*pszLabel == 0)
			break;
		*plab = *pszLabel;
	}
	for (i; i<LABEL_LEN;i++, plab++)
		*plab=' ';
	m_rec.szLabel[LABEL_LEN]=0;
}