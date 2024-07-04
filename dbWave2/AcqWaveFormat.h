#pragma once


// CWaveFormat
// data acquisition definitions
// comments, sampling rate, etc
// these parameters are stored in the header of data acquisition files
// stored either under Asyst or Atlab
// Separate routines decode the headers from such files and store the
// parameters in CWaveFormat structure

class CWaveFormat final : public CObject
{
	DECLARE_SERIAL(CWaveFormat);

public:
	WORD w_version = 8;							// header version number

	CTime acquisition_time;
	float full_scale_volts { 20.0f};			// volts full scale, gain 1
	long bin_span { 4096};						// 2^12, 2^16
	long bin_zero { 2048};						// 2048, 32768 (0 if 2's complement)

	short mode_encoding { 0};					// 0{offset binary/1{straight/2{2s complement
	short mode_clock { 0};						// 0{internal, 1{external
	short mode_trigger { 0};					// 0{internal, 1{external, 2{trigg_scan
	short scan_count { 1};						// number of channels in scan list

	float sampling_rate_per_channel { 10000.};	// channel sampling rate (Hz)
	long sample_count { 1000};					// sample count
	float duration { 0.};						// file duration (s)

	short trig_mode { 0};						// 0{soft, 1{keyboard, 2{trig_ext(TTL), 3{input+, 4{input-
	short trig_chan { 0};						// (trig_mode{input) A/D channel input
	short trig_threshold { 2048};				// (trig_mode{input) A/D channel input threshold value
	short data_flow { 0};						// 0 { continuous, 1 { burst
	short buffer_n_items { 10};					// n buffers declared
	WORD buffer_size { 1000};					// total size of one buffer (nb points [short])
	BOOL b_online_display { true};				// TRUE { online data display, FALSE{no display during acquisition
	BOOL b_ad_write_to_file { false};			// TRUE { write data to disk,  FALSE{no saving
	BOOL b_unicode { true};						// TRUE if unicode

	// comments
	CString cs_ad_card_name { _T("undefined")};	// 1 I/O card name
	CString cs_comment { _T("")};			// x 2 file annotation - used now as "cs_experiment"
	CString cs_stimulus { _T("")};		// x 3 stimulation (chemical)
	CString cs_concentration { _T("")};	// x 4 stimulus concentration
	CString cs_insect_name { _T("")};		// x 5 sensillum type
	CString cs_location { _T("")};		// x 6 location
	CString cs_sensillum { _T("")};		// x 7 sensillum nb
	CString cs_strain { _T("")};			// x 8 name of the strain
	CString cs_operator { _T("")};		// x 9 name of the operator
	CString cs_more_comment { _T("")};	// x 10 free comment space
	CString cs_stimulus2 { _T("")};		// x 11 stimulus 2
	CString cs_concentration2 { _T("")};	// x 12 concentration of stimulus 2
	CString cs_sex { _T("")};				// x 13 sex

	long insect_id { 0};						// 14
	long sensillum_id { 0};						// 15
	long repeat { 0};							// 16
	long repeat2 { 0};							// 17
	int flag { 0};								// 18

	//------------------- Total : 32 variable members
protected:
	static CString add_comments(const CString& p_separator, BOOL b_explanation, const CString& cs_explanation,
	                           const CString& cs_comment);
	void read_v8_and_before(CArchive& ar, WORD version);

public:
	CWaveFormat();
	~CWaveFormat() override;

	void copy(const CWaveFormat* arg);
	long write(CFile* datafile);
	BOOL read(CFile* datafile);
	void Serialize(CArchive& ar) override;
	CString get_comments(const CString& p_separator, BOOL b_explanations = FALSE) const;
	long get_nb_points_sampled_per_channel() const {return sample_count / static_cast<long>(scan_count);}
};
