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
	case OPC_QUALITY_GOOD:
	case OPC_QUALITY_LOCAL_OVERRIDE:
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
	case VT_INT:
		ret = std::to_string(vDataValue.intVal);
		break;
	case VT_UINT:
		ret = std::to_string(vDataValue.uintVal);
		break;
	case VT_UI8:
		ret = std::to_string((std::uint64_t)vDataValue.llVal);
	case VT_I8:
		ret = std::to_string((std::int64_t)vDataValue.llVal);
		break;
	case VT_UI4:
		ret = std::to_string((std::uint32_t)vDataValue.lVal);
		break;
	case VT_I4:
		ret = std::to_string((std::int32_t)vDataValue.lVal);
		break;
	case VT_UI2:
		ret = std::to_string((std::uint16_t)vDataValue.iVal);
		break;
	case VT_I2:
		ret = std::to_string((std::int16_t)vDataValue.iVal);
		break;
	case VT_UI1:
		ret = std::to_string((std::uint8_t)vDataValue.bVal);
		break;
	case VT_I1:
		ret = std::to_string((std::int8_t)vDataValue.bVal);
		break;			//more later
	case VT_R8:
		ret = std::to_string(vDataValue.dblVal);
		break;			//more later
	case VT_R4:
		ret = std::to_string(vDataValue.fltVal);
		break;			//more later
	case VT_BOOL:
		if (vDataValue.boolVal == VARIANT_TRUE) {
			ret = "TRUE";
		}
		else {
			ret = "FALSE";
		}
		break;			//more later
	case VT_CY:
		ret = std::to_string(vDataValue.cyVal.int64);
		break;			//more later
	case VT_DATE:
		ret = std::to_string(vDataValue.date);
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
	/*POSITION pos = GetStartPosition();
	while (pos != NULL){
		OPCItemData * data = GetNextValue(pos);
		if (data){
			delete data;
		}
	}*/
	RemoveAll();
}





