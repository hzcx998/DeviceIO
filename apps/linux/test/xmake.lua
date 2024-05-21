target("devio")
    set_kind("binary")

    top_dir = "../../../"

    -- [M] devio
    add_files(
        top_dir .. "devio/*.c", 
        top_dir .. "devio/tlsf/*.c"
    )
    
    add_includedirs(
        top_dir .. "devio",
        top_dir .. "devio/tlsf"
    )

    -- [M] drivers
    --      drivers/block/
    add_files(top_dir .. "drivers/block/ramdisk.c")
    --      drivers/other/
    add_files(top_dir .. "drivers/other/test.c")

    -- [M] app
    add_files("*.c")
    add_includedirs("./")

    add_cflags("-w")
