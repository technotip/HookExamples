#include "hookapi.h"

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)

#define DROPS 1000000

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

int64_t hook(uint32_t) {

    otxn_field(DEST_OUT, 20, sfAccount);

    hook_account(ACC_OUT, 20);
    if(BUFFER_EQUAL_20(ACC_OUT, DEST_OUT)) 
        DONE("Xahau Radio: Outgoing Transaction.");

    uint8_t amount[8];
    if(otxn_field(SBUF(amount), sfAmount) != 8)
        NOPE("Xahau Radio: Non XAH Transaction not allowed.");

   uint64_t otxn_drops = AMOUNT_TO_DROPS(amount);
   int64_t amount_xfl = float_set(-6, otxn_drops);

   if(float_compare(amount_xfl, 6089866696204910592, COMPARE_LESS) == 1)
       NOPE("Xahau Radio: Less than allowed( 1 XAH ) minimum.");


   if(float_compare(amount_xfl, 6089866696204910592, COMPARE_GREATER) == 1) {

    uint64_t fraction_part  = (otxn_drops % (uint64_t) DROPS);
    if(fraction_part == 0) 
        DONE("Xahau Radio: Successful Request.");

    {
        uint64_t drops = fraction_part; //float_int(tempState.amount_xfl, 6, 1); 
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

        uint32_t current_ledger =  ledger_seq();
        uint32_t fls = current_ledger + 1;
        uint32_t lls = fls + 4;
        etxn_reserve(1);

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
        if(emit(SBUF(emithash), SBUF(txn)) != 32)
            NOPE("Xahau Radio: Failed To Emit.");        

   }

    DONE("Xahau Radio: Successful Request.");

    _g(1,1);
    return 0;
}