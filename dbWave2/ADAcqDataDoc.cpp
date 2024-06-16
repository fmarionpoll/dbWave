#include "stdafx.h"
#include "ADAcqDataDoc.h"

#include "datafile_Awave.h"

BOOL ADAcqDataDoc::AcqDoc_DataAppendStop() const
{
	m_pXFile->DataAppendStop();
	const auto p_wave_format = get_waveformat();
	p_wave_format->sample_count = static_cast<long>(m_pXFile->m_ulbytescount / sizeof(short));
	p_wave_format->duration = static_cast<float>(p_wave_format->sample_count) / static_cast<float>(p_wave_format->
		scan_count)
		/ p_wave_format->sampling_rate_per_channel;
	m_pXFile->WriteDataInfos(p_wave_format, get_wavechan_array());
	m_pXFile->Flush();
	return TRUE;
}

BOOL ADAcqDataDoc::AcqDoc_DataAppend(short* p_buffer, const UINT bytes_length) const
{
	return m_pXFile->DataAppend(p_buffer, bytes_length);
}

BOOL ADAcqDataDoc::AcqDoc_DataAppendStart()
{
	// start from fresh?
	if (m_pXFile == nullptr || m_pXFile->m_idType != DOCTYPE_AWAVE)
	{
		delete m_pXFile;
		m_pXFile = new CDataFileAWAVE;
		ASSERT(m_pXFile != NULL);
	}
	return m_pXFile->DataAppendStart();
}


