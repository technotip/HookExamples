### Treasury funds: hook

Voluntarily lock up the amount of **XAH** going out from the treasury account every set ledger interval.

### Installing the Hook on your account

```
import { Client, Wallet, SetHookFlags } from "@transia/xrpl";
import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
} from "@transia/hooks-toolkit";

export async function main(): Promise<void> {
  const serverUrl = "wss://xahau-test.net";   // Xahau Test Network
  const client = new Client(serverUrl);
  await client.connect();
  client.networkID = await client.getNetworkID();
  const secret = "" // Your account secret
  const hookWallet = Wallet.fromSeed(secret);

  const amount_in_xfl = "0080C6A47E8D0356"; // equivalent to Decimal: 1M

  const hook1 = createHookPayload({
    version: 0,
    createFile: "treasury",
    namespace: "TREASURY",
    flags: SetHookFlags.hsfOverride,
    "HookParameters": [
        HookParameter: {
                    HookParameterName: "41", // 41 is the hex value for 'A'
                    HookParameterValue: amount_in_xfl
        },
        HookParameter: {
                    HookParameterName: "4C", // 4C is the hex value for 'L'
                    HookParameterValue: "64000000" // 100 in decimal. This is the ledger interval after which the payment can be done (explained below).
        },
        HookParameter: {
                    HookParameterName: "44", // 44 is the hex value for 'D'
                    HookParameterValue: "B675B3DE010A7C7A2DF655C54C284A2113FFD76B" // account ID of rHdkzpxr3VfabJh9tUEDv7N4DJEsA4UioT
        }
    ],
    hookOnArray: ["Payment", "EscrowCreate", "OfferCreate", "ClaimReward", "PaymentChannelCreate", "URITokenCreate", "NFTCreate", "NFTAcceptOffer", "SetHook"], // HookOn everything except GenesisMint.
  });

  const hook2 = createHookPayload({
    version: 0,
    createFile: "genesisMint",
    namespace: "GENESISMINT",
    flags: SetHookFlags.hsfOverride,
    "HookParameters": [
        HookParameter: {
                    HookParameterName: "44", // 44 is the hex value for 'D'
                    HookParameterValue: "B675B3DE010A7C7A2DF655C54C284A2113FFD76B" // account ID of rHdkzpxr3VfabJh9tUEDv7N4DJEsA4UioT
        }
    ],
    hookOnArray: ["GenesisMint"],
  });


  await setHooksV3({
    client: client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }, { Hook: hook2 }],
  } as SetHookParams);

  await client.disconnect();
}

main();
```

### Converting ledger interval of 100 to 4 byte little endian uint32

1. Represent 100 in hexadecimal:
   Decimal 100 is 0x64 in hexadecimal.

2. Pad the hexadecimal representation to 4 bytes:
   A 4-byte (32-bit) value requires 8 hex digits. So, we pad 0x64 to 0x00000064.
3. Arrange the bytes in little-endian order (least significant byte first):
   Original 4-byte hex value: 00 00 00 64
   Little-endian order: 64 00 00 00

   Thus, 100 in little-endian 4-byte uint32 is represented as: `64 00 00 00` i.e., `64000000`

### How it works

1. A person with Account A can create another account B, and install the treasury hook on account B, with account A as the hook parameter(0x44U or D for destination account). Other hook parameters: limit amount to withdraw and the ledger interval to withdraw the limit amount.

2. Now the person can transfer all his/her funds to account B (lets call it treasury account).

3. Since treasury account has all the funds, it can also claim monthly rewards. The monthly rewards will be immediately transferred to account A.
   And account A can also withdraw set amount(set as hook parameter - 0x41U or A) from treasury account every specified ledger interval(set as hook parameter - 0x4CU or L).

This way the amount of XAH inflow into the market can be predictable.