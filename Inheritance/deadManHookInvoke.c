#include "hookapi.h"

#define DONE(x) accept(SBUF(x), __LINE__)
#define NOPE(x) rollback(SBUF(x), __LINE__)
uint8_t msg_buf[30] = "You must wait 0000000 ledgers.";

// clang-format off 
uint8_t txn[229] =
{
/* size,upto */
/* 3,  0, tt = SetRegularKey     */   0x12U, 0x00U, 0x05U,
/* 5,  3, flags                  */   0x22U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 5,  8, sequence               */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 6,  13, firstledgersequence   */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,
/* 6,  19, lastledgersequence    */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,
/* 9,  25, fee                   */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
/* 35,  34, signingpubkey        */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 22,  69, account              */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 22,  91, regularkey           */   0x88U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* 116, 113  emit details        */ 
/* 0,   229                      */ 
};
// clang-format on


// TX BUILDER 
#define FLS_OUT (txn + 15U) 
#define LLS_OUT (txn + 21U) 
#define FEE_OUT (txn + 26U) 
#define ACCOUNT_OUT (txn + 71U) 
#define RKEY_OUT (txn + 93U) 
#define EMIT_OUT (txn + 113U) 

int64_t hook(uint32_t reserved) {

    uint8_t flag[1];

    if (otxn_param(SBUF(flag), "F", 1) != 1)
        NOPE("Inheritance: No Action Specified.");

    TRACEHEX(flag);

    uint32_t current_ledger = ledger_seq();
    uint32_t fls = current_ledger + 1;
    uint32_t lls = fls + 4;
    etxn_reserve(1);
    uint8_t emithash[32];    


    uint8_t trigger_acc[20];
    otxn_field(SBUF(trigger_acc), sfAccount);
    hook_account(ACCOUNT_OUT, 20);

    if( flag[0] == 0x00U )
    {
            if(!BUFFER_EQUAL_20(trigger_acc, ACCOUNT_OUT)) 
                NOPE("Inheritance: You are not authorized to do this.");

            uint8_t primary[20];
            if (otxn_param(SBUF(primary), "P", 1) != 20)
                NOPE("Inheritance: Specify The Primary Nominee.");

            if (state_set(SBUF(primary), "P", 1) != 20)
                NOPE("Inheritance: Could not update primary nominee state entry, bailing.");

            uint8_t second[20];
            if (otxn_param(SBUF(second), "S", 1) != 20)
                NOPE("Inheritance: Specify The Second Nominee.");

            if (state_set(SBUF(second), "S", 1) != 20)
                NOPE("Inheritance: Could not update second nominee state entry, bailing."); 

            if(BUFFER_EQUAL_20(primary, second)) 
                NOPE("Inheritance: You can't repeat nominees.");                         

            uint8_t third[20];
            if (otxn_param(SBUF(third), "T", 1) != 20)
                NOPE("Inheritance: Specify The Third Nominee.");  

            if (state_set(SBUF(third), "T", 1) != 20)
                NOPE("Inheritance: Could not update third nominee state entry, bailing.");  


            if(BUFFER_EQUAL_20(primary, third) || BUFFER_EQUAL_20(second, third)) 
                NOPE("Inheritance: You can't repeat nominees.");                   

            uint32_t primary_interval;
            if (otxn_param(SVAR(primary_interval), "L1", 2) != 4)
                NOPE("Inheritance: Specify The Primary Ledger Interval.");  

            if (state_set(SVAR(primary_interval), "L1", 2) != 4)
                NOPE("Inheritance: Could not update ledger interval state entry, bailing.");  

            uint32_t secondary_interval;
            if (otxn_param(SVAR(secondary_interval), "L2", 2) != 4)
                NOPE("Inheritance: Specify The Ledger Interval for non-primary nominees.");  

            if (state_set(SVAR(secondary_interval), "L2", 2) != 4)
                NOPE("Inheritance: Could not update ledger interval state entry, bailing.");   

            if(primary_interval >= secondary_interval) 
                NOPE("Inheritance: Non-primary nominee ledger interval has to be greater than the primary nominee ledger interval.");

            if (state_set(SVAR(current_ledger), "LAST", 4) != 4)
                NOPE("Inheritance: Could not set recent activity state entry, bailing.");                    

            DONE("Inheritance: Nominees and config set for your account.");
    }


    uint32_t ledgr_interval;
    uint32_t last;
    if (state(SVAR(last), "LAST", 4) != 4)
        NOPE("Inheritance: Could not retrieve last interaction state entry, bailing.");    

   if( flag[0] == 0x01U ) {
        if (state(SVAR(ledgr_interval), "L1", 2) != 4)
            NOPE("Inheritance: Could not retrieve ledger interval state entry, bailing.");  
TRACEVAR(ledgr_interval);
        if (state(RKEY_OUT, 20, "P", 1) != 20)
            NOPE("Inheritance: Could not retrieve primary nominee state entry, bailing."); 
                            
        if(BUFFER_EQUAL_20(trigger_acc, RKEY_OUT)) {
            uint32_t lgr_elapsed = last + ledgr_interval;
            if (lgr_elapsed < current_ledger)
            {
                lgr_elapsed = lgr_elapsed - current_ledger;
                msg_buf[14] += (lgr_elapsed / 1000000) % 10;
                msg_buf[15] += (lgr_elapsed / 100000) % 10;
                msg_buf[16] += (lgr_elapsed / 10000) % 10;
                msg_buf[17] += (lgr_elapsed / 1000) % 10;
                msg_buf[18] += (lgr_elapsed / 100) % 10;
                msg_buf[19] += (lgr_elapsed / 10) % 10;
                msg_buf[20] += (lgr_elapsed) % 10;
                NOPE(msg_buf);
            }
        }  else {
            NOPE("Inheritance: You are not authorized to take this action.");
        }       
   }

    if (state(SVAR(ledgr_interval), "L2", 2) != 4)
        NOPE("Inheritance: Could not retrieve ledger interval state entry, bailing.");    

   if( flag[0] == 0x02U ) {
 
        if (state(RKEY_OUT, 20, "S", 1) != 20)
            NOPE("Inheritance: Could not retrieve second nominee state entry, bailing."); 

        if(BUFFER_EQUAL_20(trigger_acc, RKEY_OUT)) {
            uint32_t lgr_elapsed = last + ledgr_interval;
            if (lgr_elapsed < current_ledger)
            {
                lgr_elapsed = lgr_elapsed - current_ledger;
                msg_buf[14] += (lgr_elapsed / 1000000) % 10;
                msg_buf[15] += (lgr_elapsed / 100000) % 10;
                msg_buf[16] += (lgr_elapsed / 10000) % 10;
                msg_buf[17] += (lgr_elapsed / 1000) % 10;
                msg_buf[18] += (lgr_elapsed / 100) % 10;
                msg_buf[19] += (lgr_elapsed / 10) % 10;
                msg_buf[20] += (lgr_elapsed) % 10;
                NOPE(msg_buf);
            }
        } else {
            NOPE("Inheritance: You are not authorized to take this action.");
        }           
   }

   if(flag[0] == 0x03U ) {
        if (state(RKEY_OUT, 20, "T", 1) != 20)
            NOPE("Inheritance: Could not retrieve third nominee state entry, bailing.");   

        if(BUFFER_EQUAL_20(trigger_acc, RKEY_OUT)) {
            uint32_t lgr_elapsed = last + ledgr_interval;
            if (lgr_elapsed < current_ledger)
            {
                lgr_elapsed = lgr_elapsed - current_ledger;
                msg_buf[14] += (lgr_elapsed / 1000000) % 10;
                msg_buf[15] += (lgr_elapsed / 100000) % 10;
                msg_buf[16] += (lgr_elapsed / 10000) % 10;
                msg_buf[17] += (lgr_elapsed / 1000) % 10;
                msg_buf[18] += (lgr_elapsed / 100) % 10;
                msg_buf[19] += (lgr_elapsed / 10) % 10;
                msg_buf[20] += (lgr_elapsed) % 10;
                NOPE(msg_buf);
            }
        } else {
            NOPE("Inheritance: You are not authorized to take this action.");
        }           
   }


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
    TRACEHEX(txn);
    if (emit(SBUF(emithash), SBUF(txn)) != 32)
        NOPE("Inheritance: Failed To Emit.");                              


    DONE("Inheritance: Ledger Interval Set.");    
    _g(1,1);
    return 0;       
}