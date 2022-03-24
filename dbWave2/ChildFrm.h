#pragma once

class CChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	void ActivateFrame(int n_cmd_show = -1) override;
	~CChildFrame() override;

	UINT m_viewON =  ID_VIEW_DATABASE;
	int m_previousviewON = ID_VIEW_DATABASE ;
	int m_cursorstate = 0;
	int m_nStatus = 0;

#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
protected:
	BOOL m_bDeleteFile{ false };
	BOOL m_bKeepChoice{ false };

	void replaceView(CRuntimeClass* pViewClass, HMENU hmenu);
	void OnToolsImportfiles(int ifilter);
	void exportASCII(int option);
	BOOL exportToExcel();
	BOOL exportToExcelAndBuildPivot(int option);
	void buildExcelPivot(void* oApp, void* odataSheet, CString csSourceDataAddress, CString csNameSheet,
	                     short XlConsolidationFunction, int col2);

protected:
	afx_msg void OnViewCursormodeNormal();
	afx_msg void OnUpdateViewCursormodeNormal(CCmdUI* pCmdUI);
	afx_msg void OnViewCursormodeMeasure();
	afx_msg void OnUpdateViewCursormodeMeasure(CCmdUI* pCmdUI);
	afx_msg void OnViewCursormodeZoomin();
	afx_msg void OnUpdateViewCursormodeZoomin(CCmdUI* pCmdUI);
	afx_msg void OnOptionsBrowsemode();
	afx_msg void OnOptionsPrintmargins();
	afx_msg void OnOptionsLoadsaveoptions();
	afx_msg void OnToolsExportdatacomments();
	afx_msg void OnToolsExportdataAsText();
	afx_msg void OnToolsExportnumberofspikes();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void ReplaceViewIndex(UINT nID);
	afx_msg void OnUpdateViewmenu(CCmdUI* pCmdUI);

public:
	afx_msg void OnRecordGotorecord();
	afx_msg void OnRecordDeletecurrent();
	afx_msg void OnRecordAdd();
	afx_msg void OnToolsRemoveMissingFiles();
	afx_msg void OnToolsRemoveduplicatefiles();
	afx_msg void OnToolsRestoredeletedfiles();
	afx_msg void OnToolsSynchronizesourceinformationsCurrentfile();
	afx_msg void OnToolsSynchronizesourceinformationsAllfiles();
	afx_msg void OnToolsRemoveartefactfiles();
	afx_msg void OnToolsCheckFilelistsConsistency();
	afx_msg void OnToolsImportATFfiles();
	//afx_msg void OnToolsImportSyntechaspkfiles();
	afx_msg void OnToolsImportDatafiles();
	afx_msg void OnToolsImportSpikefiles();
	afx_msg void OnToolsImportDatabase();
	afx_msg void OnToolsCopyAllProjectFiles();
	afx_msg void OnToolsExportdatafile();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnToolsPathsRelative();
	afx_msg void OnToolsPathsAbsolute();
	afx_msg void OnToolsPath();
	afx_msg void OnToolsRemoveunused();
	afx_msg void OnToolsImport();
	afx_msg void OnToolsSynchro();
	afx_msg void OnToolsGarbage();
	afx_msg void OnToolsCompactdatabase();

	DECLARE_MESSAGE_MAP()
};
