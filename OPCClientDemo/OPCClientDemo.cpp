// OPCClientDemo.cpp : Defines the entry point for the console application.
//

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

#include <stdio.h>
#include "opcda.h"
#include "OPCClient.h"
#include "OPCHost.h"
#include "OPCServer.h"
#include "OPCGroup.h"
#include "OPCItem.h"
#include <sys\timeb.h>
#include <iostream>
#include <string>



/**
* Code demonstrates
* 1) Browsing local servers.
* 2) Connection to local server.
* 3) Browsing local server namespace
* 4) Creation of OPC item and group
* 5) synch and asynch read of OPC item.
* 6) synch and asynch write of OPC item.
* 7) The receipt of changes to items within a group (subscribe)
* 8) group refresh.
* 9) Synch read of multiple OPC items. 
*/


/**
* Handle asynch data coming from changes in the OPC group
*/
class CMyCallback:public IAsynchDataCallback{
	public:
		void OnDataChange(COPCGroup & group, CAtlMap<COPCItem *, OPCItemData *> & changes) override {
			printf("Group %s, item changes\n", group.getName().c_str());
			POSITION pos = changes.GetStartPosition();
			while (pos != NULL){

				auto kv = changes.GetNext(pos);
				COPCItem* pItem = kv->m_key;
				OPCItemData *pVal = kv->m_value;

				std::cout << pItem->getName() << " : " << pVal->QualityString() << " = " << pVal->ToString() << std::endl;

				//COPCItem * item = changes.GetNextKey(pos);
				//std::cout << item->getName() << std::endl 
				
				//printf("-----> %s\n", item->getName().c_str());
			}
		}
		
};



/**
*	Handle completion of asynch operation
*/
class CTransComplete:public ITransactionComplete{
	std::string completionMessage;
public:
	CTransComplete(){
		completionMessage = "Asynch operation is completed";
	};

	void complete(CTransaction &transaction){
		printf("%s\n",completionMessage.c_str());
	}

	void setCompletionMessage(const std::string & message){
		completionMessage = message;
	}
};




//---------------------------------------------------------
// main



#define MESSAGEPUMPUNTIL(x)	while(!x){{MSG msg;while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE)){TranslateMessage(&msg);DispatchMessage(&msg);}Sleep(1);}}


void main(void)
{
	COPCClient::init();

	std::cout << "Input hostname: \nWarning: NOT IP ADDRESS, use sucn as:\"Jhon-PC\"" << std::endl; //See readme to find reason
	std::string hostName;
	std::getline(std::cin, hostName);
	
	std::unique_ptr<COPCHost> pHost;

	try {
		pHost = COPCClient::makeHost(hostName);
	}
	catch (const OPCException& ex) {
		std::cerr << ex.reasonString();
		return;
	}
		
	//List servers
	std::vector<std::string> localServerList;
	pHost->getListOfDAServers(IID_CATID_OPCDAServer20, localServerList);
	unsigned i = 0;
	std::cout << "Server ID List : " << std::endl;
		for (; i < localServerList.size(); i++){
		std::cout << i << " : " << localServerList[i];
	}

	// connect to OPC
	printf("\nSelect a Server ID:\n");
	int server_id;
	scanf("%d",&server_id);
	std::string serverName = localServerList[server_id];

	std::unique_ptr<COPCServer> pOpcServer;

	try {
		pOpcServer = pHost->connectDAServer(serverName);
	}
	catch (const OPCException& ex) {
		std::cerr << ex.reasonString();
		return;
	}
	// Check status
	ServerStatus status;
	pOpcServer->getStatus(status);
	std::cout << "Server(" << status.vendorInfo << ") state is " << status.dwServerState << std::endl;

	// browse server
	std::vector<std::string> opcItemNames;
	std::vector<std::string> increments;

	pOpcServer->getItemNames(opcItemNames);
	std::cout << "Got "<< opcItemNames.size() << " names" << std::endl;
	for (auto &name : opcItemNames){
		std::cout << name << std::endl;
		if (name.find("increment") != std::string::npos) {
			// Skip arrays for now 
			if (name.find("array") == std::string::npos) increments.push_back(name);
		}
	}

	// make group
	unsigned long refreshRate;
	std::unique_ptr<COPCGroup> pGroup = pOpcServer->makeGroup("Increments",true,1000,refreshRate,0.0);
	std::vector<COPCItem *> items;

	// make  a single item
	std::string changeChanNameName = opcItemNames[5];
	COPCItem * readWritableItem = pGroup->addItem(changeChanNameName, true);

	// make several items
	std::vector<std::string> itemNames;
	std::vector<COPCItem *>itemsCreated;
	std::vector<HRESULT> errors;
	//for(auto incTag : increments) 
	//itemNames.push_back(opcItemNames[15]);
	//itemNames.push_back(opcItemNames[16]);	

	if (pGroup->addItems(increments, itemsCreated,errors,true) != 0){
		std::cout << "Item create failed" << std::endl;
	}

	// get properties
	std::vector<CPropertyDescription> propDesc;
	readWritableItem->getSupportedProperties(propDesc);
	std::cout << "Supported properties for " << readWritableItem->getName() << std::endl;
	for (i = 0; i < propDesc.size(); i++){
		printf("%d id = %u, description = %s, type = %d\n", i, propDesc[i].id,propDesc[i].desc.c_str(),propDesc[i].type);
	}

	CAutoPtrArray<SPropertyValue> propVals;
	readWritableItem->getProperties(propDesc, propVals);
	for (i = 0; i < propDesc.size(); i++){
		if (propVals[i] == NULL){
			printf("Failed to get property %u\n", propDesc[i].id);
		}else{
			printf("Property %u=", propDesc[i].id);
			switch (propDesc[i].type){
				case VT_R4:printf("%f\n", propVals[i]->value.fltVal); break;
				case VT_I4:printf("%d\n", propVals[i]->value.iVal); break;
				case VT_I2:printf("%d\n", propVals[i]->value.iVal); break;
				case VT_I1:{int v = propVals[i]->value.bVal;printf("%d\n", v);} break;
				default:printf("\n");break;
			}
		}
	}


	// SYNCH READ of item
	OPCItemData data;
	readWritableItem->readSync(data,OPC_DS_DEVICE);
	printf("Synch read quality %d value %d\n", data.wQuality, data.vDataValue.iVal);

	// SYNCH read on Group
	COPCItem_DataMap opcData;
	pGroup->readSync(itemsCreated,opcData, OPC_DS_DEVICE);

	// Enable asynch - must be done before any asynch call will work
	CMyCallback usrCallBack;
	pGroup->enableAsynch(usrCallBack);
	
	// ASYNCH OPC item READ
	CTransComplete complete;
	complete.setCompletionMessage("*******Asynch read completion handler has been invoked (OPC item)");
	std::shared_ptr<CTransaction> t = readWritableItem->readAsynch(&complete);
	MESSAGEPUMPUNTIL(t->isCompeleted())
	
	const OPCItemData * asychData = t->getItemValue(readWritableItem); // not owned
	if (!FAILED(asychData->error)){
		printf("Asynch read quality %d value %d\n", asychData->wQuality, asychData->vDataValue.iVal);
	}
		// Aysnch read opc items from a group
	complete.setCompletionMessage("*******Asynch read completion handler has been invoked (OPC GROUP)");
	t = pGroup->readAsync(itemsCreated, &complete);
	MESSAGEPUMPUNTIL(t->isCompeleted())
		

	// SYNCH write
	VARIANT var;
	var.vt = VT_I2;
	var.iVal = 99;
	readWritableItem->writeSync(var);


	// ASYNCH write
	var.vt = VT_I2;
	var.iVal = 32;
	complete.setCompletionMessage("*******Asynch write completion handler has been invoked");
	t = readWritableItem->writeAsynch(var, &complete);
	MESSAGEPUMPUNTIL(t->isCompeleted())

	asychData = t->getItemValue(readWritableItem); // not owned
	if (!FAILED(asychData->error)){
		printf("Asynch write comleted OK\n");
	}else{
		printf("Asynch write failed\n");
	}

	// Group refresh (asynch operation) - pass results to CMyCallback as well
	complete.setCompletionMessage("*******Refresh completion handler has been invoked");
	t = pGroup->refresh(OPC_DS_DEVICE, /*true,*/ &complete);
	MESSAGEPUMPUNTIL(t->isCompeleted())

	// readWritableItem is member of group - look for this and use it as a guide to see if operation succeeded.
	asychData = t->getItemValue(readWritableItem); 
	if (!FAILED(asychData->error)){
		printf("refresh compeleted OK\n");
	}else{
		printf("refresh failed\n");
	}

	// just loop - changes to Items within a group are picked up here 
	MESSAGEPUMPUNTIL(false)
}