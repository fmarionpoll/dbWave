#pragma once
#include "dbTableColumnDescriptor.h"

constexpr auto COL_ID = 0;					// 0
constexpr auto COL_FILENAME = 1;			// 2
constexpr auto COL_FILESPK = 2;				// 3
constexpr auto COL_ACQDATE = 3;				// 1
constexpr auto COL_ACQ_COMMENTS = 4;		// 4
constexpr auto COL_IDINSECT = 5;			// 6
constexpr auto COL_IDSENSILLUM = 6;			// 7
constexpr auto COL_MORE = 7;				// 5
constexpr auto COL_INSECT_ID = 8;			// 12
constexpr auto COL_LOCATION_ID = 9;			// 17
constexpr auto COL_OPERATOR_ID = 10;		// 14
constexpr auto COL_SENSILLUM_ID = 11;		// 13
constexpr auto COL_PATH_ID = 12;			// 18
constexpr auto COL_PATH2_ID = 13;			// 19
constexpr auto COL_DATALEN = 14;			// 8
constexpr auto COL_NSPIKES = 15;			// 9
constexpr auto COL_NSPIKECLASSES = 16;		// 10
constexpr auto COL_STIM_ID = 17;			// 15
constexpr auto COL_CONC_ID = 18;			// 16
constexpr auto COL_STIM2_ID = 19;			// 20
constexpr auto COL_CONC2_ID = 20;			// 21
constexpr auto COL_FLAG = 21;				// 11
constexpr auto COL_STRAIN_ID = 22;			// 22
constexpr auto COL_SEX_ID = 23;				// 23
constexpr auto COL_REPEAT = 24;				// 24
constexpr auto COL_REPEAT2 = 25;			// 25
constexpr auto COL_ACQDATE_DAY = 26;		// 26
constexpr auto COL_ACQDATE_TIME = 27;		// 27
constexpr auto COL_EXPT_ID = 28;			// 28

constexpr auto CH_ID = 0;
constexpr auto CH_ACQDATE = 1;
constexpr auto CH_FILENAME = 2;
constexpr auto CH_FILESPK = 3;
constexpr auto CH_ACQ_COMMENTS = 4;

constexpr auto CH_MORE = 5;
constexpr auto CH_IDINSECT = 6;
constexpr auto CH_IDSENSILLUM = 7;
constexpr auto CH_DATALEN = 8;
constexpr auto CH_NSPIKES = 9;

constexpr auto CH_NSPIKECLASSES = 10;
constexpr auto CH_FLAG = 11;
constexpr auto CH_INSECT_ID = 12;
constexpr auto CH_SENSILLUM_ID = 13;
constexpr auto CH_OPERATOR_ID = 14;

constexpr auto CH_STIM_ID = 15;
constexpr auto CH_CONC_ID = 16;
constexpr auto CH_LOCATION_ID = 17;
constexpr auto CH_PATH_ID = 18;
constexpr auto CH_PATH2_ID = 19;

constexpr auto CH_STIM2_ID = 20;
constexpr auto CH_CONC2_ID = 21;
constexpr auto CH_STRAIN_ID = 22;
constexpr auto CH_SEX_ID = 23;
constexpr auto CH_REPEAT = 24;

constexpr auto CH_REPEAT2 = 25;
constexpr auto CH_ACQDATE_DAY = 26;
constexpr auto CH_ACQDATE_TIME = 27;
constexpr auto CH_EXPT_ID = 28;
constexpr int NCOLUMNS = 30;

#pragma warning(disable : 4995)

class CdbTableMain : public CDaoRecordset
{
public:
	CdbTableMain(CDaoDatabase* p_database = nullptr);
	DECLARE_DYNAMIC(CdbTableMain)

	// Field/Param Data - these parameters are equivalent to column headers in a table, where each row is a record

	long m_id  {0};					// 1
	CString m_file_dat  {_T("")};		// 2
	CString m_file_spk  {_T("")};		// 3
	COleDateTime m_table_acq_date  {static_cast<DATE>(0)}; // 4
	CString m_acq_comment  {_T("")}; // 5
	long m_id_insect  {0};			// 6
	long m_id_sensillum  {0};		// 7
	CString m_more  {_T("")};	// 8
	long m_insect_id  {0};			// 9
	long m_location_id  {0};		// 10
	long m_operator_id  {0};		// 11
	long m_sensillum_id  {0};		// 12
	long m_path_id {0};				// 13
	long m_path2_id  {0};			// 14 unused 
	long m_data_len  {0};			// 15
	long m_n_spikes  {0};			// 16
	long m_n_spike_classes  {0};	// 17
	long m_stimulus_id  {0};		// 18
	long m_concentration_id  {0};	// 19
	long m_stimulus2_id  {0};		// 20
	long m_concentration2_id  {0};	// 21
	long m_flag  {0};				// 22
	long m_strain_id  {0};			// 23
	long m_sex_id  {0};				// 24
	long m_repeat  {0};				// 25
	long m_repeat2  {0};			// 26
	COleDateTime m_acq_date_day  {static_cast<DATE>(0)}; // 27
	COleDateTime m_acq_date_time  {static_cast<DATE>(0)}; // 28 xxxxxxx suppress this parameter, leave dummy ?
	long m_experiment_id  {0};		// 29

	// filter parameters
	BOOL m_b_filter_on  {false};
	DB_ITEMDESC m_desc[NCOLUMNS];
	CString m_cs_default_sql  {_T("[table]")};

	// temp values
	long max_insect_id  { 0};
	long max_sensillum_id  {0};
	long max_id  {0};
	CString m_default_name  {_T("")};

	void build_and_sort_id_arrays();
	void copy_path_to_path2();
	void add_current_record_to_id_arrays();
	boolean open_table(int n_open_type = AFX_DAO_USE_DEFAULT_TYPE, LPCTSTR lpsz_sql = nullptr, int n_options = 0);

protected:
	void add_to_li_array(int i_col);
	static void add_to_id_array(CUIntArray* p_ui_id_array, long i_id);
	void add_day_to_date_array(const COleDateTime& o_time);
	void delete_date_array();

	// Overrides
public:
	CString GetDefaultDBName() override;
	CString GetDefaultSQL() override; 
	void DoFieldExchange(CDaoFieldExchange* p_fx) override; 

	// Implementation
	~CdbTableMain() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
	long get_records_count();
	BOOL set_long_value(long i_id, const CString& cs_col_name);
	BOOL set_value_null(const CString& cs_col_name);
	void get_acq_date_array(CPtrArray* p_acq_date);
	BOOL check_if_acq_date_time_is_unique(const COleDateTime* p_time);
	void get_max_id();
	BOOL find_id_in_column(long i_id, int i_column);
	int	 get_column_index(const CString& cs_name);

	void build_filters();
	void clear_filters();
	void refresh_query();
	void set_data_len(long data_len);

	void set_filter_single_item(const DB_ITEMDESC* p_desc)
	{
		m_desc[p_desc->index].b_single_filter = p_desc->b_single_filter;
		m_desc[p_desc->index].l_param_single_filter = p_desc->l_param_single_filter;
	}
};
