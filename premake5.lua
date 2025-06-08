local revision = require("tools/revision")
revision.update()

local function ConfigurationOptions()
    configurations({
        "Debug Vanilla",
        "Debug Extended",

        "Release Vanilla",
        "Release Extended",
    })

    platforms({
        "x86",
        "x64",
    })

    -- optimization
    filter("configurations:Debug*")
        defines({ "DEBUG", "WIN32" })

    filter("configurations:Release*")
        defines({ "NDEBUG", "WIN32" })
        optimize("Speed")

    -- feature flags
    filter("configurations:*Extended")
        defines({
            "KISAK_EXTENDED",
        })

    filter("configurations:*Vanilla")
        defines({
            "KISAK_PURE"
        })

    filter("platforms:x86")
        architecture("x86")
        defines({ "CPU_32BIT" })
    
    filter("platforms:x64")
        architecture("x86_64")
        defines({ "CPU_64BIT" })

    filter("configurations:Debug*")
        targetdir("bin/%{cfg.buildcfg}/%{cfg.platform}")
    filter("configurations:Release*")
        targetdir("bin/%{cfg.buildcfg}/%{cfg.platform}")

    filter({})
end

local function DX9SDK()
    includedirs({ "vendor/dxsdk/Include" })

    filter("platforms:x86")
        libdirs({ "vendor/dxsdk/Lib/x86" })

    filter("platforms:x64")
        libdirs({ "vendor/dxsdk/Lib/x64" })

    filter("")
end

local function DoPostbuild()
    -- read gamepath.txt into gamePath
	local gamePath = io.readfile(os.getcwd() .. "/tools/revision.txt")

    -- copy dlls from dlls/ to gamePath 
    if os.istarget("windows") then
        postbuildcommands({
            'copy/y dlls\\mss32.dll "' .. gamePath .. '\\mss32.dll"',
            'copy/y dlls\\miles\\milesEq.flt "' .. gamePath .. '\\miles\\milesEq.flt"',
            'copy/y dlls\\miles\\mssdolby.flt "' .. gamePath .. '\\miles\\mssdolby.flt"',
            'copy/y dlls\\miles\\mssds3d.flt "' .. gamePath .. '\\miles\\mssds3d.flt"',
            'copy/y dlls\\miles\\mssdsp.flt "' .. gamePath .. '\\miles\\mssdsp.flt"',
            'copy/y dlls\\miles\\msseax.flt "' .. gamePath .. '\\miles\\msseax.flt"',
            'copy/y dlls\\miles\\mssmp3.asi "' .. gamePath .. '\\miles\\mssmp3.asi"',
            'copy/y dlls\\miles\\msssrs.flt "' .. gamePath .. '\\miles\\msssrs.flt"',
            'copy/y dlls\\miles\\mssvoice.asi "' .. gamePath .. '\\miles\\mssvoice.asi"',
        })
    else
        -- TODO: linux variant (good luck sourcing the sos)
    end

    -- copy KisakCOD to gamePath
    postbuildcommands({
        'copy/y "$(TargetPath)" "' .. gamePath .. '"'
    })
end

systemversion("10.0.22621.0")
toolset("v143")
architecture("x64")

workspace("KisakCOD")
    location("build")

    startproject("linker")
    cppdialect("c++latest")

    symbols("On")
    staticruntime("On")
    linktimeoptimization("On")

    characterset("MBCS")

    ConfigurationOptions()

    flags({
        "NoManifest",
        "MultiProcessorCompile",
        "NoIncrementalLink"
    })

    editandcontinue("Off")

project("KisakCOD")
    targetname("KisakCOD")
    targetextension(".exe")

    language("C++")
    kind("WindowedApp")

    staticruntime("On")
    buildoptions("/MT")

    conformancemode("off")
    rtti("off")
    dpiawareness("HighPerMonitor")

    vectorextensions("SSE3")

    DoPostbuild()

    defines({
        "_CRT_SECURE_NO_WARNINGS",
    })

    includedirs({
        "src/",
        "src/**" -- TODO: remove this line once all the includes have been sorted
    })

    pchheader("stdafx.h")
    pchsource("src/stdafx.cpp")

    links({ "ws2_32", "d3d9" })

    files({
        "src/**.cpp",
        "src/**.hpp",
        "src/**.h",
    })

    removefiles({
        "src/physics/ode/stack.cpp"
    })

    DX9SDK()