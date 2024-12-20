#include "hookapi.h"

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

#define COPY_ACCOUNT(buf_raw, i)                       \
{                                                    \
    unsigned char *buf = (unsigned char *)buf_raw;   \
    *(uint64_t *)(buf + 0) = *(uint64_t *)(i + 0);   \
    *(uint64_t *)(buf + 8) = *(uint64_t *)(i + 8);   \
    *(uint32_t *)(buf + 16) = *(uint32_t *)(i + 16); \
}

// clang-format off
uint8_t txn[238] =
{
/* size,upto */
/* 3,   0,   tt = Payment           */   0x12U, 0x00U, 0x00U,
/* 5,   3,   flags                  */   0x22U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 5,   8,   sequence               */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 6,   13,  firstledgersequence    */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,
/* 6,   19,  lastledgersequence     */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,
/* 9,   25,  amount                 */   0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
/* 9,   34,  fee                    */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 35,  43,  signingpubkey          */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 22,  78,  account                */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 22,  100, destination            */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 116, 122  emit details           */ 
/* 0,   238                         */ 
};
// clang-format on

#define FLS_OUT (txn + 15U)
#define LLS_OUT (txn + 21U)
#define FEE_OUT (txn + 35U)
#define AMOUNT_OUT (txn + 26U)
#define ACC_OUT (txn + 80U)
#define DEST_OUT (txn + 102U)
#define EMIT_OUT (txn + 122U)

int64_t hook(uint32_t reserved) {

    uint64_t amt_param;
    uint8_t activity[20];
    otxn_field(SBUF(activity), sfAccount);
    hook_account(ACC_OUT, 20);   

    uint64_t dest_balance;
    if (hook_param(SVAR(dest_balance), "B", 1) != 8)
        NOPE("Topup: Misconfigured. Balance of the requesting account, while requesting.");    

    if(BUFFER_EQUAL_20(activity, ACC_OUT)) {
        uint64_t amount;
        if (otxn_param(SVAR(amount), "A", 1) != 8)
            NOPE("Topup: Specify the amount you want to send to the destination.");

        uint8_t des_acc[20];
        if (otxn_param(SBUF(des_acc), "D", 1) != 20)
            NOPE("Topup: Specify The Account Which Can Request Amount From You.");

        uint8_t keylet[34];
        if (util_keylet(keylet, 34, KEYLET_ACCOUNT, SBUF(des_acc), 0, 0, 0, 0) != 34)
            NOPE("Topup: Fetching Keylet Failed.");

        if (slot_set(SBUF(keylet), 1) == DOESNT_EXIST)
            NOPE("Topup: The Account you are trying to configure does not exist.");             

        if (state_set(SVAR(amount), SBUF(des_acc)) != 8)
            NOPE("Topup: Could not set the requester account state entry, bailing.");   

        DONE("Topup: Configured Successfully.");  
    } else {
        if (state(SVAR(amt_param), SBUF(activity)) == 8)
            COPY_ACCOUNT(DEST_OUT, activity);
    }

    if(BUFFER_EQUAL_20(activity, DEST_OUT)) {
        uint32_t current_ledger = ledger_seq();
        uint8_t keylet[34];
        if (util_keylet(SBUF(keylet), KEYLET_ACCOUNT, DEST_OUT, 20, 0, 0, 0, 0) != 34)
            NOPE("Topup: Fetching Keylet Failed.");

        slot_set(SBUF(keylet), 1);
        slot_subfield(1, sfBalance, 1);
        int64_t balance = slot_float(1);
        
        if(float_compare(balance, dest_balance, COMPARE_LESS) == 1) {
            {
                uint64_t drops = float_int(amt_param, 6, 1); 
                uint8_t *b = AMOUNT_OUT; 
                *b++ = 0b01000000 + ((drops >> 56) & 0b00111111); 
                *b++ = (drops >> 48) & 0xFFU; 
                *b++ = (drops >> 40) & 0xFFU; 
                *b++ = (drops >> 32) & 0xFFU; 
                *b++ = (drops >> 24) & 0xFFU; 
                *b++ = (drops >> 16) & 0xFFU; 
                *b++ = (drops >> 8) & 0xFFU; 
                *b++ = (drops >> 0) & 0xFFU; 
            }           
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

            DONE("Topup: Topup fulfilled successfully."); 
        } else 
            NOPE("Topup: Not yet eligible to request topup.");
    } else 
        NOPE("Topup: Unauthorized topup request.");

    DONE("Topup: Successful.");    
    _g(1,1);
    return 0;          
}