// subfileitem.cpp

#include "StdAfx.h"
#include "subfileitem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_DYNCREATE(CSubfileItem, CObject);

// Construction / Destruction
CSubfileItem::CSubfileItem()
{
	m_rec_.w_code = STRUCT_JUMP; 	// Sub-file type identifier
	set_label("JUMP");
	m_rec_.uc_encoding = UL_MODE; 	// Encoding mode of the sub-file (offset=UL)
	m_rec_.item_nb = 0; 			// item number
	m_rec_.ul_offset = 0; 			// Pointer to the beginning of the sub-file
	m_rec_.ul_length = 0; 			// Length of the sub-file
}

CSubfileItem::CSubfileItem(const unsigned char uc_code, char* sz_label, const ULONGLONG ul_offset, const ULONGLONG ul_length,
                           const unsigned char uc_encoding, const int item_nb)
{
	m_rec_.w_code = uc_code; // data, acq_def, acq_chan, ..., zero if end
	set_label(sz_label);
	m_rec_.ul_offset = ul_offset; // offset within file
	m_rec_.ul_length = ul_length; // length of the sub-file
	m_rec_.uc_encoding = uc_encoding; // type: normal(default), compressed (future implement.)
	m_rec_.item_nb = item_nb;
}

CSubfileItem::~CSubfileItem()
{
}

// long Write(CDataFile& datafile)
// write the structure in the binary file datafile
// assume that file pointer is correctly set

ULONGLONG CSubfileItem::write(CFile* p_file)
{
	const auto l_first = p_file->GetPosition(); // current file position

	p_file->Write(&m_rec_.sz_label, static_cast<UINT>(LABEL_LEN) + 1);
	m_rec_.uc_encoding = UL_MODE;
	p_file->Write(&m_rec_.uc_encoding, static_cast<UINT>(1));

	p_file->Write(&m_rec_.w_code, sizeof(WORD));
	p_file->Write(&m_rec_.ul_offset, sizeof(ULONGLONG));
	p_file->Write(&m_rec_.ul_length, sizeof(ULONGLONG));
	p_file->Write(&m_rec_.item_nb, sizeof(int));

	const auto l_last = p_file->GetPosition(); // file position after writing
	return (l_last - l_first); // return length of data written
}

// void Read(CDataFile datafile)
// read the structure in the binary file datafile
// assume file pointer correctly set

void CSubfileItem::read(CFile* p_file)
{
	p_file->Read(&m_rec_.sz_label, static_cast<UINT>(LABEL_LEN) + 1);
	UINT uc_encoding = 0;
	p_file->Read(&uc_encoding, static_cast<UINT>(1));

	if (UL_MODE == uc_encoding)
	{
		p_file->Read(&m_rec_.w_code, sizeof(WORD));
		p_file->Read(&m_rec_.ul_offset, sizeof(ULONGLONG));
		p_file->Read(&m_rec_.ul_length, sizeof(ULONGLONG));
		p_file->Read(&m_rec_.item_nb, sizeof(int));
	}
	else
	{
		ASSERT(NORMAL_MODE == uc_encoding);
		p_file->Read(&m_rec_.w_code, sizeof(WORD));
		long l_value;
		p_file->Read(&l_value, sizeof(long));
		m_rec_.ul_offset = l_value;
		p_file->Read(&l_value, sizeof(long));
		m_rec_.ul_length = l_value;
		p_file->Read(&m_rec_.item_nb, sizeof(int));
	}
}

void CSubfileItem::set_label(char* psz_label)
{
	auto p_lab = &m_rec_.sz_label[0];
	auto i = 0;
	for (auto j = i; j < LABEL_LEN; j++, i++, p_lab++, psz_label++)
	{
		if (*psz_label == 0)
			break;
		*p_lab = *psz_label;
	}

	for (auto k = i; k < LABEL_LEN; k++, p_lab++)
		*p_lab = ' ';
	m_rec_.sz_label[LABEL_LEN] = 0;
}
