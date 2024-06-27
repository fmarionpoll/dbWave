#pragma once
class db_spike
{
public:
	int database_position = -1;
	int spike_list_index = -1;
	int spike_index = -1;

	db_spike()
	{
		database_position = -1;
		spike_list_index = -1;
		spike_index = -1;
	}

	db_spike(const int index_spk_file, const int index_spk_list, const int index_spike)
	{
		database_position = index_spk_file;
		spike_list_index = index_spk_list;
		spike_index = index_spike;
	}

	void clear()
	{
		database_position = -1;
		spike_list_index = -1;
		spike_index = -1;
	}

};



