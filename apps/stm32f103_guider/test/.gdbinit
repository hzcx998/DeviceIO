target remote:3333

set auto-load safe-path /

monitor halt
monitor flash write_image erase build/windows/x64/release/xos.hex
monitor reset halt

b Reset_Handler
jump *0x8001220
