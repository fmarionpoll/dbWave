#pragma once

#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "scopescr.h"
#include "Envelope.h" 
#include "chanlistitem.h"
#include "Lineview.h"
#include "Spikedoc.h"
#include "Spikebar.h"

// CDataListCtrlRowObject command target

class CDataListCtrlRowObject : public CObject
{
	DECLARE_SERIAL(CDataListCtrlRowObject)
public:
	CDataListCtrlRowObject();
	CDataListCtrlRowObject(int i);
	virtual ~CDataListCtrlRowObject();

	BOOL			bChanged;			// flag set TRUE if contents has changed
	WORD			wversion;			// version number

	BOOL			bInit;
	UINT			index;
	long			insectID;

	CString			csComment;
	CString			csDatafileName;
	CString			csSpikefileName;
	CString			csSensillumname;
	CString			csStim1;
	CString			csConc1;
	CString			csStim2;
	CString			csConc2;
	CString			csNspk;
	CString			csFlag;
	CString			csDate;

	CLineViewWnd*   pdataWnd;
	CSpikeBarWnd*	pspikeWnd;
	CAcqDataDoc*	pdataDoc;
	CSpikeDoc*		pspikeDoc;

	void operator = (const CDataListCtrlRowObject& arg);	// operator redefinition
	virtual void Serialize(CArchive& ar);					// overridden for document i/o
};


