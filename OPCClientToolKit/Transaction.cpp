#include "Transaction.h"


CTransaction::CTransaction(ITransactionComplete * completeCB)
:_completed(FALSE), _cancelID(0xffffffff), completeCallBack(completeCB){
}



CTransaction::CTransaction(std::vector<std::unique_ptr<COPCItem>>& items, ITransactionComplete* completeCB) : _completed(FALSE), _cancelID(0xffffffff), completeCallBack(completeCB) {

	for (std::unique_ptr<COPCItem>& item : items) {
		_opcData[item.get()] = std::make_unique<OPCItemData>();
	}
}
CTransaction::CTransaction(std::vector<COPCItem*> &items, ITransactionComplete * completeCB) :_completed(FALSE), _cancelID(0xffffffff), completeCallBack(completeCB) {

	for (auto pItem : items) {
		_opcData[pItem] = std::make_unique<OPCItemData>();
	}
}

void CTransaction::setItemError(COPCItem *item, HRESULT error){
	
	_opcData[item] = std::make_unique<OPCItemData>(error);
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
	_completed = TRUE;
	if (completeCallBack){
		completeCallBack->complete(*this);
	}
}
