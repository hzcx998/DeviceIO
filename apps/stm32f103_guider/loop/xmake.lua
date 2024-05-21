ProjectName = "loop"
-- 自定义工具链
toolchain("arm-none-eabi")
    -- 标记为独立工具链
    set_kind("standalone")
    -- 定义交叉编译工具链地址
    set_sdkdir("D:\\Tools\\gcc-arm-10.2-2020.11-mingw-w64-i686-arm-none-eabi")
    set_bindir("D:\\Tools\\gcc-arm-10.2-2020.11-mingw-w64-i686-arm-none-eabi\\bin")
toolchain_end()

target("loop")
    set_toolchains("arm-none-eabi")
    set_kind("binary")

    set_filename("loop.elf")
    
    add_files(
        "startup_stm32f103xe.s"
    )

    add_cflags(
        "-mcpu=cortex-m3",
        "-mthumb",
        "-fdata-sections -ffunction-sections",
        "-g -O0 -gdwarf-2",
        { force = true }
    )
    -- 设置汇编编译参数
    add_asflags(
        "-mcpu=cortex-m3",
        "-mthumb",
        "-fdata-sections -ffunction-sections",
        "-g -O0 -x assembler-with-cpp",
        "-Wa,-mimplicit-it=thumb",
        { force = true }
    )
    -- 设置链接参数
    local MapCMD = "-Wl,-Map=" .. ProjectName .. ".map,--cref,-u,Reset_Handler"
    add_ldflags(
        "-mcpu=cortex-m3",
        "-mthumb",
        "-Tloop.lds",
        "--specs=nosys.specs",
        "-Wl,--gc-sections",
        MapCMD,
        { force = true }
    )
target_end()

after_build(function(target)
    local dir = "D:\\Tools\\gcc-arm-10.2-2020.11-mingw-w64-i686-arm-none-eabi\\bin\\"

    os.exec(dir .. "arm-none-eabi-objcopy -O ihex ./build/windows/x64/release/loop.elf ./build/windows/x64/release/loop.hex")
    os.exec(dir .. "arm-none-eabi-objcopy -O binary ./build/windows/x64/release/loop.elf ./build/windows/x64/release/loop.bin")
    print("生成已完成!")
    print("********************储存空间占用情况*****************************")
    os.exec(dir .. "arm-none-eabi-size -Bd ./build/windows/x64/release/loop.elf")
    print("****************************************************************")
end)

    --
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

