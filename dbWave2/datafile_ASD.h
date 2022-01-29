#pragma once
#include "datafile_X.h"

class CDataFileASD :
	public CDataFileX
{
public:
	CDataFileASD();
	DECLARE_DYNCREATE(CDataFileASD)
	~CDataFileASD() override;

	// Operations
public:
	BOOL ReadDataInfos(CWaveBuf* pBuf) override;
	int CheckFileType(CString& cs_filename) override;

protected:
	CString m_csOldStringID;
	CString m_csStringID;
	WORD m_wID;

	// Implementation
public:
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
