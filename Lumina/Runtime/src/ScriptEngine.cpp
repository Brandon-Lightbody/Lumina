#include "Runtime/ScriptEngine.h"
#include "Runtime/ScriptAPI.h"
#include "../ScriptAPI/deps/nethost/coreclr_delegates.h"
#include "../ScriptAPI/deps/nethost/hostfxr.h"

// Define get_hostfxr_parameters structure locally
struct get_hostfxr_parameters {
    size_t size;
    const wchar_t* dotnet_root;
};

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#include <libloaderapi.h>
#include <direct.h>
#define LIB_SUFFIX L".dll"
#else
#include <dlfcn.h>
#include <unistd.h>
#include <limits.h>
#define LIB_SUFFIX ".so"
#endif

// HostFXR error message function
using hostfxr_get_error_message_fn = int(*)(const char_t** message);

namespace Lumina {
    // Define the native_methods map in the implementation file
    static std::unordered_map<std::string, ScriptEngine::StringCallback> s_NativeMethods;

    // CoreCLR state
    static hostfxr_initialize_for_runtime_config_fn init_fptr = nullptr;
    static hostfxr_get_runtime_delegate_fn get_delegate_fptr = nullptr;
    static hostfxr_close_fn close_fptr = nullptr;
    static hostfxr_get_error_message_fn error_fptr = nullptr;
    static hostfxr_handle host_context = nullptr;
    static load_assembly_and_get_function_pointer_fn load_assembly_and_get_fn = nullptr;

#ifdef _WIN32
    std::wstring ConvertToWide(const std::string& str) {
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
        return result;
    }

    std::string ConvertToNarrow(const wchar_t* str) {
        if (!str) return "";
        int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, str, -1, &result[0], size, nullptr, nullptr);
        return result;
    }
#else
    std::string ConvertToNarrow(const char* str) {
        return str ? std::string(str) : "";
    }
#endif

    void* LoadLib(const char_t* path) {
#ifdef _WIN32
        return LoadLibraryW(path);
#else
        return dlopen(path, RTLD_LAZY);
#endif
    }

    void* GetExport(void* h, const char* name) {
#ifdef _WIN32
        return GetProcAddress(static_cast<HMODULE>(h), name);
#else
        return dlsym(h, name);
#endif
    }

    // Add nethost implementation directly to resolve linker error
#ifdef _WIN32
#define STR(s) L##s
#define CH(c) L##c
#define DIR_SEPARATOR L'\\'
    typedef wchar_t char_t;
#else
#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
    typedef char char_t;
#endif

    int get_hostfxr_path(
        char_t* buffer,
        size_t* buffer_size,
        const struct get_hostfxr_parameters* parameters)
    {
        const char_t* dotnet_root = nullptr;

        if (parameters != nullptr) {
            if (parameters->dotnet_root != nullptr) {
                dotnet_root = parameters->dotnet_root;
            }
        }

        std::filesystem::path hostfxr_path;
        if (dotnet_root != nullptr) {
            hostfxr_path = std::filesystem::path(dotnet_root);
        }
        else {
            // Cross-platform environment variable handling
#ifdef _WIN32
            wchar_t env_buffer[32767];
            DWORD result = GetEnvironmentVariableW(L"DOTNET_ROOT", env_buffer, 32767);
            if (result > 0 && result < 32767) {
                dotnet_root = env_buffer;
            }
            else {
                // Check other possible locations
                if (GetEnvironmentVariableW(L"ProgramFiles", env_buffer, 32767) > 0) {
                    hostfxr_path = std::filesystem::path(env_buffer) / L"dotnet";
                }
            }
#else
            const char* env_value = std::getenv("DOTNET_ROOT");
            if (env_value) {
                dotnet_root = env_value;
            }
            else {
                // Common Linux/MacOS locations
                const char* locations[] = {
                    "/usr/share/dotnet",
                    "/usr/local/share/dotnet",
                    "/opt/dotnet",
                    "/usr/lib/dotnet",
                    "/usr/local/lib/dotnet"
                };

                for (auto loc : locations) {
                    if (std::filesystem::exists(loc)) {
                        dotnet_root = loc;
                        break;
                    }
                }
            }
#endif

            if (dotnet_root == nullptr) {
#ifdef _WIN32
                hostfxr_path = STR("C:\\Program Files\\dotnet");
#else
                hostfxr_path = STR("/usr/share/dotnet");
#endif
            }
            else {
                hostfxr_path = std::filesystem::path(dotnet_root);
            }
        }

        hostfxr_path = hostfxr_path / STR("host") / STR("fxr");
        if (!std::filesystem::exists(hostfxr_path)) {
            std::cerr << "Host/FXR directory not found: " << hostfxr_path << "\n";
            return 1;
        }

        // Improved version discovery
        std::vector<std::string> versions;
        for (const auto& entry : std::filesystem::directory_iterator(hostfxr_path)) {
            if (entry.is_directory()) {
                std::string version = entry.path().filename().string();
                versions.push_back(version);
            }
        }

        if (versions.empty()) {
            std::cerr << "No runtime versions found in: " << hostfxr_path << "\n";
            return 2;
        }

        // Sort versions to find the highest
        std::sort(versions.begin(), versions.end(), [](const std::string& a, const std::string& b) {
            return a > b;
            });

        // Prefer .NET 8+ versions
        std::string selected_version;
        for (const auto& v : versions) {
            if (v.find("8.") == 0) {
                selected_version = v;
                break;
            }
        }

        // If no .NET 8 found, use highest available
        if (selected_version.empty()) {
            selected_version = versions[0];
            std::cerr << "WARNING: Using non-.NET 8 runtime: " << selected_version << "\n";
        }

        hostfxr_path = hostfxr_path / selected_version / (std::basic_string<char_t>(STR("hostfxr")) + LIB_SUFFIX);

        if (!std::filesystem::exists(hostfxr_path)) {
            std::cerr << "HostFXR not found: " << hostfxr_path << "\n";
            return 1;
        }

        const std::basic_string<char_t> path_str = hostfxr_path.native();
        if (path_str.size() + 1 > *buffer_size) {
            *buffer_size = path_str.size() + 1;
            return 3;
        }

#ifdef _WIN32
        wcscpy_s(buffer, *buffer_size, path_str.c_str());
#else
        strcpy(buffer, path_str.c_str());
#endif

        std::cout << "Resolved hostfxr path: " << hostfxr_path << "\n";
        return 0;
    }

    bool LoadHostFxr() {
        size_t buffer_size = MAX_PATH;
        std::vector<char_t> buffer(buffer_size);

        get_hostfxr_parameters params{};
        params.size = sizeof(params);

        int rc = get_hostfxr_path(buffer.data(), &buffer_size, &params);
        if (rc == 3) {
            buffer.resize(buffer_size);
            rc = get_hostfxr_path(buffer.data(), &buffer_size, &params);
        }

        if (rc != 0) {
            std::cerr << "get_hostfxr_path failed: " << rc << "\n";
            return false;
        }

        void* lib = LoadLib(buffer.data());
        if (!lib) {
            std::cerr << "Failed to load hostfxr library\n";
            return false;
        }

        init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetExport(lib, "hostfxr_initialize_for_runtime_config");
        get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetExport(lib, "hostfxr_get_runtime_delegate");
        close_fptr = (hostfxr_close_fn)GetExport(lib, "hostfxr_close");
        error_fptr = (hostfxr_get_error_message_fn)GetExport(lib, "hostfxr_get_error_message");

        return init_fptr && get_delegate_fptr && close_fptr;
    }

    std::filesystem::path ScriptEngine::GetCurrentModuleDirectory() {
#ifdef _WIN32
        HMODULE hm = NULL;
        // Use numeric values for flags:
        // 0x00000004 = FROM_ADDRESS
        // 0x00000002 = UNCHANGED_REFERENCE
        DWORD flags = 0x00000004 | 0x00000002;

        if (!GetModuleHandleExW(flags, (LPCWSTR)&GetCurrentModuleDirectory, &hm)) {
            DWORD error = GetLastError();
            char errorMsg[256];
            sprintf_s(errorMsg, "GetModuleHandleExW failed: 0x%08X", error);
            std::cerr << errorMsg << "\n";
            return "";
        }

        wchar_t path[MAX_PATH];
        if (GetModuleFileNameW(hm, path, MAX_PATH) == 0) {
            DWORD error = GetLastError();
            char errorMsg[256];
            sprintf_s(errorMsg, "GetModuleFileNameW failed: 0x%08X", error);
            std::cerr << errorMsg << "\n";
            return "";
        }

        std::filesystem::path p(path);
        return p.parent_path();
#else
        Dl_info dl_info;
        if (dladdr((void*)GetCurrentModuleDirectory, &dl_info) == 0) {
            std::cerr << "dladdr failed: " << dlerror() << "\n";
            return "";
        }

        std::filesystem::path p(dl_info.dli_fname);
        return p.parent_path();
#endif
    }

    bool ScriptEngine::InitializeCoreClr() {
        if (!LoadHostFxr()) {
            std::cerr << "Failed to load hostfxr\n";
            return false;
        }

        auto moduleDir = GetCurrentModuleDirectory();
        if (moduleDir.empty()) {
            std::cerr << "Failed to get module directory\n";
            return false;
        }

        // Debug output
        std::cout << "Module directory: " << moduleDir.string() << "\n";

        auto configPath = moduleDir / "ScriptAPI.runtimeconfig.json";
        auto assemblyPath = moduleDir / "ScriptAPI.dll";

        std::cout << "Config path: " << configPath.string() << "\n";
        std::cout << "Assembly path: " << assemblyPath.string() << "\n";

        if (!std::filesystem::exists(configPath)) {
            std::cerr << "Runtime config not found!\n";
            return false;
        }

        if (!std::filesystem::exists(assemblyPath)) {
            std::cerr << "Managed assembly not found!\n";
            return false;
        }

#ifdef _WIN32
        std::wstring wconfig = configPath.wstring();
        int rc = init_fptr(wconfig.c_str(), nullptr, &host_context);
#else
        int rc = init_fptr(configPath.string().c_str(), nullptr, &host_context);
#endif

        if (rc != 0 || !host_context) {
            std::cerr << "hostfxr_initialize_for_runtime_config failed: "
                << rc << " (0x" << std::hex << rc << ")\n";

            if (error_fptr) {
                const char_t* error_detail = nullptr;
                error_fptr(&error_detail);
                if (error_detail) {
                    std::cerr << "Error detail: " << ConvertToNarrow(error_detail) << "\n";
                }
            }
            return false;
        }

        // Get load_assembly_and_get_function_pointer delegate
        rc = get_delegate_fptr(host_context, hdt_load_assembly_and_get_function_pointer, (void**)&load_assembly_and_get_fn);
        if (rc != 0 || !load_assembly_and_get_fn) {
            std::cerr << "Failed to get load_assembly_and_get_function_pointer delegate: " << rc << "\n";
            return false;
        }

        return true;
    }

    bool ScriptEngine::Initialize() {
        try {
            return InitializeCoreClr();
        }
        catch (const std::exception& e) {
            std::cerr << "CoreCLR init failed: " << e.what() << "\n";
            return false;
        }
    }

    void ScriptEngine::Shutdown() {
        if (close_fptr && host_context) {
            close_fptr(host_context);
            host_context = nullptr;
        }
    }

    void ScriptEngine::RegisterNativeMethod(const std::string& name, StringCallback callback) {
        s_NativeMethods[name] = callback;
    }

    void ScriptEngine::ExecuteManagedFunction(const std::string& methodName) {
        if (!load_assembly_and_get_fn) {
            throw std::runtime_error("CoreCLR not initialized - load_assembly_and_get_function_pointer is null");
        }

        const char_t* type_name = STR("ScriptAPI.NativeExports, ScriptAPI");
        const char_t* delegate_type = UNMANAGEDCALLERSONLY_METHOD;
        void* function = nullptr;

        // Get the full path to the assembly
        auto assemblyPath = GetCurrentModuleDirectory() / "ScriptAPI.dll";

#ifdef _WIN32
        std::wstring wassembly = assemblyPath.wstring();
        std::wstring wmethod = ConvertToWide(methodName);
        const char_t* method_name = wmethod.c_str();
        const char_t* assembly_path = wassembly.c_str();
#else
        std::string sassembly = assemblyPath.string();
        const char_t* method_name = methodName.c_str();
        const char_t* assembly_path = sassembly.c_str();
#endif

        // Use load_assembly_and_get_function_pointer to get the function
        int rc = load_assembly_and_get_fn(
            assembly_path,
            type_name,
            method_name,
            delegate_type,
            nullptr,
            &function
        );

        if (rc != 0 || !function) {
            std::string error = "Failed to get function pointer: " + std::to_string(rc);

            if (error_fptr) {
                const char_t* error_detail = nullptr;
                error_fptr(&error_detail);
                if (error_detail) {
                    error += "\nDetail: " + ConvertToNarrow(error_detail);
                }
            }

            // Additional debug information
            error += "\nAssembly path: " + ConvertToNarrow(assembly_path);
            error += "\nType: " + ConvertToNarrow(type_name);
            error += "\nMethod: " + ConvertToNarrow(method_name);

            throw std::runtime_error(error);
        }

        try {
            reinterpret_cast<void(*)()>(function)();
            std::cout << "Successfully executed managed function: " << methodName << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Managed exception: " << e.what() << "\n";
            throw;
        }
    }

    void ScriptEngine::CreateString(StringCallback callback, const char* message) {
        if (callback) {
            callback(message);
        }
    }
}