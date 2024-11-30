add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

set_languages("c++20")

-- Custom repo
add_repositories("galaxy-repo https://github.com/GalaxyEngine/xmake-repo")

add_requires("imgui v1.91.1-docking", { configs = { opengl3 = true, glfw = true }})
add_requires("glad", {configs = { extensions = "GL_KHR_debug"}})
add_requires("galaxymath")
add_requires("cpp_serializer")
add_requires("nativefiledialog-extended")

set_rundir("$(projectdir)")

target("NodeEditor")
    set_kind("binary")
    add_files("src/**.cpp")
    add_headerfiles("include/**.h")
    
    
    if is_mode("debug") then
        add_defines("_DEBUG")
    end

    add_defines("IMGUI_IMPLEMENTATION", "IMGUI_DEFINE_MATH_OPERATORS")

    add_includedirs("include")

    add_packages("glfw")
    add_packages("imgui")
    add_packages("glad")
    add_packages("galaxymath")
    add_packages("cpp_serializer")
    add_packages("nativefiledialog-extended")
target_end()