#pragma once

// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

// CDTAcq32 wrapper class

class CDTAcq32 : public CWnd
{
protected:
	DECLARE_DYNCREATE(CDTAcq32)
public:
	const CLSID& GetClsid()
	{
		static const CLSID clsid
			= {0xeccba8a5, 0x1f6d, 0x11cf, {0xb6, 0x97, 0x0, 0x20, 0xaf, 0xeb, 0x10, 0xa0}};
		return clsid;
	}

	BOOL Create(LPCTSTR lpszClassName,
	            LPCTSTR lpszWindowName, DWORD dw_style,
	            const RECT& rect,
	            CWnd* pParentWnd, UINT nID,
	            CCreateContext* pContext = nullptr) override
	{
		return CreateControl(GetClsid(), lpszWindowName, dw_style, rect, pParentWnd, nID);
	}

	BOOL Create(LPCTSTR lpszWindowName, DWORD dw_style,
	            const RECT& rect, CWnd* pParentWnd, UINT nID,
	            CFile* pPersist = nullptr, BOOL bStorage = FALSE,
	            BSTR bstrLicKey = nullptr)
	{
		return CreateControl(GetClsid(), lpszWindowName, dw_style, rect, pParentWnd, nID,
		                     pPersist, bStorage, bstrLicKey);
	}

	// Attributes
public:
	CString GetBoard();
	CString GetCaption();
	short GetCascadeMode();
	short GetChannelType();
	long GetClockDivider();
	short GetClockSource();
	short GetCTMode();
	short GetDataFlow();
	CString GetDeviceName();
	short GetDmaUsage();
	long GetDTConnectXferCount();
	long GetDTConnectXferSize();
	short GetEncoding();
	double GetFrequency();
	short GetGateType();
	long GetHDass();
	long GetHDev();
	OLE_HANDLE GetHWnd();
	CString GetLastErrDescription();
	long GetLastError();
	long GetListSize();
	float GetMaxRange();
	float GetMinRange();
	long GetMultiScanCount();
	short GetNumBoards();
	short GetNumFilters();
	short GetNumGains();
	short GetNumRanges();
	short GetNumResolutions();
	short GetNumSubSystems();
	short GetPreTrigger();
	short GetPulseType();
	double GetPulseWidth();
	long GetQueue();
	short GetRange();
	long GetReserved();
	short GetResolution();
	short GetRetrigger();
	double GetRetriggerFreq();
	short GetRetriggerMode();
	short GetSubSysElement();
	short GetSubSystem();
	short GetSubSysType();
	short GetSyncDIOUsage();
	short GetTrigger();
	short GetTriggeredScan();
	short GetWrapMode();
	void SetBoard(LPCTSTR);
	void SetCaption(LPCTSTR);
	void SetCascadeMode(short);
	void SetChannelType(short);
	void SetClockDivider(long);
	void SetClockSource(short);
	void SetCTMode(short);
	void SetDataFlow(short);
	void SetDeviceName(LPCTSTR);
	void SetDmaUsage(short);
	void SetDTConnectXferCount(long);
	void SetDTConnectXferSize(long);
	void SetEncoding(short);
	void SetFrequency(double);
	void SetGateType(short);
	void SetHDass(long);
	void SetHDev(long);
	void SetHWnd(OLE_HANDLE);
	void SetLastErrDescription(LPCTSTR);
	void SetLastError(long);
	void SetListSize(long);
	void SetMaxRange(float);
	void SetMinRange(float);
	void SetMultiScanCount(long);
	void SetNumBoards(short);
	void SetNumFilters(short);
	void SetNumGains(short);
	void SetNumRanges(short);
	void SetNumResolutions(short);
	void SetNumSubSystems(short);
	void SetPreTrigger(short);
	void SetPulseType(short);
	void SetPulseWidth(double);
	void SetQueue(long);
	void SetRange(short);
	void SetReserved(long);
	void SetResolution(short);
	void SetRetrigger(short);
	void SetRetriggerFreq(double);
	void SetRetriggerMode(short);
	void SetSubSysElement(short);
	void SetSubSystem(short);
	void SetSubSysType(short);
	void SetSyncDIOUsage(short);
	void SetTrigger(short);
	void SetTriggeredScan(short);
	void SetWrapMode(short);

	// Operations
public:
	void Abort();
	void AboutBox();
	void ClearError();
	void Config();
	void Continue();
	long CTReadEvents();
	void DTConnectBurst(long TransferSize, long TransferCount);
	void EnumBoards();
	void EnumSS();
	void EnumSSCaps(short uiCap);
	void Flush();
	CString GetBoardList(short index);
	short GetChannelList(short index);
	short GetDevCaps(short olDc);
	long GetDIOList(short index);
	double GetFilterList(short index);
	double GetFilterValues(short index);
	double GetGainList(short index);
	double GetGainValues(short index);
	BOOL GetInhibitList(short index);
	double GetMaxRangeValues(short index);
	double GetMinRangeValues(short index);
	short GetQueueSize(short uiQueue);
	short GetResolutionValues(short index);
	long GetSingleValue(long uiChannel, double dGain);
	long GetSSCaps(short OlSSc);
	double GetSSCapsEx(short OlSSc);
	CString GetSubSystemList(short index);
	void MeasureFrequency(double dCountDuration);
	void Pause();
	void PutSingleValue(long uiChannel, double dGain, long lValue);
	void Reset();
	void SetChannelList(short index, short nNewValue);
	void SetDIOList(short index, long nNewValue);
	void SetFilterList(short index, double newValue);
	void SetGainList(short index, double newValue);
	void SetInhibitList(short index, BOOL bNewValue);
	void Start();
	void Stop();
};
