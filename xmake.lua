target("devio")
    set_kind("binary")

    add_files("*.c")
    add_includedirs("./")

    add_cflags("-w")
