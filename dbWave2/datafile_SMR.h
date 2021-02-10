#pragma once
#include "datafile_X.h"

class CDataFileSMR :
    public CDataFileX
{
public:
	CDataFileSMR();
	DECLARE_DYNCREATE(CDataFileSMR)
	virtual ~CDataFileSMR();

	// Operations
public:
	int  CheckFileType(CFile* file) override;
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray) override;


	// Implementation
public:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStringA	m_csFiledesc;
	
	// Generated message map functions
protected:
};

