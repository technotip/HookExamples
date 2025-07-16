#include "hookapi.h"

#define FLIP_ENDIAN(n) ((uint32_t) (((n & 0xFFU) << 24U) | \
                                   ((n & 0xFF00U) << 8U) | \
                                 ((n & 0xFF0000U) >> 8U) | \
                                ((n & 0xFF000000U) >> 24U)))

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

uint8_t txn[278] =
{
    /* size,upto */
    /* 3,  0, tt = Payment           */   0x12U, 0x00U, 0x00U,
    /* 5,  3, flags                  */   0x22U, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 5,  8, sequence               */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 6,  13, firstledgersequence   */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 6,  19, lastledgersequence    */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,
    /*  49, 25  amount               */   0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                         
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99,
    /* 9,   74,  fee                 */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 35,  83, signingpubkey        */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* 22,  118, account             */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* 22,  140, destination         */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* 116, 162  emit details        */ 
    /* 0,   278                      */ 
};

// TX BUILDER
#define FLS_OUT    (txn + 15U) 
#define LLS_OUT    (txn + 21U) 
#define FEE_OUT    (txn + 75U) 
#define AMOUNT_OUT (txn + 25U)
#define HOOK_ACC   (txn + 120U)
#define DEST_ACC   (txn + 142U)
#define EMIT_OUT   (txn + 162U) 


int64_t hook(uint32_t reserved) {
    uint32_t current_ledger = ledger_seq();

    uint8_t eur_issuer[20];
    if(hook_param(SBUF(eur_issuer), "EUR_I", 5) != 20)
        NOPE("Misconfigured. EUR issuer not set as Hook Parameter.");    

    uint8_t eur_currency[20];
    if(hook_param(SBUF(eur_currency), "EUR_C", 5) != 20)
        NOPE("Misconfigured. EUR currency not set as Hook Parameter.");            

   // uint8_t etb_issuer[20];
    if(hook_param(DEST_ACC, 20, "ETB_I", 5) != 20)
        NOPE("Misconfigured. ETB issuer not set as Hook Parameter.");    

    uint8_t etb_currency[20];
    if(hook_param(SBUF(etb_currency), "ETB_C", 5) != 20)
        NOPE("Misconfigured. ETB currency not set as Hook Parameter.");               

    uint64_t conversion_rate;
    if(hook_param(SVAR(conversion_rate), "R", 1) != 8)
        NOPE("Misconfigured. Conversion rate not set as Hook Parameter.");   

    uint8_t dest_acc[20];
    otxn_field(SBUF(dest_acc), sfAccount);
    hook_account(HOOK_ACC, 20);

    if (BUFFER_EQUAL_20(HOOK_ACC, dest_acc)) 
        DONE("Outgoing Transaction.");

    uint8_t amount[48];
    otxn_field(amount, 48, sfAmount); // < xlf 8b req amount, 20b currency, 20b issuer >

    if (!BUFFER_EQUAL_20(amount + 28, eur_issuer)) 
        DONE("Incoming Transaction.");  
        
    if (!BUFFER_EQUAL_20(amount + 8, eur_currency)) 
        DONE("Incoming Transaction.");           

    int64_t amount_xfl = float_sto_set(amount, 8);

    int64_t swap_amount = float_multiply(amount_xfl, conversion_rate);

    if(float_sto(AMOUNT_OUT,  49, etb_currency, 20, DEST_ACC, 20, swap_amount, sfAmount) < 0) 
        rollback(SBUF("Atomic Swap: Wrong AMT - < xlf 8b req amount, 20b currency, 20b issuer >"), 7);
                  
    etxn_reserve(1);
    uint32_t fls = (uint32_t)ledger_seq() + 1;
    *((uint32_t *)(FLS_OUT)) = FLIP_ENDIAN(fls);
    uint32_t lls = fls + 4;
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
    if(emit(SBUF(emithash), SBUF(txn)) != 32)
        rollback(SBUF("Failed To Emit."), 12);           

    DONE("Atomic Swap Successful.");   
    _g(1,1);
    return 0;    
}