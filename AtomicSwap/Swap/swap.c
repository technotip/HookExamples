#include "hookapi.h"

#define FLIP_ENDIAN(n) ((uint32_t) (((n & 0xFFU) << 24U) | \
                                   ((n & 0xFF00U) << 8U) | \
                                 ((n & 0xFF0000U) >> 8U) | \
                                ((n & 0xFF000000U) >> 24U)))

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

uint8_t txn[312] =
{
    /* size,upto */
    /* 3,  0, tt = Payment           */   0x12U, 0x00U, 0x00U,
    /* 5,  3, flags                  */   0x22U, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 5,  8, sequence               */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 6,  13, firstledgersequence   */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 6,  19, lastledgersequence    */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 34,  25, invoiceid            */   0x50U, 0x11U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    
    /*  49, 59  amount               */   0x61U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,                         
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U,
                                        0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99U, 0x99,
    /* 9,   108,  fee                 */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    /* 35,  117, signingpubkey        */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* 22,  152, account             */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* 22,  174, destination         */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /* 116, 196  emit details        */ 
    /* 0,   312                      */ 
};

// TX BUILDER
#define FLS_OUT    (txn + 15U) 
#define LLS_OUT    (txn + 21U) 
#define FEE_OUT    (txn + 109U) 
#define AMOUNT_OUT (txn + 59U)
#define HOOK_ACC   (txn + 154U)
#define DEST_ACC   (txn + 176U)
#define EMIT_OUT   (txn + 196U) 
#define INVOICE_ID_OUT (txn + 27U) 


int64_t hook(uint32_t reserved) {
    uint32_t current_ledger = ledger_seq();

    uint8_t eur_issuer[20];
    if(hook_param(SBUF(eur_issuer), "EUR_I", 5) != 20)
        NOPE("Misconfigured. EUR issuer not set as Hook Parameter.");    

    uint8_t eur_currency[20];
    if(hook_param(SBUF(eur_currency), "EUR_C", 5) != 20)
        NOPE("Misconfigured. EUR currency not set as Hook Parameter.");            

    uint8_t etb_issuer[20];
    if(hook_param(SBUF(etb_issuer), "ETB_I", 5) != 20)
        NOPE("Misconfigured. ETB issuer not set as Hook Parameter.");    

    uint8_t etb_currency[20];
    if(hook_param(SBUF(etb_currency), "ETB_C", 5) != 20)
        NOPE("Misconfigured. ETB currency not set as Hook Parameter.");               

    uint64_t conversion_rate;
    if(hook_param(SVAR(conversion_rate), "R", 1) != 8)
        NOPE("Misconfigured. Conversion rate not set as Hook Parameter.");   

    uint8_t eur_whitelisted_accounts[20];
    if(hook_param(SBUF(eur_whitelisted_accounts), "EUR_W", 5) != 20)
        NOPE("Misconfigured. Whitelisted accounts not set as Hook Parameter.");   

    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);
    hook_account(HOOK_ACC, 20);

    if (BUFFER_EQUAL_20(HOOK_ACC, account)) 
        DONE("Outgoing Transaction.");    

    if(otxn_field(INVOICE_ID_OUT, 32, sfInvoiceID) != 32)
        NOPE("No Invoice ID passed.");    // Check if we have to just accept this instead.

    uint8_t amount[48];
    if(otxn_field(amount, 48, sfAmount) != 48) // < xlf 8b req amount, 20b currency, 20b issuer >       
        DONE("Probably XAH Transaction.");

    int64_t amount_xfl = float_sto_set(amount, 8); 

    if(BUFFER_EQUAL_20(eur_whitelisted_accounts, account)){
        if (!BUFFER_EQUAL_20(amount + 28, eur_issuer)) 
            DONE("Incoming Transaction.");  
        
        if (!BUFFER_EQUAL_20(amount + 8, eur_currency)) 
            DONE("Incoming Transaction.");   

        int64_t swap_amount = float_multiply(amount_xfl, conversion_rate);

        if(hook_param(DEST_ACC, 20, "ETB_W", 5) != 20)
            NOPE("Misconfigured. Whitelisted account not set as Hook Parameter.");           

        if(float_sto(AMOUNT_OUT,  49, etb_currency, 20, DEST_ACC, 20, swap_amount, sfAmount) < 0) 
            rollback(SBUF("Atomic Swap: Wrong AMT - < xlf 8b req amount, 20b currency, 20b issuer >"), 7);            
    } else if(BUFFER_EQUAL_20(etb_issuer, account)){
        if (!BUFFER_EQUAL_20(amount + 28, etb_issuer)) 
            DONE("Incoming Transaction.");  
        
        if (!BUFFER_EQUAL_20(amount + 8, etb_currency)) 
            DONE("Incoming Transaction.");   

        int64_t swap_amount = float_divide(amount_xfl, conversion_rate);

        if(hook_param(DEST_ACC, 20, "EUR_W", 5) != 20)
            NOPE("Misconfigured. Whitelisted accounts not set as Hook Parameter.");           

        if(float_sto(AMOUNT_OUT,  49, eur_currency, 20, eur_issuer, 20, swap_amount, sfAmount) < 0) 
            rollback(SBUF("Atomic Swap: Wrong AMT - < xlf 8b req amount, 20b currency, 20b issuer >"), 7);            
    } else {
        DONE("Incoming Transaction.");
    }


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
    TRACEHEX(txn);
    uint8_t emithash[32]; 
    if(emit(SBUF(emithash), SBUF(txn)) != 32)
        rollback(SBUF("Failed To Emit."), 12);           

    DONE("Atomic Swap Successful.");   
    _g(1,1);
    return 0;    
}