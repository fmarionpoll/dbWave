#pragma once

class CChanlistItem : public CObject
{
public:
	CChanlistItem();
	CChanlistItem(CEnvelope* pX, int iX, CEnvelope* pY, int iY);
	DECLARE_SERIAL(CChanlistItem)
	void Serialize(CArchive& ar) override;

	void			InitDisplayParms(WORD new_penwidth, WORD new_color, int new_zero, int new_yextent, WORD new_drawmode=1);
public:
	CEnvelope*		pEnvelopeAbcissa;		// pointer to abcissa Envelope
	CEnvelope*		pEnvelopeOrdinates;		// pointer to ordinates	Envelope
	CString			dl_comment;				// comment (40 chars initially)
	void			GetEnvelopeArrayIndexes(int& x, int& y) const;
	void			SetEnvelopeArrays(CEnvelope* px, int x, CEnvelope* py, int y);

protected:	
	int				dl_yzero{};				// data bin value for zero volts
	int				dl_yextent{};				// max to min extent
	WORD			dl_penwidth = 0;			// pen size
	WORD			dl_color{};				// color
	WORD			dl_bprint{};				// draw mode 

	float			dl_datavoltspbin{};		// scale factor (1 unit (0-4095) -> y volts)
	float			dl_datavoltspan{};		// amplitude of data displayed
	long			dl_databinzero{};			// value of zero volts
	long			dl_databinspan{};			// nb of bins encoding values within envelope

	BOOL			dl_bHZtagsPrint{};		// print HZ tags flag
	int				dl_indexabcissa;
	int				dl_indexordinates;

// helpers	
public:   
	inline int		GetYzero()							const {return dl_yzero;}
	inline int		GetYextent()						const {return dl_yextent;}
	inline WORD		GetPenWidth()						const {return dl_penwidth;}
	inline WORD		GetColor()							const {return dl_color;}
	inline WORD		GetflagPrintVisible()				const {return dl_bprint;}
	inline float	GetVoltsperDataBin()				const {return dl_datavoltspbin;}
	inline float	ConvertDataBinsToVolts(int bins)	const {return ((float)(bins))*dl_datavoltspbin;}
	inline int		ConvertVoltsToDataBins(float volts)	const {return (int) (volts/dl_datavoltspbin);}
	inline BOOL		GetHZtagsPrintFlag()				const {return dl_bHZtagsPrint;}

	inline int		GetDataBinZero()					const {return dl_databinzero;}
	inline int		GetDataBinSpan()					const {return dl_databinspan;}
	inline float	GetDataVoltsSpan()					const {return dl_datavoltspan;}
	inline void		SetDataBinFormat(long binzero, long binspan) { dl_databinzero = binzero; dl_databinspan = binspan; }
	inline void		SetDataVoltsFormat(float VperBin, float VoltsSpan) { dl_datavoltspbin = VperBin; dl_datavoltspan = VoltsSpan; }

	inline void		SetYzero(int zero)					{dl_yzero=zero;}
	inline void		SetYextent(int yextent)				{dl_yextent=yextent;}
	inline void		SetPenWidth(WORD penwidth)			{dl_penwidth=penwidth;}
	inline void		SetColor(WORD color)				{dl_color=color;}
	inline void		SetflagPrintVisible(WORD drawmode)	{dl_bprint=drawmode;}

	inline void		SetHZtagsPrintFlag(BOOL bPrint)						{dl_bHZtagsPrint = bPrint;}	
	inline void		SetOrdinatesSourceData(int chan, int transform)		{pEnvelopeOrdinates->SetSourceData(chan, transform);}
};
