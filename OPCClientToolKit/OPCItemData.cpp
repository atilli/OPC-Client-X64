#include <atlstr.h>
#include <atlcoll.h>
#include "OPCItemData.h"
#include "OPCClient.h"


OPCItemData::OPCItemData(HRESULT err):error(err){
	vDataValue.vt = VT_EMPTY;
}



OPCItemData::OPCItemData(FILETIME time, WORD qual, VARIANT & val, HRESULT err){
	vDataValue.vt = VT_EMPTY;
	HRESULT result = VariantCopy( &vDataValue, &val);
	if (FAILED(result)){
		throw OPCException("VarCopy failed");
	}

	ftTimeStamp = time;
	wQuality = qual;
	error = err;
}
std::string OPCItemData::QualityString() {
	
	switch(wQuality)
	{
	case 192:
	case 216:
		return "GOOD";
		break;
	default:
		return "BAD";
		break;
	}
}

std::string OPCItemData::ToString() {
	
	std::string ret = "?";
	USES_CONVERSION;

	switch (vDataValue.vt)
	{
	case VT_BSTR:
		ret = W2A(vDataValue.bstrVal);
		break;
	case VT_UI8:
		ret = std::to_string((std::uint64_t)vDataValue.llVal);
	case VT_I8:
		ret = std::to_string((std::int64_t)vDataValue.llVal);
		break;
	case VT_I4:
		ret = std::to_string(vDataValue.lVal);
		break;
	case VT_I2:
		ret = std::to_string(vDataValue.iVal);
		break;			//more later
	}
	return ret;
}

OPCItemData::OPCItemData(){
	vDataValue.vt = VT_EMPTY;
}



OPCItemData::~OPCItemData(){
	VariantClear(&vDataValue);
}


void OPCItemData::set(OPCITEMSTATE &itemState){
	HRESULT result = VariantCopy( &vDataValue, &itemState.vDataValue);
	if (FAILED(result)){
		throw OPCException("VarCopy failed");
	}

	ftTimeStamp = itemState.ftTimeStamp;
	wQuality = itemState.wQuality;
}


void OPCItemData::set(FILETIME time, WORD qual, VARIANT & val){
	HRESULT result = VariantCopy( &vDataValue, &val);
	if (FAILED(result)){
		throw OPCException("VarCopy failed");
	}

	ftTimeStamp = time;
	wQuality = qual;
}

OPCItemData & OPCItemData::operator=(OPCItemData &itemData){
	HRESULT result = VariantCopy( &vDataValue, &(itemData.vDataValue));
	if (FAILED(result)){
		throw OPCException("VarCopy failed");
	}

	ftTimeStamp = itemData.ftTimeStamp;
	wQuality = itemData.wQuality;

	return *this;
}





COPCItem_DataMap::~COPCItem_DataMap(){
	POSITION pos = GetStartPosition();
	while (pos != NULL){
		OPCItemData * data = GetNextValue(pos);
		if (data){
			delete data;
		}
	}
	RemoveAll();
}





