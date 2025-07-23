#include "hookapi.h"

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

int64_t hook(uint32_t reserved) {

    uint8_t swap_hook[32] = { 0xF4U, 0xD4U, 0xBAU, 0x0EU, 0xB8U, 0x5AU, 0xBDU, 0x1CU, 0x9CU, 0xD7U, 0xE4U, 0xD2U, 0x85U, 0xDEU, 0xE0U, 0x54U, 0x6BU, 0x72U, 0x15U, 0x65U, 0x13U, 0x25U, 0x5CU, 0x57U, 0x35U, 0x1AU, 0xB5U, 0xB5U, 0x2BU, 0x88U, 0x73U, 0xFCU };

    uint8_t whitelisted_account[20];
    if(hook_param(SBUF(whitelisted_account), "R_ACC", 5) != 20)
        NOPE("Misconfigured. Whitelisted account not set as Hook Parameter.");     

    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);

    uint8_t hook_acc[20];
    hook_account(hook_acc, 20);

    if (!BUFFER_EQUAL_20(whitelisted_account, account)) 
        NOPE("Not Whitelisted Account.");

    uint64_t conversion_rate;
    if(otxn_param(SVAR(conversion_rate), "R", 1) != 8)
        NOPE("Misconfigured. Conversion rate not set as Hook Parameter."); 
        
    if(state_foreign_set(SVAR(conversion_rate), "R", 1, SBUF(swap_hook), SBUF(hook_acc)) != 8)
        NOPE("Failed To Set Conversion Rate.");

    DONE("New Conversion Rate Set.");   
    _g(1,1);
    return 0;    
}    