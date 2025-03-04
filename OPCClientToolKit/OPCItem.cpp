/*
OPCClientToolKit
Copyright (C) 2005 Mark C. Beharrell

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/
#include "OPCServer.h"
#include "OPCItem.h"
#include "OPCGroup.h"





COPCItem::COPCItem(std::string &itemName, COPCGroup &g):
_name(itemName), _group(g){
}



COPCItem::~COPCItem()
{
	HRESULT *itemResult;
	_group.getItemManagementInterface()->RemoveItems(1, &_serversItemHandle, &itemResult);
	COPCClient::comFree(itemResult);
}


void COPCItem::setOPCParams(OPCHANDLE handle, VARTYPE type, DWORD dwAccess){
	_serversItemHandle = handle; 
	_vtCanonicalDataType = type; 
	_dwAccessRights = dwAccess;
}



void COPCItem::writeSync(VARIANT &data){
	HRESULT * itemWriteErrors; 
	HRESULT result = _group.getSychIOInterface()->Write(1, &_serversItemHandle, &data, &itemWriteErrors);
	if (FAILED(result))
	{
		throw OPCException("write failed");
	} 

	if (FAILED(itemWriteErrors[0])){
		COPCClient::comFree(itemWriteErrors);
		throw OPCException("write failed");
	}

	COPCClient::comFree(itemWriteErrors);
}



void COPCItem::readSync(OPCItemData &data, OPCDATASOURCE source){
	
	std::unique_ptr<OPCItemData> pReadData;
	_group.readSync(*this, pReadData, source);

	if (pReadData.get()!=nullptr && !FAILED(pReadData->error)){
		data = *pReadData;
		return;
	} 
	throw OPCException("Read failed");
}
	/*
	if ((dwAccessRights && OPC_READABLE) != OPC_READABLE){
		throw OPCException("Item is not readable");
	}

	HRESULT *itemResult;
	OPCITEMSTATE *itemState;

	HRESULT	result = group.getSychIOInterface()->Read(source, 1, &serversItemHandle, &itemState, &itemResult);
	if (FAILED(result))
	{
		throw OPCException("Read failed");
	} 

	if (FAILED(itemResult[0])){
		COPCClient::comFree(itemResult);
		COPCClient::comFree(itemState);
		throw OPCException("Read failed");
	}
		
	COPCClient::comFree(itemResult);
	
	data.set(itemState[0]);

	VariantClear(&itemState[0].vDataValue);
	COPCClient::comFree(itemState);
}*/



std::shared_ptr<CTransaction> COPCItem::readAsynch(ITransactionComplete *transactionCB){
	/*std::vector<COPCItem*> items;
	items.push_back(this);
	return group.readAsync(items, transactionCB);*/
	return std::make_shared<CTransaction>();
}

std::shared_ptr<CTransaction> COPCItem::writeAsynch(VARIANT &data, ITransactionComplete *transactionCB){
	DWORD cancelID;
	HRESULT * individualResults;
	
	std::shared_ptr<CTransaction> pTrans = std::make_shared<CTransaction>(*this,transactionCB);
	
	DWORD transactionID = pTrans->CreateTransactionID();

	COPCGroup::AddTransaction(pTrans, transactionID);

	HRESULT result = _group.getAsych2IOInterface()->Write(1,&_serversItemHandle,&data, transactionID,&cancelID,&individualResults);
	
	if (FAILED(result)){
		//delete trans;
		COPCGroup::PopTransaction(transactionID);
		throw OPCException("Asynch Write failed");
	}
	
	pTrans->setCancelId(cancelID);

	if (FAILED(individualResults[0])){
		pTrans->setItemError(*this,individualResults[0]);
		pTrans->setCompleted(); // if all items return error then no callback will occur. p 104
		//activeMap.erase(transactionID);
	}

	COPCClient::comFree(individualResults);
	return pTrans;
}

void COPCItem::getSupportedProperties(std::vector<CPropertyDescription> &desc){
	DWORD noProperties = 0;
	DWORD *pPropertyIDs;
	LPWSTR *pDescriptions;
	VARTYPE *pvtDataTypes;

	USES_CONVERSION;
	HRESULT res = _group.getServer().getPropertiesInterface()->QueryAvailableProperties(T2OLE(_name.c_str()), &noProperties, &pPropertyIDs, &pDescriptions, &pvtDataTypes);
	if (FAILED(res)){
		throw OPCException("Failed to restrieve properties", res);
	}

	std::string tmp;
	for (unsigned i = 0; i < noProperties; i++){
		tmp=CW2A (pDescriptions[i]);
		desc.push_back(CPropertyDescription(pPropertyIDs[i], std::string(tmp), pvtDataTypes[i]));
	}

	COPCClient::comFree(pPropertyIDs);
	COPCClient::comFree(pDescriptions);
	COPCClient::comFree(pvtDataTypes);
}


void COPCItem::getProperties(const std::vector<CPropertyDescription> &propsToRead, ATL::CAutoPtrArray<SPropertyValue> &propsRead){
	DWORD noProperties = (DWORD)propsToRead.size();
	VARIANT *pValues = NULL;
	HRESULT *pErrors = NULL;
	
	std::unique_ptr<DWORD[]> pPropertyIDs(new DWORD[noProperties]);

	for (unsigned i = 0; i < noProperties; i++){
		pPropertyIDs.get()[i] = propsToRead[i].id;
	}
	propsRead.RemoveAll();
	propsRead.SetCount(noProperties);
	
	USES_CONVERSION;
	HRESULT res = _group.getServer().getPropertiesInterface()->GetItemProperties(T2OLE(_name.c_str()), noProperties, pPropertyIDs.get(), &pValues, &pErrors);
	
	if (FAILED(res)){
		throw OPCException("Failed to restrieve property values", res);
	}

	for (unsigned i = 0; i < noProperties; i++){
		CAutoPtr<SPropertyValue> v;
		if (!FAILED(pErrors[i])){
			v.Attach(new SPropertyValue(propsToRead[i], pValues[i]));
		}
		propsRead[i]=v;
	}

	COPCClient::comFree(pErrors);
	COPCClient::comFreeVariant(pValues, noProperties);
}