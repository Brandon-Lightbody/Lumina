#include "ScriptAPI/ScriptEngine.h"
#include "coreclr_delegates.h"
#include "hostfxr.h"
#include "EmbeddedAssembly.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
#include <fileapi.h>
#include <direct.h>
#pragma comment(lib, "Shlwapi.lib")
#else
#include <dlfcn.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace Lumina {
    std::unordered_map<std::string, ScriptEngine::StringCallback> ScriptEngine::native_methods;

    // CoreCLR state
    static hostfxr_initialize_for_runtime_config_fn init_fptr = nullptr;
    static hostfxr_get_runtime_delegate_fn get_delegate_fptr = nullptr;
    static hostfxr_close_fn close_fptr = nullptr;
    static hostfxr_handle host_context = nullptr;
    static load_assembly_and_get_function_pointer_fn load_assembly = nullptr;

    // Implement get_hostfxr_path directly
#ifdef _WIN32
    using char_t = wchar_t;
#else
    using char_t = char;
#endif

    // Helper to parse version strings
    static bool parse_version(const std::string& version_str, std::vector<int>& parsed) {
        std::istringstream iss(version_str);
        std::string token;
        parsed.clear();

        while (std::getline(iss, token, '.')) {
            try {
                parsed.push_back(std::stoi(token));
            }
            catch (...) {
                return false;
            }
        }
        return !parsed.empty();
    }

    // Find latest version in a directory
    static std::string find_latest_version(const std::string& base_path) {
        std::vector<std::string> versions;
        std::string latest;
        std::vector<int> latest_ver;

#ifdef _WIN32
        std::string search_path = base_path + "\\*";
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
        if (hFind == INVALID_HANDLE_VALUE) return "";
        do {
            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                std::string dir_name = find_data.cFileName;
                if (dir_name != "." && dir_name != "..") {
                    versions.push_back(dir_name);
                }
            }
        } while (FindNextFileA(hFind, &find_data));
        FindClose(hFind);
#else
        DIR* dir = opendir(base_path.c_str());
        if (!dir) return "";
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR) {
                std::string dir_name = entry->d_name;
                if (dir_name != "." && dir_name != "..") {
                    versions.push_back(dir_name);
                }
            }
        }
        closedir(dir);
#endif

        for (const auto& ver : versions) {
            std::vector<int> current_ver;
            if (!parse_version(ver, current_ver)) continue;

            // Compare versions: major.minor.patch
            bool is_newer = false;
            for (size_t i = 0; i < current_ver.size(); ++i) {
                if (i >= latest_ver.size()) {
                    is_newer = true;
                    break;
                }
                if (current_ver[i] > latest_ver[i]) {
                    is_newer = true;
                    break;
                }
                else if (current_ver[i] < latest_ver[i]) {
                    break;
                }
            }

            if (is_newer || latest_ver.empty()) {
                latest = ver;
                latest_ver = current_ver;
            }
        }
        return latest;
    }

    // Updated hostfxr path discovery
    static int get_hostfxr_path(char_t* buffer, size_t* buffer_size, const char_t* assembly_path) {
        (void)assembly_path; // unused

        std::string base_path;
        const char* env_path = std::getenv("DOTNET_ROOT");
        if (env_path) {
            base_path = env_path;
        }
        else {
#ifdef _WIN32
            base_path = "C:\\Program Files\\dotnet";
#else
            base_path = "/usr/share/dotnet";
#endif
        }

        // Check host/fxr/<version> directory
        std::string fxr_path = base_path;
#ifdef _WIN32
        fxr_path += "\\host\\fxr";
#else
        fxr_path += "/host/fxr";
#endif

        std::string version = find_latest_version(fxr_path);
        if (version.empty()) {
            return 1; // Version not found
        }

        // Construct full hostfxr path
        std::string full_path = fxr_path;
#ifdef _WIN32
        full_path += "\\" + version + "\\hostfxr.dll";
        std::wstring wpath(full_path.begin(), full_path.end());
        if (wpath.size() + 1 > *buffer_size) {
            *buffer_size = wpath.size() + 1;
            return 1;
        }
        wcscpy((wchar_t*)buffer, wpath.c_str());
#else
        full_path += "/" + version + "/libhostfxr.so";
        if (full_path.size() + 1 > *buffer_size) {
            *buffer_size = full_path.size() + 1;
            return 1;
        }
        strcpy(buffer, full_path.c_str());
#endif
        return 0;
    }

    // Platform abstraction
#ifdef _WIN32
    void* LoadLibrary(const wchar_t* path) {
        return LoadLibraryW(path);
    }
    void* GetExport(void* h, const char* name) {
        return GetProcAddress((HMODULE)h, name);
    }
#else
    void* LoadLibrary(const char* path) {
        return dlopen(path, RTLD_LAZY);
    }
    void* GetExport(void* h, const char* name) {
        return dlsym(h, name);
    }
#endif

    // HostFxr loader
    bool LoadHostFxr(hostfxr_initialize_for_runtime_config_fn* init_fptr,
        hostfxr_get_runtime_delegate_fn* get_delegate_fptr,
        hostfxr_close_fn* close_fptr)
    {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
#else
        char buffer[PATH_MAX];
#endif

        size_t buffer_size = sizeof(buffer) / sizeof(buffer[0]);
        int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
        if (rc != 0) {
            return false;
        }

#ifdef _WIN32
        HMODULE lib = LoadLibraryW(buffer);
        if (!lib) {
            DWORD error = GetLastError();
            std::cerr << "Failed to load hostfxr: " << error << "\n";
            return false;
        }

        *init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(lib, "hostfxr_initialize_for_runtime_config");
        *get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(lib, "hostfxr_get_runtime_delegate");
        *close_fptr = (hostfxr_close_fn)GetProcAddress(lib, "hostfxr_close");
#else
        void* lib = dlopen(buffer, RTLD_LAZY);
        if (!lib) {
            std::cerr << "Failed to load hostfxr: " << dlerror() << "\n";
            return false;
        }

        *init_fptr = (hostfxr_initialize_for_runtime_config_fn)dlsym(lib, "hostfxr_initialize_for_runtime_config");
        *get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)dlsym(lib, "hostfxr_get_runtime_delegate");
        *close_fptr = (hostfxr_close_fn)dlsym(lib, "hostfxr_close");
#endif

        return (*init_fptr && *get_delegate_fptr && *close_fptr);
    }

    void ScriptEngine::InitializeCoreClr() {
        if (!LoadHostFxr(&init_fptr, &get_delegate_fptr, &close_fptr)) {
            throw std::runtime_error("Failed to load hostfxr");
        }

        char temp_path[MAX_PATH];
#ifdef _WIN32
        GetTempPathA(MAX_PATH, temp_path);
        std::string config_path = std::string(temp_path) + "ManagedApp.runtimeconfig.json";
#else
        strcpy(temp_path, "/tmp/");
        std::string config_path = "/tmp/ManagedApp.runtimeconfig.json";
#endif

        std::ofstream config_file(config_path);
        config_file << R"({
            "runtimeOptions": {
                "tfm": "net8.0",
                "framework": {
                    "name": "Microsoft.NETCore.App",
                    "version": "8.0.0"
                }
            }
        })";
        config_file.close();

        hostfxr_initialize_parameters init_params{};
        init_params.size = sizeof(init_params);

#ifdef _WIN32
        std::wstring wconfig_path = std::wstring(config_path.begin(), config_path.end());
        const wchar_t* config_path_ptr = wconfig_path.c_str();
#else
        const char* config_path_ptr = config_path.c_str();
#endif

        int rc = init_fptr(config_path_ptr, &init_params, &host_context);
        if (rc != 0 || host_context == nullptr) {
            if (close_fptr) close_fptr(host_context);
            throw std::runtime_error("Failed to initialize CoreCLR");
        }

        rc = get_delegate_fptr(
            host_context,
            hdt_load_assembly_and_get_function_pointer,
            (void**)&load_assembly
        );
        if (rc != 0 || load_assembly == nullptr) {
            if (close_fptr) close_fptr(host_context);
            throw std::runtime_error("Failed to get load assembly delegate");
        }
    }

    void ScriptEngine::LoadEmbeddedAssembly() {
        char temp_path[MAX_PATH];
#ifdef _WIN32
        GetTempPathA(MAX_PATH, temp_path);
        std::string assembly_path = std::string(temp_path) + "Lumina.Managed.dll";
#else
        strcpy(temp_path, "/tmp/");
        std::string assembly_path = "/tmp/Lumina.Managed.dll";
#endif

        FILE* file = fopen(assembly_path.c_str(), "wb");
        if (file) {
            fwrite(Lumina::Embedded::assembly, 1, Lumina::Embedded::assembly_length, file);
            fclose(file);
        }
        else {
            throw std::runtime_error("Failed to write embedded assembly to temp file");
        }
    }

    bool ScriptEngine::Initialize() {
        try {
            InitializeCoreClr();
            LoadEmbeddedAssembly();
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[ScriptEngine] Initialization failed: " << e.what() << "\n";
            return false;
        }
    }

    void ScriptEngine::Shutdown() {
        if (close_fptr && host_context) {
            close_fptr(host_context);
        }
    }

    void ScriptEngine::RegisterNativeMethod(const std::string& name, StringCallback callback) {
        native_methods[name] = callback;
    }

    void ScriptEngine::ExecuteManagedFunction(const std::string& methodName) {
        if (!load_assembly) throw std::runtime_error("CoreCLR not initialized");

        using managed_fn = void (*)();
        managed_fn function = nullptr;

#ifdef _WIN32
        std::wstring wmethodName = std::wstring(methodName.begin(), methodName.end());
        const wchar_t* method_name = wmethodName.c_str();
        const wchar_t* assembly_path = L"ManagedApp.dll";
        const wchar_t* type_name = L"ManagedApp, ManagedApp";
#else
        const char* method_name = methodName.c_str();
        const char* assembly_path = "ManagedApp.dll";
        const char* type_name = "ManagedApp, ManagedApp";
#endif

        int rc = load_assembly(
            assembly_path,
            type_name,
            method_name,
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&function
        );

        if (rc == 0 && function) {
            function();
        }
        else {
            std::cerr << "[ScriptEngine] Failed to execute managed function: " << methodName << "\n";
        }
    }

    void ScriptEngine::CreateString(StringCallback callback, const char* message) {
        if (callback) {
            callback(message);
        }
    }
} // namespace Lumina