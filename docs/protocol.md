# NCA wire protocol

NCA is a one-way addressed byte-stream protocol. One orchestrator transmits;
receivers do not acknowledge or reply. The library does not prescribe the
physical transport, although current instrument-cluster deployments use UART
at 250000 baud, 8N1.

All multi-byte values are little endian. The default maximum payload is 192
bytes and can be changed at compile time with `NCA_PROTOCOL_MAX_PAYLOAD`.

## Frame

```text
SOF  DESTINATION  COMMAND  LENGTH_LE16  PAYLOAD[LENGTH]  CHECKSUM
A5       xx          xx       xx xx          ...            xx
```

`0x5A` is broadcast. Device addresses are `0x01..0xFE`, excluding `0x5A` and
`0xFF`. Address `0x00` is reserved for host-side listen-all parsers and is not
a valid wire destination for a device.

The checksum is XOR of the command, both length bytes, and all payload bytes.
SOF and destination are excluded. Invalid, oversized, misaddressed, and
checksum-damaged frames are ignored.

## Raw device commands

| Command | Name | Payload |
|---:|---|---|
| `0x01` | `SET_ALL` | zero to `segment_count` state bytes from segment 0 |
| `0x02` | `SET_RANGE` | `offset:u16 count:u16 state[count]` |
| `0x03` | `SET_PACKED_RANGE` | `offset:u16 bit_count:u16 packed_bits...` |
| `0x30` | `SET_RGB_ENABLED` | one byte: zero default colors, nonzero custom colors |
| `0x31` | `SET_RGB_ALL` | exactly `segment_count` RGB triplets |
| `0x32` | `SET_RGB_RANGE` | `offset:u16 count:u16 rgb[count]` |
| `0x33` | `SET_ADDRESS` | one valid address; accepted only by unicast policy |

State bytes use zero for off and nonzero for on. Packed state is LSB-first.
RGB triplets use R,G,B order. Range commands are atomic and rejected unless
their exact length and entire range are valid.

## Interactive commands

Raw mode (`0`) is the initial mode. Interactive mode (`1`) exposes cached,
typed cluster values to a renderer. Publications are decoded in either mode,
so switching mode can immediately render the latest values.

### `0x10` PUBLISH

```text
version:u8  topic:u8  sequence:u16  data...
```

Version is `1`. Sequence is a wrapping per-topic diagnostic counter.

| Topic | Name | Data | Engineering value |
|---:|---|---|---|
| `0x01` | RPM | `uint16_t` | exact RPM |
| `0x02` | speed | `uint16_t` | value / 10 |
| `0x03` | oil temperature | `int16_t` | value / 10 degrees C |
| `0x04` | outdoor temperature | `int16_t` | value / 10 degrees C |
| `0x05` | coolant temperature | `int16_t` | value / 10 degrees C |
| `0x06` | fuel level | `int16_t` | value / 10 |
| `0x07` | indicators | `uint32_t[2]` | bits 0..63, low word first |
| `0x08` | odometer | `uint32_t` | kilometres / 10 |
| `0x20..0x29` | generic 0..9 | `int16_t` | value / 10 |

The Digifiz ESP32 publisher assigns generic topics 0..9 to MFA value, MFA
clock value, fuel consumption, intake voltage, fuel pressure, uptime hours,
raw speed, brightness percent, gear, and MFA page respectively.
Values outside the signed deci-unit range are saturated.

Indicator bits 0..14 are lights, front fog, rear fog, rear-window heat,
high beam, left turn, right turn, brake, oil pressure, battery, MFA1, MFA2,
check engine, low fuel, and backlight. Bits 15..23 are reserved.

Indicator bits 24 through 27 are reserved as high-oil-temperature,
high-outdoor-temperature, high-coolant-temperature, and low-fuel respectively.
Bits 28 through 63 remain spare. Topic values `0x40..0x7F` are reserved for
future standards and `0x80..0xFE` for product extensions.

### `0x20` SET_MODE

The payload is one byte: `0` for raw or `1` for interactive.

### `0x21` SET_CONFIG

```text
version:u8  topic:u8  TLV...
TLV := key:u8 length:u8 value[length]
```

Version is `1`. Topic zero means the receiver's assigned topic. Key `0x01` is
the IEEE-754 binary32 minimum and key `0x02` is the binary32 maximum. A range
is applied atomically only when minimum is less than maximum. Unknown TLVs are
skipped; malformed TLVs reject the complete command.

Commands `0x40..0x45` are reserved for application-specific bootloader policy.
Their identifiers are shared so a parser can transport them, but flash layout,
pacing, reset, validation, and boot selection deliberately remain outside this
platform-independent library.
