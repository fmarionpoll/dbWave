#pragma once
#include "StdAfx.h"


//#import "C:\\Program Files (x86)\\Alligator Technologies\\SystemViewUSBPxx\\Distributable SDK Files\\win32\\USBPxxS1COM.dll" \
//		no_namespace raw_interfaces_only raw_native_types named_guids

enum	FIRMWARE_PARAMETER_ID_SET
{
	ID_INITIALIZE,
	ID_READ_LPFC,
	ID_READ_HPFC,
	ID_READ_GAIN,
	ID_READ_COUPLING,
	ID_READ_CLOCKSOURCE,
	ID_READ_PCLOCK,
	ID_READ_CHANNELNUMBER,
	ID_READ_DESCRIPTION,
	ID_READ_LPFILTERTYPE,
	ID_READ_HPFILTERTYPE,
	ID_READ_SERIALNUMBER,
	ID_READ_PRODUCTID,
	ID_READ_REVISION,
	ID_WRITE_LPFC,
	ID_WRITE_HPFC,
	ID_WRITE_GAIN,
	ID_WRITE_COUPLING,
	ID_WRITE_CLOCKSOURCE,
	ID_WRITE_PCLOCK,
	ID_WRITE_CHANNELNUMBER,
	ID_WRITE_DESCRIPTION,
	ID_WRITE_NVRAM,
	ID_WRITE_LPFILTERTYPE,
	ID_WRITE_HPFILTERTYPE,
	ID_WRITE_SERIALNUMBER,
	ID_WRITE_PRODUCTID
};

enum	DEVICE_COMMAND_ID_SET
{
	DCID_GET_TOTAL_CONNECTED_CHANNELS = 0x100,
	DCID_GET_CHANNEL_HANDLE
};

enum	ERROR_CODE_SET
{
	EC_SUCCESSFUL,
	EC_LP_FC_RANGE,
	EC_HP_FC_RANGE,
	EC_GAIN_RANGE
};

enum	LOW_PASS_FILTER_TYPE_SET
{
	LPFT_CE,
	LPFT_B,
	LPFT_L,
	LPFT_HC,
	LPFT_LP,
	LPFT_HLP
};

enum	HIGH_PASS_FILTER_TYPE_SET
{
	HPFT_NONE,
	HPFT_LN,
	HPFT_LE,
	HPFT_BN,
	HPFT_BE
};

enum	PRODUCT_ID_SET
{
	USBPGFS1,
	USBPIAS1,
	USBPBPS1,
	USBPHPS1
};

typedef	struct
{
	unsigned char	value;
	unsigned char	valueEx;
}
EXTENDED_CHAR;

typedef	struct
{
	unsigned int	index;
	EXTENDED_CHAR	ec;
}
CHARACTER_PACKET;

typedef	union
{
	struct
	{
		unsigned int	Low;
		unsigned int	High;
	}Rlong;
	long	Whole;
}
REVISION_TYPE;

typedef	union
{
	VARIANT	IVAR;
	long	LVAR;
	float	FVAR;
	CHARACTER_PACKET CVAR;
	EXTENDED_CHAR EVAR;
}
VARIANT_TYPE, *pVARIANT_TYPE;

#define	USBPxxS1_DESCRIPTION_SIZE_LIMIT	30






