#pragma once
// subfileitem.h

// Definition of Awave files and I/O functions

///////////////////////////////////////////////////////////////////
// Proprietary data file
///////////////////////////////////////////////////////////////////

// CSubfileItem
// subfile description item: ID, label, subfile offset, length, encoding mode
//
// Subfile identifiers
#define	STRUCT_NOP			1	// empty descriptor
#define	STRUCT_ACQCHAN		2	// data acq channels
#define STRUCT_ACQDEF		3	// data acquisition parameters
#define STRUCT_DATA			4	// data acquisition block
#define STRUCT_JUMP			5	// chain to descriptor located elsewhere in the file
#define STRUCT_VTAGS		6	// vertical tags
#define STRUCT_HZTAGS		7	// horizontal tags

// add new subfiles identifiers here

// ............
// last subfile #ID
#define	STRUCT_END			0	// end of chain of descriptors

// Encoding data mode (describe how data are encoded; provisional)
#define NORMAL_MODE			0	// encoding mode of aWave files 1.0
#define UL_MODE				1	// encoding with ULONGLONG

// add new encoding mode here
// ................
#define LABEL_LEN 8

struct SUBF_DESCRIP
{
	WORD			wCode;					// Subfile type identifier
	int				itemnb;					// index current item
	unsigned char	ucEncoding;				// Encoding mode of the subfile
	char			szLabel[LABEL_LEN + 1];	// Subfile type label
	ULONGLONG		ulOffset;				// pointer to the beginning of the subfile (long)
	ULONGLONG		ulLength;				// length of the subfile
};

class CSubfileItem : public CObject
{
	DECLARE_DYNCREATE(CSubfileItem)
protected:
	// Attributes

	SUBF_DESCRIP m_rec;

public:
	CSubfileItem();
	CSubfileItem(unsigned char ucCode, char* csLabel, ULONGLONG ulOffset,
		ULONGLONG ulLength, unsigned char ucEncoding, int itemnb = 0);
	~CSubfileItem();

	// Helpers
	WORD		GetCode() const { return m_rec.wCode; }
	CStringA	GetLabel() const { return m_rec.szLabel; }
	unsigned char GetDataEncoding() const { return m_rec.ucEncoding; };
	int		GetItemnb() const { return m_rec.itemnb; }
	ULONGLONG GetDataOffset() const { return m_rec.ulOffset; }
	ULONGLONG GetDataLength() const { return m_rec.ulLength; }

	void SetCode(unsigned char ucCode) { m_rec.wCode = ucCode; }
	void SetLabel(char* pszLabel);
	void SetDataEncoding(unsigned char ucEncoding) { m_rec.ucEncoding = ucEncoding; }
	void SetItemnb(int item) { m_rec.itemnb = item; }
	void SetDataOffset(ULONGLONG ulOffset) { m_rec.ulOffset = ulOffset; }
	void SetDataLength(ULONGLONG ulLength) { m_rec.ulLength = ulLength; }

	// I/O Operations
	void Read(CFile* pdatafile);
	ULONGLONG Write(CFile* pdatafile);
};
