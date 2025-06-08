local revision = require("tools/revision")
revision.update()

local submodule = require("tools/submodule")
submodule.define({
    name = "zlib",
    libmode = "staticlib",
    files = {
        "vendor/zlib/*.c",
        "vendor/zlib/*.h",
    },
    includes = {
        "vendor/zlib"
    }
})

submodule.define({
    name = "speex",
    libmode = "staticlib",
    files = {
        "vendor/speex/*.c",
        "vendor/speex/*.h",
    },
    includes = {
        "vendor/speex"
    }
})

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

    filter("configurations:Debug*")
        links({ "d3dx9d" })

    filter("configurations:Release*")
        links({ "d3dx9" })

    filter({})
end

local function DoPostbuild()
    -- read gamepath.txt into gamePath
    local gamePath = io.readfile(os.getcwd() .. "/tools/revision.txt")
    local cwd = os.getcwd()

    -- copy dlls from dlls/ to gamePath 
    if os.istarget("windows") then
        postbuildcommands({
            'copy/y "' .. cwd .. '\\dlls\\mss32.dll" "' .. gamePath .. '\\mss32.dll"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\milesEq.flt" "' .. gamePath .. '\\miles\\milesEq.flt"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\mssdolby.flt" "' .. gamePath .. '\\miles\\mssdolby.flt"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\mssds3d.flt" "' .. gamePath .. '\\miles\\mssds3d.flt"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\mssdsp.flt" "' .. gamePath .. '\\miles\\mssdsp.flt"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\msseax.flt" "' .. gamePath .. '\\miles\\msseax.flt"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\mssmp3.asi" "' .. gamePath .. '\\miles\\mssmp3.asi"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\msssrs.flt" "' .. gamePath .. '\\miles\\msssrs.flt"',
            'copy/y "' .. cwd .. '\\dlls\\miles\\mssvoice.asi" "' .. gamePath .. '\\miles\\mssvoice.asi"',
        })
    else
        -- TODO: linux variant (good luck sourcing the sos)
    end

    -- copy KisakCOD to gamePath
    postbuildcommands({
        'copy/y "$(TargetPath)" "' .. gamePath .. '"'
    })

    -- just do this here since we have gamePath already lol
    debugdir(gamePath)
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
    --dpiawareness("HighPerMonitor")

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

    DX9SDK()
    libdirs({ "src/msslib" })
    links({ "mss32", "dsound", "ddraw", "dxguid", "ws2_32", "d3d9",  "winmm" })
    submodule.include({ "zlib", "speex" })

    files({
        "src/**.c",
        "src/**.cpp",
        "src/**.hpp",
        "src/**.h",
    })

    filter({ "files:**.c" })
        compileas("C++")

    filter({})

    removefiles({
        "src/gfx_d3d/r_model_skin_sse.cpp",

        "src/script/scr_compiler.cpp",
        "src/script/scr_yacc.cpp",

        "src/physics/ode/array.cpp",
        "src/physics/ode/array.h",
        "src/physics/ode/collision_trimesh.cpp",
        "src/physics/ode/collision_trimesh_ccylinder.cpp",
        "src/physics/ode/collision_trimesh_distance.cpp",
        "src/physics/ode/collision_trimesh_ray.cpp",
        "src/physics/ode/collision_trimesh_sphere.cpp",
        "src/physics/ode/collision_trimesh_trimesh.cpp",
        "src/physics/ode/lcp.cpp",
        "src/physics/ode/lcp.h",
        "src/physics/ode/mat.cpp",
        "src/physics/ode/mat.h",
        "src/physics/ode/memory.cpp",
        "src/physics/ode/obstack.cpp",
        "src/physics/ode/odeext.h",
        "src/physics/ode/odemisc.cpp",
        "src/physics/ode/stack.cpp",
        "src/physics/ode/step.cpp",
        "src/physics/ode/step.h",
        "src/physics/ode/stepfast.cpp",
        "src/physics/ode/testing.cpp",
        "src/physics/ode/testing.h",
    })

group("vendor")
    submodule.register_all()