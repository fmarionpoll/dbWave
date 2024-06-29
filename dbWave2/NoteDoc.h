#pragma once
#include <afxrich.h>		// MFC rich edit classes

class CNoteDoc : public CRichEditDoc
{
protected: // create from serialization only
	CNoteDoc();
	DECLARE_DYNCREATE(CNoteDoc)

	// Overrides
public:
	BOOL OnNewDocument() override;
	void Serialize(CArchive& ar) override;

	BOOL OnOpenDocument(LPCTSTR lpsz_path_name) override;
	CRichEditCntrItem* CreateClientItem(REOBJECT* p_re_object) const override;
	BOOL open_project_files(CString& cs_path_name);

	// Implementation
public:
	~CNoteDoc() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected:
	BOOL open_file_list(CString& cs_path_name, CStringArray& cs_array_files, CStringArray& cs_descriptions_array, int n_columns) const;

	static int extract_list(CRichEditCtrl& p_edit, CStringArray& cs_array_file_names, CStringArray& cs_descriptors_array);
	static BOOL add_file_name(const CString& res_token, CStringArray& cs_array_ok, CStringArray& cs_array_tested);
	static void display_files_imported(CRichEditCtrl& p_edit, const CStringArray& cs_descriptors_list);
	static int parse_row2(const CString& cs_row, CStringArray& cs_columns);
	static void add_row_to_array(const CStringArray& row_array, CStringArray& cs_descriptors_array);
	static BOOL is_file_present(const CString& cs_filename)
	{
		CFileStatus status;
		return CFile::GetStatus(cs_filename, status);
	}

	DECLARE_MESSAGE_MAP()
};
