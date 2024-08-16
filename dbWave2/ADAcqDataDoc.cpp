#include "stdafx.h"
#include "ADAcqDataDoc.h"

#include "datafile_Awave.h"

BOOL ADAcqDataDoc::AcqDoc_DataAppendStop() const
{
	x_file->data_append_stop();
	const auto p_wave_format = get_wave_format();
	p_wave_format->sample_count = static_cast<long>(x_file->m_ul_bytes_count / sizeof(short));
	p_wave_format->duration = static_cast<float>(p_wave_format->sample_count) / static_cast<float>(p_wave_format->
		scan_count)
		/ p_wave_format->sampling_rate_per_channel;
	x_file->write_data_infos(p_wave_format, get_wave_channels_array());
	x_file->Flush();
	return TRUE;
}

BOOL ADAcqDataDoc::AcqDoc_DataAppend(short* p_buffer, const UINT bytes_length) const
{
	return x_file->data_append(p_buffer, bytes_length);
}

BOOL ADAcqDataDoc::AcqDoc_DataAppendStart()
{
	// start from fresh?
	if (x_file == nullptr || x_file->m_id_type != DOCTYPE_AWAVE)
	{
		delete x_file;
		x_file = new CDataFileAWAVE;
		ASSERT(x_file != NULL);
	}
	return x_file->data_append_start();
}


