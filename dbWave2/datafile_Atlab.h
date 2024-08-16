#pragma once
#include "AcqWaveChanArray.h"
#include "AcqWaveFormat.h"
#include "datafile_X.h"
#include "WaveBuf.h"


class CDataFileATLAB final : public CDataFileX
{
public:
	CDataFileATLAB();
	DECLARE_DYNCREATE(CDataFileATLAB)

	BOOL read_data_infos(CWaveBuf* p_buf) override;
	int check_file_type(CString& cs_filename) override;
protected:
	void init_dummy_channels_info(int chan_list_index) const;
	void load_channel_from_cyber(int channel, char* p_cyber_chan) const;
	void init_channels_from_cyber_a320(char* p_header) const;
	static CString get_cyber_a320_filter(int n_code);

	CWaveFormat* m_p_w_format_{nullptr};
	CWaveChanArray* m_p_array_{nullptr};

public:
	~CDataFileATLAB() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
