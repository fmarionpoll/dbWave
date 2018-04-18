// CUSBPxxS1Ctl0.h  : Declaration of ActiveX Control wrapper class(es) created by Microsoft Visual C++

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CUSBPxxS1Ctl0

class CUSBPxxS1Ctl0 : public CWnd
{
protected:
	DECLARE_DYNCREATE(CUSBPxxS1Ctl0)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x9A90F3D9, 0x6EEA, 0x4735, { 0x87, 0xCB, 0xE2, 0xDC, 0x7A, 0xF1, 0x6E, 0xC6 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID,
						CCreateContext* pContext = nullptr)
	{
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID);
	}

	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
				UINT nID, CFile* pPersist = nullptr, BOOL bStorage = FALSE,
				BSTR bstrLicKey = nullptr)
	{
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey);
	}

// Attributes
public:

// Operations
public:

	void USBPxxS1Command(long Handle, long CmdID, VARIANT * DataInPtr, VARIANT * DataOutPtr)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT ;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, nullptr, parms, Handle, CmdID, DataInPtr, DataOutPtr);
	}


};
