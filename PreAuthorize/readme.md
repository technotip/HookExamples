### Back story
This involves use of centralized server/backend to generate a signature.
The explanation below assumes use of an xApp in Xaman.

https://x.com/Satish_nl/status/1831962572297531847

### How it works
1. Generate a random private key and public key. Set the public key as hook parameter = 33 bytes.
2. When a user initiates a payment via your xApp: use an endpoint to generate a hex blob(signature + payload).
3. Payload: User account ID = 20 bytes. DestinationTag = 4 bytes. Amount = 8 Bytes. Current Sequence no = 4 bytes. Total = 36 bytes.
4. Use the private key from step 1, and sign the payload of 36 bytes from step 3.
5. Now concatinate the 64 bytes signature followed by the 36 bytes payload. 64 + 36 = 100 bytes.
6. Send this 100 bytes back to the user, and use it as **HookParameter** in the Payment payload of the user.
7. Send the payment to the issuer address. Do not foget the **destination tag**. As destination tag represents different users.
8. Now the issuer account has a hook installed, which checks if the signature is valid, using the Publickey from step 1.

### Setting the Hook
Make sure to set a publicKey as HookParam while installing the hook. 
And use ParameterName as `{ 0x50U }` or `P`

### HookParameter (Transaction Parameter)
Key is `{ 0x49U }` or `I`

### Payload
```
const accountId = rippleAddressCodec.decodeAccountID(Account)
const payload = Buffer.concat([ accountId, destination_tag, amount_xfl, sequence_no ])
const signature = Buffer.from(sign(payload.toString('hex'), Voucher.privateKey), 'hex')

const Blob = Buffer.concat([ signature, payload ]).toString('hex')
```

### Utils:
[raddress to account id](https://github.com/technotip/HooksTools/blob/main/raddress-to-accountid.ts)

[float to XFL](https://github.com/technotip/HooksTools/blob/main/float-to-xfl.ts)
