#include ".\transaction.h"


CTransaction::CTransaction(ITransactionComplete * completeCB)
:_completed(FALSE), _cancelID(0xffffffff), completeCallBack(completeCB){
}



CTransaction::CTransaction(std::vector<COPCItem *>&items, ITransactionComplete * completeCB)
:_completed(FALSE), _cancelID(0xffffffff), completeCallBack(completeCB){
	for (unsigned i = 0; i < items.size(); i++){
		opcData.SetAt(items[i],NULL);
	}
}


void CTransaction::setItemError(COPCItem *item, HRESULT error){
	auto pair = opcData.Lookup(item);
	opcData.SetValueAt(pair,new OPCItemData(error));
}



void CTransaction::setItemValue(COPCItem *item, FILETIME time, WORD qual, VARIANT & val, HRESULT err){
	auto pair = opcData.Lookup(item);
	opcData.SetValueAt(pair,new OPCItemData(time, qual, val, err));
}


const OPCItemData * CTransaction::getItemValue(COPCItem *item) const{
	auto pair = opcData.Lookup(item);
	if (!pair) return nullptr; // abigious - we do'nt know if the key does not exist or there is no value - TODO throw exception

	return pair->m_value;
}

void CTransaction::setCompleted(){
	_completed = TRUE;
	if (completeCallBack){
		completeCallBack->complete(*this);
	}
}
