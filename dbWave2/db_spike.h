#pragma once
class db_spike
{
public:
	long record_id = -1;
	int spike_list_index = -1;
	int spike_index = -1;

	db_spike()
	{
		record_id = -1;
		spike_list_index = -1;
		spike_index = -1;
	}

	db_spike(const long current_record_id, const int index_spk_list, const int index_spike)
	{
		record_id = current_record_id;
		spike_list_index = index_spk_list;
		spike_index = index_spike;
	}

	void clear()
	{
		record_id = -1;
		spike_list_index = -1;
		spike_index = -1;
	}

};



