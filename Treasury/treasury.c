#include "hookapi.h"

#define OTXN_AMT_TO_XFL(buf) float_set(-6, (AMOUNT_TO_DROPS(buf)))

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

#define FLS_OUT    (txn + 15U) 
#define LLS_OUT    (txn + 21U) 
#define FEE_OUT    (txn + 35U) 
#define AMOUNT_OUT (txn + 26U)
#define ACC_OUT    (txn + 80U) 
#define DEST_OUT   (txn + 102U) 
#define EMIT_OUT   (txn + 122U) 

int64_t hook(uint32_t reserved)
{  
    uint64_t limit_amt;
    if(hook_param(SVAR(limit_amt), "A", 1) != 8) 
        rollback(SBUF("Treasury: Misconfigured. Amount 'A' not set as Hook parameter"), 1);

    int32_t limit_ledger;
    if(hook_param(SVAR(limit_ledger), "L", 1) != 4)
        rollback(SBUF("Treasury: Misconfigured. Ledger limit 'L' not set as Hook parameter"), 2);

    if(hook_param(DEST_OUT, 20, "D", 1) != 20)
        rollback(SBUF("Treasury: Misconfigured. Destination 'D' not set as Hook parameter"), 3);        

    int64_t type = otxn_type(); 
    if (type == ttCLAIM_REWARD)
        accept(SBUF("Treasury: ClaimReward Successful."), 4);

    if (type != ttPAYMENT)
        rollback(SBUF("Treasury: Only ClaimReward and Payment txns are allowed."), 5);

    if(otxn_field(AMOUNT_OUT, 8, sfAmount) != 8) 
        rollback(SBUF("Treasury: Non XAH currency payments are forbidden."), 6);

    uint8_t account[20];
    otxn_field(SBUF(account), sfAccount);

    hook_account(ACC_OUT, 20);
    if(!BUFFER_EQUAL_20(ACC_OUT, account)) 
        accept(SBUF("Treasury: Incoming Transaction."), 7);

    int32_t last_release = 0;
    state(SVAR(last_release), "LAST", 4);

    int32_t current_ledger =  ledger_seq();
    if ((last_release + limit_ledger) > current_ledger)
        rollback(SBUF("Treasury: You need to wait longer to withdraw."), 8);

    int64_t amount_xfl = OTXN_AMT_TO_XFL(AMOUNT_OUT);  
    if(float_compare(amount_xfl, limit_amt, COMPARE_GREATER) == 1)
        rollback(SBUF("Treasury: Outgoing transaction exceeds the limit set by you."), 9);

    etxn_reserve(1);
        
    uint32_t fls = (uint32_t)current_ledger + 1;
    *((uint32_t *)(FLS_OUT)) = FLIP_ENDIAN(fls);

    uint32_t lls = fls + 4;
    *((uint32_t *)(LLS_OUT)) = FLIP_ENDIAN(lls);

    etxn_details(EMIT_OUT, 116U);

    // TXN PREPARE: Fee
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
        rollback(SBUF("Treasury: Failed To Emit."), 10);          

    if (state_set(SVAR(current_ledger), "LAST", 4) != 4)
        rollback(SBUF("Treasury: Could not update state entry, bailing."), 11);

    accept(SBUF("Treasury: Released successfully."), 12);
    _g(1,1);
    return 0;    
}