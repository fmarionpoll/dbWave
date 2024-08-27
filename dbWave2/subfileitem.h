#pragma once


// CSubfileItem
// sub-file description item: ID, label, sub-file offset, length, encoding mode
//
// Sub-file identifiers
#define	STRUCT_NOP			1	// empty descriptor
#define	STRUCT_ACQCHAN		2	// data acq channels
#define STRUCT_ACQDEF		3	// data acquisition parameters
#define STRUCT_DATA			4	// data acquisition block
#define STRUCT_JUMP			5	// chain to descriptor located elsewhere in the file
#define STRUCT_VTAGS		6	// vertical tags
#define STRUCT_HZTAGS		7	// horizontal tags

// add new sub-files identifiers here
// last sub-file #ID
#define	STRUCT_END			0	// end of chain of descriptors
#define NORMAL_MODE			0	// encoding mode of aWave files 1.0
#define UL_MODE				1	// encoding with ULONGLONG
#define LABEL_LEN 			8

struct SUBF_DESCRIP
{
	WORD w_code; 				// Sub-file type identifier
	int item_nb; 				// index current item
	unsigned char uc_encoding; 	// Encoding mode of the sub-file
	char sz_label[LABEL_LEN + 1]; // Sub-file type label
	ULONGLONG ul_offset; 		// pointer to the beginning of the su-file (long)
	ULONGLONG ul_length; 		// length of the sub-file
};

class CSubfileItem : public CObject
{
	DECLARE_DYNCREATE(CSubfileItem)
protected:
	// Attributes

	SUBF_DESCRIP m_rec_;

public:
	CSubfileItem();
	CSubfileItem(unsigned char uc_code, char* sz_label, ULONGLONG ul_offset,
	             ULONGLONG ul_length, unsigned char uc_encoding, int item_nb = 0);
	~CSubfileItem() override;

	// Helpers
	WORD get_code() const { return m_rec_.w_code; }
	CStringA get_label() const { return m_rec_.sz_label; }
	unsigned char get_data_encoding() const { return m_rec_.uc_encoding; };
	int get_item_nb() const { return m_rec_.item_nb; }
	ULONGLONG get_data_offset() const { return m_rec_.ul_offset; }
	ULONGLONG get_data_length() const { return m_rec_.ul_length; }

	void set_code(const unsigned char uc_code) { m_rec_.w_code = uc_code; }
	void set_label(char* psz_label);
	void set_data_encoding(const unsigned char uc_encoding) { m_rec_.uc_encoding = uc_encoding; }
	void set_item_nb(const int item) { m_rec_.item_nb = item; }
	void set_data_offset(const ULONGLONG ul_offset) { m_rec_.ul_offset = ul_offset; }
	void set_data_length(const ULONGLONG ul_length) { m_rec_.ul_length = ul_length; }

	// I/O Operations
	void read(CFile* p_file);
	ULONGLONG write(CFile* p_file);
};
