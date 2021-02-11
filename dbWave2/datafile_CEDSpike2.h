#pragma once
#include "datafile_X.h"

class CDataFileFromCEDSpike2 :
    public CDataFileX
{
public:
	CDataFileFromCEDSpike2();
	DECLARE_DYNCREATE(CDataFileFromCEDSpike2)
	virtual ~CDataFileFromCEDSpike2();

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
	CStringA		m_csFiledesc;
	CWaveFormat*	m_pWFormat;
	CWaveChanArray* m_pArray;
	
	// Generated message map functions
protected:
};

