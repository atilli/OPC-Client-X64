#pragma once

#include "OPCClientToolKitDLL.h"
#include "opcda.h"
#include <string>

class COPCItem;


/**
*Wrapper for OPC data. Hides COM memory management.
*/
struct  OPCItemData{
	FILETIME ftTimeStamp;
	WORD wQuality;
	VARIANT vDataValue;
	HRESULT error;

	OPCItemData(HRESULT err);
	OPCItemData(FILETIME time, WORD qual, VARIANT & val, HRESULT err);
	OPCItemData();

	~OPCItemData();

	void set(OPCITEMSTATE &itemState);
	void set(FILETIME time, WORD qual, VARIANT & val);

	OPCItemData & operator=(OPCItemData &itemData);
	std::string ToString();
	std::string QualityString();
	std::string ConvertToString();
};