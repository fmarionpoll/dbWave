#pragma once

// ASDFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDataFileASD
class CDataFileASD : public CDataFileX
{
public:
	CDataFileASD();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDataFileASD)

	// Operations
public:
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray);
	BOOL CheckFileType(CFile* file);

protected:
	CString	m_csOldStringID;
	CString	m_csStringID;
	WORD	m_wID;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CDataFileASD)
		//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CDataFileASD();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
