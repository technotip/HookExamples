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
    /* 49, 25  amount               */   0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                         
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

    uint8_t currency[20];
    if(hook_param(SBUF(currency), "IOU", 3) != 20)
        NOPE("Misconfigured. Currency not set as Hook Parameter.");

    uint8_t invoke_acc[20];
    if(hook_param(SBUF(invoke_acc), "W_ACC", 5) != 20)
        NOPE("Misconfigured. Whitelist account not set as Hook Parameter.");    

    if(hook_param(DEST_ACC, 20, "D_ACC", 5) != 20)
        NOPE("Misconfigured. Destination account not set as Hook Parameter.");    

    uint8_t amount[8];
    if(otxn_param(SBUF(amount), "AMT", 3) != 8)
        NOPE("Misconfigured. Amount to issue not passed as otxn parameter.");            
 
    int64_t amount_xfl = *((int64_t*)(amount));        
        
    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);
    hook_account(HOOK_ACC, 20);

    if (BUFFER_EQUAL_20(HOOK_ACC, account)) 
        DONE("Outgoing Invoke Transaction.");   

    if (!BUFFER_EQUAL_20(account, invoke_acc)) 
        DONE("Some Incoming Transaction.");             

    if(float_sto(AMOUNT_OUT, 49, currency, 20, HOOK_ACC, 20, amount_xfl, sfAmount) < 0) 
        NOPE("Wrong AMT - < xlf 8b req amount, 20b currency, 20b issuer >");  

   
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
        DONE("Failed To Emit.");    
        
    TRACEHEX(txn);

    DONE("Additional tokens issued Successful.");   
    _g(1,1);
    return 0;    
}