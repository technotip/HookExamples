#include "hookapi.h"

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

// clang-format off 
uint8_t txn[229] =
{
/* size,upto */
/* 3,  0, tt = Invoke            */   0x12U, 0x00U, 0x63U,
/* 5,  3, flags                  */   0x22U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 5,  8, sequence               */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 6,  13, firstledgersequence   */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,
/* 6,  19, lastledgersequence    */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,
/* 9,  25, fee                   */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 35,  34, signingpubkey        */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 22,  69, account              */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 22,  91, destination          */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 116, 113  emit details        */ 
/* 0,   229                      */ 
};
// clang-format on


// TX BUILDER 
#define FLS_OUT (txn + 15U) 
#define LLS_OUT (txn + 21U) 
#define FEE_OUT (txn + 26U) 
#define ACCOUNT_OUT (txn + 71U) 
#define DEST_OUT (txn + 93U) 
#define EMIT_OUT (txn + 113U) 

int64_t hook(uint32_t reserved) {

    uint8_t activity[20];
    otxn_field(SBUF(activity), sfAccount);
    hook_account(ACCOUNT_OUT, 20);   

    uint64_t amt_param;
    if (hook_param(SVAR(amt_param), "A", 1) != 8)
        NOPE("Topup: Misconfigured. Minimum Amount 'A' not set as Hook parameter.");    

    if (hook_param(DEST_OUT, 20, "D", 1) != 20)
        NOPE("Topup: Misconfigured. Account to pull the payment from is not set: 'D' not set as Hook parameter.");         

    if(BUFFER_EQUAL_20(activity, ACCOUNT_OUT)) {
        uint8_t keylet[34];
        if (util_keylet(SBUF(keylet), KEYLET_ACCOUNT, ACCOUNT_OUT, 20, 0, 0, 0, 0) != 34)
            NOPE("Topup: Fetching Keylet Failed.");

        slot_set(SBUF(keylet), 1);
        slot_subfield(1, sfBalance, 1);
        int64_t balance = slot_float(1);

        uint8_t amount[8];
        if(otxn_field(SBUF(amount), sfAmount) != 8)
         DONE("Topup: Non XAH Transaction Successful.");     

        uint64_t otxn_drops = AMOUNT_TO_DROPS(amount);
        int64_t amount_xfl = float_set(-6, otxn_drops);             

        int64_t final =  float_sum(balance, float_negate(amount_xfl));

        if(float_compare(final, amt_param, COMPARE_LESS) == 1) {
            etxn_reserve(1);
            uint32_t current_ledger = ledger_seq();
            uint32_t fls = current_ledger + 1;
            uint32_t lls = fls + 4;
            *((uint32_t *)(FLS_OUT)) = FLIP_ENDIAN(fls);
            *((uint32_t *)(LLS_OUT)) = FLIP_ENDIAN(lls);
            etxn_details(EMIT_OUT, 116U);
            {
                int64_t fee = etxn_fee_base(SBUF(txn));
                uint8_t *b = FEE_OUT;
                *b++ = 0b01000000 + ((fee >> 56) & 0b00111111);
                *b++ = (fee >> 48) & 0xFFU;
                *b++ = (fee >> 40) & 0xFFU;
                *b++ = (fee >> 32) & 0xFFU;
                *b++ = (fee >> 24) & 0xFFU;
                *b++ = (fee >> 16) & 0xFFU;
                *b++ = (fee >> 8) & 0xFFU;
                *b++ = (fee >> 0) & 0xFFU;
            }
            
            uint8_t emithash[32];  
            if (emit(SBUF(emithash), SBUF(txn)) != 32)
                NOPE("Topup: Failed To Emit.");

            DONE("Topup: Payment Request Sent Successfully."); 
        }     
    }

    DONE("Topup: Payment Successful.");    
    _g(1,1);
    return 0;          
}