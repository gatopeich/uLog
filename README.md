# uLog logging system by gatopeich (c) 2018
Minimalistic single header C logging system where messages can be enabled or disabled individually

## Design goals
- GCC+C11 compatibility
- Minimal cache misses on disabled messages
- Clean, simple interface

## Features:
- Single header
- Per-message, per level enabling/disabling
- Output to stderr/file
- Printf formatting
- TBD: UTC timestamps
- TBD: Add file, line, function to messages and regex-able text
- TBD: Output to syslog/udp/tcp
- TBD: Per-message throttling
- TBD: Background thread to offload work
- TBD: Multiple sinks?
