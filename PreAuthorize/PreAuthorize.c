#include "hookapi.h"

int64_t hook(uint32_t reserved) {

    uint8_t user_acc_id[20];
    otxn_field(SBUF(user_acc_id), sfAccount);

    uint8_t hook_acc_id[20];
    hook_account(hook_acc_id, 20);

    int8_t equal = 0; BUFFER_EQUAL(equal, hook_acc_id, user_acc_id, 20);
    if(equal)
        accept(SBUF("Pre-Authorize: Outgoing Transaction."), 1);    

    uint32_t dest_tag;
    if(otxn_field(SVAR(dest_tag), sfDestinationTag) != 4) 
        rollback(SBUF("Pre-Authorize: Destination Tag Missing"), 2);

    uint8_t publickey[1] = { 0x50U };
    uint8_t publickey_ptr[33];
    if(hook_param(publickey_ptr, 33, publickey, 1) != 33)
        rollback(SBUF("Pre-Authorize: PublicKey not set as Hook parameter"), 3);  

    uint8_t inputKey[1] = { 0x49U };
    uint8_t input[104];
    if(otxn_param(input, 104, inputKey, 1) != 104)
        rollback(SBUF("Pre-Authorize: HookParameter must be 64 + 20 + 4 + 8 + 8 = 104 bytes."), 4);   

    if(util_verify(input + 64, 40, input, 64, publickey_ptr, 33) != 1)
        rollback(SBUF("Pre-Authorize: Unauthorized Transaction."), 5);   

    BUFFER_EQUAL(equal, input + 64, user_acc_id, 20);
    if(!equal)
        rollback(SBUF("Pre-Authorize: Unauthorized Account."), 6);  

    uint32_t dest = *((int32_t*)input[84]);
    if(float_compare(dest_tag, dest, COMPARE_EQUAL) != 1)
        rollback(SBUF("Pre-Authorize: Wrong Recipient."), 7);        

    uint64_t amount   = *((int64_t*)input[88]);
    uint8_t amount_buf[48];
    otxn_field(SBUF(amount_buf), sfAmount);    
    uint64_t txn_amount = *((int64_t*)amount_buf);
    if(float_compare(amount, txn_amount, COMPARE_EQUAL) != 1)
        rollback(SBUF("Pre-Authorize: Unauthorized Amount."), 8);

    uint64_t sequence = *((int64_t*)input[96]);
    uint64_t user_acc_seq;
    otxn_field(SVAR(user_acc_seq), sfSequence);
    if(float_compare(sequence, user_acc_seq, COMPARE_EQUAL) != 1)
        rollback(SBUF("Pre-Authorize: Authorization Previously Used."), 9);
  
    accept(SBUF("Pre-Authorize: Payment Verified and Accepted."), 10);
    _g(1,1);
    return 0;    
}