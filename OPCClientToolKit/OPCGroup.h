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

#if !defined(AFX_OPCGROUP_H__BE6D983E_3D18_4952_A2B3_84A9FCDFC5CE__INCLUDED_)
#define AFX_OPCGROUP_H__BE6D983E_3D18_4952_A2B3_84A9FCDFC5CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OPCClient.h"
#include "Transaction.h"


#include <map>
#include <unordered_map>
#include <iostream>


/**
* Forward decl.
*/
class COPCItem;

/**
* used internally to implement the asynch callback
*/
class CAsynchDataCallback; 




/**
* Client sided abstraction of an OPC group, wrapping the COM interfaces to the group within the OPC server.
*/
class COPCGroup  
{
public:
	// STORE in host ??
	static std::map<DWORD, std::shared_ptr<CTransaction>>& TransactionsMap() { return _transactionPointers;  }
	static void AddTransaction(std::shared_ptr<CTransaction>& pTrans, DWORD id) {

		std::cout << "ADD :" << id << std::endl;

		_transactionPointers[id] = pTrans;
	} 
	static std::shared_ptr<CTransaction> PopTransaction(DWORD id) {

		std::cout << "POP :" << id << std::endl;

		std::shared_ptr<CTransaction> pFound = _transactionPointers[id];
		if (pFound != nullptr) {
			_transactionPointers.erase(id);
		}
		return pFound;
	}

private:
	static std::map<DWORD, std::shared_ptr<CTransaction>> _transactionPointers;

	ATL::CComPtr<IOPCGroupStateMgt>	_iStateManagement;
	ATL::CComPtr<IOPCSyncIO>		_iSychIO;
	ATL::CComPtr<IOPCAsyncIO2>		_iAsych2IO;
	ATL::CComPtr<IOPCItemMgt>		_iItemManagement;

	/**
	* Used to keep track of the connection point for the
	* AsynchDataCallback
	*/
	ATL::CComPtr<IConnectionPoint> _iAsynchDataCallbackConnectionPoint;


	/**
	* handle given the group by the server
	*/
	DWORD _groupHandle;

	/**
	* The server this group belongs to
	*/
	COPCServer &_opcServer;


	/**
	* Callback for asynch data at the group level
	*/
	ATL::CComPtr<CAsynchDataCallback> asynchDataCallBackHandler;


	/**
	* list of OPC items associated with this goup. 
	*/
	std::unordered_map<OPCHANDLE,std::shared_ptr<COPCItem>> _items;


	/**
	* Name of the group
	*/
	const std::string _name;


	/**
	* Handle given to callback by server.
	*/
	DWORD callbackHandle;


	/**
	* Users hander to handle asynch data 
	* NOT OWNED.
	*/
	IAsynchDataCallback *userAsynchCBHandler;
	CAsynchDataCallback* _CAsynchDataCallback;

	/**
	* Caller owns returned array
	*/
	OPCHANDLE* buildServerHandleList(std::vector<std::shared_ptr<COPCItem>>& items);
	OPCHANDLE* buildServerHandleList(COPCItem& item);

public:
	COPCGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand, COPCServer &server);
	void updateOPCData(std::map<COPCItem*, std::unique_ptr<OPCItemData>>& opcData, DWORD count, OPCHANDLE* clienthandles, VARIANT* values, WORD* quality, FILETIME* time, HRESULT* errors);

	virtual ~COPCGroup();

	std::shared_ptr<COPCItem> addItem(std::string &itemName, bool active);
	std::shared_ptr<COPCItem> findItem(OPCHANDLE clientHandle);
	
	/**
	* returns the number of failed item creates
	* itemsCreated[x] will be null if could not create and will contain error code in corresponding error entry
	*/
	int addItems(std::vector<std::string>& itemName, std::vector<std::shared_ptr<COPCItem>>& itemsCreated, std::vector<HRESULT>& errors, bool active);


	/**
	* enable Asynch IO
	*/
	void enableAsynch(IAsynchDataCallback &handler);


	/**
	* disable Asych IO 
	*/
	void disableAsynch();


	/**
	* set the group state values.
	*/
	void setState(DWORD reqUpdateRate_ms, DWORD &returnedUpdateRate_ms, float deadBand, BOOL active);



	/**
	* Read set of OPC items synchronously.
	*/
	void readSync(std::vector<std::shared_ptr<COPCItem>>& items, std::map<COPCItem *, std::unique_ptr<OPCItemData>> &opcData, OPCDATASOURCE source);
	void readSync(COPCItem& item, std::unique_ptr<OPCItemData>& opcData, OPCDATASOURCE source); // Added simple read from single item...


	/**
	* Read a defined group of OPC item asynchronously
	*/
	std::shared_ptr<CTransaction> readAsync(std::vector<std::shared_ptr<COPCItem>>& items, ITransactionComplete *transactionCB = nullptr);


	/**
	* Refresh is an asysnch operation.
	* retreives all active items in the group, which will be stored in the transaction object
	* Transaction object is owned by caller.
	* If group asynch is disabled then this call will not work
	*/ 
	std::shared_ptr<CTransaction> refresh(OPCDATASOURCE source, ITransactionComplete *transactionCB = nullptr);



	ATL::CComPtr<IOPCSyncIO> & getSychIOInterface(){
		return _iSychIO;
	}


	ATL::CComPtr<IOPCAsyncIO2> & getAsych2IOInterface(){
		return _iAsych2IO;
	}


	ATL::CComPtr<IOPCItemMgt> &getItemManagementInterface(){
		return _iItemManagement;
	}

	const std::string & getName() const {
		return _name;
	}

	IAsynchDataCallback *getUsrAsynchHandler(){
		return userAsynchCBHandler;
	}

	/**
	* returns reaference to the OPC server that this group belongs to.
	*/
	COPCServer & getServer(){
		return _opcServer;
	}
};

#endif // !defined(AFX_OPCGROUP_H__BE6D983E_3D18_4952_A2B3_84A9FCDFC5CE__INCLUDED_)
