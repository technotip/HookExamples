#include "hookapi.h"

#define OTXN_AMT_TO_XFL(buf) float_set(-6, AMOUNT_TO_DROPS(buf))

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

int64_t hook(uint32_t reserved)
{  
    // first sanity check all the parameters, if any are missing we just pass all txns until the hook is correctly configured
    uint64_t limit_amt;
    if(hook_param(SVAR(limit_amt), "A", 1) != 8)       // supply A in HookParameters as little endian 8 byte xfl
        DONE("Lockup: Misconfigured. Amount 'A' not set as Hook parameter");

    uint32_t limit_ledger;
    if(hook_param(SVAR(limit_ledger), "L", 1) != 4)    // supply L in HookParameters as little endian 4 byte uint32
        DONE("Lockup: Misconfigured. Ledger limit 'L' not set as Hook parameter");

    // pass all ClaimReward transactions
    int64_t type = otxn_type(); 
    if (type == ttCLAIM_REWARD)
        DONE("Lockup: Passing ClaimReward.");

    // block any txn that isn't ClaimReward or Payment
    if (type != ttPAYMENT)
        NOPE("Lockup: Only ClaimReward and Payment txns are allowed.");

    // block any payment that isn't XAH
    uint8_t amount[8];
    if(otxn_field(SBUF(amount), sfAmount) != 8) 
        NOPE("Lockup: Non XAH currency payments are forbidden.");

    // pass any incoming txns
    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);
    uint8_t hook_acc[20];
    hook_account(hook_acc, 20);
    if(!BUFFER_EQUAL_20(hook_acc, account)) 
        DONE("Lockup: Incoming Transaction.");

    // fetch the last time a release occured, if the state entry doesn't exist yet this is 0
    uint32_t last_release = 0;
    state(SVAR(last_release), "LAST", 4); // last released ledger

    // enforce the release interval
    uint32_t current_ledger =  ledger_seq();
    if (last_release + limit_ledger > current_ledger)
        NOPE("Lockup: You need to wait longer before a release.");

    // enforce the release limit
    int64_t amount_xfl = OTXN_AMT_TO_XFL(amount);  
    if(float_compare(amount_xfl, limit_amt, COMPARE_GREATER) == 1)
        NOPE("Lockup: Outgoing transaction exceeds the limit set by you."));

    // update the last released state entry
    if (state_set(SVAR(current_ledger), "LAST", 4) != 4)
        NOPE("Lockup: Could not update state entry, bailing.");

    DONE("Lockup: Released successfully.");

    _g(1,1);
    return 0;    
}
