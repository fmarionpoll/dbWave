// chanlistitem.h
/////////////////////////////////////////////////////////////////////////////
//	CChanlistItem
//
// lineview keeps a list of channels to be displayed in a list
// each item of this list group together the parameters necessary to
// display the channel, ie color, etc and a pointer to structures
// describing the abcissa and the ordinates
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CChanlistItem : public CObject
{
public:
	CChanlistItem();
	CChanlistItem(CEnvelope* pX, int iX, CEnvelope* pY, int iY);
	DECLARE_SERIAL(CChanlistItem)
	virtual void	Serialize (CArchive &ar);
	void			InitDisplayParms(WORD new_penwidth, WORD new_color, int new_zero, 
							int new_yextent, WORD new_drawmode=1);
public:
	CEnvelope*		pAbcissa;		// pointer to abcissa Envelope
	CEnvelope*		pOrdinates;		// pointer to ordinates	Envelope
	CString			dl_comment;		// comment (40 chars initially)
	void			GetEnvelopeArrayIndexes(int& x, int& y);
	void			SetEnvelopeArrays(CEnvelope* px, int x, CEnvelope* py, int y);

protected:	
	int				dl_zero;		// zero volts
	int				dl_yextent;		// max to min extent
	WORD			dl_penwidth;	// pen size
	WORD			dl_color;		// color
	WORD			dl_bprint;		// draw mode 
	float			dl_voltsperbin;	// scale factor (1 unit (0-4095) -> y volts)
	float			dl_voltsextent;	// amplitude of data displayed
	long			dl_binzero;		// value of zero volts
	long			dl_binspan;		// nb of bins encoding values within envelope
	BOOL			dl_bHZtagsPrint;// print HZ tags flag
	int				dl_indexabcissa;
	int				dl_indexordinates;

// helpers	
public:   
	inline int		GetZero()							const {return dl_zero;}
	inline int		GetExtent()							const {return dl_yextent;}
	inline WORD		GetPenWidth()						const {return dl_penwidth;}
	inline WORD		GetColor()							const {return dl_color;}
	inline WORD		GetflagPrintVisible()				const {return dl_bprint;}
	inline float	GetVoltsperBin()					const {return dl_voltsperbin;}
	inline float	ConvertBintoVolts(int bins)			const {return ((float)(bins))*dl_voltsperbin;}
	inline int		ConvertVoltstoBins(float nvolts)	const {return (int) (nvolts/dl_voltsperbin);}
	inline BOOL		GetHZtagsPrintFlag()				const {return dl_bHZtagsPrint;}
	inline int		GetBinZero()						const {return 0;}
	inline float	GetVoltsExtent()					const {return dl_voltsextent;}

	inline void		SetZero(int zero)					{dl_zero=zero;}
	inline void		SetExtent(int yextent)				{dl_yextent=yextent;}
	inline void		SetVoltsExtent(float yVoltsExtent)	{dl_voltsextent = yVoltsExtent;}
	inline void		SetPenWidth(WORD penwidth)			{dl_penwidth=penwidth;}
	inline void		SetColor(WORD color)				{dl_color=color;}
	inline void		SetflagPrintVisible(WORD drawmode)	{dl_bprint=drawmode;}
	inline void		SetBinFormat(float VoltsperBin, long binzero, long binspan)  {dl_voltsperbin=VoltsperBin;dl_binzero=binzero; dl_binspan=binspan;}
	inline void		SetHZtagsPrintFlag(BOOL bPrint)		{dl_bHZtagsPrint = bPrint;}	
	inline void		SetOrdinatesSourceData(int chan, int transform) {pOrdinates->SetSourceData(chan, transform);}
};
