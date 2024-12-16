# Smart Contract: Nominee Management System for Account Access

## Overview

This smart contract system is designed to manage account access through a **nominee hierarchy**. It allows users to designate up to three nominees (primary, second, and third) who can take over the control of a designated account (the "hook account") after specific ledger intervals.

---

## Video Tutorial

[Inheritance Hook Tutorial](https://www.youtube.com/watch?v=XgQx49K81do) - Complete video walkthrough of Inheritance hook implementation from start to finish.

---

## Key Features

- **Nominee Hierarchy:** Users can set a primary nominee with priority control, followed by second and third nominees.
- **Time-Based Access:** Control transfers based on predefined ledger intervals.
- **Regular Key Assignment:** Nominees gain access by assigning their account as regular key to the hook account, and hence gaining access to all the funds(XAH, IOUs, URITokens etc) in it.

---

## How It Works

### 1. **Installing the Hook**

Users begin by installing two hook hashes into their account:

1. **First Hook (Activity Tracking Hook)**

   - Must hook on all transaction types
   - Uses hookon value: `0x0000000000000000000000000000000000000000000000000000000000000000`
   - Triggers on everything except SetHook transactions
   - Covers future transaction types
   - Purpose: Records the active state of the hook account

2. **Second Hook (Nominee Management Hook)**
   - Triggers on `Invoke` transaction type only
   - Used for:
     - Configuring nominees
     - Setting account takeover criteria
     - Enabling nominees to take over the hook account when criteria is met

### 2. **Configuring the Hook**

The next step involves setting up the nominee hierarchy and specifying access rules through the following parameters:

- **Primary Nominee:** The first account that can take control.
- **Second and Third Nominees:** Backup accounts that can take over after the primary (or in case primary nominee looses access to their own account.)
- **Ledger Intervals:**
  - **First Ledger Interval:** Time when the primary nominee can assign their account as a regular key.
  - **Second Ledger Interval:** Time when the second and third nominees can assign their accounts as regular key.
    > **Note:** The second ledger interval must always be greater than the first to ensure the primary nominee has priority access.

### 3. **Nominee Access Flow**

- Once the primary nominee assigns their account as a regular key to the hook account, they gain full control.
- If the primary nominee stops using the account, the second and third nominees can take over after the second ledger interval.
- This hierarchy allows for long-term account management and transfer of control.

---

## Example Scenario

1. **User Setup:**

   - A user configures their account with a primary nominee (Alice), a second nominee (Bob), and a third nominee (Charlie).
   - The first ledger interval is set to 100,000 ledgers, and the second to 200,000 ledgers.

2. **Primary Nominee Control:**

   - If the hook account isn't active(doesn't send out payment) for 100,000 ledgers, Alice can assign herself as a regular key and take control.

3. **Fallback:**

   - If Alice relinquishes control, Bob and Charlie can take over after 200,000 ledgers of inactivity.

---

## Binary Format Example

### Converting Ledger Interval to Little-Endian uint32

Here's an example of converting the decimal number 100 to a 4-byte little-endian uint32:

1. Convert 100 to hexadecimal: `0x64`
2. Pad to 4 bytes (8 hex digits): `0x00000064`
3. Convert to little-endian order:
   - Original: `00 00 00 64`
   - Little-endian: `64 00 00 00`

Final representation: `64000000`

## Useful Resources

- [Hooks Builder](https://hooks-builder.xrpl.org/develop) - Development environment for XRPL hooks
- [Hooks Documentation](https://xrpl-hooks.readme.io/reference/hook-api-conventions) - API reference and conventions
- [Transaction Type Visualizer](https://transia-rnd.github.io/xrpl-tt-visualizer/) - Visual guide for XRPL transaction types
- [Hooks Services](https://hooks.services/tools/) - Tools and utilities for hooks development
- [Binary Format Documentation](https://xrpl.org/docs/references/protocol/binary-format#token-amount-format) - XRPL binary format reference
- [SetRegularKey Documentation](https://xrpl.org/docs/references/protocol/transactions/types/setregularkey#example-setregularkey-json) - SetRegularKey transaction details
- [Inheritance Hook Tutorial](https://www.youtube.com/watch?v=XgQx49K81do) - Complete video walkthrough of Inheritance hook implementation from start to finish
