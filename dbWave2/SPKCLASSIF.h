#pragma once

// spike classification parameters

class SPKCLASSIF : public CObject
{
	DECLARE_SERIAL(SPKCLASSIF)
public:
	BOOL bChanged; // flag set TRUE if contents has changed
	WORD wversion; // version number

	// sort with parameters
	int dataTransform; // dummy
	int iparameter; // type of parameter measured
	int ileft; // position of first cursor
	int iright; // position of second cursor
	int ilower; // binary value of lower
	int iupper; // binary value of upper
	int ixyleft; // position of left cursor on xy display
	int ixyright; // position of right cursor on xy display
	int sourceclass; // source class
	int destclass; // destination class

	// display parameters for spikeview
	int rowheight;
	int coltext;
	int colspikes;
	int colseparator;
	int vsourceclass;
	int vdestclass;
	float fjitter_ms;
	BOOL bresetzoom;

	// sort with templates
	int nintparms;
	int hitrate;
	int hitratesort;
	int kleft;
	int kright;

	int nfloatparms;
	float ktolerance;
	float mvmin; // display limits
	float mvmax;

	void* ptpl;

public:
	SPKCLASSIF();
	~SPKCLASSIF() override;
	SPKCLASSIF& operator =(const SPKCLASSIF& arg); // operator redefinition
	void Serialize(CArchive& ar) override;
	void CreateTPL();
};
