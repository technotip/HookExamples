# Dynamic Issuance Hook

A Xahau hook that allows dynamic issuance of ETB tokens/IOU through invoke transactions.

## Setup

### 1. Configure Issuer Account ID

Enter a valid issuer account ID for `etb_issuer` using the tool here: [raddress to accountid](https://hooks.services/tools/raddress-to-accountid)

### 2. Install the Hook

Make sure to install this hook on the issuer account. The hook should be configured to trigger on **Invoke** transaction.

#### Required Hook Parameter

While installing the hook, make sure to include the following hook parameter:

- **`ISSUER`** - Set the token issuer account ID. 20 bytes.
- **`IOU`** - Set the currency to be issued by the issuer. 20 bytes.

- **`W_ACC`** - This is a whitelisted account ID that can trigger the issuer with an invoke call to issue more ETB tokens/IOU.
- **`D_ACC`** - This is a whitelisted account ID that will receive the ETB tokens/IOU when **W_ACC** invokes the issuer account..

## Usage

### Dynamic Token Issuance

Once the hook is installed, here's how to dynamically issue more ETB tokens:

1. The whitelisted account sends an **invoke transaction** to the issuer (using the issuer account as the destination account)
2. Pass the number of tokens to be issued as a parameter using: [float to xfl](https://hooks.services/tools/float-to-xfl)
3. The issuer will automatically issue the specified number of tokens
4. And the tokens will be sent to the account whitelisted while installing the hook **D_ACC**

That's it! The issuer will issue the requested tokens based on the invoke transaction parameters.

## Tools

- [Account ID Converter](https://hooks.services/tools/raddress-to-accountid) - Convert addresses to account IDs
- [Float to XFL Converter](https://hooks.services/tools/float-to-xfl) - Convert float values to XFL format for parameters
