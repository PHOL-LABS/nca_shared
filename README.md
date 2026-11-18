# nca_shared

Portable C99 implementation of the New Cluster Architecture (NCA) wire
protocol and device state model. The library has no dependency on an MCU,
operating system, transport, graphics API, heap allocator, or C runtime beyond
the standard integer/string headers.

It is intended to be shared by STM32/AT32 receivers and bootloaders, ESP32
topic generators, Raspberry Pi receivers/renderers, and host-side tools.

## Features

- incremental addressed/broadcast frame parser and frame encoder;
- raw segment and RGB override commands;
- raw/interactive mode switching;
- typed topic payload generation and processing;
- RPM, speed, temperature, fuel, indicator, and generic topics;
- range configuration TLVs and protected runtime address changes;
- caller-owned storage and no dynamic allocation.

The library deliberately contains no UART, DMA, GPIO, reset, flash, bootloader
policy, physical LED maps, or rendering code.

The canonical frame, command, topic, and payload specification is in
[`docs/protocol.md`](docs/protocol.md).

## CMake

```cmake
add_subdirectory(path/to/nca_shared)
target_link_libraries(my_target PRIVATE nca::shared)
```

When used directly as an ESP-IDF component, `CMakeLists.txt` calls
`idf_component_register` automatically.

Host tests use Catch2:

```sh
cmake -S . -B build -G Ninja -DNCA_SHARED_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Catch2 v3 is found locally first, then fetched as a pinned test-only dependency
when `NCA_SHARED_FETCH_CATCH2=ON`.

## Make and CubeIDE

GNU Make projects can include `nca_shared.mk`. Legacy ESP-IDF Make projects can
use `component.mk`.

For STM32CubeIDE, add every file under `src/` and add `nca_shared/include` to
the compiler include paths. The public umbrella include is:

```c
#include <nca/nca.h>
```

## API model

Generators first create a typed payload with `nca_publish_encode_*()` or
`nca_config_encode_range()`, then wrap it with `nca_frame_encode()`. Raw-mode
generators pass the documented raw payload directly to `nca_frame_encode()`.

Receivers initialize `NcaProtocol` with a callback and feed arbitrary chunks
from their serial, socket, or file transport to `nca_protocol_feed()`. The
callback can pass accepted commands to `nca_device_process()`, which maintains
caller-owned raw/RGB buffers and the `NcaInteractiveState`. A renderer may also
use `NcaInteractiveState` and `nca_interactive_process()` directly when it does
not need the raw physical-segment model.

Address `NCA_LISTEN_ALL_ADDRESS` is intended for Raspberry Pi and host tools
that need to observe every valid destination. Device receivers should use
their actual address. No API allocates memory or performs I/O.
