#pragma once

class SCOPECHANNEL : public CObject
{
	DECLARE_SERIAL(SCOPECHANNEL)
	SCOPECHANNEL();
	~SCOPECHANNEL() override;
	SCOPECHANNEL& operator = (const SCOPECHANNEL& arg);
	void Serialize(CArchive& ar) override;

	int version{ 1 };
	int iextent {2048};
	int izero {0};
};
