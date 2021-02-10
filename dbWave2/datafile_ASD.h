#pragma once
#include "datafile_X.h"

class CDataFileASD : 
	public CDataFileX
{
public:
	CDataFileASD();     
	DECLARE_DYNCREATE(CDataFileASD)
	virtual ~CDataFileASD();

	// Operations
public:
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray) override;
	int CheckFileType(CFile* file) override;

protected:
	CString	m_csOldStringID;
	CString	m_csStringID;
	WORD	m_wID;

	// Implementation
public:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
