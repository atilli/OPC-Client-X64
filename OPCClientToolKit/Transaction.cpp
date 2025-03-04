#include "Transaction.h"


CTransaction::CTransaction(ITransactionComplete * completeCB)
:_completed(false), _cancelID(0xffffffff), _completeCallBack(completeCB){
}



CTransaction::CTransaction(std::vector<std::shared_ptr<COPCItem>>& items, ITransactionComplete* completeCB) : _completed(FALSE), _cancelID(0xffffffff), _completeCallBack(completeCB) {

	for (std::shared_ptr<COPCItem>& item : items) {
		_opcData[item.get()] = std::make_unique<OPCItemData>();
	}
}
CTransaction::CTransaction(std::unordered_map<OPCHANDLE, std::shared_ptr<COPCItem>>& items, ITransactionComplete* completeCB) :_completed(FALSE), _cancelID(0xffffffff), _completeCallBack(completeCB) {

	for (auto kvPair : items) {
		auto pItem = kvPair.second;
		_opcData[pItem.get()] = std::make_unique<OPCItemData>();
	}
}
CTransaction::CTransaction(COPCItem& item, ITransactionComplete* completeCB) :_completed(FALSE), _cancelID(0xffffffff), _completeCallBack(completeCB) {
	_opcData[&item] = std::make_unique<OPCItemData>();
}

void CTransaction::setItemError(COPCItem& item, HRESULT error){
	
	_opcData[&item] = std::make_unique<OPCItemData>(error);
}



void CTransaction::setItemValue(COPCItem *item, FILETIME time, WORD qual, VARIANT & val, HRESULT err){
	_opcData[item] = std::make_unique<OPCItemData>(time, qual, val, err);
}


const OPCItemData * CTransaction::getItemValue(COPCItem *item) const{
	
	OPCItemData* pFoundValue = nullptr;

	auto ite = _opcData.find(item);
	if (ite != _opcData.end()) {
		pFoundValue = ite->second.get();
	}
	else {
		// abigious - we do'nt know if the key does not exist or there is no value - TODO throw exception
	}
	return pFoundValue;
}

void CTransaction::setCompleted(){
	_completed = true;
	if (_completeCallBack){
		_completeCallBack->complete(*this);
	}
}
