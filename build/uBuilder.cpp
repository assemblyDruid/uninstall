//
// Understone Engine Build Tools
//
// This application autogenerates source files and baked data from shaders, compiles the
// Understone Engine and it's projects, and performs code analysis. It is shipped as a
// single C++ source file for simple building. The only requirement to build the Understone
// Engine, including this tool, is a C++ compiler with >= std::c++20 features.
//
// clang-format off
//
// The following are reccommended compilation invocations for common compilers.
// Note: you may need to update -std=c++<version> and -o <executable_path> for OS executable extensions.
//
// [ clang::release ]:
// clang-format -i uBuilder.cpp && clang++ -Wall -Werror -pedantic -g0 -O3 --optimize -Ofast3 -std=c++20 uBuilder.cpp -o ../bin/uBuilder
//
// [ clang::debug   ]:
// clang-format -i uBuilder.cpp && clang++ -Wall -Werror -pedantic -std=c++20 -g3 -O0 uBuilder.cpp -o ../bin/uBuilder
//
// clang-format on
//

// [ cfarvin::TODO ] Feature
//
// It should be possible to search both the code base and the Vulkan header for uses of functions
// that have been promoted from their *KHR versions to being part of the Vulkan core. We could emit
// warnings when, for example, the codebase uses "vkGetPhysicalDeviceFeatures2KHR" when
// "vkGetPhysicalDeviceFeatures2" has been added to the version of the Vulkan header on any machine
// which compiles the Understone Engine. It would be quite a bit of string parsing: every line of
// every file would have to be checked for "vk", and if found, would then need to determine if that
// element ends in "KHR". If so, we would have to then copare that line against functions defined in
// the vulkan header, which could be parsed ahead of time to produce a map of the KHR functions
// which have been moved to the Vulkan core. These functions are typedef'ed in the Vulkan header as
// so:
//
// typedef vkGetPhysicalDeviceFeatures2 vkGetPhiscalDeviceFeatures2KHR
//
// Given the expense of all these string comparisons, it would likely be best to have such a feature
// enabled only by a flag, or only for release builds.

// [ cfarvin::TODO ] Source inclusion system.
//
// We changed the Understone Engine from using single file headers for different toolsets into the
// more traditional header/source pairing style. In the old system, we included all of the headers
// for the Understone Engine in every project. This was easy because extra headers make no difference
// if none of the information in them is used except for marginally longer compilation times due to
// the file inclusion. It was a good interim solution, but after the change to have traditional .h and
// .c files, it was necessary to ensure that all projects receive compiler invocations that include the
// Understone source (.c) files. Since some files are meant to be OS specific, including them all does not
// work as well. Additionally, we're in a stage where things are being massively refactored, and making
// major changes to the build system isn't viable with the state of the code.
//
// Ideally, this build system would be written in C and not C++, since this is another change that we've made.
// As for the source inclusion system, we can lean on the .uProject files to have some kind of source file
// specifier, like @Source "my_source_file.c", similar to the other specifiers in .uProject files. As a first
// iteration solution to the issue of having different source files used for different OS's, variations of
// the keyword can be used as so: "@SourceWin32" and "@SourceNix", etc.
//
// A secondary issue that presents itself on account of no longer having all code in header files for the
// Understone core functions is that we are no longer able to add the -Wno-unused-function flag only to the
// Understone project. This is because, for now, we're choosing to go ahead with the inclusion of any ".c"
// in the Understone directory for every project with a focus on Win32.
//
// So, to consider this issue completely solved, the following would need to happen:
//    1. The build tool is written in C.
//    2. The build tool allows for @Source/@SourceWin32/@SourceNix/etc in the .uProject files.
//    3. The build tool allows for _only_ the Understone project to have the -Wno-unused-function flag by default.

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
#define CLI_NOCA_STR    "-no_code_analysis"

const std::string           kUnderstoneShaderTypeEnumPrefix      = "uVK_SHADER_TYPE_";
const std::string           kShaderNameId                        = "@CommonName";
const std::string           kShaderTypeId                        = "@ShaderType";
const std::string           kAutoGenFileNameId                   = "@AutoGenFileName";
const std::string           kVertexTypeString                    = "VERTEX";
const std::string           kFragmentTypeString                  = "FRAGMENT";
const std::string           kEngineSourceFolderName              = "understone";
const std::string           kBuildFolderName                     = "build";
const std::string           kAnalysisFolderName                  = "analysis";
const std::string           kWarningFileName                     = "WARNING.txt";
const std::string           kBuildProgramName                    = "uBuilder";
const std::string           kBuildProgramFileName                = kBuildProgramName + ".cpp";
const std::string           kProjectFileName                     = ".uProject";
const std::string           kProjectNameProjectTag               = "@ProjectName";
const std::string           kExecutableNameProjectTag            = "@ExecutableName";
const std::string           kExecutableOutputSubfolderProjectTag = "@ExecutableOutputSubfolder";
const std::string           kRootShaderDirectoryProjectTag       = "@RootShaderDirectory";
const std::string           kSourcesProjectTag                   = "@Sources";
const std::string           kUnderstoneEngineProjectName         = "Understone Engine";
const std::filesystem::path kCurrentDirectory                    = std::filesystem::current_path();
const std::set<std::string> kHeaderFileTypes                     = { ".h", ".hpp" };
const std::set<std::string> kSourceFileTypes                     = { ".c", ".cpp", ".cc" };
const std::set<std::string> kShaderFileTypes = { ".vert", ".frag", ".geom", ".tess" };

std::string understone_root_dir = "";

using CommandLineArguments = std::vector<std::string>;
using CommandLineArgumentOptions =
  std::vector<std::pair<std::string /*arg*/, std::string /*desc*/> >;
const CommandLineArgumentOptions kAllCommandLineArgs = {
    { CLI_HELP_STR, "Shows this message." },
    { CLI_CLANGCL_STR, "Compile using clang-cl" },
    { CLI_CLANG_STR, "Compile using clang" },
    { CLI_DEBUG_STR, "Compile using debug settings" },
    { CLI_RELEASE_STR, "Compile using release settings" },
    { CLI_VSDK_STR, "Full path to your Vulkan SDK install." },
    { CLI_NOCF_STR, "Do not run clang-format on source & header files." },
    { CLI_NOCA_STR, "Do not run code analysis after a successful build." },
};

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

enum class CompilationFlags
{
    kDebug = 0,
    kRelease,
    kCount
};
using UserCompilationFlags                                = std::map<CompilationFlags, bool>;
const UserCompilationFlags default_user_compilation_flags = { { CompilationFlags::kDebug, true },
                                                              { CompilationFlags::kRelease,
                                                                false } };

enum class ShaderType
{
    kNone = 0,
    kVertex,
    kFragment
};

struct BakedShaderInfo
{
    std::string spirv_path                = "";
    std::string auto_gen_file_name        = "";
    std::string auto_gen_output_file_path = "";
    std::string common_name               = "";
    ShaderType  shader_type               = ShaderType::kNone;
};
using BakedShaderMap = std::map<std::string /*shader path*/, BakedShaderInfo>;

struct Project
{
    std::string           project_name                = {};
    std::string           executable_name             = {};
    std::string           executable_output_subfolder = {};
    std::string           root_shader_directory       = {};
    std::set<std::string> source_files                = {};
    std::set<std::string> header_files                = {};
    BakedShaderMap        baked_shader_map            = {};
};
using ProjectMap = std::map<std::string /*project directory*/, Project>;

enum class CompilationOptions
{
    kVulkanSdkPath = 0,
    kCount
};
using UserCompilationOptions = std::map<CompilationOptions, std::string>;
const UserCompilationOptions default_user_compilation_options = {
    { CompilationOptions::kVulkanSdkPath, "" }
};

enum class BuildFlags
{
    kRunCodeFormatter = 0,
    kRunCodeAnalysis,
    kCount
};
using UserBuildFlags                          = std::map<BuildFlags, bool>;
const UserBuildFlags default_user_build_flags = { { BuildFlags::kRunCodeAnalysis, true },
                                                  { BuildFlags::kRunCodeFormatter, true } };

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

bool
ValidateUserCompilationFlags(const UserCompilationFlags& user_compilation_flags,
                             std::stringstream&          error_message,
                             size_t&                     errors_logged)
{
    if (user_compilation_flags.size() != static_cast<size_t>(CompilationFlags::kCount))
    {
        error_message << "  UserCompilationFlags -- expected size: "
                      << static_cast<size_t>(CompilationFlags::kCount)
                      << ", actual size: " << user_compilation_flags.size() << std::endl;
        errors_logged++;

        return false;
    }

    return true;
}

bool
ValidateUserCompilationOptions(const UserCompilationOptions& user_compilation_options,
                               std::stringstream&            error_message,
                               size_t&                       errors_logged)
{
    if (user_compilation_options.size() != static_cast<size_t>(CompilationOptions::kCount))
    {
        error_message << "  UserCompilationOptions -- expected size: "
                      << static_cast<size_t>(CompilationOptions::kCount)
                      << ", actual size: " << user_compilation_options.size() << std::endl;
        errors_logged++;

        return false;
    }

    return true;
}

bool
ValidateUserBuildFlags(const UserBuildFlags& user_build_flags,
                       std::stringstream&    error_message,
                       size_t&               errors_logged)
{
    if (user_build_flags.size() != static_cast<size_t>(BuildFlags::kCount))
    {
        error_message << "  UserBuildFlags -- expected size: "
                      << static_cast<size_t>(BuildFlags::kCount)
                      << ", actual size: " << user_build_flags.size() << std::endl;
        errors_logged++;

        return false;
    }

    return true;
}

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
TrimWhiteSpaceAndQuotes(std::string& str)
{
    // Trim leading whitespace and quotes
    while (str[0] == ' ' || str[0] == '\'' || str[0] == '\"')
    {
        str.erase(0, 1);
    }

    // Trim trailing whitespace and quotes
    while (str[str.size() - 1] == ' ' || str[str.size() - 1] == '\'' || str[str.size() - 1] == '\"')
    {
        str.erase(str.size() - 1, 1);
    }
}

void
PrintLn(const std::string& message, OutputType output_type = OutputType::kInfo)
{
    Print((message + "\n"), output_type);
}

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

const std::string
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
    std::vector<char> stream_buffer(max_buffer_len);
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
const std::string
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

const std::string
GetCodeFormatInvocation(const Project& project)
{
    const std::string invocation_defaults = "clang-format -i -Werror --sort-includes -style=file ";

    static bool clang_format_available = false;
    if (!clang_format_available)
    {
        // Determine if clang-format is in the system env path
        size_t         clang_format_return = -1;
        PipeReturnType pipe_return         = {};

        pipe_return = RunProcess("clang-format --version >nul 2>&1", "r");
        if (pipe_return.success)
        {
            clang_format_return = pipe_return.return_code;
        }

        if (0 != clang_format_return)
        {
            PrintLn("Cannot use clang-format, please ensure it is defined in the "
                    "system environment path.",
                    OutputType::kError);
#if __linux__
            PrintLn("Note: Ceate a symlink in /usr/bin/ for 'clang-format' to point to your "
                    "current version. The clang and clang++ compilers are automatically symlinked "
                    "to their current versions on most installations, but not clang-format.",
                    OutputType::kError);
#endif // __linux__
            return "";
        }

        clang_format_available = true;
    }

    std::string code_format_invocation = invocation_defaults;

    // Add source files
    for (const auto& source_file : project.source_files)
    {
        code_format_invocation += source_file + " ";
    }

    // Add baked shader files
    for (const auto& baked_shader : project.baked_shader_map)
    {
        std::filesystem::path baked_shader_path(baked_shader.second.auto_gen_output_file_path);
        if (std::filesystem::exists(baked_shader_path))
        {
            code_format_invocation += baked_shader.second.auto_gen_output_file_path + " ";
        }
        else
        {
            PrintLn("Unable to find file for code formatting: " +
                      baked_shader.second.auto_gen_output_file_path,
                    OutputType::kError);
            return "";
        }
    }

    // Add header files
    for (const auto& header_file : project.header_files)
    {
        code_format_invocation += header_file + " ";
    }

    return code_format_invocation;
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
        return false;
    }

    return true;
}

class CompilerInvocationGenerator
{
  public:
    CompilerInvocationGenerator(const UserCompilationFlags&   user_compilation_flags,
                                const UserCompilationOptions& user_compilation_options,
                                const UserBuildFlags&         user_build_flags) // mutable
    {
        if (!UserArgumentSanityCheck(user_compilation_flags,
                                     user_compilation_options,
                                     user_build_flags))
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

    const std::string
    GetSourceInvocationByProjectName(const std::string& project_name)
    {
        std::string ret        = "";
        const auto  invocation = project_source_invocations_.find(project_name);

        if (invocation == project_source_invocations_.end())
        {
            PrintLn("Unable to obtain source invocation for project with name " + project_name,
                    OutputType::kError);
        }
        else
        {
            ret = invocation->second;
        }

        return ret;
    }

    const std::string
    GetShaderInvocationByProjectName(const std::string& project_name)
    {
        std::string ret        = "";
        const auto  invocation = project_shader_invocations_.find(project_name);

        if (invocation != project_shader_invocations_.end())
        {
            ret = invocation->second;
        }

        return ret;
    }

    void
    GenerateShaderInvocation(const ShaderCompiler&       shader_compiler,
                             const UserCompilationFlags& user_compilation_flags,
                             ProjectMap&                 projects)
    {
        if (!is_ok_)
        {
            return;
        }

        PrintLn("Generating shader compilation invocation(s)...");
        switch (shader_compiler)
        {
            case (ShaderCompiler::kGlslc):
            {
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
                    break;
                }

                GenerateGlslcInvocation(user_compilation_flags, projects);
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

    // [ cfarvin::REMOVE ] Source inclusion system.
    //
    // When the system is complete, we may not need the understone_project parameter.
    // It was added to compliment the temporary system.
    void
    GenerateSourceInvocation(const Compiler&               compiler,
                             const UserCompilationFlags&   user_compilation_flags,
                             const UserCompilationOptions& user_compilation_options,
                             const UserBuildFlags&         user_build_flags,
                             Project&                      understone_project,
                             Project&                      project,
                             const std::string             project_directory)
    {
        if (!is_ok_)
        {
            return;
        }

        // Ensure that the engine source directory is in the understone root dir
        const std::string expected_understone_source_directory = understone_root_dir + '/' +
                                                                 kEngineSourceFolderName;
        if (!std::filesystem::exists(expected_understone_source_directory))
        {
            PrintLn("Understone source directory not found. Expected: " +
                      expected_understone_source_directory,
                    OutputType::kError);
            is_ok_ = false;
            return;
        }

        // Ensure that the project directory exists
        if (!std::filesystem::exists(project_directory))
        {
            PrintLn(
              "Cannot generate source invocation; invalid project directory: " + project_directory,
              OutputType::kError);
            is_ok_ = false;
            return;
        }

        switch (compiler)
        {
            case (Compiler::kClang):
            {
                // Ensure that clang is in the sys env path (clang returns 0 on success)
                int            clang_ret_value = -1;
                PipeReturnType pipe_return     = {};
#if __linux__
                pipe_return = RunProcess("clang --version > /dev/null 2>&1", "r");
#else
                pipe_return = RunProcess("clang --version >nul 2>&1", "r");
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

                GenerateClangInvocation(user_compilation_flags,
                                        user_compilation_options,
                                        user_build_flags,
                                        understone_project,
                                        project,
                                        project_directory);
                break;
            }
            case (Compiler::kClangCL):
            {
#if __linux__
                PrintLn("clang-cl is not supported on POSIX systems.", OutputType::kError);
                is_ok_ = false;
#elif _WIN32
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
                    break;
                }

                GenerateClangCLInvocation(user_compilation_flags,
                                          user_compilation_options,
                                          user_build_flags,
                                          understone_project,
                                          project,
                                          project_directory);
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
    CreateEveryDirectoryInPath(std::filesystem::path input_path)
    {
        std::string element_iter_string = "";
        for (auto element_iter = input_path.begin(); element_iter != input_path.end();
             ++element_iter)
        {
            element_iter_string += element_iter->string() + '/';
            std::filesystem::path element_iter_path(element_iter_string);
            if (!std::filesystem::exists(element_iter_path) &&
                !std::filesystem::create_directory(element_iter_path))
            {
                return false;
            }
        }

        return true;
    }

    bool
    UserArgumentSanityCheck(
      const UserCompilationFlags&   user_compilation_flags   = default_user_compilation_flags,
      const UserCompilationOptions& user_compilation_options = default_user_compilation_options,
      const UserBuildFlags&         user_build_flags         = default_user_build_flags)
    {
        bool              success       = true;
        size_t            errors_logged = 0;
        std::stringstream error_message;
        error_message << "The following arguments have failed the sanity check:" << std::endl;

        success &= ValidateUserCompilationFlags(user_compilation_flags,
                                                error_message,
                                                errors_logged);
        success &= ValidateUserCompilationOptions(user_compilation_options,
                                                  error_message,
                                                  errors_logged);
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
    const std::string
    GetLatestDefaultVulkanSdkPathWin32()
    {
        if (!is_ok_)
        {
            return "";
        }

        std::filesystem::path default_location("C:/VulkanSDK/");
        if (!std::filesystem::exists(default_location))
        {
            PrintLn("Vulkan does not appear to be installed in the default location.",
                    OutputType::kError);
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

            size_t temp_version[4]   = { 0, 0, 0, 0 };
            size_t last_dot_location = 0;
            size_t dot_count         = 0;
            size_t substr_start      = 0;
            size_t substr_end        = 0;
            for (size_t index = 0; index < dir_name.size(); index++)
            {
                // Write the first three numbers to temp_version[0 .. 2]
                if (dir_name[index] == '.' && index != 0)
                {
                    substr_start            = last_dot_location == 0 ? last_dot_location
                                                                     : last_dot_location + 1;
                    substr_end              = index - substr_start;
                    temp_version[dot_count] = std::stoul(dir_name.substr(substr_start, substr_end));
                    last_dot_location       = index;
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

            // Write the last number to temp_version[3]
            substr_start            = last_dot_location + 1;
            substr_end              = dir_name.size();
            temp_version[dot_count] = std::stoul(dir_name.substr(substr_start, substr_end));

            // Use the latest version
            for (size_t index = 0; index < 4; index++)
            {
                if (temp_version[index] > version[index])
                {
                    for (size_t _index = index; _index < 4; _index++)
                    {
                        version[_index] = temp_version[_index];
                    }

                    break;
                }
                else if (temp_version[index] == version[index])
                {
                    continue;
                }

                break;
            }
        }

        latest_sdk_path = "C:/VulkanSDK/" + std::to_string(version[0]) + "." +
                          std::to_string(version[1]) + "." + std::to_string(version[2]) + "." +
                          std::to_string(version[3]);
        if (!std::filesystem::exists(latest_sdk_path))
        {
            PrintLn("Function GetLatestDefaultVulkanSdkPath() returned an invalid path.",
                    OutputType::kError);
            is_ok_ = false;
            return "";
        }

        return latest_sdk_path;
    }

    // [ cfarvin::REMOVE ] Source inclusion system.
    //
    // When the system is complete, we may not need the understone_project parameter.
    // It was added to compliment the temporary system.
    void
    GenerateClangCLInvocation(const UserCompilationFlags&   user_compilation_flags,
                              const UserCompilationOptions& user_compilation_options,
                              const UserBuildFlags&         user_build_flags,
                              Project&                      understone_project,
                              Project&                      project,
                              const std::string&            project_directory)
    {
        // Skip projects that don't have sources
        if (!is_ok_ || !project.source_files.size())
        {
            return;
        }

        // [ cfarvin::REMOVE ] Source inclusion system.
        //
        // We've added the -Wno-unused-function flag to all compilations for the short term.
        std::string invocation =
          "clang-cl -WX -W4 /std:c++latest -Wno-gnu-anonymous-struct -Wno-nested-anon-types "
          "-Wno-gnu-zero-variadic-macro-arguments -Wno-c99-extensions -Wno-unused-function ";

        // [ cfarvin::RESTORE ] Source inclusion system.
        //
        // Restore this when we improve the source inclusion system.
        //
        // Understone engine should ignore the unused function warning.
        // if (project.project_name == kUnderstoneEngineProjectName)
        // {
        //     invocation += "-Wno-unused-function ";
        // }

        // Compilation flags
        std::string compilation_flags_invocation = "";
        {
            bool              debug_flag   = user_compilation_flags.at(CompilationFlags::kDebug);
            const std::string debug_string = "-Od -MTd -Zi -D" +
                                             std::string(UNDERSTONE_DEBUG_MACRO_STR) + "#1 ";
            const std::string release_string = "-MT -O2 -Ot -Ob2 ";
            debug_flag ? (compilation_flags_invocation += debug_string)
                       : (compilation_flags_invocation += release_string);
        }

        // Link Paramters (hard-coded vulkan-1.lib)
        std::string default_link_parameters = "-link -SUBSYSTEM:CONSOLE -NXCOMPAT -MACHINE:x64 "
                                              "-NODEFAULTLIB:MSVCRTD user32.lib vulkan-1.lib ";
        std::string vulkan_sdk_path = user_compilation_options.at(
          CompilationOptions::kVulkanSdkPath);
        if (!vulkan_sdk_path.size())
        {
            std::string latest_vulkan_sdk_path = ToPosixPath(GetLatestDefaultVulkanSdkPathWin32());
            if (!latest_vulkan_sdk_path.size())
            {
                PrintLn(
                  "Unable to locate a Vulkan SDK installation. Please specify the precise path.",
                  OutputType::kError);
                is_ok_ &= false;
                return;
            }

            vulkan_sdk_path = latest_vulkan_sdk_path;
        }
        default_link_parameters += "-LIBPATH:" + vulkan_sdk_path + "/Lib ";

        // Understone, Vulkan, and project headers
        std::string header_directories_invocation = "-I" + understone_root_dir + +" -I" +
                                                    project_directory + " -I" + vulkan_sdk_path +
                                                    "/Include ";

        // Source files
        std::string source_files_invocation = "";
        for (const std::string& source : project.source_files)
        {
            source_files_invocation += (source + " ");
        }

        // [ cfarvin::REMVOE ] Source inclusion system.
        //
        // This turned out to be the wrong solution. Likely need to extend the .uProject file system.
        //
        // All projects have access to Understone source files.
        if (project.project_name != kUnderstoneEngineProjectName)
        {
            for (const std::string& source : understone_project.source_files)
            {
                source_files_invocation += (source + " ");
            }
        }

        // Compilation options & executable name
        std::string           compilation_options_invocation = "";
        std::filesystem::path bin_directory(understone_root_dir + "/bin/" +
                                            project.executable_output_subfolder);
        const std::string     executable_name = ToPosixPath(bin_directory.string()) + '/' +
                                            project.executable_name + ".exe ";

        if (!CreateEveryDirectoryInPath(bin_directory))
        {
            PrintLn("Unable to find or create the output directory specified by the project: " +
                      project.project_name,
                    OutputType::kError);
            PrintLn("  Output folder: " + bin_directory.string(), OutputType::kError);
            PrintLn("  Executable name: " + executable_name, OutputType::kError);

            is_ok_ &= false;
            return;
        }

        compilation_options_invocation += "-Fe" + executable_name + " ";

        // Add link parameters to main invocation.
        compilation_options_invocation += default_link_parameters;

        // Add this entry to the invocation structure
        if (is_ok_)
        {
            const std::string project_invocation = invocation + compilation_flags_invocation +
                                                   source_files_invocation +
                                                   header_directories_invocation +
                                                   compilation_options_invocation;
            project_source_invocations_[project.project_name] = project_invocation;
        }
    }
#endif // _WIN32

    // [ cfarvin::REMOVE ] Source inclusion system.
    //
    // When the system is complete, we may not need the understone_project parameter.
    // It was added to compliment the temporary system.
    void
    GenerateClangInvocation(const UserCompilationFlags&   user_compilation_flags,
                            const UserCompilationOptions& user_compilation_options,
                            const UserBuildFlags&         user_build_flags,
                            Project&                      understone_project,
                            Project&                      project,
                            const std::string&            project_directory)
    {
        // Skip projects that don't have sources
        if (!is_ok_ || !project.source_files.size())
        {
            return;
        }
        // [ cfarvin::REMOVE ] Source inclusion system.
        //
        // We've added the -Wno-unused-function flag to all compilations for the short term.
        std::string invocation =
          "clang -Wall -Werror -pedantic "
          "-Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-gnu-zero-variadic-macro-arguments "
          "-Wno-c99-extensions -Wno-unused-function ";

        // [ cfarvin::RESTORE ] Source inclusion system.
        //
        // Restore this when we improve the source inclusion system.
        //
        // Understone engine should ignore the unused function warning.
        // if (project.project_name == kUnderstoneEngineProjectName)
        // {
        //     invocation += "-Wno-unused-function ";
        // }

        // Compilation flags
        std::string compilation_flags_invocation = "";
        {
            bool              debug_flag   = user_compilation_flags.at(CompilationFlags::kDebug);
            const std::string debug_string = "-g3 -O0 -D" +
                                             std::string(UNDERSTONE_DEBUG_MACRO_STR) + "=1 ";
            const std::string release_string = "-g0 -O3 -Ofast3 ";
            debug_flag ? (compilation_flags_invocation += debug_string)
                       : (compilation_flags_invocation += release_string);
        }

        // Link Paramters (hard-coded vulkan-1.lib)
        std::string vulkan_sdk_path = user_compilation_options.at(
          CompilationOptions::kVulkanSdkPath);
        std::string link_parameters = "-L"; // Note: no space, will be appended to
        if (!vulkan_sdk_path.size())
        {
#if _WIN32
            const std::string latest_vulkan_sdk_path = ToPosixPath(
              GetLatestDefaultVulkanSdkPathWin32());
            if (!latest_vulkan_sdk_path.size())
            {
                PrintLn(
                  "Unable to locate a Vulkan SDK installation. Please specify the precise path.",
                  OutputType::kError);
                is_ok_ &= false;
                return;
            }

            vulkan_sdk_path = latest_vulkan_sdk_path;
            link_parameters += vulkan_sdk_path + "/Lib -lvulkan-1.lib -luser32.lib ";
#else
            link_parameters += vulkan_sdk_path + " -lvulkan ";
#endif // _WIN32
        }

        // Understone, Vulkan, and project headers
        std::string header_directories_invocation = "-I" + understone_root_dir + +" -I" +
                                                    project_directory + " -I" + vulkan_sdk_path +
                                                    "/Include ";

        // Source files
        std::string source_files_invocation = "";
        for (const std::string& source : project.source_files)
        {
            source_files_invocation += (source + " ");
        }

        // [ cfarvin::REMVOE ] Source inclusion system.
        //
        // This turned out to be the wrong solution. Likely need to extend the .uProject file system.
        //
        // All projects have access to Understone source files.
        if (project.project_name != kUnderstoneEngineProjectName)
        {
            for (const std::string& source : understone_project.source_files)
            {
                source_files_invocation += (source + " ");
            }
        }

        // Compilation options & executaable name
        std::string           compilation_options_invocation = "";
        std::filesystem::path bin_directory(understone_root_dir + "/bin/" +
                                            project.executable_output_subfolder);
        std::string           executable_name = ToPosixPath(bin_directory.string()) + '/' +
                                      project.executable_name;

        if (!CreateEveryDirectoryInPath(bin_directory))
        {
            PrintLn("Unable to find or create the output directory specified by the project: " +
                      project.project_name,
                    OutputType::kError);
            PrintLn("  Output folder: " + bin_directory.string(), OutputType::kError);
            PrintLn("  Executable name: " + executable_name, OutputType::kError);

            is_ok_ &= false;
            return;
        }

        compilation_options_invocation += "-o ";
#if _WIN32
        compilation_options_invocation += executable_name + ".exe ";
#else
        compilation_options_invocation += executable_name + " ";
#endif

        // Add link parameters to main invocation.
        compilation_options_invocation += link_parameters;

        if (is_ok_)
        {
            const std::string project_invocation = invocation + compilation_flags_invocation +
                                                   source_files_invocation +
                                                   header_directories_invocation +
                                                   compilation_options_invocation;
            project_source_invocations_[project.project_name] = project_invocation;
        }
    }

    void
    GenerateGlslcInvocation(const UserCompilationFlags& user_compilation_flags,
                            ProjectMap&                 projects)
    {
        if (!is_ok_)
        {
            return;
        }

        // Note: At this time, Glslc can only compile & link a single shader at a
        // time. Thus, It must be run once for every shader.
        std::string invocation      = "";
        std::string invocation_base = "glslc -Werror ";

        // General compilation flags
        bool        debug_flag     = user_compilation_flags.at(CompilationFlags::kDebug);
        std::string debug_string   = "-g -O0 ";
        std::string release_string = "-O ";
        debug_flag ? invocation_base += debug_string : invocation_base += release_string;

        for (auto& project : projects)
        {
            // Do not generate shader compilation invocation if no root shader directory was
            // specified
            if (project.second.root_shader_directory.empty())
            {
                continue;
            }

            // Ensure that the project directory is valid
            if (!std::filesystem::exists(project.first) ||
                !std::filesystem::is_directory(project.first))
            {
                PrintLn("Project directory invalid for project: " + project.second.project_name,
                        OutputType::kError);
                is_ok_ = false;
                return;
            }

            // Ensure that the spir-v output directory is valid
            const std::string spirv_output_directory = ToPosixPath(project.first) + '/' +
                                                       project.second.root_shader_directory +
                                                       "/bin";
            std::filesystem::path spirv_output_directory_path(spirv_output_directory);
            if (!std::filesystem::exists(spirv_output_directory_path) &&
                !std::filesystem::create_directory(spirv_output_directory_path))
            {
                PrintLn("Unable to create shader output directory for project: " +
                          project.second.project_name,
                        OutputType::kError);
                PrintLn(
                  "  Specified shader output directory  " + spirv_output_directory_path.string(),
                  OutputType::kError);
                is_ok_ = false;
                return;
            }

            // We expect the baked_shader_info.shader_path field to be filled in when the initial
            // directory was scanned. Now, we will pre-fill the spir-v output file information
            // generated here, assuming that the compilation succeeds.
            bool use_logical_and = false;
            for (auto& shader_info : project.second.baked_shader_map)
            {
                const std::string shader_path = shader_info.first;

                // Ensure that the shader path is valid
                if (!shader_path.size() || !std::filesystem::exists(shader_info.first))
                {
                    PrintLn("Cannot generate compilation invocation for invalid shader file: " +
                              shader_path,
                            OutputType::kError);
                    is_ok_ = false;
                    return;
                }

                // Chaining multiple invocations requires logical and
                if (use_logical_and)
                {
                    invocation += "&& ";
                }

                invocation += invocation_base;

                // Shader sources
                invocation += shader_path + " ";

                // Output directory
                const std::string original_file_name =
                  std::filesystem::path(shader_info.first).filename().string();
                const std::string original_extension =
                  std::filesystem::path(shader_info.first).extension().string();
                const std::string spirv_file_name =
                  original_file_name.substr(0,
                                            original_file_name.size() - original_extension.size()) +
                  "_" + original_extension.substr(1, original_extension.size()) + ".spv";
                const std::string spirv_full_path = spirv_output_directory + '/' + spirv_file_name;

                // Complete invocation
                invocation += "-o " + spirv_full_path + " ";

                // Fill in BakedShaderInfo with spir-v output name
                shader_info.second.spirv_path = spirv_full_path;

                use_logical_and = true;
            }

            if (is_ok_)
            {
                project_shader_invocations_[project.second.project_name] = invocation;
            }
        }
    }

    bool is_ok_ = true;
    std::map<std::string /*project name*/, std::string /*project source invocation*/>
      project_source_invocations_ = {};
    std::map<std::string /*project name*/, std::string /*project shader invocation*/>
      project_shader_invocations_ = {};
}; // class CompilerInvocationGenerator

// Look along the current working directory path for the root Understone directory.
const std::string
DetermineUnderstoneRootDirectory()
{
    std::string           root_understone_dir = "";
    std::filesystem::path partial_path        = kCurrentDirectory;
    auto                  path_element        = kCurrentDirectory.end();
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
            // Only test directories
            if (!dir_contents.is_directory())
            {
                continue;
            }

            // Search directory contents for requried files
            try
            {
                for (const auto& subdir_contents :
                     std::filesystem::directory_iterator(dir_contents))
                {
                    // Only test (sub)directories
                    std::filesystem::perms dir_permissions = subdir_contents.status().permissions();
                    if (!subdir_contents.is_directory() ||
                        ((dir_permissions & std::filesystem::perms::others_all) ==
                         std::filesystem::perms::none))
                    {
                        continue;
                    }

                    const std::string expected_path = ToPosixPath(subdir_contents.path().string());
                    const std::string build_path    = expected_path + '/' + kBuildFolderName;
                    const std::string source_path   = expected_path + '/' + kEngineSourceFolderName;
                    const std::string source_project_file = source_path + '/' + ".uProject";
                    if (std::filesystem::exists(build_path) &&
                        std::filesystem::exists(source_path) &&
                        std::filesystem::exists(source_project_file))
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

        if (path_element != kCurrentDirectory.end())
        {
            partial_path = kCurrentDirectory.string().substr(
              0,
              kCurrentDirectory.string().find(path_element->string()));
        }
        path_element--;
    } while (path_element != kCurrentDirectory.begin());

    return root_understone_dir;
}

bool
ParseShaderHeader(const std::filesystem::path& project_item_path,
                  std::set<std::string>&       unique_shader_names,
                  BakedShaderMap&              baked_shader_map)
{
    const std::string shader_path = ToPosixPath(
      project_item_path.lexically_relative(kCurrentDirectory).string());
    if (!std::filesystem::exists(shader_path) || !std::filesystem::is_regular_file(shader_path))
    {
        PrintLn("Cannot parse shader header; invalid file: " + shader_path, OutputType::kError);
        return false;
    }

    BakedShaderInfo& shader_info        = baked_shader_map[shader_path];
    std::string&     shader_name        = shader_info.common_name;
    std::string&     auto_gen_file_name = shader_info.auto_gen_file_name;
    ShaderType&      shader_type        = shader_info.shader_type;

    std::ifstream file_stream(shader_path);
    if (!file_stream.is_open())
    {
        PrintLn("Unable to open file stream with path: " + shader_path, OutputType::kError);
        return false;
    }

    std::string file_stream_line = "";
    size_t      string_find_pos  = std::string::npos;
    while (std::getline(file_stream, file_stream_line) &&
           (shader_name.empty() || shader_type == ShaderType::kNone || auto_gen_file_name.empty()))
    {
        if (shader_name.empty()) // Find shader name in header
        {
            string_find_pos = file_stream_line.find(kShaderNameId);
            if (string_find_pos != std::string::npos)
            {
                std::string temp_shader_name = file_stream_line.substr(
                  (string_find_pos + kShaderNameId.size() + 1),
                  file_stream_line.size());
                TrimWhiteSpaceAndQuotes(temp_shader_name);

                const auto set_insert_return = unique_shader_names.insert(temp_shader_name);
                if (!set_insert_return.second)
                {
                    std::string duplicate_shader_name_error =
                      "The shader at " + shader_path + " has " + kShaderNameId + " \"" +
                      temp_shader_name +
                      "\", which already exists. Shaders must have unique names.";
                    PrintLn(duplicate_shader_name_error, OutputType::kError);
                    return false;
                }

                shader_name = temp_shader_name;
            }
        }

        if (shader_type == ShaderType::kNone) // Find shader type in header
        {
            string_find_pos = file_stream_line.find(kShaderTypeId);
            if (string_find_pos != std::string::npos)
            {
                std::string temp_shader_type = file_stream_line.substr(
                  (string_find_pos + kShaderTypeId.size() + 1),
                  file_stream_line.size());
                TrimWhiteSpaceAndQuotes(temp_shader_type);

                ShaderType temp_type = StringToShaderType(temp_shader_type);
                if (temp_type != ShaderType::kNone)
                {
                    shader_type = temp_type;
                }
                else
                {
                    std::string shader_type_error_mismatch =
                      "The shader at " + shader_path + " has an " + kShaderTypeId + " \"" +
                      temp_shader_type +
                      "\", which is invalid.\nValid shader names are: " + kVertexTypeString + ", " +
                      kFragmentTypeString;
                    PrintLn(shader_type_error_mismatch, OutputType::kError);
                    return false;
                }
            }
        }

        if (auto_gen_file_name.empty()) // Find auto gen file name in header
        {
            string_find_pos = file_stream_line.find(kAutoGenFileNameId);
            if (string_find_pos != std::string::npos)
            {
                std::string temp_auto_gen_file_name = file_stream_line.substr(
                  (string_find_pos + kAutoGenFileNameId.size() + 1),
                  file_stream_line.size());
                TrimWhiteSpaceAndQuotes(temp_auto_gen_file_name);

                bool   shader_name_is_header_type = false;
                size_t pos                        = 0;
                for (const std::string& header_extension : kHeaderFileTypes)
                {
                    pos = temp_auto_gen_file_name.find(header_extension);
                    if (pos != std::string::npos && // Ensure that the file type matches a known
                                                    // header extension
                        pos == temp_auto_gen_file_name.size() -
                                 header_extension.size()) // Ensure that the header extension is at
                                                          // the end of the string
                    {
                        shader_name_is_header_type = true;
                        break;
                    }
                }

                // User provided shader names must comply with traditional header file extensions
                if (!shader_name_is_header_type)
                {
                    std::string shader_name_header_type_error =
                      "The shader at " + shader_path + " has " + kAutoGenFileNameId + " \"" +
                      temp_auto_gen_file_name +
                      "\", which is not a known header extension. Known header extensions are:";
                    for (const std::string& header_extension : kHeaderFileTypes)
                    {
                        shader_name_header_type_error += (" " + header_extension);
                    }
                    PrintLn(shader_name_header_type_error, OutputType::kError);

                    return false;
                }

                auto_gen_file_name = temp_auto_gen_file_name;
            }
        }
    }

    file_stream.close();
    if (shader_name.empty() || shader_type == ShaderType::kNone || auto_gen_file_name.empty())
    {
        const std::string shader_header_error =
          "Unable to parse shader header for file: " + shader_path +
          "\nAll shaders are required to have commented headers in the form:\n"
          "\t// @uAutoGenFileName: <name for auto generation> (header)\n"
          "\t// @uName: <unique name>\n"
          "\t// @uType: <type>\n";
        PrintLn(shader_header_error, OutputType::kError);
        shader_info = {};
        return false;
    }

    return true;
}

bool
ParseProjectFile(Project& temp_project, const std::string& project_path)
{
    const std::string project_file_full_path = project_path + '/' + kProjectFileName;
    if (!std::filesystem::exists(project_file_full_path))
    {
        PrintLn("Project file not found at: " + project_file_full_path, OutputType::kError);
        return false;
    }

    std::ifstream file_stream(project_file_full_path);
    if (!file_stream.is_open())
    {
        PrintLn("Unable to open file stream with path: " + project_file_full_path,
                OutputType::kError);
        return false;
    }

#define CHECK_TAG_FIRST_COLUMN_ERR                                              \
    if (pos != 0)                                                               \
    {                                                                           \
        PrintLn("Project at: " + project_file_full_path +                       \
                  ": All @ tags must begin on the first column of a new line.", \
                OutputType::kError);                                            \
        return false;                                                           \
    }

#define CHECK_STR_LEN_ERR(temp_string)                                                   \
    if (temp_string.empty())                                                             \
    {                                                                                    \
        PrintLn("Project at: " + project_file_full_path + ": Found tag " + temp_string + \
                  ", but no information provided.",                                      \
                OutputType::kError);                                                     \
        return false;                                                                    \
    }

#define CHECK_MULTIPLE_TAG_ERR(string_variable, tag)                           \
    if (!string_variable.empty())                                              \
    {                                                                          \
        PrintLn("Project at: " + project_file_full_path + ": The tag " + tag + \
                " has already been defined with value: " + string_variable);   \
        return false;                                                          \
    }

    std::string project_name          = "";
    std::string exe_name              = "";
    std::string exe_output_subfolder  = "";
    std::string root_shader_directory = "";

    size_t      pos = std::string::npos;
    std::string file_stream_line;
    while (std::getline(file_stream, file_stream_line))
    {
        // Project name
        pos = file_stream_line.find(kProjectNameProjectTag);
        if (pos != std::string::npos)
        {
            CHECK_TAG_FIRST_COLUMN_ERR;
            std::string temp_name = file_stream_line.substr(kProjectNameProjectTag.size(),
                                                            file_stream_line.size());
            TrimWhiteSpaceAndQuotes(temp_name);
            CHECK_MULTIPLE_TAG_ERR(project_name, kProjectNameProjectTag);
            CHECK_STR_LEN_ERR(temp_name);
            project_name = temp_name;
            continue;
        }

        // Executable name
        pos = file_stream_line.find(kExecutableNameProjectTag);
        if (pos != std::string::npos)
        {
            CHECK_TAG_FIRST_COLUMN_ERR;
            std::string temp_exe_name = file_stream_line.substr(kExecutableNameProjectTag.size(),
                                                                file_stream_line.size());
            TrimWhiteSpaceAndQuotes(temp_exe_name);
            CHECK_MULTIPLE_TAG_ERR(exe_name, kExecutableNameProjectTag);
            CHECK_STR_LEN_ERR(temp_exe_name);
            exe_name = temp_exe_name;
            continue;
        }

        // Executable output sub folder
        pos = file_stream_line.find(kExecutableOutputSubfolderProjectTag);
        if (pos != std::string::npos)
        {
            CHECK_TAG_FIRST_COLUMN_ERR;
            std::string temp_exe_output_subfolder = file_stream_line.substr(
              kExecutableOutputSubfolderProjectTag.size(),
              file_stream_line.size());
            TrimWhiteSpaceAndQuotes(temp_exe_output_subfolder);
            CHECK_MULTIPLE_TAG_ERR(exe_output_subfolder, kExecutableOutputSubfolderProjectTag);
            CHECK_STR_LEN_ERR(temp_exe_output_subfolder);
            exe_output_subfolder = temp_exe_output_subfolder;
            continue;
        }

        // Root shader directory
        pos = file_stream_line.find(kRootShaderDirectoryProjectTag);
        if (pos != std::string::npos)
        {
            CHECK_TAG_FIRST_COLUMN_ERR;
            std::string temp_root_shader_directory = file_stream_line.substr(
              kRootShaderDirectoryProjectTag.size(),
              file_stream_line.size());
            TrimWhiteSpaceAndQuotes(temp_root_shader_directory);
            CHECK_MULTIPLE_TAG_ERR(root_shader_directory, kRootShaderDirectoryProjectTag);
            CHECK_STR_LEN_ERR(temp_root_shader_directory);
            root_shader_directory = temp_root_shader_directory;
            continue;
        }
    }

#undef CHECK_TAG_FIRST_COLUMN_ERR
#undef CHECK_STR_LEN_ERR
#undef CHECK_MULTIPLE_TAG_ERR

    file_stream.close();

#define REQUIRED_TAG_CHECK_ERR(tag_variable, tag_string)                                         \
    if (tag_variable.empty())                                                                    \
    {                                                                                            \
        PrintLn("Project at: " + project_file_full_path + "failed to provide tag " + tag_string, \
                OutputType::kError);                                                             \
        return false;                                                                            \
    }

    // Note: root shader dir not required; exe output required if exe name provided
    REQUIRED_TAG_CHECK_ERR(project_name, kProjectNameProjectTag);
    if (!exe_name.empty())
    {
        REQUIRED_TAG_CHECK_ERR(exe_output_subfolder, kExecutableOutputSubfolderProjectTag);
    }

    temp_project.project_name                = project_name;
    temp_project.executable_name             = exe_name;
    temp_project.executable_output_subfolder = exe_output_subfolder;
    temp_project.root_shader_directory       = root_shader_directory;

    return true;
}

bool
AcquireProjectDependencies(ProjectMap& projects)
{
    PrintLn("Acquiring project dependencies...");
    std::filesystem::path root(understone_root_dir);
    if (!std::filesystem::exists(root))
    {
        PrintLn("Cannot build dependencies; corrupt paths.", OutputType::kError);
        return false;
    }

    std::set<std::string> unique_shader_names = {};
    for (const auto& item : std::filesystem::recursive_directory_iterator(root))
    {
        const std::string item_path_string      = ToPosixPath(item.path().string());
        const std::string expected_project_name = item_path_string + '/' + kProjectFileName;

        // Only asses directories with .uProject files
        if (!std::filesystem::is_directory(item_path_string) ||
            !std::filesystem::exists(expected_project_name))
        {
            continue;
        }

        Project temp_project = {};
        ParseProjectFile(temp_project, item_path_string);

        for (const auto& project_item : std::filesystem::recursive_directory_iterator(item.path()))
        {
            const std::string project_item_path = project_item.path().string();
            const std::string extension         = project_item.path().extension().string();
            const std::string file_name         = project_item.path().filename().string();

            if (kHeaderFileTypes.find(extension) != kHeaderFileTypes.end())
            {
                // Note: -1 on substring index is to drop the '\' at the end of directory names
                const std::string header_directory = project_item_path.substr(
                  0,
                  project_item_path.find(file_name) - 1);
                temp_project.header_files.emplace(
                  ToPosixPath(project_item.path().lexically_relative(kCurrentDirectory).string()));
            }
            else if (kSourceFileTypes.find(extension) != kSourceFileTypes.end())
            {
                temp_project.source_files.emplace(
                  ToPosixPath(project_item.path().lexically_relative(kCurrentDirectory).string()));
            }
            else if (kShaderFileTypes.find(extension) != kShaderFileTypes.end())
            {
                BakedShaderMap& baked_shader_map = temp_project.baked_shader_map;
                if (!ParseShaderHeader(project_item_path, unique_shader_names, baked_shader_map))
                {
                    return false;
                }
            }
        }

        projects[item_path_string] = temp_project;
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
    Analyzer()
    {
        analysis_output_path_ = std::filesystem::path(understone_root_dir + "/analysis");
        if (!std::filesystem::exists(analysis_output_path_) &&
            !std::filesystem::create_directory(analysis_output_path_))
        {
            PrintLn("Unable to acquire or create analysis output directory: " +
                      analysis_output_path_.string(),
                    OutputType::kError);
            is_ok_ = false;
        }
    }

    bool
    RunTagAnalysis(const ProjectMap& projects)
    {
        for (const auto& project : projects)
        {
            const std::set<std::string>& source_files = project.second.source_files;
            const std::set<std::string>& header_files = project.second.header_files;

            std::set<std::string> shader_files = {};
            for (const auto& shader_info : project.second.baked_shader_map)
            {
                shader_files.insert(shader_info.first);
            }

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

        std::filesystem::path report_path = std::filesystem::path(analysis_output_path_.string() +
                                                                  "/analysis_results.txt");
        std::ofstream         file_stream(report_path);
        if (!file_stream.is_open())
        {
            PrintLn("Cannot continue with analysis report; cannot open file stream.",
                    OutputType::kError);
            return false;
        }

        // Write preamble
        const std::string preamble_autogen_string =
          "- This is an autogenerated file; content changes will be overwritten "
          "-";
        std::string preamble_autogen_decorator = "";
        for (size_t i = 0; i < preamble_autogen_string.size(); i++)
        {
            preamble_autogen_decorator += "-";
        }
        file_stream << preamble_autogen_decorator << std::endl
                    << preamble_autogen_string << std::endl
                    << preamble_autogen_decorator << std::endl
                    << std::endl;

        // Wire LOC info:
        file_stream << "Source LOC:         " << source_lines_ << std::endl;
        file_stream << "Source Comment LOC: " << source_comment_lines_ << std::endl;
        file_stream << "Shader LOC:         " << shader_lines_ << std::endl;
        file_stream << "Shader Comment LOC: " << shader_comment_lines_ << std::endl;
        file_stream << std::endl;

        for (const auto& tag_info : entries_by_tag_type_)
        {
            file_stream << tag_info.second.size() << " " << tag_enum_to_string_.at(tag_info.first)
                        << " tags:" << std::endl;

            for (const auto& tag_entry : tag_info.second)
            {
                file_stream << "   [" << tag_entry.developer_tag << "] ("
                            << tag_entry.file.filename() << ", " << tag_entry.line_number
                            << "): " << tag_entry.context << std::endl;
            }

            file_stream << std::endl;
        }

        file_stream.close();
        return true;
    }

  private:
    bool
    TagAnalysisByFileType(const std::string& file_path_string,
                          size_t&            code_line_counter,
                          size_t&            comment_line_counter)
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
                    size_t tag_indicator_pos     = file_line_string.find(tag_indicator_text,
                                                                     first_open_brace_pos);
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
                        if (first_open_brace_pos == std::string::npos ||
                            first_open_brace_pos > tag_indicator_pos)
                        {
                            // We will warn here, but still consider it a valid tag without a
                            // dev_name
                            PrintLn("Tag parser (likely) found a malformed tag.\n   in file: " +
                                      file_path_string +
                                      "\n   on line: " + std::to_string(file_line_number) +
                                      "\n   with text: " + file_line_string,
                                    OutputType::kWarning);
                        }
                        else
                        {
                            // This is a valid tag w/ a dev name
                            dev_name = file_line_string.substr(
                              first_open_brace_pos,
                              tag_indicator_pos - first_open_brace_pos);
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
                        std::string context = file_line_string.erase(
                          first_open_brace_pos,
                          first_close_brace_pos - first_open_brace_pos + 1);
                        while (context[0] == ' ' || context[0] == '/')
                        {
                            context.erase(0, 1);
                        }

                        // We're done -- add the tag entry to the map member
                        entries_by_tag_type_[tag_type].push_back(
                          { std::filesystem::path(file_path_string),
                            context,
                            dev_name,
                            file_line_number });
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

    const std::map<TagType, std::string> tag_enum_to_string_ = {
        { TagType::kTodo, "TODO" },          { TagType::kPerf, "PERF" },
        { TagType::kNote, "NOTE" },          { TagType::kTemp, "TEMP" },
        { TagType::kTest, "TEST" },          { TagType::kStudy, "STUDY" },
        { TagType::kDebug, "DEBUG" },        { TagType::kRemove, "REMOVE" },
        { TagType::kFindMe, "FINDME" },      { TagType::kRevisit, "REVISIT" },
        { TagType::kStepInto, "STEP_INTO" }, { TagType::kCleanUp, "CLEAN_UP" },
        { TagType::kRestore, "RESTORE" },
    };

    std::filesystem::path                     analysis_output_path_     = {};
    std::string                               analysis_output_file_name = "AnalysisResults";
    std::map<TagType, std::vector<TagEntry> > entries_by_tag_type_      = {};
    bool                                      is_ok_                    = true;
    size_t                                    shader_lines_             = 0;
    size_t                                    shader_comment_lines_     = 0;
    size_t                                    source_lines_             = 0;
    size_t                                    source_comment_lines_     = 0;
};

bool
BakeShaders(BakedShaderMap& baked_shader_map, const UserCompilationFlags& user_compilation_flags)
{
    const std::string compilation_flag = "// Compilation: " +
                                         (user_compilation_flags.at(CompilationFlags::kDebug)
                                            ? std::string("DEBUG")
                                            : std::string("RELEASE"));
    const std::string includes = "#include <" + kEngineSourceFolderName +
                                 "/shader_tools.h>\n#include<" + kEngineSourceFolderName +
                                 "/type_tools.h>";
    const std::string new_lines = "\n\n";
    const std::string comment   = "//";
    const std::string generic_header =
      "//\n// This file is autogenerated; content changes will be overwritten.\n//\n";

    struct BakedFileAttributes
    {
        std::filesystem::path baked_shader_dir; // Directory the baked shader will be written to
        std::string
          raw_shaders;    // Original shader file to bake; will be stored as comment in baked file
        std::string body; // Body of the baked file
    };

    std::set<std::filesystem::path> files_created_this_compilation                         = {};
    std::map<std::string /* baked file path */, BakedFileAttributes> baked_file_attributes = {};

    for (auto& baked_shader : baked_shader_map)
    {
        // Ensure that the baked_shaders folder exists
        std::string baked_shaders_dir_string =
          std::filesystem::path(baked_shader.first).remove_filename().string() + "baked_shaders";
        std::filesystem::path baked_shaders_dir(baked_shaders_dir_string);
        std::filesystem::create_directory(baked_shaders_dir);
        if (!std::filesystem::exists(baked_shaders_dir))
        {
            PrintLn("The baked shader path " + baked_shaders_dir.string() + " is invalid.",
                    OutputType::kError);
            return false;
        }

        // Ensure each shader has an associated spir-v module
        std::string baked_shader_full_path = baked_shaders_dir.string() + '/' +
                                             baked_shader.second.auto_gen_file_name;
        if (baked_shader.second.spirv_path.empty())
        {
            PrintLn("The shader source " + baked_shader.first +
                      " was sent for baking without an associated SPIR-V module.",
                    OutputType::kError);
            return false;
        }

        // Establish and fill in baked directory for the clean-up step
        baked_file_attributes[baked_shader_full_path].baked_shader_dir = baked_shaders_dir;

        // Read spirv binary data
        std::vector<uint32_t> file_data              = {};
        size_t                file_size              = 0;
        size_t                file_size_32_bit_words = 0;
        {
            FILE* file = NULL;
#if _WIN32
            fopen_s(&file, baked_shader.second.spirv_path.c_str(), "rb");
#else
            file = fopen(baked_shader.spirv_path.c_str(), "rb");
#endif // _WIN32
            if (!file)
            {
                PrintLn("Cannot open shader file: " + baked_shader.first, OutputType::kError);
                fclose(file);
                return false;
            }

            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            if (file_size % 4 != 0)
            {
                PrintLn("Invalid SPIR-V file size: " + std::to_string(file_size) +
                          "; must be a multiple of four.",
                        OutputType::kError);
                fclose(file);
                return false;
            }
            fseek(file, 0, SEEK_SET);
            file_size_32_bit_words = file_size / 4;
            file_data.reserve(file_size_32_bit_words);
            size_t items_read = fread(file_data.data(),
                                      sizeof(uint32_t),
                                      file_size_32_bit_words,
                                      file);
            if (items_read != file_size_32_bit_words)
            {
                PrintLn("Unable to read spirv data from file: " + baked_shader.first,
                        OutputType::kError);
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
        }

        std::string shader_type_string = ShaderTypeToString(baked_shader.second.shader_type);
        if (!shader_type_string.size())
        {
            PrintLn("Invalid shader type; cannot continue shader baking.\n", OutputType::kError);
            return false;
        }

        baked_file_attributes[baked_shader_full_path].raw_shaders +=
          "// " + baked_shader.second.common_name + ": " + baked_shader.first + "\n";

        std::string       spirv_data_array_name = baked_shader.second.common_name + "_data";
        std::stringstream spirv_data_array_definition;
        spirv_data_array_definition << "const u32 " << spirv_data_array_name << "["
                                    << (file_size / 4) << "] = {\n";
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
        body_ss << "uShader " << baked_shader.second.common_name << " = \n{\n";
        body_ss << "\t.name = \"" << baked_shader.second.common_name << "\",\n";
        body_ss << "\t.data = &" << spirv_data_array_name << "[0],\n";
        body_ss << "\t.data_size = " << std::to_string(file_size) << ",\n";
        body_ss << "\t.type = " << kUnderstoneShaderTypeEnumPrefix << shader_type_string << ",\n";
        body_ss << "\t.module = 0\n";
        body_ss << "};\n";

        // Newline if adding another baked shader block to an existing file
        std::string& baked_shader_body_text = baked_file_attributes[baked_shader_full_path].body;
        if (!baked_shader_body_text.empty()) baked_shader_body_text += "\n\n";
        baked_shader_body_text += body_ss.str();

        // Log file name to baked shaders, so that compilation step can find them
        baked_shader.second.auto_gen_output_file_path = baked_shader_full_path;
    }

    // Write files
    {
        for (const auto& attribute_iter : baked_file_attributes)
        {
            std::ofstream file_stream(attribute_iter.first, std::ios::out);
            if (!file_stream.is_open())
            {
                PrintLn("Cannot open shader file: " + attribute_iter.first, OutputType::kError);
                return false;
            }

            const std::string pragma_once = "#pragma once\n\n";
            file_stream << generic_header << attribute_iter.second.raw_shaders << compilation_flag
                        << "\n"
                        << comment << new_lines << pragma_once << includes << new_lines
                        << attribute_iter.second.body;

            file_stream.close();

            // Log file name to files created this shader compilation, so we can clean up old files
            files_created_this_compilation.insert(std::filesystem::path(attribute_iter.first));
        }
    }

    // Delete files which were not created during this most recent bake
    for (const auto& attribute_iter : baked_file_attributes)
    {
        for (auto& file_or_empty_dir :
             std::filesystem::directory_iterator(attribute_iter.second.baked_shader_dir))
        {
            std::filesystem::path del_path = file_or_empty_dir.path();
            if (del_path.filename() == kWarningFileName) continue;

            if (files_created_this_compilation.find(del_path) ==
                files_created_this_compilation.end())
            {
                if (std::filesystem::is_regular_file(file_or_empty_dir) ||
                    (std::filesystem::is_directory(file_or_empty_dir) &&
                     std::filesystem::is_empty(file_or_empty_dir)))
                {
                    std::filesystem::remove(file_or_empty_dir);
                }
            }
        }

        // Create warning file if one doesn't exist
        {
            std::string warning_file_path = attribute_iter.second.baked_shader_dir.string() + '/' +
                                            kWarningFileName;
            if (!std::filesystem::exists(std::filesystem::path(warning_file_path)))
            {
                std::ofstream file_stream(warning_file_path, std::ios::out);
                if (!file_stream.is_open())
                {
                    PrintLn("Cannot open baked shader warning file for writing.",
                            OutputType::kError);
                    return false;
                }

                file_stream << "!!\n!! WARNING\n!!\n\n- The top-level of this directory is for "
                               "auto generated baked "
                               "shaders only.\n- "
                               "For build tool compilation, top-level files which were were not "
                               "generated by that "
                               "compilation will be "
                               "deleted.\n- "
                               "Subdirectories with contents will remain unaffected.";
            }
        }
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

    std::string options_str     = "Options:";
    std::string sub_options_str = "Description:";
    std::string table_header    = options_str +
                               spacing_string.substr(options_str.size(), spacing_string.size()) +
                               sub_options_str;
    std::string table_header_separator = "";
    for (size_t s = 0; s < table_header.size(); s++)
    {
        table_header_separator += "-";
    }

    std::stringstream help_text;
    help_text << std::endl
              << "Usage: " << kBuildProgramName << " [ options ] [ sub-options ]" << std::endl
              << std::endl;
    help_text << table_header << std::endl << table_header_separator << std::endl;

    for (const auto& arg_pair : kAllCommandLineArgs)
    {
        help_text << arg_pair.first
                  << spacing_string.substr(arg_pair.first.size(), spacing_string.size())
                  << arg_pair.second << std::endl;
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
    understone_root_dir = DetermineUnderstoneRootDirectory();
    if (understone_root_dir.empty())
    {
        std::string error_message = "Unable to find the root Understone directory along any sub or "
                                    "parent directory of the current path: " +
                                    kCurrentDirectory.string();
        PrintLn(error_message, OutputType::kError);
        return -1;
    }
    understone_root_dir = ToPosixPath(understone_root_dir);

    // Build source dependency lists
    ProjectMap projects = {};
    if (!AcquireProjectDependencies(projects))
    {
        PrintLn("Unable to acquire project dependency information.", OutputType::kError);
        return -1;
    }

    // Find Understone project.
    bool    understone_project_found = false;
    Project understone_project       = {};
    for (const auto& project : projects)
    {
        if (project.second.project_name == kUnderstoneEngineProjectName)
        {
            understone_project       = project.second;
            understone_project_found = true;
        }
    }

    if (false == understone_project_found)
    {
        PrintLn("Unable to find the understone project.", OutputType::kError);
        return -1;
    }

    Compiler               user_compiler            = default_user_compiler;
    ShaderCompiler         user_shader_compiler     = default_user_shader_compiler;
    UserCompilationFlags   user_compilation_flags   = default_user_compilation_flags;
    UserCompilationOptions user_compilation_options = default_user_compilation_options;
    UserBuildFlags         user_build_flags         = default_user_build_flags;

    // Parse command line arguments
    CommandLineArguments command_line_args(argc - 1);
    for (int arg_index = 1; arg_index < argc; arg_index++)
    {
        std::string temp_string(argv[arg_index]);
        if (temp_string.size() > 1)
        {
            command_line_args[arg_index - 1] = temp_string;
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

    // Note: Sanity checks for other generators are called on construction
    if (!UserArgumentSanityCheck(user_build_flags))
    {
        return -1;
    }

    CompilerInvocationGenerator compiler_generator = CompilerInvocationGenerator(
      user_compilation_flags,
      user_compilation_options,
      user_build_flags);
    if (!compiler_generator.IsOk())
    {
        return -1;
    }

    // Shader invocation & compilation
    compiler_generator.GenerateShaderInvocation(user_shader_compiler,
                                                user_compilation_flags,
                                                projects);
    if (!compiler_generator.IsOk())
    {
        return -1;
    }

    // Format Understone Engine files first.
    if (user_build_flags.at(BuildFlags::kRunCodeFormatter))
    {
        PrintLn("Formatting Understone Engine source files...");
        const std::string expected_understone_project_dir = understone_root_dir + '/' +
                                                            kEngineSourceFolderName;
        const auto& find_result = projects.find(expected_understone_project_dir);
        if (find_result == projects.end())
        {
            PrintLn("Cannot format engine files; Understone project cannot be found in expected "
                    "location: " +
                    expected_understone_project_dir);
            return -1;
        }
        else if (std::system(GetCodeFormatInvocation(find_result->second).c_str()))
        {
            return -1;
        }
    }

    PrintLn("");
    for (auto& project : projects)
    {
        const std::string project_name = project.second.project_name;
        const std::string shader_compilation_invocation =
          compiler_generator.GetShaderInvocationByProjectName(project_name);

        PrintLn("Building project " + project_name + ':');

        // [ cfarvin::REVISIT ] Source inclusion system.
        //
        // Given that we've made some changes to the way sources are included for projects, it
        // may not be necessary in the future to skip these steps for the Undrestone Project.
        // We probably will want to skip them, but we should make sure once the new system is
        // in place.
        //
        // Don't compile shaders or source files for the Understone Project.
        if (project.second.project_name != kUnderstoneEngineProjectName)
        {
            if (!shader_compilation_invocation.empty())
            {
                PrintLn("  Compiling & baking shaders...");
                if ((std::system(shader_compilation_invocation.c_str())) ||
                    (!BakeShaders(project.second.baked_shader_map, user_compilation_flags)))
                {
                    PrintLn("\nCompiled with:\n" + shader_compilation_invocation);
                    return -1;
                }
            }
            else
            {
                PrintLn("  No shader info provided...");
            }

            PrintLn("  Generating C/C++ compilation invocation...");
            compiler_generator.GenerateSourceInvocation(user_compiler,
                                                        user_compilation_flags,
                                                        user_compilation_options,
                                                        user_build_flags,
                                                        understone_project,
                                                        project.second,
                                                        project.first);

            if (user_build_flags.at(BuildFlags::kRunCodeFormatter))
            {
                PrintLn("  Formatting source files...");
                const std::string format_invocation = GetCodeFormatInvocation(project.second);
                if (std::system(format_invocation.c_str()))
                {
                    PrintLn("\nFormatted with:\n" + format_invocation);
                    return -1;
                }
            }

            PrintLn("  Compiling source files...");
            const std::string source_compilation_invocation =
              compiler_generator.GetSourceInvocationByProjectName(project.second.project_name);
            if (!compiler_generator.IsOk() || std::system(source_compilation_invocation.c_str()))
            {
                PrintLn("\nCompiled with:\n" + source_compilation_invocation);
                return -1;
            }

            PrintLn("  Complete.\n");
        }
    }

    // Code analysis
    if (user_build_flags.at(BuildFlags::kRunCodeAnalysis))
    {
        PrintLn("Running code analysis...");
        Analyzer analyzer = Analyzer();

        if (!analyzer.RunTagAnalysis(projects))
        {
            return -1;
        }

        if (!analyzer.RunAnalysisReport())
        {
            return -1;
        }
    }

    // Don't count understone engine project.
    const size_t num_projects = projects.size();
    if (num_projects <= 1)
    {
        PrintLn("Invalid project count.", OutputType::kError);
        return -1;
    }

    std::cout << "Successfully built " + std::to_string(num_projects - 1) + " projects."
              << std::endl;

    return 0;
}
