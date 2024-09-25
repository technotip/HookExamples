### Lock up funds: hook

Voluntarily lock up the amount of **XAH** going out from the account every month. Also commit to only making one outgoing XAH transaction per month.

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
    createFile: "lockup",
    namespace: "LOCKUP",
    flags: SetHookFlags.hsfOverride,
    "HookParameters": [
        HookParameter: {
                    HookParameterName: "41", // 41 is the hex value for 'A'
                    HookParameterValue: amount_in_xfl
        },
        HookParameter: {
                    HookParameterName: "4C", // 4C is the hex value for 'L'
                    HookParameterValue: "00000064" // 100 in decimal. This is the ledger interval after which the payment can be done
        }
    ],
    hookOnArray: ["Payment", "EscrowCreate", "OfferCreate"],
  });

  await setHooksV3({
    client: client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }],
  } as SetHookParams);

  await client.disconnect();
}

main();
```
