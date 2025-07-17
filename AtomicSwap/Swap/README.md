# AtomicSwap Hook

This hook enables atomic swaps between EUR and ETB currencies on the Xahau, using custom issuers, whitelisted accounts, and a fixed conversion rate.

## Important

The account on which the hook is installed must have both EUR and ETB balances.

## Hook Parameters

When installing the hook, you must provide the following parameters:

| Name  | Size | Example Value                              | Description             |
| ----- | ---- | ------------------------------------------ | ----------------------- |
| EUR_I | 20B  | `A9846C3AA735E0FF20B6031689D76937F0328EB2` | EUR Issuer AccountID    |
| EUR_C | 20B  | `0000000000000000000000004555520000000000` | EUR Currency Code (hex) |
| ETB_I | 20B  | `3F2E26FAFA9240BEC339490CE74BE337BFC1EF4B` | ETB Issuer AccountID    |
| ETB_C | 20B  | `0000000000000000000000004554420000000000` | ETB Currency Code (hex) |
| R     | 8B   | `0080E03779C39154`                         | Conversion Rate (XFL)   |
| EUR_W | 20B  | `7FAABCA6FD4192F69FD3F43F1E24EB1951547BE1` | EUR Whitelisted Account |
| ETB_W | 20B  | `3F2E26FAFA9240BEC339490CE74BE337BFC1EF4B` | ETB Whitelisted Account |

## How it Works

The hook supports two types of swaps:

1. EUR to ETB Swap:

   - Only the whitelisted EUR account ( "EUR_W" - hook parameter) can send EUR to the hook account
   - When a valid EUR payment is received:
     - The amount is converted to ETB using the provided conversion rate ( Hook Parameter "R" )
     - A new payment transaction is constructed, sending the equivalent ETB amount to the ETB whitelisted account ("ETB_W" - hook parameter)
     - The hook emits this transaction for processing

2. ETB to EUR Swap:
   - When the ETB issuer sends ETB to the hook account:
     - The amount is converted to EUR using the provided conversion rate ( Hook Parameter "R" )
     - A new payment transaction is constructed, sending the equivalent EUR amount to the EUR whitelisted account ("EUR_W" - hook parameter)
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
- If the transaction lacks an InvoiceID, it is rejected
- If the sender is not authorized (not EUR_W for EUR payments or not ETB issuer for ETB payments), the transaction is simply accepted.
- If the transaction currency/issuer doesn't match the configured parameters, it is ignored
