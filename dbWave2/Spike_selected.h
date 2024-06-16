#pragma once
class Spike_selected
{
public:
	int database_position = -1;
	int spike_list_index = -1;
	int spike_index = -1;

	Spike_selected()
	{
		database_position = -1;
		spike_list_index = -1;
		spike_index = -1;
	}

	Spike_selected(const int index_spk_file, const int index_spk_list, const int index_spike)
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

	Spike_selected& operator =(const Spike_selected& arg)
	{
		if (this != &arg)
		{
			database_position = arg.database_position;
			spike_list_index = arg.spike_list_index;
			spike_index = arg.spike_index;
		}
		return *this;
	}
};

