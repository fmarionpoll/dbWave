#pragma once

#include "dbIndexTable.h"
#include "dbMainTable.h"
#include "dbWdatabase.h"
#include "spikepar.h"		// spike parameters options
#include "Spikelist.h"		// list of spikes
#include "Spikeclas.h"		// spike classes
#include "Spikedoc.h"
#include "AcqDataDoc.h"

struct sourceData 
{
	CString			cs_dat_file{};
	CString			cs_spk_file{};
	CString			cs_path{};
	int				ilastbackslashposition = -1;
	boolean			b_dat_present = false;
	boolean			b_spik_present = false;
	CWaveFormat*	p_wave_format = nullptr;
};

class CdbWaveDoc : public COleDocument
{
protected: // create from serialization only
	CdbWaveDoc();
	DECLARE_DYNCREATE(CdbWaveDoc)

	// Attributes
public:
	CAcqDataDoc*	m_pDat;
	CSpikeDoc*		m_pSpk;
	HMENU			m_hMyMenu;
	CIntervalsAndLevels m_stimsaved;

	// database
	CdbWdatabase*	m_pDB;
	BOOL			m_validTables;
	CDWordArray		m_selectedRecords;
	CString			m_dbFilename;
	CString			m_ProposedDataPathName;

protected:
	CString			m_currentDatafileName;
	CString			m_currentSpikefileName;

	BOOL			m_bcallnew;
	CStringArray	m_csfiles_to_delete;
	BOOL			m_bClearMdbOnExit;
	BOOL			m_bTranspose;

	// Operations
public:
	void			ImportFileList(CStringArray& filename, BOOL bOnlygenuine = FALSE);
	BOOL			IsExtensionRecognizedAsDataFile(CString string) const;
	inline BOOL		IsFilePresent(CString csFilename) { CFileStatus status; return CFile::GetStatus(csFilename, status); }
	CWaveFormat*	GetWaveFormat(CString filename, BOOL bIsDatFile);
	BOOL			ExtractFilenamesFromDatabase(LPCSTR filename, CStringArray& filenames);
	BOOL			ImportDatabase(CString& filename);
	BOOL			OnNewDocument(LPCTSTR lpszPathName);
	CAcqDataDoc*	OpenCurrentDataFile();
	CAcqDataDoc*	GetcurrentDataDocument() const { return m_pDat; }
	void			CloseCurrentDataFile();
	CSpikeDoc*		OpenCurrentSpikeFile();
	CSpikeDoc*		GetcurrentSpkDocument() const { return m_pSpk; }

	void			RemoveDuplicateFiles();
	void			RemoveMissingFiles();
	void			RemoveFalseSpkFiles();
	void			DeleteErasedFiles();

	long			GetDB_nbspikes();				// assuming only 1 spike list / spike file (which might be wrong)
	void			SetDB_nbspikes(long nspikes);	// assuming only 1 spike list / file
	long			GetDB_nbspikeclasses();
	void			SetDB_nbspikeclasses(long nclasses);
	void			GetAllSpkMaxMin(BOOL bAll, BOOL bRecalc, int* max, int* min);
	CSize			GetSpkMaxMin_y1(BOOL bAll);

	inline void		SetClearMdbOnExit(BOOL bClear) { m_bClearMdbOnExit = bClear; }

	void			ExportDataAsciiComments(CSharedFile* pSF); //, OPTIONS_VIEWDATA* pVD);
	void			ExportNumberofSpikes(CSharedFile* pSF);
	CString			ExportDatabaseData(int ioption = 0);
	void			ExportSpkDescriptors(CSharedFile* pSF, CSpikeList* p_spike_list, int kclass);
	void			SynchronizeSourceInfos(BOOL bAll);
	BOOL			UpdateWaveFmtFromDatabase(CWaveFormat* p_wave_format) const;

	void			ExportDatafilesAsTXTfiles();
	BOOL			CopyAllFilesintoDirectory(const CString& path);
	bool			BinaryFileCopy(LPCTSTR pszSource, LPCTSTR pszDest);
	BOOL			CreateDirectories(CString path);

protected:
	BOOL			transposeFileForExcel(CSharedFile* pSF);
	int				checkFilesCanbeOpened(CStringArray& filenames, CSharedFile* psf);
	sourceData*		getWaveFormatFromEitherFile(CString cs_filename);
	void			setRecordFileNames(sourceData* record);
	boolean			setRecordSpkClasses(sourceData* record);
	void			setRecordWaveFormat(sourceData* record);

	// Overrides
public:
	void			Serialize(CArchive& ar) override;

	virtual BOOL	OnNewDocument();
	virtual BOOL	OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL	OnSaveDocument(LPCTSTR lpszPathName);
	virtual HMENU	GetDefaultMenu(); // get menu depending on state
	void			UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint);

	// Implementation
public:
	virtual			~CdbWaveDoc();
#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif

	// DAO database functions
public:
	inline long		GetDB_NRecords() { return m_pDB->GetNRecords(); }
	inline CDaoRecordset* GetDB_Recordset() { return &m_pDB->m_mainTableSet; }
	inline void		DBRefreshQuery() { if (m_pDB->m_mainTableSet.IsBOF()) m_pDB->m_mainTableSet.SetFieldNull(nullptr); m_pDB->m_mainTableSet.RefreshQuery(); }
	void			DBDeleteCurrentRecord();
	CString			GetDB_CurrentDatFileName(const BOOL bTest = FALSE);
	CString			GetDB_CurrentSpkFileName(const BOOL bTest = FALSE);
	CString			SetDB_CurrentSpikeFileName();
	inline void		SetDB_DataLen(long len) { m_pDB->SetDataLen(len); }
	long			GetDB_DataLen();
	void			SetDB_CurrentRecordFlag(int flag);
	inline int		GetDB_CurrentRecordFlag() { return m_pDB->m_mainTableSet.m_flag; }
	void			SetDB_PathsRelative_to_DataBaseFile();
	void			SetDB_PathsAbsolute();
	void			DBTransferDatPathToSpkPath();
	void			DBDeleteUnusedEntries();

	long			GetDB_CurrentRecordPosition();
	long			GetDB_CurrentRecordID();
	inline BOOL		SetDB_CurrentRecordPosition(long ifile) { return m_pDB->SetIndexCurrentFile(ifile); }
	inline BOOL		DBMoveToID(long recordID) { return m_pDB->MoveToID(recordID); }
	inline BOOL		DBMoveFirst() { return m_pDB->MoveRecord(ID_RECORD_FIRST); }
	inline BOOL		DBMoveNext() { return m_pDB->MoveRecord(ID_RECORD_NEXT); }
	inline BOOL		DBMovePrev() { return m_pDB->MoveRecord(ID_RECORD_PREV); }
	inline BOOL		DBMoveLast() { return m_pDB->MoveRecord(ID_RECORD_LAST); }

protected:
	BOOL			OpenDatabase(LPCTSTR lpszPathName);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
