#pragma once

// dbWaveDoc.h : interface of the CdbWaveDoc class
//
/////////////////////////////////////////////////////////////////////////////

#include "dbIndexTable.h"
#include "dbMainTable.h"
#include "dbWdatabase.h"
#include "spikepar.h"		// spike parameters options
#include "spikelist.h"		// list of spikes
#include "spikeclas.h"		// spike classes
#include "Spikedoc.h"
#include "acqdatad.h"


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
	CStimLevelSeries m_stimsaved;
	
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
	int				m_icurSpkList;
	BOOL			m_bClearMdbOnExit;
	BOOL			m_bTranspose;

// Operations
public:
	void			ImportDescFromFileList(CStringArray& filename, BOOL bOnlygenuine=FALSE);
	BOOL			IsDatFile(CString csFilename);
	inline BOOL		IsFilePresent(CString csFilename) {CFileStatus rStatus; return CFile::GetStatus(csFilename, rStatus);}
	CWaveFormat*	GetWaveFormat(CString filename, BOOL bIsDatFile);
	BOOL			ExtractFilenamesFromDatabase(LPCSTR filename, CStringArray& filenames);
	BOOL			ImportDatabase (CString &filename);
	BOOL			OnNewDocument(LPCTSTR lpszPathName);
	BOOL			OpenCurrentDataFile();
	BOOL			OpenCurrentSpikeFile();
	void			RemoveDuplicateFiles();
	void			RemoveMissingFiles();
	void			RemoveFalseSpkFiles();
	void			DeleteErasedFiles();

	long			Getnbspikes();				// assuming only 1 spike list / spike file (which might be wrong)
	void			Setnbspikes(long nspikes);	// assuming only 1 spike list / file
	long			Getnbspikeclasses();
	void			Setnbspikeclasses(long nclasses);
	void			GetAllSpkMaxMin(BOOL bAll, BOOL bRecalc, int* max, int* min);
	inline int		GetcurrentSpkListIndex() const {return m_icurSpkList;}
	inline void		SetcurrentSpkListIndex(int icur) {m_icurSpkList = icur;}
	inline int		GetcurrentSpkListSize() const {return m_pSpk->GetSpkListSize();}

	inline void		SetClearMdbOnExit(BOOL bClear) {m_bClearMdbOnExit = bClear;}
	
	void			ExportDataAsciiComments(CSharedFile* pSF); //, OPTIONS_VIEWDATA* pVD);
	void			ExportNumberofSpikes(CSharedFile* pSF);
	CString			ExportDatabaseData(int ioption = 0);
	void			ExportSpkDescriptors(CSharedFile* pSF, CSpikeList* pSL, int kclass);
	void			SynchronizeSourceInfos(BOOL bAll);
	BOOL			UpdateWaveFmtFromDatabase (CWaveFormat* pWF);

	void			ExportDatafilesAsTXTfiles	();
	BOOL			CopyAllFilesintoDirectory	(CString path);
	bool			BinaryFileCopy				(LPCTSTR pszSource, LPCTSTR pszDest);
	BOOL			CreateDirectories			(CString path);

protected:
	BOOL			TransposeFile(CSharedFile* pSF);
	int				CheckifFilesCanbeOpened (CStringArray& filenames, CSharedFile* psf);

// Overrides
public:
	virtual BOOL	OnNewDocument();
	virtual void	Serialize(CArchive& ar);
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
	inline long DBGetNRecords()						{ return m_pDB->GetNRecords(); }
	inline CDaoRecordset* DBGetRecordset()			{ return &m_pDB->m_tableSet; }
	inline void DBRefreshQuery()					{ if (m_pDB->m_tableSet.IsBOF()) m_pDB->m_tableSet.SetFieldNull(NULL); m_pDB->m_tableSet.RefreshQuery();}
	void		DBDeleteCurrentRecord();
	CString		DBGetCurrentDatFileName(BOOL bTest = FALSE);
	CString		DBGetCurrentSpkFileName(BOOL bTest = FALSE);
	CString		DBDefineCurrentSpikeFileName();
	inline void DBSetDataLen(long len) { m_pDB->SetDataLen(len); }
	long		DBGetDataLen();
	void		DBSetCurrentRecordFlag(int flag);
	inline int	DBGetCurrentRecordFlag() { return m_pDB->m_tableSet.m_flag; }
	void		DBSetPathsRelative_to_DataBaseFile();
	void		DBSetPathsAbsolute();
	void		DBTransferDatPathToSpkPath();
	void		DBDeleteUnusedEntries();

	long		DBGetCurrentRecordPosition();
	long		DBGetCurrentRecordID();
	inline BOOL	DBSetCurrentRecordPosition(long ifile) { return m_pDB->SetIndexCurrentFile(ifile); }
	inline BOOL	DBMoveToID(long recordID)			{ return m_pDB->MoveToID(recordID); }
	inline BOOL DBMoveFirst()						{ return m_pDB->MoveRecord(ID_RECORD_FIRST); }
	inline BOOL DBMoveNext()						{ return m_pDB->MoveRecord(ID_RECORD_NEXT); }
	inline BOOL DBMovePrev()						{ return m_pDB->MoveRecord(ID_RECORD_PREV); }
	inline BOOL DBMoveLast()						{ return m_pDB->MoveRecord(ID_RECORD_LAST); }

protected:
	BOOL		OpenDatabase (LPCTSTR lpszPathName);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

};

