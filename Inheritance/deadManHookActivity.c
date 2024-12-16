#include "hookapi.h"

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

int64_t hook(uint32_t reserved) {

    uint8_t activity[20];
    otxn_field(SBUF(activity), sfAccount);
    
    uint8_t hook_acc[20];
    hook_account(SBUF(hook_acc));   

    if(BUFFER_EQUAL_20(activity, hook_acc)) {
        uint32_t current_ledger = ledger_seq();
        if (state_set(SVAR(current_ledger), "LAST", 4) != 4)
            NOPE("Inheritance: Could not set recent activity state entry, bailing.");    
    }

    DONE("Inheritance: Payment Successful.");    
    _g(1,1);
    return 0;          
}