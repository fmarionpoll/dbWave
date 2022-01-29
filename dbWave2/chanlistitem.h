#pragma once

class CChanlistItem : public CObject
{
public:
	CChanlistItem();
	CChanlistItem(CEnvelope* pX, int iX, CEnvelope* pY, int iY);
	DECLARE_SERIAL(CChanlistItem)
	void Serialize(CArchive& ar) override;

	void InitDisplayParms(WORD new_penwidth, WORD new_color, int new_zero, int new_yextent, WORD new_drawmode = 1);
public:
	CEnvelope* pEnvelopeAbcissa = nullptr; // pointer to abcissa Envelope
	CEnvelope* pEnvelopeOrdinates = nullptr; // pointer to ordinates	Envelope
	CString dl_comment{}; // comment (40 chars initially)
	void GetEnvelopeArrayIndexes(int& x, int& y) const;
	void SetEnvelopeArrays(CEnvelope* px, int x, CEnvelope* py, int y);

protected:
	int dl_yzero{}; // data bin value for zero volts
	int dl_yextent{}; // max to min extent
	WORD dl_penwidth = 0; // pen size
	WORD dl_color{}; // color
	WORD dl_bprint{}; // draw mode

	float dl_datavoltspbin{}; // scale factor (1 unit (0-4095) -> y volts)
	float dl_datavoltspan{}; // amplitude of data displayed
	long dl_databinzero{}; // value of zero volts
	long dl_databinspan{}; // nb of bins encoding values within envelope

	BOOL dl_bHZtagsPrint{}; // print HZ tags flag
	int dl_indexabcissa{};
	int dl_indexordinates{};

	// helpers
public:
	int GetYzero() const { return dl_yzero; }
	int GetYextent() const { return dl_yextent; }
	WORD GetPenWidth() const { return dl_penwidth; }
	WORD GetColor() const { return dl_color; }
	WORD GetflagPrintVisible() const { return dl_bprint; }
	float GetVoltsperDataBin() const { return dl_datavoltspbin; }
	BOOL GetHZtagsPrintFlag() const { return dl_bHZtagsPrint; }
	void GetMaxMin(int* pmax, int* pmin) { pEnvelopeOrdinates->GetEnvelopeMaxMin(pmax, pmin); }
	int GetBinAt(int index) { return pEnvelopeOrdinates->GetPointAt(index); }
	CString GetComment() { return dl_comment; }
	int GetSourceChan() { return pEnvelopeOrdinates->GetSourceChan(); }
	int GetTransformMode() { return pEnvelopeOrdinates->GetSourceMode(); }

	int GetDataBinZero() const { return dl_databinzero; }
	int GetDataBinSpan() const { return dl_databinspan; }
	float GetDataVoltsSpan() const { return dl_datavoltspan; }

	void SetDataBinFormat(long binzero, long binspan)
	{
		dl_databinzero = binzero;
		dl_databinspan = binspan;
	}

	void SetDataVoltsFormat(float VperBin, float VoltsSpan)
	{
		dl_datavoltspbin = VperBin;
		dl_datavoltspan = VoltsSpan;
	}

	void SetYzero(int zero) { dl_yzero = zero; }
	void SetYextent(int yextent) { dl_yextent = yextent; }
	void SetPenWidth(WORD penwidth) { dl_penwidth = penwidth; }
	void SetColor(WORD color) { dl_color = color; }
	void SetflagPrintVisible(WORD drawmode) { dl_bprint = drawmode; }
	void SetHZtagsPrintFlag(BOOL bPrint) { dl_bHZtagsPrint = bPrint; }
	void SetOrdinatesSourceData(int chan, int transform) { pEnvelopeOrdinates->SetSourceData(chan, transform); }

	float ConvertDataBinsToVolts(int bins) const { return static_cast<float>(bins) * dl_datavoltspbin; }
	int ConvertVoltsToDataBins(float volts) const { return static_cast<int>(volts / dl_datavoltspbin); }
};
