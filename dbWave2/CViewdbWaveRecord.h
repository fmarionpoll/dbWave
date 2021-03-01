#pragma once
//#include <afxdao.h>
#pragma warning(disable : 4995)


class CViewdbWaveRecord : public CDaoRecordView
{
	DECLARE_DYNCREATE(CViewdbWaveRecord)

protected:
	CViewdbWaveRecord();
	virtual ~CViewdbWaveRecord();

public:
	enum { IDD = 0 };
	
	CdbWaveDoc* GetDocument();
	virtual void OnDraw(CDC* pDC); 
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()

public:
	BOOL	PreCreateWindow(CREATESTRUCT& cs)  override;
	virtual CDaoRecordset* OnGetRecordset();

};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* CViewdbWaveRecord::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif

