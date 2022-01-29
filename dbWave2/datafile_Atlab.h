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

	BOOL ReadDataInfos(CWaveBuf* p_buf) override;
	int CheckFileType(CString& cs_filename) override;
protected:
	void init_dummy_chans_info(int chanlistindex) const;
	void load_channel_from_cyber(int channel, char* pcyberchan) const;
	void init_channels_from_cyber_a320(char* p_header) const;
	static CString get_cyber_a320_filter(int ncode);

	CWaveFormat* m_pWFormat{nullptr};
	CWaveChanArray* m_pArray{nullptr};

public:
	~CDataFileATLAB() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
