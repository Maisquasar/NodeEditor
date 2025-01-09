add_rules("mode.debug", "mode.release")

add_deps("NodeEditor")

target("example")
    set_default(true)
    set_kind("binary")
    add_files("src/**.cpp")
    add_headerfiles("src/**.hpp")
    
    add_defines("IMGUI_IMPLEMENTATION", "IMGUI_DEFINE_MATH_OPERATORS")
    
    add_packages("galaxymath")
    add_packages("glfw")
    add_packages("imgui")
    add_packages("glad")
target_end()