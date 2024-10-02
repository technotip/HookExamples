#include "hookapi.h"

#define OTXN_AMT_TO_XFL(buf) float_set(-6, AMOUNT_TO_DROPS(buf))

int64_t hook(uint32_t reserved) {  
    uint8_t key[1] = { 0x4CU }; // 4C is the hex value for 'L'
    int32_t current_ledger =  ledger_seq();

    // We need to block more transactions like URIToken offer etc. We need to decide on this. Also SetHook at position.
    int64_t type = otxn_type();
    if(type == ttOFFER_CREATE || type == ttESCROW_CREATE) 
        rollback(SBUF("Lockup: Transaction Type not allowed."), 1);

    // if type is ttPAYMENT
    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);
    uint8_t hook_acc[20];
    hook_account(hook_acc, 20);

    if(!BUFFER_EQUAL_20(hook_acc, account)) 
        accept(SBUF("Lockup: Incoming Transaction."), 2);

    uint8_t limit_ledger[4];
    if(hook_param(SBUF(limit_ledger), key, 1) != 4)
        rollback(SBUF("Lockup: Ledger limit not set as Hook parameter"), 6);

    int32_t paid_on = 0;
    state(SVAR(paid_on), hook_acc, 32);

    int32_t ledger_ptr = UINT32_FROM_BUF(limit_ledger);
    if((paid_on + ledger_ptr) > current_ledger)
        rollback(SBUF("Lockup: You need to wait more before making the transaction."), current_ledger - (paid_on + ledger_ptr));

    uint8_t amount[8];
    if(otxn_field(SBUF(amount), sfAmount) != 8)  
        accept(SBUF("Lockup: Outgoing non XAH currency/token."), 3);
    int64_t amount_xfl = OTXN_AMT_TO_XFL(amount);
    
    key[0] =  0x41U;       // 41 is the hex value for 'A'    
    int64_t limit_amt;
    if(hook_param(SVAR(limit_amt), key, 1) != 8)
        rollback(SBUF("Lockup: Transaction limit (Amount) not set as Hook parameter"), 4);

    if(float_compare(amount_xfl, limit_amt, COMPARE_GREATER) == 1)
        rollback(SBUF("Lockup: Outgoing transaction exceeds the limit set by you."), limit_amt);


    state_set(SVAR(current_ledger), hook_acc, 32);
    accept(SBUF("Lockup: Successful payment for the ledger limit/interval."), 8);

    _g(1,1);
    return 0;    
}