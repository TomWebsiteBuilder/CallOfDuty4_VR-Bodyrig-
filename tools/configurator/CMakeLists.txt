project(KisakCOD-VR-Configurator LANGUAGES CXX)

add_executable(KisakCOD-VR-Ladder-Tests ../../tests/ladder_climb_tests.cpp)
target_include_directories(KisakCOD-VR-Ladder-Tests PRIVATE ../../src)
target_compile_features(KisakCOD-VR-Ladder-Tests PRIVATE cxx_std_17)
add_custom_target(KisakCOD-VR-Ladder-Check
    COMMAND $<TARGET_FILE:KisakCOD-VR-Ladder-Tests>
    DEPENDS KisakCOD-VR-Ladder-Tests
    VERBATIM
)
if(NOT CMAKE_CROSSCOMPILING)
    add_dependencies(KisakCOD-sp KisakCOD-VR-Ladder-Check)
endif()

add_executable(KisakCOD-VR-World-Canvas-Tests ../../tests/world_canvas_tests.cpp)
target_include_directories(KisakCOD-VR-World-Canvas-Tests PRIVATE ../../src ../../deps/openxr-sdk/include)
target_compile_features(KisakCOD-VR-World-Canvas-Tests PRIVATE cxx_std_17)
add_custom_target(KisakCOD-VR-World-Canvas-Check
    COMMAND $<TARGET_FILE:KisakCOD-VR-World-Canvas-Tests>
    DEPENDS KisakCOD-VR-World-Canvas-Tests
    VERBATIM
)
if(NOT CMAKE_CROSSCOMPILING)
    add_dependencies(KisakCOD-sp KisakCOD-VR-World-Canvas-Check)
endif()

# The settings suite does not exercise the physical grenade state machine.
# Keep this focused suite available in the same Windows build as the game.
add_executable(KisakCOD-VR-Grenade-Tests ../../tests/grenade_interaction_tests.cpp)
target_include_directories(KisakCOD-VR-Grenade-Tests PRIVATE ../../src)
target_compile_features(KisakCOD-VR-Grenade-Tests PRIVATE cxx_std_17)

# Embedded HLSL is otherwise only compiled when the game starts in VR.
add_executable(KisakCOD-VR-Shader-Tests ../../tests/vr_shader_compile_tests.cpp)
target_compile_features(KisakCOD-VR-Shader-Tests PRIVATE cxx_std_17)
if(MSVC)
    set_property(TARGET KisakCOD-VR-Shader-Tests PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_options(KisakCOD-VR-Shader-Tests PRIVATE /W4 /WX)
endif()

add_custom_target(KisakCOD-VR-Shader-Check
    COMMAND $<TARGET_FILE:KisakCOD-VR-Shader-Tests>
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src/vr/vr_openxr.cpp"
    DEPENDS KisakCOD-VR-Shader-Tests
    VERBATIM
)
if(NOT CMAKE_CROSSCOMPILING)
    add_dependencies(KisakCOD-sp KisakCOD-VR-Shader-Check)
endif()

add_executable(
    KisakCOD-VR-Configurator
    WIN32
    configurator.cpp
    compatibility_probe_win32.cpp
    compatibility_probe_win32.h
    settings_core.cpp
    settings_core.h
    ../../src/vr/vr_calibration.h
    ../../src/vr/vr_hud_layout.cpp
    ../../src/vr/vr_hud_layout.h
    ../../src/vr/vr_input_bindings.cpp
    ../../src/vr/vr_input_bindings.h
    ../../src/vr/vr_interactions.h
    ../../src/vr/vr_compatibility.cpp
    ../../src/vr/vr_compatibility.h
    ../../src/vr/vr_openxr_profiles.cpp
    ../../src/vr/vr_openxr_profiles.h
    ../../src/vr/vr_packed_layout.h
    ../../src/vr/vr_postfx_policy.h
    ../../src/vr/vr_saved_screen_policy.h
    ../../src/vr/vr_prompt_labels.cpp
    ../../src/vr/vr_prompt_labels.h
    ../../src/vr/vr_weapon_profiles.cpp
    ../../src/vr/vr_weapon_profiles.h
)

target_include_directories(
    KisakCOD-VR-Configurator
    PRIVATE
        ../../src
)

target_compile_features(
    KisakCOD-VR-Configurator
    PRIVATE
        cxx_std_20
)

target_compile_definitions(
    KisakCOD-VR-Configurator
    PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0A00
)

target_link_libraries(
    KisakCOD-VR-Configurator
    PRIVATE
        OpenVR::openvr_api
        comctl32
        comdlg32
        advapi32
        dxgi
        shell32
)

if(MSVC)
    set_property(
        TARGET KisakCOD-VR-Configurator
        PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(
        KisakCOD-VR-Configurator
        PRIVATE
            /W4
            /permissive-
            /utf-8
    )
endif()

set_target_properties(
    KisakCOD-VR-Configurator
    PROPERTIES
        OUTPUT_NAME "KisakCOD-VR-Configurator"
)

add_executable(
    KisakCOD-VR-Configurator-Tests
    ../../tests/configurator_settings_tests.cpp
    compatibility_probe_win32.cpp
    compatibility_probe_win32.h
    settings_core.cpp
    settings_core.h
    ../../src/vr/vr_calibration.h
    ../../src/vr/vr_hud_layout.cpp
    ../../src/vr/vr_hud_layout.h
    ../../src/vr/vr_input_bindings.cpp
    ../../src/vr/vr_input_bindings.h
    ../../src/vr/vr_interactions.h
    ../../src/vr/vr_compatibility.cpp
    ../../src/vr/vr_compatibility.h
    ../../src/vr/vr_openvr_input.cpp
    ../../src/vr/vr_openvr_input.h
    ../../src/vr/vr_openxr_profiles.cpp
    ../../src/vr/vr_openxr_profiles.h
    ../../src/vr/vr_packed_layout.h
    ../../src/vr/vr_postfx_policy.h
    ../../src/vr/vr_saved_screen_policy.h
    ../../src/vr/vr_prompt_labels.cpp
    ../../src/vr/vr_prompt_labels.h
    ../../src/vr/vr_weapon_profiles.cpp
    ../../src/vr/vr_weapon_profiles.h
)

target_include_directories(
    KisakCOD-VR-Configurator-Tests
    PRIVATE
        ../../src
        ../../deps/openvr/headers
)

target_compile_features(
    KisakCOD-VR-Configurator-Tests
    PRIVATE
        cxx_std_20
)

target_link_libraries(
    KisakCOD-VR-Configurator-Tests
    PRIVATE
        OpenVR::openvr_api
        advapi32
        dxgi
)

if(MSVC)
    set_property(
        TARGET KisakCOD-VR-Configurator-Tests
        PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    target_compile_options(
        KisakCOD-VR-Configurator-Tests
        PRIVATE
            /W4
            /permissive-
            /utf-8
    )
endif()

add_executable(
    KisakCOD-VR-Test
    WIN32
    test_launcher.cpp
)

target_compile_features(
    KisakCOD-VR-Test
    PRIVATE
        cxx_std_17
)

target_link_libraries(
    KisakCOD-VR-Test
    PRIVATE
        shell32
)

if(MSVC)
    set_property(
        TARGET KisakCOD-VR-Test
        PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()
