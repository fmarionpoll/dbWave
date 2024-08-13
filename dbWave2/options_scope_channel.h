#pragma once

class options_scope_channel final : public CObject
{
	DECLARE_SERIAL(options_scope_channel)
	options_scope_channel();
	~options_scope_channel() override;
	options_scope_channel& operator =(const options_scope_channel& arg);
	void Serialize(CArchive& ar) override;

	int version{1};
	int i_extent{2048};
	int i_zero{0};
};
