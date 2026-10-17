// Compile the actual embedded startup shaders without a headset or D3D device.
// C++ compilation alone treats HLSL as text and cannot detect its syntax errors.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace
{
using Microsoft::WRL::ComPtr;

// Read one raw-string initializer, including adjacent literal fragments used to
// stay within MSVC's per-literal length limit. Fail if the source format changes
// instead of accidentally testing an empty or incomplete copy of the shader.
std::string ReadShader(const std::string& source, const char* function,
                       const char* declaration)
{
    const auto functionStart = source.find(function);
    if (functionStart == std::string::npos)
        throw std::runtime_error(std::string("Missing function: ") + function);

    const auto initializer = source.find(declaration, functionStart);
    if (initializer == std::string::npos)
        throw std::runtime_error(std::string("Missing shader: ") + declaration);

    auto cursor = initializer + std::char_traits<char>::length(declaration);
    std::string shader;
    for (;;)
    {
        cursor = source.find_first_not_of(" \t\r\n", cursor);
        if (cursor == std::string::npos)
            throw std::runtime_error("Unterminated shader initializer");

        if (source.compare(cursor, 2, "//") == 0)
        {
            cursor = source.find('\n', cursor);
            if (cursor == std::string::npos)
                throw std::runtime_error("Unterminated shader comment");
            continue;
        }
        if (source[cursor] == ';' && !shader.empty())
            return shader;
        if (source.compare(cursor, 3, "R\"(") != 0)
            throw std::runtime_error("Expected an embedded HLSL raw string");

        cursor += 3;
        const auto end = source.find(")\"", cursor);
        if (end == std::string::npos)
            throw std::runtime_error("Unterminated HLSL raw string");
        shader.append(source, cursor, end - cursor);
        cursor = end + 2;
    }
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::fprintf(stderr, "Usage: %s path/to/src/vr/vr_openxr.cpp\n", argv[0]);
        return 2;
    }

    try
    {
        std::ifstream file(argv[1], std::ios::binary);
        if (!file)
            throw std::runtime_error(std::string("Cannot read ") + argv[1]);
        const std::string source((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

        const auto compositor =
            ReadShader(source, "bool VR_CreateCapturedFrameBlitResources()",
                       "static const char shaderSourcePart1[] =") +
            ReadShader(source, "bool VR_CreateCapturedFrameBlitResources()",
                       "static const char shaderSourcePart2[] =");
        const auto scene = ReadShader(source, "bool VR_CreateHeadTrackedScene()",
                                      "static const char* shaderSource =");
        const auto controllerVertex =
            ReadShader(source, "bool VR_CreateControllerProxyResources()",
                       "static const char* vertexShaderSource =");
        const auto controllerPixel =
            ReadShader(source, "bool VR_CreateControllerProxyResources()",
                       "static const char* pixelShaderSource =");

        // Match VR_TestCompileShader's DLL preference, profiles, and flags on
        // both native Windows and the game's Proton installation.
        HMODULE module = nullptr;
        for (const char* dll : {"d3dcompiler_47.dll", "d3dcompiler_46.dll",
                                "d3dcompiler_43.dll"})
        {
            module = LoadLibraryA(dll);
            if (module)
            {
                std::printf("Shader compiler: %s\n", dll);
                break;
            }
        }
        if (!module)
            throw std::runtime_error("Cannot load a D3DCompiler DLL");
        // Keep the module loaded until all compiler-owned blobs are released.
        const auto compile = reinterpret_cast<decltype(&D3DCompile)>(
            GetProcAddress(module, "D3DCompile"));
        if (!compile)
            throw std::runtime_error("D3DCompile export unavailable");

        struct Shader
        {
            const char* name;
            const std::string* text;
            const char* entry;
            const char* profile;
        };
        const Shader shaders[] = {
            {"compositor", &compositor, "VSMain", "vs_4_0"},
            {"compositor", &compositor, "PSMain", "ps_4_0"},
            {"compositor", &compositor, "PSEasu", "ps_4_0"},
            {"compositor", &compositor, "PSRcas", "ps_4_0"},
            {"compositor", &compositor, "PSScope", "ps_4_0"},
            {"head-tracked scene", &scene, "VSMain", "vs_4_0"},
            {"head-tracked scene", &scene, "PSMain", "ps_4_0"},
            {"controller proxy", &controllerVertex, "VSMain", "vs_4_0"},
            {"controller proxy", &controllerPixel, "PSMain", "ps_4_0"},
        };
        int failures = 0;
        for (const auto& shader : shaders)
        {
            ComPtr<ID3DBlob> code;
            ComPtr<ID3DBlob> errors;
            const HRESULT hr = compile(
                shader.text->data(), shader.text->size(), shader.name,
                nullptr, nullptr, shader.entry, shader.profile,
                D3DCOMPILE_ENABLE_STRICTNESS, 0,
                code.GetAddressOf(), errors.GetAddressOf());
            if (FAILED(hr) || !code || code->GetBufferSize() == 0)
            {
                ++failures;
                std::fprintf(stderr, "FAIL %s/%s (%s), HRESULT 0x%08lX\n",
                             shader.name, shader.entry, shader.profile,
                             static_cast<unsigned long>(hr));
                if (errors)
                    std::fwrite(errors->GetBufferPointer(), 1,
                                errors->GetBufferSize(), stderr);
            }
            else
            {
                std::printf("PASS %s/%s (%s): %zu bytes\n", shader.name,
                            shader.entry, shader.profile, code->GetBufferSize());
            }
        }
        std::printf("%zu shaders, %d failures\n", std::size(shaders), failures);
        return failures == 0 ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "%s\n", error.what());
        return 2;
    }
}
