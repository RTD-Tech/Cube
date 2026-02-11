project "Box2D"
    location "../../../Build/Build-Files"
    kind "StaticLib"
    language "C++"

    targetdir ("../../../Build/Binaries/" .. OutputDir .. "/Dependencies/%{prj.name}")
    objdir ("../../../Build/Binaries-Intermediate/" .. OutputDir .. "/Dependencies/%{prj.name}")

    includedirs {
        "box2d/include",
    }

    files {
        "box2d/src/**cpp",
        "box2d/src/**c",
        "box2d/src/**h",
        "box2d/include/**h",
    }

    filter "system:windows"
        systemversion "latest"
        defines {
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:linux"
        pic "On"

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"