// subfileitem.cpp

#include "StdAfx.h"
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
	m_rec.wCode = STRUCT_JUMP; // Subfile type identifier
	SetLabel("JUMP");
	m_rec.ucEncoding = UL_MODE; // Encoding mode of the subfile (offset=UL)
	m_rec.itemnb = 0; // item number
	m_rec.ulOffset = 0; // Pointer to the beginning of the subfile
	m_rec.ulLength = 0; // Length of the subfile
}

CSubfileItem::CSubfileItem(unsigned char ucCode, char* szLabel, ULONGLONG ulOffset, ULONGLONG ulLength,
                           unsigned char ucEncoding, int itemnb)
{
	m_rec.wCode = ucCode; // data, acqdef, acqchan, ..., zero if end
	SetLabel(szLabel);
	m_rec.ulOffset = ulOffset; // offset within file
	m_rec.ulLength = ulLength; // length of the subfile
	m_rec.ucEncoding = ucEncoding; // type: normal(default), compressed (future implement.)
	m_rec.itemnb = itemnb;
}

CSubfileItem::~CSubfileItem()
{
}

// long Write(CDataFile& datafile)
// write the structure in the binary file datafile
// assume that file pointer is correctly set

ULONGLONG CSubfileItem::Write(CFile* pfile)
{
	const auto l_first = pfile->GetPosition(); // current file position

	pfile->Write(&m_rec.szLabel, static_cast<UINT>(LABEL_LEN) + 1);
	m_rec.ucEncoding = UL_MODE;
	pfile->Write(&m_rec.ucEncoding, static_cast<UINT>(1));

	pfile->Write(&m_rec.wCode, sizeof(WORD));
	pfile->Write(&m_rec.ulOffset, sizeof(ULONGLONG));
	pfile->Write(&m_rec.ulLength, sizeof(ULONGLONG));
	pfile->Write(&m_rec.itemnb, sizeof(int));

	const auto l_last = pfile->GetPosition(); // file position after writing
	return (l_last - l_first); // return length of data written
}

// void Read(CDataFile datafile)
// read the structure in the binary file datafile
// assume file pointer correctly set

void CSubfileItem::Read(CFile* pfile)
{
	pfile->Read(&m_rec.szLabel, static_cast<UINT>(LABEL_LEN) + 1);
	UINT uc_encoding = 0;
	pfile->Read(&uc_encoding, static_cast<UINT>(1));

	if (UL_MODE == uc_encoding)
	{
		pfile->Read(&m_rec.wCode, sizeof(WORD));
		pfile->Read(&m_rec.ulOffset, sizeof(ULONGLONG));
		pfile->Read(&m_rec.ulLength, sizeof(ULONGLONG));
		pfile->Read(&m_rec.itemnb, sizeof(int));
	}
	else
	{
		ASSERT(NORMAL_MODE == uc_encoding);
		pfile->Read(&m_rec.wCode, sizeof(WORD));
		long l_value;
		pfile->Read(&l_value, sizeof(long));
		m_rec.ulOffset = l_value;
		pfile->Read(&l_value, sizeof(long));
		m_rec.ulLength = l_value;
		pfile->Read(&m_rec.itemnb, sizeof(int));
	}
}

void CSubfileItem::SetLabel(char* pszLabel)
{
	auto plab = &m_rec.szLabel[0];
	auto i = 0;
	for (auto j = i; j < LABEL_LEN; j++, i++, plab++, pszLabel++)
	{
		if (*pszLabel == 0)
			break;
		*plab = *pszLabel;
	}

	for (auto k = i; k < LABEL_LEN; k++, plab++)
		*plab = ' ';
	m_rec.szLabel[LABEL_LEN] = 0;
}
