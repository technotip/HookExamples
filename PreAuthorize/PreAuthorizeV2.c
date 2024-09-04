#include "hookapi.h"

int64_t hook(uint32_t reserved) {

    uint8_t user_acc_id[20];
    otxn_field(SBUF(user_acc_id), sfAccount);

    uint8_t hook_acc_id[20];
    hook_account(hook_acc_id, 20);

    int8_t equal = 0; BUFFER_EQUAL(equal, hook_acc_id, user_acc_id, 20);
    if(equal) accept(SBUF("Pre-Authorize: Outgoing Transaction."), 1);    

    uint32_t dest_tag;
    if(otxn_field(SVAR(dest_tag), sfDestinationTag) != 4) 
        rollback(SBUF("Pre-Authorize: Destination Tag Missing"), 2);

    uint8_t publickey[1] = { 0x50U };
    uint8_t publickey_ptr[65];
    if(hook_param(publickey_ptr, 65, publickey, 1) != 65)
        rollback(SBUF("Pre-Authorize: PublicKey not set as Hook parameter"), 3);  

    uint8_t payload_key[1] = { 0x49U };
    uint8_t payload[40];
    if(otxn_param(payload, 40, payload_key, 1) != 40)
        rollback(SBUF("Pre-Authorize: HookParameter Payload must be 20 + 4 + 8 + 8 = 40 bytes."), 4);   

    uint8_t sign_key[1] = { 0x53U };
    uint8_t signature[70];
    if(otxn_param(signature, 70, sign_key, 1) != 70)
        rollback(SBUF("Pre-Authorize: HookParameter Signature must be 70 bytes."), 5);          

    if(util_verify(signature, 70, payload, 40, publickey_ptr, 65) != 1)
        rollback(SBUF("Pre-Authorize: Unauthorized Transaction."), 6);   

    BUFFER_EQUAL(equal, payload, user_acc_id, 20);
    if(!equal) rollback(SBUF("Pre-Authorize: Unauthorized Account."), 7);  

    uint32_t dest = *((int32_t*)(payload + 20));
    if(float_compare(dest_tag, dest, COMPARE_EQUAL) != 1)
        rollback(SBUF("Pre-Authorize: Wrong Recipient."), 8);        

    uint8_t amount_buf[48];
    if(otxn_field(SBUF(amount_buf), sfAmount) != 48) 
        rollback(SBUF("Pre-Authorize: Invalid Issued Currency."), 9);

    uint64_t amount   = *((int64_t*)(payload + 24));    
    uint64_t txn_amount = *((int64_t*)amount_buf);
    if(float_compare(amount, txn_amount, COMPARE_EQUAL) != 1)
        rollback(SBUF("Pre-Authorize: Unauthorized Amount."), 10);

    uint64_t sequence = *((int64_t*)(payload + 32));
    uint64_t user_acc_seq;
    otxn_field(SVAR(user_acc_seq), sfSequence);
    if(float_compare(sequence, user_acc_seq, COMPARE_EQUAL) != 1)
        rollback(SBUF("Pre-Authorize: Authorization Previously Used."), 11);
  
    accept(SBUF("Pre-Authorize: Payment Verified and Accepted."), 12);
    _g(1,1);
    return 0;    
}