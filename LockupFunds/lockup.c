#include "hookapi.h"

#define SETUP_CURRENT_MONTH()\
uint8_t current_month = 0;\
{\
    int64_t s = ledger_last_time() + 946684800;\
    int64_t z = s / 86400 + 719468;\
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;\
    uint64_t doe = (uint64_t)(z - era * 146097);\
    uint64_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;\
    int64_t y = (int64_t)(yoe) + era * 400;\
    uint64_t doy = doe - (365*yoe + yoe/4 - yoe/100);\
    uint64_t mp = (5*doy + 2)/153;\
    uint64_t m = mp + (mp < 10 ? 3 : -9);\
    current_month = m;\
}

int64_t hook(uint32_t reserved) {  
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

    int64_t amount;
    if(otxn_field(SVAR(amount), sfAmount) > 8)  
        accept(SBUF("Lockup: Outgoing non XAH currency/token."), 3);

    SETUP_CURRENT_MONTH();

    uint8_t paid;
    if(state(SVAR(paid), hook_acc, 32) == 1) {
        if(paid == current_month) {
            rollback(SBUF("Lockup: Already made Payment for the month."), 4);
        }
    }

    uint8_t limit[1] = { 0x41U };
    int64_t limit_ptr;
    if(hook_param(SVAR(limit_ptr), limit, 1) != 8)
        rollback(SBUF("Lockup: Transaction limit (Amount) not set as Hook parameter"), 5);

    if(float_compare(amount, limit_ptr, COMPARE_GREATER) == 1)
        rollback(SBUF("Lockup: Outgoing transaction exceeds the limit set by you."), 6);

    state_set(SVAR(current_month), hook_acc, 32);
    accept(SBUF("Lockup: Successful payment for the month."), 7);

    _g(1,1);
    return 0;    
}