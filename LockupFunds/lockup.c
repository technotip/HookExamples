#include "hookapi.h"

#define BUFFER_TO_INT32_BE(buffer) \
    ((int32_t)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3] << 0)))

int64_t hook(uint32_t reserved) {  
    int32_t current_ledger =  ledger_seq();
    int64_t type = otxn_type();
    if(type == ttOFFER_CREATE || type == ttESCROW_CREATE) 
        rollback(SBUF("Lockup: Transaction Type not allowed."), 1);

    // if type is ttPAYMENT
    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);
    uint8_t hook_acc[20];
    hook_account(hook_acc, 20);

    int8_t equal = 0; BUFFER_EQUAL(equal, hook_acc, account, 20);
    if(!equal) accept(SBUF("Lockup: Incoming Transaction."), 2);

    uint8_t amount[8];
    if(otxn_field(SBUF(amount), sfAmount) > 8)  
        accept(SBUF("Lockup: Outgoing non XAH currency/token."), 3);

    
    uint8_t limit[1] = { 0x41U }; // 41 is the hex value for 'A'
    int64_t limit_ptr;
    if(hook_param(SVAR(limit_ptr), limit, 1) != 8)
        rollback(SBUF("Lockup: Transaction limit (Amount) not set as Hook parameter"), 5);

    uint64_t otxn_drops = AMOUNT_TO_DROPS(amount);
    int64_t amount_xfl = float_set(-6, otxn_drops);

    // int64_t amount_xfl = -INT64_FROM_BUF(amount);

    if(float_compare(amount_xfl, limit_ptr, COMPARE_GREATER) == 1)
        rollback(SBUF("Lockup: Outgoing transaction exceeds the limit set by you."), 6);

    uint8_t ledger_limit[1] = { 0x4CU }; // 4C is the hex value for 'L'
    uint8_t raw_ptr[4];
    if(hook_param(SBUF(raw_ptr), ledger_limit, 1) != 4)
        rollback(SBUF("Lockup: Ledger limit not set as Hook parameter"), 5);

    int32_t paid_on = 0;
    state(SVAR(paid_on), hook_acc, 32);

    int32_t ledger_ptr = BUFFER_TO_INT32_BE(raw_ptr);

    if((paid_on + ledger_ptr) > current_ledger) {
        rollback(SBUF("Lockup: You need to wait more before making the transaction."), current_ledger - (paid_on + ledger_ptr));
    }

    state_set(SVAR(current_ledger), hook_acc, 32);
    accept(SBUF("Lockup: Successful payment for the month."), 7);

    _g(1,1);
    return 0;    
}