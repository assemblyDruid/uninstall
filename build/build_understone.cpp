//
// Understone Engine Build Tools
//
// This application autogenerates source files and baked data, compiles
// Understone Engine, and performs code analysis. It is shipped as a single C++
// source file for simple building. The only requirement to build the Understone
// Engine, including this tool, is a C++ compiler with >= std::c++20 features.
//
// clang-format off
//
// Reccommended compilation invocations for common compilers (may need to update -std=c++<version> and -o <executable_path> for OS executable extensions):
// [ clang::release ]: clang-format -i build_understone.cpp && clang++ -Wall -Werror -pedantic -g0 -O3 --optimize -Ofast3 -std=c++20 build_understone.cpp -o ../bin/build_understone
// [ clang::debug   ]: clang-format -i build_understone.cpp && clang++ -Wall -Werror -pedantic -std=c++20 -g3 -O0 build_understone.cpp -o ../bin/build_understone
//
// clang-format on
//

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

#define UNDERSTONE_DEBUG_MACRO_STR "__UE_debug__"
#define UNDERSTONE_EXE_NAME        "Understone"

#if __linux__
#define P_OPEN_(command, mode) popen(command, mode)
#define P_CLOSE_(pipe)         pclose(pipe)
#else
#define P_OPEN_(command, mode) _popen(command, mode)
#define P_CLOSE_(pipe)         _pclose(pipe)
#endif // __linux__

#define CLI_HELP_STR    "-help"
#define CLI_CLANGCL_STR "-clang-cl"
#define CLI_CLANG_STR   "-clang"
#define CLI_DEBUG_STR   "-debug"
#define CLI_RELEASE_STR "-release"
#define CLI_VSDK_STR    "-vulkan_sdk"
#define CLI_NOCF_STR    "-no_code_format"
#define CLI_RAB_STR     "-run_after_build"
#define CLI_NOCA_STR    "-no_code_analysis"
#define CLI_RARGS_STR   "-run_after_build_args"

const std::string kUnderstoneShaderTypeEnumPrefix = "uVK_SHADER_TYPE_";
const std::string kShaderNameId                   = "@uCommonName";
const std::string kShaderTypeId                   = "@uShaderType";
const std::string kAutoGenFileNameId              = "@uAutoGenFileName";
const std::string kVertexTypeString               = "VERTEX";
const std::string kFragmentTypeString             = "FRAGMENT";

using CommandLineArguments                           = std::vector< std::string >;
using CommandLineArgumentOptions                     = std::vector< std::pair< std::string /*arg*/, std::string /*desc*/ > >;
const CommandLineArgumentOptions kAllCommandLineArgs = {
    { CLI_HELP_STR, "Shows this message." },
    { CLI_CLANGCL_STR, "Compile using clang-cl" },
    { CLI_CLANG_STR, "Compile using clang" },
    { CLI_DEBUG_STR, "Compile using debug settings" },
    { CLI_RELEASE_STR, "Compile using release settings" },
    { CLI_VSDK_STR, "Full path to your Vulkan SDK install." },
    { CLI_NOCF_STR, "Do not run clang-format on source & header files." },
    { CLI_RAB_STR, "Run Understone after a sucessful build." },
    { CLI_RARGS_STR, "Comma separated (no spaces) arguments to pass to Understone when running post-build." },
    { CLI_NOCA_STR, "Do not run code analysis after a successful build." },
};

// Supported compilers
enum class Compiler
{
    kClang = 0,
    kClangCL,
    kCount
};
const Compiler default_user_compiler = Compiler::kClang;

enum class ShaderCompiler
{
    kGlslc = 0,
    kCount
};
const ShaderCompiler default_user_shader_compiler = ShaderCompiler::kGlslc;

// Compilation flags (bools)
enum class CompilationFlags
{
    kDebug = 0,
    kRelease,
    kCount
};
using UserCompilationFlags                                = std::map< CompilationFlags, bool >;
const UserCompilationFlags default_user_compilation_flags = { { CompilationFlags::kDebug, true }, { CompilationFlags::kRelease, false } };

bool
ValidateUserCompilationFlags(const UserCompilationFlags& user_compilation_flags, std::stringstream& error_message, size_t& errors_logged)
{
    if (user_compilation_flags.size() != static_cast< size_t >(CompilationFlags::kCount))
    {
        error_message << "  UserCompilationFlags -- expected size: " << static_cast< size_t >(CompilationFlags::kCount) << ", actual size: " << user_compilation_flags.size()
                      << std::endl;
        errors_logged++;

        return false;
    }

    return true;
}

// Compilation options (strings)
enum class CompilationOptions
{
    kVulkanSdkPath = 0,
    kCount
};
using UserCompilationOptions                                  = std::map< CompilationOptions, std::string >;
const UserCompilationOptions default_user_compilation_options = { { CompilationOptions::kVulkanSdkPath, "" } };

bool
ValidateUserCompilationOptions(const UserCompilationOptions& user_compilation_options, std::stringstream& error_message, size_t& errors_logged)
{
    if (user_compilation_options.size() != static_cast< size_t >(CompilationOptions::kCount))
    {
        error_message << "  UserCompilationOptions -- expected size: " << static_cast< size_t >(CompilationOptions::kCount) << ", actual size: " << user_compilation_options.size()
                      << std::endl;
        errors_logged++;

        return false;
    }

    return true;
}

// Build flags (bools)
enum class BuildFlags
{
    kRunCodeFormatter = 0,
    kRunCodeAnalysis,
    kRunAfterBuild,
    kCount
};
using UserBuildFlags                          = std::map< BuildFlags, bool >;
const UserBuildFlags default_user_build_flags = { { BuildFlags::kRunAfterBuild, false }, { BuildFlags::kRunCodeAnalysis, true }, { BuildFlags::kRunCodeFormatter, true } };
std::string          kRunAfterBuildArgs       = "";

bool
ValidateUserBuildFlags(const UserBuildFlags& user_build_flags, std::stringstream& error_message, size_t& errors_logged)
{
    if (user_build_flags.size() != static_cast< size_t >(BuildFlags::kCount))
    {
        error_message << "  UserBuildFlags -- expected size: " << static_cast< size_t >(BuildFlags::kCount) << ", actual size: " << user_build_flags.size() << std::endl;
        errors_logged++;

        return false;
    }

    return true;
}

enum class OutputType
{
    kInfo = 0,
    kWarning,
    kError,
    kCount
};

struct PipeReturnType
{
    size_t      return_code = -1;
    std::string output      = "";
    bool        success     = false;
};

void
Print(const std::string& message, OutputType output_type = OutputType::kInfo)
{
    switch (output_type)
    {
        case (OutputType::kInfo):
        {
            std::cout << message << std::flush;
            break;
        }
        case (OutputType::kWarning):
        {
            std::cout << "[ warning ] " << message << std::flush;
            break;
        }
        case (OutputType::kError):
        {
            std::cerr << "[ error ] " << message << std::flush;
            break;
        }
        default:
        {
            Print("[ Print()/PrintLn() ] Unsupported OutputType", OutputType::kError);
        }
    }
}

void
TrimString(std::string& str)
{
    // Trim leading whitespace
    while (str[0] == ' ')
    {
        str.erase(0, 1);
    }

    // Trim trailing whitespace
    while (str[str.size() - 1] == ' ')
    {
        str.erase(str.size() - 1, 1);
    }
}

void
PrintLn(const std::string& message, OutputType output_type = OutputType::kInfo)
{
    Print((message + "\n"), output_type);
}

enum class ShaderType
{
    kNone = 0,
    kVertex,
    kFragment
};

struct BakedShaderInfo
{
    std::string shader_path;
    std::string spirv_path;
    std::string auto_gen_file_name;
    std::string auto_gen_output_file_path;
    std::string common_name;
    ShaderType  shader_type;
};

ShaderType
StringToShaderType(const std::string& type)
{
    if (type == kVertexTypeString)
    {
        return ShaderType::kVertex;
    }
    else if (type == kFragmentTypeString)
    {
        return ShaderType::kFragment;
    }
    else
    {
        return ShaderType::kNone;
    }
}

std::string
ShaderTypeToString(const ShaderType& type)
{
    switch (type)
    {
        case (ShaderType::kVertex):
        {
            return "VERTEX";
        }
        case (ShaderType::kFragment):
        {
            return "FRAGMENT";
        }
        default:
        {
            return "";
        }
    }
}

PipeReturnType
RunProcess(const std::string& command, const std::string& mode, const size_t& max_buffer_len = 256)
{
    PipeReturnType p_open_return = {};

    std::string shell_consious_command = command;
    // On POSIX, popen calls 'sh'. Most folks use 'bash', or something else, which is where
    // their aliases will be handled. We want the good people to have their aliases.
#if __linux__
    char* user_shell = getenv("SHELL");
    if (nullptr != user_shell)
    {
        shell_consious_command = std::string(user_shell) + " -p -c '" + command + '\'';
    }
#endif // __linux__

    FILE* stream = P_OPEN_(shell_consious_command.c_str(), mode.c_str());
    if (nullptr == stream)
    {
        PrintLn("Unable to open OS pipe.", OutputType::kError);
        p_open_return.success = false;
        return p_open_return;
    }
    std::vector< char > stream_buffer(max_buffer_len);
    fgets(stream_buffer.data(), max_buffer_len, stream);
    p_open_return.output = std::string(stream_buffer.data());

    // It is not an error to not have read from the entire pipe, continue
    p_open_return.return_code = P_CLOSE_(stream);
    p_open_return.success     = true;
    return p_open_return;
}

#ifndef __linux__
#define ToPosixPath(str) _API_TO_POSIX_PATH(str)
#else
#define ToPosixPath(str) str
#endif // __linux__
// Attn: multiple copies
std::string
_API_TO_POSIX_PATH(std::string path)
{
    for (size_t index = 0; index < path.size(); index++)
    {
        if (path[index] == '\\')
        {
            path[index] = '/';
        }
    }

    return path;
}

class BuildFlagInvocationGenerator
{
  public:
    BuildFlagInvocationGenerator(const std::string                     understone_root_dir,
                                 const std::set< std::string >&        source_files,
                                 const std::set< std::string >&        header_files,
                                 const std::vector< BakedShaderInfo >& baked_shader_info,
                                 const UserBuildFlags&                 user_build_flags)
    {
        if (!UserArgumentSanityCheck(user_build_flags))
        {
            PrintLn("Failed user argument sanity check. Will not continue with build "
                    "flag invocation generation.",
                    OutputType::kError);
            is_ok_ = false;
            return;
        }

        GenerateBuildFlagInvocation(understone_root_dir, source_files, header_files, baked_shader_info, user_build_flags);
        if (!is_ok_)
        {
            PrintLn("Unable to generate build flag invocation.", OutputType::kError);
            return;
        }
    }

    bool
    IsOk()
    {
        return is_ok_;
    }

    std::string
    GetCodeFormatInvocation()
    {
        return code_format_invocation_;
    }

  private:
    void
    GenerateBuildFlagInvocation(const std::string                     understone_root_dir,
                                const std::set< std::string >&        source_files,
                                const std::set< std::string >&        header_files,
                                const std::vector< BakedShaderInfo >& baked_shader_info,
                                const UserBuildFlags&                 user_build_flags)
    {
        if (!is_ok_)
        {
            PrintLn("Cannot continue with build flag invocation generation, "
                    "generator reports invalid state.",
                    OutputType::kError);
            return;
        }

        // Code formatting
        if (user_build_flags.at(BuildFlags::kRunCodeFormatter))
        {
            std::string invocation_exe      = "";
            std::string invocation_defaults = "-i -Werror --sort-includes ";

            // Determine if clang-format is in the system env path
            size_t         clang_format_return = -1;
            PipeReturnType pipe_return         = {};
#if _WIN32
            pipe_return = RunProcess("clang-format --version >nul 2>&1", "r");
            if (pipe_return.success)
            {
                invocation_exe      = "clang-format ";
                clang_format_return = pipe_return.return_code;
            }
#else
            // First check to see if "clang-format" is exe name on Posix (likely)
            // Note: clang-format returns 0 for success on posix
            pipe_return = RunProcess("clang-format --version > /dev/null 2>&1", "r");

            if (0 != pipe_return.return_code || !pipe_return.success)
            {
                // Before failing, check for "clang-format.exe" as exe name on posix
                // (rare) Seems unintuative, but it happens and causes issues.
                pipe_return = RunProcess("clang-format.exe --version > /dev/null 2>&1", "r");
                if (0 != pipe_return.return_code || !pipe_return.success)
                {
                    clang_format_return = pipe_return.return_code;
                    invocation_exe      = "clang-format.exe ";
                }
            }
            else
            {
                clang_format_return = pipe_return.return_code;
                invocation_exe      = "clang-format ";
            }
#endif // _WIN32

            if (0 != clang_format_return)
            {
                PrintLn("Cannot use clang-format, please ensure it is defined in the "
                        "system environment path.",
                        OutputType::kError);
#if __linux__
                PrintLn("Note: Ceate a symlink in /usr/bin/ for 'clang-format' to point to your current version. The clang and clang++ "
                        "compilers are automatically symlinked to their current versions on most installations, but not clang-format.");
#endif // __linux__
                is_ok_ &= false;
                return;
            }

            std::string invocation = invocation_exe + invocation_defaults + " ";

            // Add source files
            for (const auto& source_file : source_files)
            {
                invocation += source_file + " ";
            }

            // Add baked shader files
            for (const auto& baked_shader : baked_shader_info)
            {
                std::filesystem::path baked_shader_path(baked_shader.auto_gen_output_file_path);
                if (std::filesystem::exists(baked_shader_path))
                {
                    invocation += baked_shader.auto_gen_output_file_path + " ";
                }
                else
                {
                    PrintLn("Unable to find file for code formatting: " + baked_shader.auto_gen_output_file_path, OutputType::kError);
                    is_ok_ &= false;
                    return;
                }
            }

            // Add header files
            for (const auto& header_file : header_files)
            {
                invocation += header_file + " ";
            }

            code_format_invocation_ = invocation;
        }
    }

    bool
    UserArgumentSanityCheck(const UserBuildFlags& user_build_flags)
    {
        bool              success       = true;
        size_t            errors_logged = 0;
        std::stringstream error_message;
        error_message << "The following arguments have failed the sanity check:" << std::endl;

        success &= ValidateUserBuildFlags(user_build_flags, error_message, errors_logged);

        if (errors_logged || !success)
        {
            PrintLn(error_message.str(), OutputType::kError);
            is_ok_ &= false;
            return false;
        }

        return true;
    }

    bool        is_ok_                  = true;
    std::string code_format_invocation_ = "";
};

class CompilerInvocationGenerator
{
  public:
    CompilerInvocationGenerator(const UserCompilationFlags&   user_compilation_flags,
                                const UserCompilationOptions& user_compilation_options,
                                const UserBuildFlags&         user_build_flags) // mutable
    {
        if (!UserArgumentSanityCheck(user_compilation_flags, user_compilation_options, user_build_flags))
        {
            PrintLn("Failed user argument sanity check. Will not continue with "
                    "compilation invocation generation.",
                    OutputType::kError);
            is_ok_ = false;
        }
    }

    bool
    IsOk()
    {
        return is_ok_;
    }

    std::string
    GetSourceInvocation()
    {
        return source_invocation_;
    }

    std::string
    GetShaderInvocation()
    {
        return shader_invocation_;
    }

    void
    GenerateShaderInvocation(const std::string&              understone_root_dir,
                             const std::set< std::string >&  shader_files,
                             const ShaderCompiler&           shader_compiler,
                             const UserCompilationFlags&     user_compilation_flags,
                             std::vector< BakedShaderInfo >& baked_shader_info) // mutable
    {
        if (!is_ok_)
        {
            return;
        }

        PrintLn("Generating shader compilation invocation...");
        switch (shader_compiler)
        {
            case (ShaderCompiler::kGlslc):
            {
                GenerateGlslcInvocation(understone_root_dir, shader_files, user_compilation_flags, baked_shader_info);
                break;
            }
            default:
            {
                PrintLn("Unsupported shader compiler", OutputType::kError);
                is_ok_ = false;
                return;
            }
        }
    }

    void
    GenerateSourceInvocation(const std::string&              understone_root_dir,
                             const std::set< std::string >&  source_files,
                             const std::set< std::string >&  header_files,
                             const std::set< std::string >&  header_directories,
                             const Compiler&                 compiler,
                             const UserCompilationFlags&     user_compilation_flags,
                             const UserCompilationOptions&   user_compilation_options,
                             const UserBuildFlags&           user_build_flags,
                             std::vector< BakedShaderInfo >& baked_shader_info) // mutable
    {
        if (!is_ok_)
        {
            return;
        }

        PrintLn("Generating source compilation invocation...");
        switch (compiler)
        {
            case (Compiler::kClang):
            {
                GenerateClangInvocation(understone_root_dir,
                                        source_files,
                                        header_directories,
                                        user_compilation_flags,
                                        user_compilation_options,
                                        user_build_flags,
                                        baked_shader_info);
                break;
            }
            case (Compiler::kClangCL):
            {
#if __linux__
                PrintLn("clang-cl is not supported on POSIX systems.", OutputType::kError);
                is_ok_ = false;
#elif _WIN32
                GenerateClangCLInvocation(understone_root_dir,
                                          source_files,
                                          header_directories,
                                          user_compilation_flags,
                                          user_compilation_options,
                                          user_build_flags,
                                          baked_shader_info);
#endif // __linux__
                break;
            }
            default:
            {
                PrintLn("Unsupported compiler", OutputType::kError);
                is_ok_ = false;
                return;
            }
        }
    }

  private:
    bool
    UserArgumentSanityCheck(const UserCompilationFlags&   user_compilation_flags   = default_user_compilation_flags,
                            const UserCompilationOptions& user_compilation_options = default_user_compilation_options,
                            const UserBuildFlags&         user_build_flags         = default_user_build_flags)
    {
        bool              success       = true;
        size_t            errors_logged = 0;
        std::stringstream error_message;
        error_message << "The following arguments have failed the sanity check:" << std::endl;

        success &= ValidateUserCompilationFlags(user_compilation_flags, error_message, errors_logged);
        success &= ValidateUserCompilationOptions(user_compilation_options, error_message, errors_logged);
        success &= ValidateUserBuildFlags(user_build_flags, error_message, errors_logged);

        if (errors_logged || !success)
        {
            PrintLn(error_message.str(), OutputType::kError);
            is_ok_ = false;
            return false;
        }

        return true;
    }

#ifdef _WIN32
    std::string
    GetLatestDefaultVulkanSdkPathWin32()
    {
        if (!is_ok_)
        {
            return "";
        }

        std::filesystem::path default_location("C:/VulkanSDK/");
        if (!std::filesystem::exists(default_location))
        {
            PrintLn("Vulkan does not appear to be installed in the default location.", OutputType::kError);
            is_ok_ = false;
            return "";
        }

        std::string latest_sdk_path = "";
        size_t      version[4]      = { 0, 0, 0, 0 };
        for (const auto& dir : std::filesystem::directory_iterator(default_location))
        {
            std::string dir_name = ToPosixPath(dir.path().string());
            dir_name             = dir_name.substr(dir_name.find_last_of('/') + 1, dir_name.size());
            if (!dir_name.size())
            {
                PrintLn("Corrupt path when searching for Vulkan SDK.", OutputType::kError);
                is_ok_ = false;
                return "";
            }

            size_t tmp_version[4]    = { 0, 0, 0, 0 };
            size_t last_dot_location = 0;
            size_t dot_count         = 0;
            size_t substr_start      = 0;
            size_t substr_end        = 0;
            for (size_t index = 0; index < dir_name.size(); index++)
            {
                // Write the first three numbers to tmp_version[0 .. 2]
                if (dir_name[index] == '.' && index != 0)
                {
                    substr_start           = last_dot_location == 0 ? last_dot_location : last_dot_location + 1;
                    substr_end             = index - substr_start;
                    tmp_version[dot_count] = std::stoul(dir_name.substr(substr_start, substr_end));
                    last_dot_location      = index;
                    dot_count++;
                }
            }

            if (dot_count != 3)
            {
                PrintLn("Unable to parse Vulkan SDK directory name. Typical #.#.#.# "
                        "not present.",
                        OutputType::kError);
                is_ok_ = false;
                return "";
            }

            // Write the last number to tmp_version[3]
            substr_start           = last_dot_location + 1;
            substr_end             = dir_name.size();
            tmp_version[dot_count] = std::stoul(dir_name.substr(substr_start, substr_end));

            // Use the latest version
            for (size_t index = 0; index < 4; index++)
            {
                if (tmp_version[index] > version[index])
                {
                    for (size_t _index = index; _index < 4; _index++)
                    {
                        version[_index] = tmp_version[_index];
                    }

                    break;
                }
                else if (tmp_version[index] == version[index])
                {
                    continue;
                }

                break;
            }
        }

        latest_sdk_path = "C:/VulkanSDK/" + std::to_string(version[0]) + "." + std::to_string(version[1]) + "." + std::to_string(version[2]) + "." + std::to_string(version[3]);
        if (!std::filesystem::exists(latest_sdk_path))
        {
            PrintLn("Function GetLatestDefaultVulkanSdkPath() returned an invalid path.", OutputType::kError);
            is_ok_ = false;
            return "";
        }

        return latest_sdk_path;
    }

    void
    GenerateClangCLInvocation(const std::string&              understone_root_dir,
                              const std::set< std::string >&  source_files,
                              const std::set< std::string >&  header_directories,
                              const UserCompilationFlags&     user_compilation_flags,
                              const UserCompilationOptions&   user_compilation_options,
                              const UserBuildFlags&           user_build_flags,
                              std::vector< BakedShaderInfo >& baked_shader_info) // mutable
    {
        if (!is_ok_)
        {
            return;
        }

        // Ensure that clang-cl is in the sys env path (clang-cl returns 0 on
        // success)
        int            clang_cl_ret_value = -1;
        PipeReturnType pipe_return        = RunProcess("clang-cl --version >nul 2>&1", "r");
        clang_cl_ret_value                = pipe_return.return_code;
        if (clang_cl_ret_value || !pipe_return.success)
        {
            PrintLn("Cannot use clang-cl, please ensure it is defined in the system "
                    "environment path.",
                    OutputType::kError);
            is_ok_ = false;
            return;
        }

        std::string invocation = "clang-cl -WX -W4 /std:c++latest -Wno-gnu-anonymous-struct "
                                 "-Wno-nested-anon-types -Wno-gnu-zero-variadic-macro-arguments "
                                 "-Wno-c99-extensions ";

        // Compilation flags
        std::string compilation_flags_invocation = "";
        {
            bool              debug_flag     = user_compilation_flags.at(CompilationFlags::kDebug);
            const std::string debug_string   = "-Od -MTd -Zi -D" + std::string(UNDERSTONE_DEBUG_MACRO_STR) + "#1 ";
            const std::string release_string = "-MT -O2 -Ot -Ob2 ";
            debug_flag ? (compilation_flags_invocation += debug_string) : (compilation_flags_invocation += release_string);
        }

        // Source files
        std::string source_files_invocation = "";
        {
            for (const std::string& source : source_files)
            {
                source_files_invocation += (source + " ");
            }
        }

        // Baked shader files (enforce header syntax)
        std::string baked_shader_files_invocation = "";
        {
            for (const BakedShaderInfo& baked_shader : baked_shader_info)
            {
                if (std::filesystem::exists(baked_shader.auto_gen_output_file_path))
                {
                    baked_shader_files_invocation += ("-I" + baked_shader.auto_gen_output_file_path + " ");
                }
                else
                {
                    PrintLn("Unable to locate a baked shader listed for compilation: " + baked_shader.auto_gen_output_file_path);
                    is_ok_ &= false;
                    return;
                }
            }
        }

        // Header directories
        std::string header_directories_invocation = "";
        {
            // Note: Win32 vulkan headers will be added in compilation options section
            for (const std::string& header_dir : header_directories)
            {
                header_directories_invocation += ("-I" + header_dir + " ");
            }
        }

        // Compilation options
        std::string compilation_options_invocation = "";
        {
            // kOutputExecutableName
            std::filesystem::path bin_directory(understone_root_dir + "/bin");
            if (std::filesystem::exists(bin_directory) || std::filesystem::create_directory(bin_directory))
            {
                compilation_options_invocation += "-Fe" + ToPosixPath(bin_directory.string()) + '/' + std::string(UNDERSTONE_EXE_NAME) + ".exe ";
            }
            else
            {
                PrintLn("Unable to acquire the Understone bin directory.", OutputType::kError);
                is_ok_ &= false;
                return;
            }

            // kLinkParameters
            // Note: hard-coded vulkan-1.lib
            std::string default_link_parameters = "-link -SUBSYSTEM:CONSOLE -NXCOMPAT -MACHINE:x64 "
                                                  "-NODEFAULTLIB:MSVCRTD user32.lib vulkan-1.lib ";
            // kVulkanSdkPath
            std::string vulkan_sdk_path = user_compilation_options.at(CompilationOptions::kVulkanSdkPath);
            if (!vulkan_sdk_path.size())
            {
                std::string latest_vulkan_sdk_path = ToPosixPath(GetLatestDefaultVulkanSdkPathWin32());
                if (!latest_vulkan_sdk_path.size())
                {
                    PrintLn("Unable to locate a Vulkan SDK installation. Please specify "
                            "the precise path.",
                            OutputType::kError);
                    is_ok_ &= false;
                    return;
                }

                vulkan_sdk_path = latest_vulkan_sdk_path;
            }
            default_link_parameters += "-LIBPATH:" + vulkan_sdk_path + "/Lib ";

            compilation_options_invocation += default_link_parameters;

            // Note: Add Win32 Vulkan header include now that SDK location is known
            header_directories_invocation += "-I" + vulkan_sdk_path + "/Include ";
        }

        if (is_ok_)
        {
            source_invocation_ =
              invocation + compilation_flags_invocation + source_files_invocation + header_directories_invocation + baked_shader_files_invocation + compilation_options_invocation;
        }
    }
#endif // _WIN32

    void
    GenerateClangInvocation(const std::string&              understone_root_dir,
                            const std::set< std::string >&  source_files,
                            const std::set< std::string >&  header_directories,
                            const UserCompilationFlags&     user_compilation_flags,
                            const UserCompilationOptions&   user_compilation_options,
                            const UserBuildFlags&           user_build_flags,
                            std::vector< BakedShaderInfo >& baked_shader_info) // mutable
    {
        if (!is_ok_)
        {
            return;
        }

        // Ensure that clang is in the sys env path (clang returns 0 on success)
        int            clang_ret_value = -1;
        PipeReturnType pipe_return     = {};
#if __linux__
        pipe_return = RunProcess("clang++ --version > /dev/null 2>&1", "r");
#else
        pipe_return = RunProcess("clang++ --version >nul 2>&1", "r");
#endif
        clang_ret_value = pipe_return.return_code;
        if (clang_ret_value || !pipe_return.success)
        {
            PrintLn("Cannot use clang, please ensure it is defined in the system "
                    "environment path.",
                    OutputType::kError);
            is_ok_ = false;
            return;
        }

        std::string invocation = "clang++ -Wall -Werror -pedantic -std=c++20 "
                                 "-Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-gnu-zero-variadic-macro-arguments  "
                                 "-Wno-c99-extensions ";

        // Compilation flags
        std::string compilation_flags_invocation = "";
        {
            bool              debug_flag     = user_compilation_flags.at(CompilationFlags::kDebug);
            const std::string debug_string   = "-g3 -O0 -D" + std::string(UNDERSTONE_DEBUG_MACRO_STR) + "=1 ";
            const std::string release_string = "-g0 -O3 -Ofast3 ";
            debug_flag ? (compilation_flags_invocation += debug_string) : (compilation_flags_invocation += release_string);
        }

        // Source files
        std::string source_files_invocation = "";
        {
            for (const std::string& source : source_files)
            {
                source_files_invocation += (source + " ");
            }
        }

        // Baked shader files (enforce header syntax)
        std::string baked_shader_files_invocation = "";
        {
            for (const BakedShaderInfo& baked_shader : baked_shader_info)
            {
                if (std::filesystem::exists(baked_shader.auto_gen_output_file_path))
                {
                    baked_shader_files_invocation += ("-I" + baked_shader.auto_gen_output_file_path + " ");
                }
                else
                {
                    PrintLn("Unable to locate a baked shader listed for compilation: " + baked_shader.auto_gen_output_file_path);
                    is_ok_ &= false;
                    return;
                }
            }
        }

        // Header directories
        std::string header_directories_invocation = "";
        {
            // Note: Win32 vulkan headers will be added in compilation options section
            for (const std::string& header_dir : header_directories)
            {
                header_directories_invocation += ("-I" + header_dir + " ");
            }
        }

        // Compilation options
        std::string compilation_options_invocation = "";
        {
            // kVulkanSdkPath
            // Note: hard-coded vulkan-1.lib
            std::string vulkan_sdk_path = user_compilation_options.at(CompilationOptions::kVulkanSdkPath);
            if (!vulkan_sdk_path.size())
            {
#if _WIN32
                std::string latest_vulkan_sdk_path = ToPosixPath(GetLatestDefaultVulkanSdkPathWin32());
                if (!latest_vulkan_sdk_path.size())
                {
                    PrintLn("Unable to locate a Vulkan SDK installation. Please specify "
                            "the precise path.",
                            OutputType::kError);
                    is_ok_ &= false;
                    return;
                }

                vulkan_sdk_path = latest_vulkan_sdk_path;
#endif // _WIN32
            }

            // Note: Add Win32 Vulkan header include now that SDK location is known
            header_directories_invocation += "-I" + vulkan_sdk_path + "/Include ";

            // Link options
            compilation_options_invocation += "-L" + vulkan_sdk_path + "/Lib ";
#if _WIN32
            compilation_options_invocation += "-lvulkan-1.lib ";
            compilation_options_invocation += "-luser32.lib ";
#else
            compilation_options_invocation += "-lvulkan ";
#endif // _WIN32
        }

        // Output directory
        std::string output_directory_invocation = "";
        {
            std::filesystem::path bin_directory(understone_root_dir + "/bin");
            if (std::filesystem::exists(bin_directory) || std::filesystem::create_directory(bin_directory))
            {
                output_directory_invocation += "-o " + ToPosixPath(bin_directory.string());
#if _WIN32
                output_directory_invocation += '/' + std::string(UNDERSTONE_EXE_NAME) + ".exe ";
#else
                output_directory_invocation += '/' + std::string(UNDERSTONE_EXE_NAME) + " ";
#endif
            }
            else
            {
                PrintLn("Unable to acquire the Understone bin directory.", OutputType::kError);
                is_ok_ &= false;
                return;
            }
        }

        if (is_ok_)
        {
            source_invocation_ = invocation + compilation_flags_invocation + source_files_invocation + header_directories_invocation + baked_shader_files_invocation +
                                 compilation_options_invocation + output_directory_invocation;
        }
    }

    void
    GenerateGlslcInvocation(const std::string&              understone_root_dir,
                            const std::set< std::string >&  shader_files,
                            const UserCompilationFlags&     user_compilation_flags,
                            std::vector< BakedShaderInfo >& baked_shader_info)
    {
        if (!is_ok_)
        {
            return;
        }

        // Ensure that glslc is in the sys env path (glslc returns 0 on success)
        int            glslc_ret_value = -1;
        PipeReturnType pipe_return     = {};
#ifdef _WIN32
        pipe_return = RunProcess("glslc --version >nul 2>&1", "r");
#else
        pipe_return = RunProcess("glslc --version > /dev/null 2>&1", "r");
#endif // _WIN32

        glslc_ret_value = pipe_return.return_code;
        if (glslc_ret_value || !pipe_return.success)
        {
            PrintLn("Cannot use glslc, please ensure it is defined in the system "
                    "environment path.",
                    OutputType::kError);
            is_ok_ = false;
            return;
        }

        // Note: At this time, Glslc can only compile & link a single shader at a
        // time. Thus, It must be run once for every shader.
        std::string invocation      = "";
        std::string invocation_base = "glslc -Werror ";

        std::filesystem::path bin_directory(understone_root_dir + "/bin");
        std::filesystem::path shaders_bin_directory(understone_root_dir + "/bin/shaders");
        if (!std::filesystem::exists(bin_directory) && !std::filesystem::create_directory(bin_directory) && !std::filesystem::create_directory(shaders_bin_directory))
        {
            PrintLn("Unable to acquire the Understone bin/shaders directory.", OutputType::kError);
            is_ok_ &= false;
            return;
        }

        // Compilation flags
        {
            bool        debug_flag     = user_compilation_flags.at(CompilationFlags::kDebug);
            std::string debug_string   = "-g -O0 ";
            std::string release_string = "-O ";
            debug_flag ? invocation_base += debug_string : invocation_base += release_string;
        }

        // Ensure that the output directory already exists
        std::string spirv_output_directory = ToPosixPath(bin_directory.string()) + "/shaders/";
        std::filesystem::create_directory(spirv_output_directory);

        std::map< std::string /* source path */, std::string /* spirv path */ > shader_to_spirv_map  = {};
        size_t                                                                  num_shaders_compiled = 0;
        for (const auto& shader_source : shader_files)
        {
            if (num_shaders_compiled)
            {
                invocation += "&& ";
            }

            invocation += invocation_base;

            // Shader sources
            {
                invocation += shader_source + " ";
            }

            // Output directory
            {
                std::string original_file_name = std::filesystem::path(shader_source).filename().string();
                std::string original_extension = std::filesystem::path(shader_source).extension().string();

                std::string spirv_file_name =
                  original_file_name.substr(0, original_file_name.size() - original_extension.size()) + "_" + original_extension.substr(1, original_extension.size()) + ".spv";
                std::string spirv_full_path = spirv_output_directory + spirv_file_name;

                // Append to invocation
                invocation += "-o " + spirv_full_path + " ";

                // Fill in map of <original shader file, compiled spirv> for shader baking
                shader_to_spirv_map.insert(std::pair< std::string, std::string >(shader_source, spirv_full_path));
            }

            num_shaders_compiled++;
        }

        // Pair sources with compiled spir-v modules
        {
            for (auto& shader_info : baked_shader_info)
            {
                const auto find_result = shader_to_spirv_map.find(shader_info.shader_path);
                if (find_result == shader_to_spirv_map.end())
                {
                    PrintLn("Unable to match shader source: " + shader_info.shader_path + " with compiled SPIR-V module.", OutputType::kError);
                    return;
                }

                shader_info.spirv_path = find_result->second;
            }
        }

        if (is_ok_)
        {
            shader_invocation_ = invocation;
        }
    }

    bool        is_ok_             = true;
    std::string source_invocation_ = "";
    std::string shader_invocation_ = "";
}; // class CompilerInvocationGenerator

// Look along the current working directory path for the root Understone
// directory. Specifically, the first directory which contains all of the
// following subdirectories will be considered to be the root: "build", "bin",
// "source", "analysis", and "assets". The "source" directory must have a file
// named "Understone.cpp".
std::string
DetermineUnderstoneRootDirectory()
{
    std::string                 root_understone_dir = "";
    const std::filesystem::path CWD                 = std::filesystem::current_path();

    std::filesystem::path partial_path = CWD;
    auto                  path_element = CWD.end();
    do
    {
        std::filesystem::directory_entry dir(partial_path);
        if (!dir.is_directory())
        {
            std::stringstream error_message("Invalid path : ");
            error_message << dir.path().string() << " is not a directory.";
            PrintLn(error_message.str(), OutputType::kError);
            break;
        }

        // Asses all files in path
        bool root_understone_dir_found = false;
        for (const auto& dir_contents : std::filesystem::directory_iterator(dir))
        {
            // Choose all directories
            if (!dir_contents.is_directory())
            {
                continue;
            }

            // Search directory contents for requried files
            try
            {
                for (const auto& subdir_contents : std::filesystem::directory_iterator(dir_contents))
                {
                    // Choose only directories
                    std::filesystem::perms dir_permissions = subdir_contents.status().permissions();
                    if (!subdir_contents.is_directory() || ((dir_permissions & std::filesystem::perms::others_all) == std::filesystem::perms::none))
                    {
                        continue;
                    }

                    std::string expected_path = subdir_contents.path().string();
                    if (std::filesystem::exists(expected_path + "/build") && std::filesystem::exists(expected_path + "/analysis") &&
                        std::filesystem::exists(expected_path + "/source") && std::filesystem::exists(expected_path + "/source/" + UNDERSTONE_EXE_NAME + ".cpp"))
                    {
                        root_understone_dir_found = true;
                        root_understone_dir       = expected_path;
                        break;
                    }
                }
            }
            catch (...)
            {
                continue;
            }

            if (root_understone_dir_found)
            {
                break;
            }
        }

        if (path_element != CWD.end())
        {
            partial_path = CWD.string().substr(0, CWD.string().find(path_element->string()));
        }
        path_element--;
    } while (path_element != CWD.begin());

    return root_understone_dir;
}

bool
GetDependencyPathInfo(const std::string&              understone_root_dir,
                      std::set< std::string >&        source_files,
                      std::set< std::string >&        header_files,
                      std::set< std::string >&        header_directories,
                      std::set< std::string >&        shader_files,
                      std::vector< BakedShaderInfo >& baked_shader_info)
{
    PrintLn("Acquiring dependency paths...");
    std::filesystem::path source(understone_root_dir + "/source");

    if (!std::filesystem::exists(source))
    {
        PrintLn("Cannot build dependencies; corrupt paths.", OutputType::kError);
        return false;
    }

    const std::set< std::string > header_file_types   = { ".h", ".hpp" };
    const std::set< std::string > source_file_types   = { ".c", ".cpp", ".cc" };
    const std::set< std::string > shader_file_types   = { ".vert", ".frag", ".geom", ".tess" };
    std::set< std::string >       unique_shader_names = {};
    for (const auto& item : std::filesystem::recursive_directory_iterator(source))
    {
        std::filesystem::path current_path = std::filesystem::current_path();

        // Skip ignored directories
        if (ToPosixPath(current_path.string()).find("baked_shaders", 0) != std::string::npos)
        {
            std::cout << "SKIP\n\n" << std::endl << std::flush;
            continue;
        }

        std::string item_path = item.path().string();
        std::string extension = item.path().extension().string();
        std::string file_name = item.path().filename().string();

        if (header_file_types.find(extension) != header_file_types.end())
        {
            // Note: -1 on substring index is to drop the '\' at the end of directory
            // names
            std::string header_directory = item_path.substr(0, item_path.find(file_name) - 1);
            header_directories.emplace(ToPosixPath(header_directory));
            header_files.emplace(ToPosixPath(item.path().lexically_relative(current_path).string()));
        }
        else if (source_file_types.find(extension) != source_file_types.end())
        {
            source_files.emplace(ToPosixPath(item.path().lexically_relative(current_path).string()));
        }
        else if (shader_file_types.find(extension) != shader_file_types.end())
        {
            std::string shader_path = ToPosixPath(item.path().lexically_relative(current_path).string());
            shader_files.emplace(shader_path);

            BakedShaderInfo shader_info = {};
            shader_info.shader_path     = shader_path;

            // Attempt to parse shader header
            {
                std::ifstream file_stream(shader_path);
                std::string   file_stream_line         = "";
                bool          shader_name_found        = false;
                bool          shader_type_found        = false;
                bool          auto_gen_file_name_found = false;
                size_t        string_find_pos          = std::string::npos;
                while (std::getline(file_stream, file_stream_line) && (!shader_name_found || !shader_type_found || !auto_gen_file_name_found))
                {
                    if (!shader_name_found) // Find shader name in header
                    {
                        string_find_pos = file_stream_line.find(kShaderNameId);
                        if (string_find_pos != std::string::npos)
                        {
                            std::string shader_name = file_stream_line.substr((string_find_pos + kShaderNameId.size() + 1), file_stream_line.size());
                            TrimString(shader_name);

                            shader_info.common_name      = shader_name;
                            const auto set_insert_return = unique_shader_names.insert(shader_name);
                            if (!set_insert_return.second)
                            {
                                std::string duplicate_shader_name_error =
                                  "The shader at " + shader_path + " has " + kShaderNameId + " \"" + shader_name + "\", which already exists. Shaders must have unique names.";
                                PrintLn(duplicate_shader_name_error, OutputType::kError);
                                return false;
                            }

                            shader_name_found = true;
                        }
                    }

                    if (!shader_type_found) // Find shader type in header
                    {
                        string_find_pos = file_stream_line.find(kShaderTypeId);
                        if (string_find_pos != std::string::npos)
                        {
                            std::string shader_type = file_stream_line.substr((string_find_pos + kShaderTypeId.size() + 1), file_stream_line.size());
                            TrimString(shader_type);

                            ShaderType temp_type = StringToShaderType(shader_type);
                            if (temp_type != ShaderType::kNone)
                            {
                                shader_info.shader_type = temp_type;
                            }
                            else
                            {
                                std::string shader_type_error_mismatch = "The shader at " + shader_path + " has an " + kShaderTypeId + " \"" + shader_type +
                                                                         "\", which is invalid.\nValid shader names are: " + kVertexTypeString + ", " + kFragmentTypeString;
                                PrintLn(shader_type_error_mismatch, OutputType::kError);
                                return false;
                            }

                            shader_type_found = true;
                        }
                    }

                    if (!auto_gen_file_name_found) // Find auto gen file name in header
                    {
                        string_find_pos = file_stream_line.find(kAutoGenFileNameId);
                        if (string_find_pos != std::string::npos)
                        {
                            std::string auto_gen_file_name = file_stream_line.substr((string_find_pos + kAutoGenFileNameId.size() + 1), file_stream_line.size());
                            TrimString(auto_gen_file_name);

                            bool   shader_name_is_header_type = false;
                            size_t pos                        = 0;
                            for (const std::string& header_extension : header_file_types)
                            {
                                pos = auto_gen_file_name.find(header_extension);
                                if (pos != std::string::npos &&                                 // Ensure that the file type matches a known header extension
                                    pos == auto_gen_file_name.size() - header_extension.size()) // Ensure that the header extension is at the end of the string
                                {
                                    shader_name_is_header_type = true;
                                    break;
                                }
                            }

                            // User provided shader names must comply with traditional header file extensions
                            if (!shader_name_is_header_type)
                            {
                                std::string shader_name_header_type_error = "The shader at " + shader_path + " has " + kAutoGenFileNameId + " \"" + auto_gen_file_name +
                                                                            "\", which is not a known header extension. Known header extensions are:";
                                for (const std::string& header_extension : header_file_types)
                                {
                                    shader_name_header_type_error += (" " + header_extension);
                                }
                                PrintLn(shader_name_header_type_error, OutputType::kError);

                                return false;
                            }

                            if (auto_gen_file_name.size() < 1)
                            {
                                std::string auto_gen_file_name_error = "The shader at " + shader_path + " has an " + kAutoGenFileNameId + " \"" + auto_gen_file_name +
                                                                       "\", which is invalid.\nValid names have at least 1 character.";
                                PrintLn(auto_gen_file_name_error, OutputType::kError);
                                return false;
                            }
                            shader_info.auto_gen_file_name = auto_gen_file_name;
                            auto_gen_file_name_found       = true;
                        }
                    }
                }

                file_stream.close();
                if (!shader_name_found || !shader_type_found || !auto_gen_file_name_found)
                {
                    std::string shader_header_error = "Unable to parse shader header for file: " + shader_path +
                                                      "\nAll shaders are required to have commented headers in the form:\n"
                                                      "\t// @uAutoGenFileName: <name for auto generation> (header)\n"
                                                      "\t// @uName: <unique name>\n"
                                                      "\t// @uType: <type>\n";
                    PrintLn(shader_header_error, OutputType::kError);
                    return false;
                }

                baked_shader_info.push_back(shader_info);
            }
        }
    }

    return true;
}

void
StringToLower(std::string& str)
{
    for (size_t c = 0; c < str.size(); c++)
    {
        str[c] = std::tolower(str[c]);
    }
}

class Analyzer
{
  public:
    Analyzer(const std::string& understone_root_dir)
    {
        analysis_output_path_ = std::filesystem::path(understone_root_dir + "/analysis");
        if (!std::filesystem::exists(analysis_output_path_) && !std::filesystem::create_directory(analysis_output_path_))
        {
            PrintLn("Unable to acquire or create analysis output directory: " + analysis_output_path_.string(), OutputType::kError);
            is_ok_ = false;
        }
    }

    bool
    RunTagAnalysis(const std::set< std::string >& source_files, const std::set< std::string >& header_files, const std::set< std::string >& shader_files)
    {
        if (!is_ok_)
        {
            PrintLn("Cannot continue with tag analysis, analyzer reports it is in an "
                    "invalid state.",
                    OutputType::kError);
            return false;
        }

        for (const auto& source : source_files)
        {
            if (!TagAnalysisByFileType(source, source_lines_, source_comment_lines_))
            {
                is_ok_ = false;
                PrintLn("Cannot continue tag-parsing source files.", OutputType::kError);
                break;
            }
        }

        for (const auto& header : header_files)
        {
            if (!TagAnalysisByFileType(header, source_lines_, source_comment_lines_))
            {
                is_ok_ = false;
                PrintLn("Cannot continue tag-parsing header files.", OutputType::kError);
                break;
            }
        }

        for (const auto& shader : shader_files)
        {
            if (!TagAnalysisByFileType(shader, shader_lines_, shader_comment_lines_))
            {
                is_ok_ = false;
                PrintLn("Cannot continue tag-parsing shader files.", OutputType::kError);
                break;
            }
        }

        return is_ok_;
    }

    bool
    RunAnalysisReport()
    {
        if (!is_ok_)
        {
            PrintLn("Cannot continue with analysis report; analyzer reports it is in "
                    "an invalid state.",
                    OutputType::kError);
            return false;
        }

        std::filesystem::path report_path = std::filesystem::path(analysis_output_path_.string() + "/analysis_results.txt");
        std::ofstream         file_stream(report_path);
        if (!file_stream.is_open())
        {
            PrintLn("Cannot continue with analysis report; cannot open file stream.", OutputType::kError);
            return false;
        }

        // Write preamble
        std::string preamble_autogen_string = "- This is an autogenerated file; content changes will be overwritten "
                                              "-";
        std::string preamble_autogen_decorator = "";
        for (size_t i = 0; i < preamble_autogen_string.size(); i++)
        {
            preamble_autogen_decorator += "-";
        }
        file_stream << preamble_autogen_decorator << std::endl << preamble_autogen_string << std::endl << preamble_autogen_decorator << std::endl << std::endl;

        // Wire LOC info:
        file_stream << "Source LOC:         " << source_lines_ << std::endl;
        file_stream << "Source Comment LOC: " << source_comment_lines_ << std::endl;
        file_stream << "Shader LOC:         " << shader_lines_ << std::endl;
        file_stream << "Shader Comment LOC: " << shader_comment_lines_ << std::endl;
        file_stream << std::endl;

        for (const auto& tag_info : entries_by_tag_type_)
        {
            file_stream << tag_info.second.size() << " " << tag_enum_to_string_.at(tag_info.first) << " tags:" << std::endl;

            for (const auto& tag_entry : tag_info.second)
            {
                file_stream << "   [" << tag_entry.developer_tag << "] (" << tag_entry.file.filename() << ", " << tag_entry.line_number << "): " << tag_entry.context << std::endl;
            }

            file_stream << std::endl;
        }

        file_stream.close();
        return true;
    }

  private:
    bool
    TagAnalysisByFileType(const std::string& file_path_string, size_t& code_line_counter, size_t& comment_line_counter)
    {
        size_t        file_line_number = 0;
        std::string   file_line_string = "";
        std::ifstream file_stream(file_path_string);
        while (std::getline(file_stream, file_line_string))
        {
            file_line_number++;

            // Source/comment count
            // Note: we will not count or try to differentiate  /**/ style comments
            size_t double_slash_pos = file_line_string.find("//");
            if (double_slash_pos != std::string::npos)
            {
                // Is this a leading or a trailing comment?
                if (0 == double_slash_pos)
                {
                    comment_line_counter++;
                }
                else
                {
                    // Trailing comments follow a LOC
                    code_line_counter++;
                }

                // Tag example 1: // [ devName::TAG ] tag comments
                //                int a = b;

                // Tag example 2: int a = b; // [ devName::TAG ] tag comments

                const std::string tag_text_prefix = "::";
                for (const auto& tag_info : tag_enum_to_string_)
                {
                    const TagType     tag_type           = tag_info.first;
                    const std::string tag_indicator_text = tag_text_prefix + tag_info.second;
                    if (tag_type == TagType::kCount)
                    {
                        is_ok_ = false;
                        PrintLn("Invalid tag type.", OutputType::kError);
                    }

                    // Determine if a tag is present in the current line
                    size_t first_open_brace_pos  = file_line_string.find('[', double_slash_pos);
                    size_t tag_indicator_pos     = file_line_string.find(tag_indicator_text, first_open_brace_pos);
                    size_t first_close_brace_pos = file_line_string.find(']', tag_indicator_pos);
                    if (tag_indicator_pos != std::string::npos)
                    {
                        if (first_open_brace_pos == std::string::npos)
                        {
                            first_open_brace_pos = file_line_string.size();
                        }
                        if (first_close_brace_pos == std::string::npos)
                        {
                            first_close_brace_pos = file_line_string.size();
                        }

                        // Determine the dev name (if present)
                        std::string dev_name = "";
                        if (first_open_brace_pos == std::string::npos || first_open_brace_pos > tag_indicator_pos)
                        {
                            // We will warn here, but still consider it a valid tag without a
                            // dev_name
                            PrintLn("Tag parser (likely) found a malformed tag.\n   in file: " + file_path_string + "\n   on line: " + std::to_string(file_line_number) +
                                      "\n   with text: " + file_line_string,
                                    OutputType::kWarning);
                        }
                        else
                        {
                            // This is a valid tag w/ a dev name
                            dev_name = file_line_string.substr(first_open_brace_pos, tag_indicator_pos - first_open_brace_pos);
                            while (dev_name[0] == ' ' || dev_name[0] == '[')
                            {
                                dev_name.erase(0, 1);
                            }
                            while (dev_name[dev_name.size() - 1] == ' ')
                            {
                                dev_name.erase(dev_name.size() - 1, 1);
                            }
                        }

                        // Determine file context
                        std::string context = file_line_string.erase(first_open_brace_pos, first_close_brace_pos - first_open_brace_pos + 1);
                        while (context[0] == ' ' || context[0] == '/')
                        {
                            context.erase(0, 1);
                        }

                        // We're done -- add the tag entry to the map member
                        entries_by_tag_type_[tag_type].push_back({ std::filesystem::path(file_path_string), context, dev_name, file_line_number });
                    }
                }
            }
            else
            {
                // This is a typical LOC
                code_line_counter++;
            }
        }

        file_stream.close();
        return true;
    }

    enum class TagType
    {
        kTodo = 0,
        kPerf,
        kNote,
        kTemp,
        kTest,
        kStudy,
        kDebug,
        kRemove,
        kFindMe,
        kRevisit,
        kRestore,
        kStepInto,
        kCleanUp,
        kCount
    };

    struct TagEntry
    {
        const std::filesystem::path file;
        const std::string           context;
        const std::string           developer_tag;
        const size_t                line_number;
    };

    const std::map< TagType, std::string > tag_enum_to_string_ = {
        { TagType::kTodo, "TODO" },          { TagType::kPerf, "PERF" },        { TagType::kNote, "NOTE" },       { TagType::kTemp, "TEMP" },     { TagType::kTest, "TEST" },
        { TagType::kStudy, "STUDY" },        { TagType::kDebug, "DEBUG" },      { TagType::kRemove, "REMOVE" },   { TagType::kFindMe, "FINDME" }, { TagType::kRevisit, "REVISIT" },
        { TagType::kStepInto, "STEP_INTO" }, { TagType::kCleanUp, "CLEAN_UP" }, { TagType::kRestore, "RESTORE" },
    };

    std::filesystem::path                        analysis_output_path_     = {};
    std::string                                  analysis_output_file_name = "AnalysisResults";
    std::map< TagType, std::vector< TagEntry > > entries_by_tag_type_      = {};
    bool                                         is_ok_                    = true;
    size_t                                       shader_lines_             = 0;
    size_t                                       shader_comment_lines_     = 0;
    size_t                                       source_lines_             = 0;
    size_t                                       source_comment_lines_     = 0;
};

bool
BakeShaders(const std::string&              understone_root_dir,
            std::vector< BakedShaderInfo >& baked_shader_info,
            const UserCompilationFlags&     user_compilation_flags,
            std::set< std::string >&        header_files)
{
    // Ensure that the baked_shaders folder exists
    std::filesystem::path baked_shaders_dir(understone_root_dir + "/source/shaders/baked_shaders");
    std::filesystem::create_directory(baked_shaders_dir);

    const std::string compilation_flag = "// Compilation: " + (user_compilation_flags.at(CompilationFlags::kDebug) ? std::string("DEBUG") : std::string("RELEASE"));
    const std::string includes         = "#include <shader_tools.h>\n#include<type_tools.h>";
    const std::string new_lines        = "\n\n";
    const std::string comment          = "//";
    const std::string generic_header   = "//\n// This file is autogenerated; content changes will be overwritten.\n//\n";

    struct FileContents
    {
        std::string raw_shaders;
        std::string body;
    };

    std::set< std::filesystem::path >                     files_created_this_compilation = {};
    std::map< std::string /* file path */, FileContents > file_path_to_contents          = {};
    // Generate files
    {
        for (BakedShaderInfo& baked_shader : baked_shader_info)
        {
            std::string baked_shader_full_path = baked_shaders_dir.string() + "/" + baked_shader.auto_gen_file_name;
            if (!baked_shader.spirv_path.size())
            {
                PrintLn("The shader source " + baked_shader.shader_path + " was sent for baking without an associated SPIR-V module.", OutputType::kError);
                return false;
            }

            // Read spirv binary data
            FILE* file = NULL;
#if _WIN32
            fopen_s(&file, baked_shader.spirv_path.c_str(), "rb");
#else
            file = fopen(baked_shader.spirv_path.c_str(), "rb");
#endif // _WIN32
            if (!file)
            {
                PrintLn("Cannot open shader file: " + baked_shader.shader_path, OutputType::kError);
                fclose(file);
                return false;
            }

            fseek(file, 0, SEEK_END);
            size_t file_size = ftell(file);
            if (file_size % 4 != 0)
            {
                PrintLn("Invalid SPIR-V file size: " + std::to_string(file_size) + "; must be a multiple of four.", OutputType::kError);
                fclose(file);
                return false;
            }
            fseek(file, 0, SEEK_SET);
            size_t file_size_32_bit_words = file_size / 4;

            std::vector< uint32_t > file_data(file_size_32_bit_words);
            size_t                  items_read = fread(file_data.data(), sizeof(uint32_t), file_size_32_bit_words, file);
            if (items_read != file_size_32_bit_words)
            {
                PrintLn("Unable to read spirv data from file: " + baked_shader.shader_path, OutputType::kError);
                return false;
            }

            if (file_data[0] != 119734787 && // SPIR-V Magic Number (LSB)
                file_data[0] != 50471687)    // SPIR-V Magic Number (MSB)
            {
                PrintLn("Invalid SPIR-V magic number!", OutputType::kError);
                fclose(file);
                return false;
            }

            fclose(file);

            std::string shader_type_string = ShaderTypeToString(baked_shader.shader_type);
            if (!shader_type_string.size())
            {
                PrintLn("Invalid shader type; cannot continue shader baking.\n", OutputType::kError);
                return false;
            }

            file_path_to_contents[baked_shader_full_path].raw_shaders += "// " + baked_shader.common_name + ": " + baked_shader.shader_path + "\n";

            std::string       spirv_data_array_name = baked_shader.common_name + "_data";
            std::stringstream spirv_data_array_definition;
            spirv_data_array_definition << "const u32 " << spirv_data_array_name << "[" << (file_size / 4) << "] = {\n";
            for (size_t byte_idx = 0; byte_idx < file_size_32_bit_words; byte_idx++)
            {
                spirv_data_array_definition << file_data[byte_idx];
                if (byte_idx < file_size_32_bit_words - 1)
                {
                    spirv_data_array_definition << ", ";
                }
            }
            spirv_data_array_definition << "};\n";

            std::stringstream body_ss;
            body_ss << spirv_data_array_definition.str();
            body_ss << "uVulkanShader " << baked_shader.common_name << " = \n{\n";
            body_ss << "\t.name = \"" << baked_shader.common_name << "\",\n";
            body_ss << "\t.data = &" << spirv_data_array_name << "[0],\n";
            body_ss << "\t.data_size = " << std::to_string(file_size) << ",\n";
            body_ss << "\t.type = " << kUnderstoneShaderTypeEnumPrefix << shader_type_string << ",\n";
            body_ss << "\t.module = {}\n";
            body_ss << "};\n\n";

            file_path_to_contents[baked_shader_full_path].body += body_ss.str();

            // Log file name to baked shaders, so that compilation step can find them
            baked_shader.auto_gen_output_file_path = baked_shader_full_path;
        }
    }

    // Write files
    {
        for (const auto& baked_shader : file_path_to_contents)
        {
            std::ofstream file_stream(baked_shader.first, std::ios::out);
            if (!file_stream.is_open())
            {
                PrintLn("Cannot open shader file: " + baked_shader.first, OutputType::kError);
                return false;
            }

            file_stream << generic_header << baked_shader.second.raw_shaders << compilation_flag << "\n"
                        << comment << new_lines << includes << new_lines << baked_shader.second.body;

            file_stream.close();

            // Log file name to files created this compilation, so we can clean up old files
            files_created_this_compilation.insert(std::filesystem::path(baked_shader.first));
        }
    }

    // Delete files which were not created during this most recent bake
    for (auto& file_or_empty_dir : std::filesystem::directory_iterator(baked_shaders_dir))
    {
        if (files_created_this_compilation.find(file_or_empty_dir.path()) == files_created_this_compilation.end())
        {
            if (std::filesystem::is_regular_file(file_or_empty_dir) || (std::filesystem::is_directory(file_or_empty_dir) && std::filesystem::is_empty(file_or_empty_dir)))
            {
                std::filesystem::remove(file_or_empty_dir);

                // Ensure that we remove this entry from header file list
                header_files.erase(file_or_empty_dir.path().string());
            }
        }
    }

    // Create warning file
    {
        std::ofstream file_stream((baked_shaders_dir.string() + "/WARNING.txt"), std::ios::out);
        if (!file_stream.is_open())
        {
            PrintLn("Cannot open baked shader warning file for writing.", OutputType::kError);
            return false;
        }

        file_stream << "!!\n!! WARNING\n!!\n\n- The top-level of this directory is for auto generated baked shaders only.\n- For build tool compilation, top-level files which "
                       "were were not generated by that compilation will be deleted.\n- Subdirectories with contents will remain unaffected.";
    }

    return true;
}

void
PrintLnHelpMessage()
{
    size_t largest_arg_str_len = 0;
    for (const auto& arg_pair : kAllCommandLineArgs)
    {
        if (arg_pair.first.size() > largest_arg_str_len)
        {
            largest_arg_str_len = arg_pair.first.size();
        }
    }

    std::string spacing_string = "";
    for (size_t s = 0; s < largest_arg_str_len; s++)
    {
        spacing_string += ' ';
    }
    spacing_string += "  |  ";

    std::string options_str            = "Options:";
    std::string sub_options_str        = "Description:";
    std::string table_header           = options_str + spacing_string.substr(options_str.size(), spacing_string.size()) + sub_options_str;
    std::string table_header_separator = "";
    for (size_t s = 0; s < table_header.size(); s++)
    {
        table_header_separator += "-";
    }

    std::stringstream help_text;
    help_text << std::endl << "Usage: build_understone [ options ] [ sub-options ]" << std::endl << std::endl;
    help_text << table_header << std::endl << table_header_separator << std::endl;

    for (const auto& arg_pair : kAllCommandLineArgs)
    {
        help_text << arg_pair.first << spacing_string.substr(arg_pair.first.size(), spacing_string.size()) << arg_pair.second << std::endl;
    }

    std::cout << help_text.str() << std::endl;
}

bool
ParseCommandLineArgs(const CommandLineArguments& command_line_args,
                     Compiler&                   user_compiler,
                     ShaderCompiler&             user_shader_compiler,
                     UserCompilationFlags&       user_compilation_flags,
                     UserCompilationOptions&     user_compilation_options,
                     UserBuildFlags&             user_build_flags,
                     size_t&                     return_code)
{
    if (!command_line_args.size())
    {
        // This is not an error - we just use the deafult options.
        return_code = 0;
        return false; // false -> do not exit application on return
    }

#define PRINT_ERR_MSG_RET_NEG_ONE(msg) \
    exit_app_on_return = true;         \
    return_code        = -1;           \
    PrintLn(msg, OutputType::kError);  \
    PrintLnHelpMessage();              \
    break

    bool   exit_app_on_return = false;
    size_t total_num_cli_args = command_line_args.size();
    for (size_t arg_idx = 0; arg_idx < total_num_cli_args; arg_idx++)
    {
        std::string arg = command_line_args[arg_idx];
        StringToLower(arg);

        if (arg == CLI_HELP_STR)
        {
            PrintLnHelpMessage();
            exit_app_on_return = true;
            return_code        = 0;
            break;
        }
        else if (arg == CLI_CLANGCL_STR)
        {
            user_compiler = Compiler::kClangCL;
        }
        else if (arg == CLI_CLANG_STR)
        {
            user_compiler = Compiler::kClang;
        }
        else if (arg == CLI_DEBUG_STR)
        {
            user_compilation_flags[CompilationFlags::kDebug]   = true;
            user_compilation_flags[CompilationFlags::kRelease] = false;
        }
        else if (arg == CLI_RELEASE_STR)
        {
            user_compilation_flags[CompilationFlags::kDebug]   = false;
            user_compilation_flags[CompilationFlags::kRelease] = true;
        }
        else if (arg == CLI_VSDK_STR)
        {
            if (arg_idx + 1 < total_num_cli_args)
            {
                arg = command_line_args[++arg_idx];

                // Weak check to make sure next argument is not another valid cli option
                if (arg[0] == '-')
                {
                    PRINT_ERR_MSG_RET_NEG_ONE("-vulkan_sdk option expects a path. Got: " + arg);
                }
                else if (!std::filesystem::exists(arg))
                {
                    PRINT_ERR_MSG_RET_NEG_ONE("-vulkan_sdk option expects a path: "
                                              "Provided path does not exist: " +
                                              arg);
                }

                user_compilation_options[CompilationOptions::kVulkanSdkPath] = arg;
            }
            else
            {
                PRINT_ERR_MSG_RET_NEG_ONE("-vulkan_sdk option expects a path.");
            }
        }
        else if (arg == CLI_NOCF_STR)
        {
            user_build_flags[BuildFlags::kRunCodeFormatter] = false;
        }
        else if (arg == CLI_NOCA_STR)
        {
            user_build_flags[BuildFlags::kRunCodeAnalysis] = false;
        }
        else if (arg == CLI_RAB_STR)
        {
            user_build_flags[BuildFlags::kRunAfterBuild] = true;
        }
        else if (arg == CLI_RARGS_STR)
        {
            std::string run_after_build_args = "";
            if (arg_idx + 1 < total_num_cli_args)
            {
                std::string tmp_arg = command_line_args[++arg_idx];
                size_t      pos     = 0;
                while (std::string::npos != (pos = tmp_arg.find(',')))
                {
                    run_after_build_args += tmp_arg.substr(0, pos) + " ";
                    tmp_arg.erase(0, pos + 1);
                }
                run_after_build_args += tmp_arg;
                kRunAfterBuildArgs = run_after_build_args;
            }
            else
            {
                std::stringstream build_args_error_message;
                build_args_error_message << "-run_after_build_args option expects a comma separated list of "
                                            "arguments without spaces."
                                         << std::endl
                                         << "    Example: -run_after_build args arg_1,arg_2,...,arg_n" << std::endl
#if _WIN32
                                         << "    Will be passed as: " + std::string(UNDERSTONE_EXE_NAME) + ".exe arg_1 arg_2 ... arg_n";
#else
                                         << "    Will be passed as: ./" + std::string(UNDERSTONE_EXE_NAME) + " arg_1 arg_2 ... arg_n";
#endif // _WIN32
                PRINT_ERR_MSG_RET_NEG_ONE(build_args_error_message.str());
            }
        }
        else if (arg == CLI_NOCA_STR)
        {
            user_build_flags[BuildFlags::kRunCodeAnalysis] = false;
            PrintLn("NOT kRunning CODE ANALYSIS");
        }
        else
        {
            PRINT_ERR_MSG_RET_NEG_ONE("The folllowing argument is not recognized as valid: " + arg);
        }

#undef PRINT_ERR_MSG_RET_NEG_ONE
    }

    return exit_app_on_return;
}

int
main(int argc, char** argv)
{
    // Find Understone root directory
    std::string understone_root_dir = DetermineUnderstoneRootDirectory();
    if (!understone_root_dir.size())
    {
        std::string error_message = "Unable to find the root Understone directory "
                                    "along any subdirectory of the current path: ";
        error_message += std::filesystem::current_path().string();
        PrintLn(error_message, OutputType::kError);
        return -1;
    }
    understone_root_dir = ToPosixPath(understone_root_dir);

    // Build source dependency lists
    std::set< std::string >        shader_files       = {};
    std::set< std::string >        source_files       = {};
    std::set< std::string >        header_files       = {};
    std::set< std::string >        header_directories = {};
    std::vector< BakedShaderInfo > baked_shader_info  = {};
    if (!GetDependencyPathInfo(understone_root_dir, source_files, header_files, header_directories, shader_files, baked_shader_info))
    {
        PrintLn("Unable to acquire dependency path information.", OutputType::kError);
        return -1;
    }

    Compiler               user_compiler            = default_user_compiler;
    ShaderCompiler         user_shader_compiler     = default_user_shader_compiler;
    UserCompilationFlags   user_compilation_flags   = default_user_compilation_flags;
    UserCompilationOptions user_compilation_options = default_user_compilation_options;
    UserBuildFlags         user_build_flags         = default_user_build_flags;

    // Parse command line arguments
    {
        CommandLineArguments command_line_args(argc - 1);
        for (int arg_index = 1; arg_index < argc; arg_index++)
        {
            std::string tmp_string(argv[arg_index]);
            if (tmp_string.size() > 1)
            {
                command_line_args[arg_index - 1] = tmp_string;
            }
        }

        size_t parse_args_return_status = 0;
        if (ParseCommandLineArgs(command_line_args,
                                 user_compiler,
                                 user_shader_compiler,
                                 user_compilation_flags,
                                 user_compilation_options,
                                 user_build_flags,
                                 parse_args_return_status))
        {
            return parse_args_return_status;
        }
    }

    CompilerInvocationGenerator compiler_generator = CompilerInvocationGenerator(user_compilation_flags, user_compilation_options, user_build_flags);
    if (!compiler_generator.IsOk())
    {
        return -1;
    }

    // Shader invocation & compilation
    {
        compiler_generator.GenerateShaderInvocation(understone_root_dir, shader_files, user_shader_compiler, user_compilation_flags, baked_shader_info);
        if (!compiler_generator.IsOk())
        {
            return -1;
        }
        PrintLn("Compiling shaders...");
        if (std::system(compiler_generator.GetShaderInvocation().c_str()))
        {
            return -1;
        }
    }

    // Shader baking
    {
        PrintLn("Baking shaders...");
        if (!BakeShaders(understone_root_dir, baked_shader_info, user_compilation_flags, header_files))
        {
            return -1;
        }
    }

    // Source invocation & compilation
    {
        compiler_generator.GenerateSourceInvocation(understone_root_dir,
                                                    source_files,
                                                    header_files,
                                                    header_directories,
                                                    user_compiler,
                                                    user_compilation_flags,
                                                    user_compilation_options,
                                                    user_build_flags,
                                                    baked_shader_info);
        if (!compiler_generator.IsOk())
        {
            return -1;
        }

        PrintLn("Compiling source files...");
        if (std::system(compiler_generator.GetSourceInvocation().c_str()))
        {
            return -1;
        }
    }

    // Build Flags
    {
        BuildFlagInvocationGenerator build_flag_generator = BuildFlagInvocationGenerator(understone_root_dir, source_files, header_files, baked_shader_info, user_build_flags);
        if (!build_flag_generator.IsOk())
        {
            return -1;
        }
        if (user_build_flags.at(BuildFlags::kRunCodeFormatter))
        {
            PrintLn("Formatting source files...");
            if (std::system(build_flag_generator.GetCodeFormatInvocation().c_str()))
            {
                return -1;
            }
        }

        // Code analysis
        if (user_build_flags.at(BuildFlags::kRunCodeAnalysis))
        {
            PrintLn("Running code analysis...");
            Analyzer analyzer(understone_root_dir);

            if (!analyzer.RunTagAnalysis(source_files, header_files, shader_files))
            {
                return -1;
            }

            if (!analyzer.RunAnalysisReport())
            {
                return -1;
            }
        }

        if (user_build_flags.at(BuildFlags::kRunAfterBuild))
        {
            std::string launch_message = "Launching Understone Engine";
            if (kRunAfterBuildArgs.size())
            {
                launch_message += " with args: " + kRunAfterBuildArgs;
            }
            launch_message += "...";
            PrintLn(launch_message);

            std::string launch_invocation = "";
#if _WIN32
            launch_invocation = understone_root_dir + "/bin/" + std::string(UNDERSTONE_EXE_NAME) + ".exe " + kRunAfterBuildArgs;
#else
            launch_invocation = "./" + understone_root_dir + "/bin/" + std::string(UNDERSTONE_EXE_NAME) + " " + kRunAfterBuildArgs;
#endif // _WIN32

            if (!std::system(launch_invocation.c_str()))
            {
                return -1;
            }
        }
    }

    std::cout << "[ success ]" << std::endl;

    return 0;
}
