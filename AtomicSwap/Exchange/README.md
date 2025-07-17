# AtomicSwap Hook

This hook enables atomic swaps between EUR and ETB currencies on the Xahau, using custom issuers and a fixed conversion rate.

## Important

The account on which the hook is installed must have ETB balance.

## Hook Parameters

When installing the hook, you must provide the following parameters:

| Name  | Size | Example Value                              | Description             |
| ----- | ---- | ------------------------------------------ | ----------------------- |
| EUR_I | 20B  | `A9846C3AA735E0FF20B6031689D76937F0328EB2` | EUR Issuer AccountID    |
| EUR_C | 20B  | `0000000000000000000000004555520000000000` | EUR Currency Code (hex) |
| ETB_I | 20B  | `3F2E26FAFA9240BEC339490CE74BE337BFC1EF4B` | ETB Issuer AccountID    |
| ETB_C | 20B  | `0000000000000000000000004554420000000000` | ETB Currency Code (hex) |
| R     | 8B   | `0080E03779C39154`                         | Conversion Rate (XFL)   |

## How it Works

- The hook listens for incoming payments in EUR (with the specified issuer and currency).
- When a payment is detected:
  - The amount is converted to ETB using the provided conversion rate.
  - A new payment transaction is constructed, sending the equivalent ETB amount (with the specified issuer and currency) to the original sender.
  - The hook emits this transaction for processing.
- Outgoing transactions from the hook account are ignored.
- If the transaction does not match the expected EUR issuer/currency, it is ignored.

**Note:**

If there is not enough ETB balance, then the EUR will be sent back to the original sender.

## Useful Tools

- **Conversion rate**: Use [float-to-xfl](https://hooks.services/tools/float-to-xfl) to convert a float to 8-byte XFL hex.
- **Currency code**: Use [currency-code-to-hex](https://hooks.services/tools/currency-code-to-hex) to convert a 3-letter code to 20-byte hex.
- **AccountID**: Use [raddress-to-accountid](https://hooks.services/tools/raddress-to-accountid) to convert an r-address to a 20-byte AccountID.

## Error Handling

- If any parameter is missing or misconfigured, the hook will reject the transaction.
- If the transaction is not a matching EUR payment, it is ignored.
