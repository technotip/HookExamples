import { Client, Wallet, SetHookFlags } from "@transia/xrpl";
import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
  iHookParamValue,
  iHookParamName,
  iHookParamEntry,
} from "@transia/hooks-toolkit";

import { xrpAddressToHex, currencyToHex } from "@transia/binary-models";

export async function main(): Promise<void> {
  //const serverUrl = "wss://xahau.network";  // Xahau Main Network
  const serverUrl = "wss://xahau-test.net"; // Xahau Test Network
  const client = new Client(serverUrl);
  await client.connect();
  client.networkID = await client.getNetworkID();
  const secret = ""; // Your Secret Key

  const hookWallet = Wallet.fromSeed(secret);

  const R_ACC = xrpAddressToHex("rUAuppQ65tkqzh1ejP8iqqhVMEA2WQz4KR"); // Account which changes the conversion rate
  const IN_I = xrpAddressToHex("rGTKqKECp9AzJ4o3WiBRRQxnuBebeFatJG"); // Incoming Issuer. Ex: EUR
  const IN_C = currencyToHex("EUR");
  const OUT_I = xrpAddressToHex("ramhioTkJq2hFrULHyBwqDcyjt4tsqifMG"); // Outgoing Issuer. Ex: ETB
  const OUT_C = currencyToHex("ETB");
  const OUT_W = xrpAddressToHex("ramhioTkJq2hFrULHyBwqDcyjt4tsqifMG"); // Whitelisted Account. Ex: for now it's the same as OUT_I

  const param = new iHookParamEntry(
    new iHookParamName("R_ACC"),
    new iHookParamValue(R_ACC, true)
  );

  const param1 = new iHookParamEntry(
    new iHookParamName("IN_I"),
    new iHookParamValue(IN_I, true)
  );
  const param2 = new iHookParamEntry(
    new iHookParamName("IN_C"),
    new iHookParamValue(IN_C, true)
  );

  const param3 = new iHookParamEntry(
    new iHookParamName("OUT_I"),
    new iHookParamValue(OUT_I, true)
  );

  const param4 = new iHookParamEntry(
    new iHookParamName("OUT_C"),
    new iHookParamValue(OUT_C, true)
  );

  const param5 = new iHookParamEntry(
    new iHookParamName("OUT_W"),
    new iHookParamValue(OUT_W, true)
  );

  const hook1 = createHookPayload({
    hookHash:
      "A90D335BC1811C551FF05047B57CB3FECE5655B4D040A3AB96E33EAFDDC66491", // replace with proper hook hash
    flags: SetHookFlags.hsfOverride,
    hookParams: [param.toXrpl()],
    hookOnArray: ["Invoke"],
  });

  const hook2 = createHookPayload({
    hookHash:
      "230ACD076069D23D5D184DB71197D3718AEF6D37AA5D6CD15B143E6060BBD9DC", // replace with proper hook hash
    flags: SetHookFlags.hsfOverride,
    hookParams: [
      param1.toXrpl(),
      param2.toXrpl(),
      param3.toXrpl(),
      param4.toXrpl(),
      param5.toXrpl(),
    ],
    hookOnArray: ["Payment"],
  });

  await setHooksV3({
    client: client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }, { Hook: hook2 }],
  } as SetHookParams);

  await client.disconnect();
}

main();
