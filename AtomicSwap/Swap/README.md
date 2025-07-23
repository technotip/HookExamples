# AtomicSwap Hook

This hook enables atomic swaps from EUR to ETB, on the Xahau network.

There are 2 hooks:
Hook 1: Triggered on Invoke Transaction. This is to set the "Conversion Rate".
Hook 2: For the actual Atomic Swap.

## Important

The account on which the hook is installed must have ETB balance. If sufficient ETB balance is not there for the swap then the transaction will be rejected. The hook account accepts ETB from the ETB issuer only.

### Hook 1 ( Invoke Transaction )

While installing this hook, make sure to pass the `R_ACC` as hook parameter. Here you need to pass 20 byte account ID of the account which will be setting the conversion rate - whenever required. No other accounts can set the conversion rate.

So while setting the conversion rate, send an Invoke transaction with transaction parameter `R` and XFL value. For testing you can use this tool for the conversion: [Float To XFL](https://hooks.services/tools/float-to-xfl).

```
import {
  floatToLEXfl,
} from "@transia/binary-models";

const R = floatToLEXfl("5"); // Conversion Rate. Ex: 5
console.log(R);
```

### Hook 2 ( Payment Transaction )

When installing the hook 2, you must provide the following parameters:

| Name  | Size | Example Value                              | Description             |
| ----- | ---- | ------------------------------------------ | ----------------------- |
| EUR_I | 20B  | `A9846C3AA735E0FF20B6031689D76937F0328EB2` | EUR Issuer AccountID    |
| EUR_C | 20B  | `0000000000000000000000004555520000000000` | EUR Currency Code (hex) |
| ETB_I | 20B  | `3F2E26FAFA9240BEC339490CE74BE337BFC1EF4B` | ETB Issuer AccountID    |
| ETB_C | 20B  | `0000000000000000000000004554420000000000` | ETB Currency Code (hex) |
| ETB_W | 20B  | `3F2E26FAFA9240BEC339490CE74BE337BFC1EF4B` | ETB Whitelisted Account |

## How it Works

EUR to ETB Swap:

- Only the whitelisted EUR account can send EUR to the hook account - whitelisting is done natively using `DepositPreauth` on the hook account.
- When a valid EUR payment is received:
  - The amount is converted to ETB using the provided conversion rate ( This should be set via Hook 1 - "R" )
  - A new payment transaction is constructed, sending the equivalent ETB amount to the ETB whitelisted account ("ETB_W" - hook parameter). For now, ETB_W is the issuer of ETB, hence ETB gets burnt.
  - The hook emits this transaction for processing

All incoming transactions must include an InvoiceID, which is carried forward to the emitted transaction for reference and tracking purposes.

- Outgoing transactions from the hook account are simply accepted.
- If the transaction does not match the expected currency/issuer or lacks an InvoiceID, it is rejected

## Useful Tools

- **Conversion rate**: Use [float-to-xfl](https://hooks.services/tools/float-to-xfl) to convert a float to 8-byte XFL hex
- **Currency code**: Use [currency-code-to-hex](https://hooks.services/tools/currency-code-to-hex) to convert a 3-letter code to 20-byte hex
- **AccountID**: Use [raddress-to-accountid](https://hooks.services/tools/raddress-to-accountid) to convert an r-address to a 20-byte AccountID

## Error Handling

- If any parameter is missing or misconfigured, the hook will reject the transaction
- If the transaction lacks an InvoiceID, it is rejected.
- If the transaction currency/issuer doesn't match the configured parameters, it is rejected.
