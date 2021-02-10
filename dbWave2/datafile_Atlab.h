// AtlabFile.h : header file
//
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDataFileATLAB document

class CDataFileATLAB : public CDataFileX
{
public:
	CDataFileATLAB();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDataFileATLAB)

	// Attributes
public:

	// Operations
public:
	BOOL ReadDataInfos(CWaveFormat* pWFormat, CWaveChanArray* pArray);
	int  CheckFileType(CFile* file);
protected:
	void InitDummyChansInfo(short chanlistindex);
	void LoadChanFromCyber(short i, char* pcyberchan);
	void InitChansFromCyberA320(char* pHeader, short version);
	CString GetCyberA320filter(int ncode);

	CWaveFormat* m_pWFormat;
	CWaveChanArray* m_pArray;

	// Implementation
public:
	virtual ~CDataFileATLAB();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
};
