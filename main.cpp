////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:        wx-flags.cpp
// Purpose:     make wx-config work with modern static wxWidgets libraries on Windows
// Author:      archivesilver
// Created:     2024-08-18
// Copyright:   (c) archivesilver
// Licence:     wxWidgets licence
////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <fstream>
#include <iostream>
#include <windows.h>

std::string WXWIN, WXCFG, WXVER = "3.2"; // 3.2 is the current and the fallback version

bool fileExists(const std::string &filename) {
    DWORD attributes = GetFileAttributes(filename.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

void strstrip(std::string &str) {
    int size = str.length(), begin = 0;
    if(size <= 0) return;
    while(size > 0 && isspace(str[size - 1])) size--;
    while(size > begin && isspace(str[begin])) begin++;
    str = str.substr(begin, size - begin);
}

// fix WXWIN and WXCFG
void fixWXvars() {
    strstrip(WXWIN);
    if(WXWIN[-1] == '/' || WXWIN[-1] == '\\') WXWIN.pop_back();
    strstrip(WXCFG);
    if(WXCFG[-1] == '/' || WXCFG[-1] == '\\') WXCFG.pop_back();
    if(WXCFG[0] == '/' || WXCFG[0] == '\\') WXCFG.erase(0, 1);
}

// read environment variables
bool getWXvars() {
    if(getenv("WXWIN") && getenv("WXCFG")) {
        WXWIN = getenv("WXWIN");
        WXCFG = getenv("WXCFG");
        fixWXvars();
        return true;
    }
    return false;
}

std::string toUpperCase(const std::string &str) {
    std::string upperStr = str;
    CharUpperBuffA(&upperStr[0], upperStr.length()); // Modify inplace
    return upperStr;
}

// create build.cfg
void createBuildCfg() {
    if(WXWIN == "" || WXCFG == "") getWXvars();
    if(WXWIN == "" || WXCFG == "") return;

    std::string filename = WXWIN + "\\lib\\" + WXCFG + "\\build.cfg",
                setuph = WXWIN + "\\lib\\" + WXCFG + "\\wx\\setup.h", line = "";
    size_t pos, pos2;
    // skip if file exists
    if(!fileExists(filename)) {
        std::string config[39] = {
            "WXVER_MAJOR=",      "WXVER_MINOR=",  "WXVER_RELEASE=", "BUILD=",
            "MONOLITHIC=",       "SHARED=",       "UNICODE=",       "TOOLKIT=",
            "TOOLKIT_VERSION=",  "WXUNIV=",       "CFG=",           "VENDOR=",
            "OFFICIAL_BUILD=",   "DEBUG_FLAG=",   "DEBUG_INFO=",    "RUNTIME_LIBS=",
            "USE_EXCEPTIONS=",   "USE_RTTI=",     "USE_THREADS=",   "USE_AUI=",
            "USE_GUI=",          "USE_HTML=",     "USE_MEDIA=",     "USE_OPENGL=",
            "USE_QA=",           "USE_PROPGRID=", "USE_RIBBON=",    "USE_RICHTEXT=",
            "USE_STC=",          "USE_WEBVIEW=",  "USE_XRC=",       "COMPILER=",
            "COMPILER_VERSION=", "CC=",           "CXX=",           "CFLAGS=",
            "CPPFLAGS=",         "CXXFLAGS=",     "LDFLAGS="};
        // predefined values
        // I couldn't find a convenient place to read these values
        // so I fill them with the following values
        // feel free to change the code before execution or the file after
        config[3] += "debug";   // BUILD
        config[4] += "0";       // MONOLITHIC
        config[5] += "0";       // SHARED
        config[9] += "0";       // WXUNIV
        config[11] += "custom"; // VENDOR
        config[12] += "0";      // OFFICIAL_BUILD
        config[13] += "0";      // DEBUG_FLAG
        config[14] += "0";      // DEBUG_INFO
        config[15] += "static"; // RUNTIME_LIBS
        config[17] += "1";      // USE_RTTI  (is this equivalent to wxUSE_EXTENDED_RTTI?)
        config[24] += "1";      // USE_QA
        config[31] += "clang";  // COMPILER  (can this be read from wxINSTALL_PREFIX?)
        config[33] += "cc";     // CC
        config[34] += "c++";    // CXX

        // version
        // hoping WXWIN includes version info
        std::string ver = "";
        if(WXWIN != "") {
            std::string wxWidgets = "wxWidgets-";
            pos = WXWIN.rfind(wxWidgets);
            if(pos != std::string::npos) {
                pos2 = WXWIN.find("\\", pos);
                if(pos2 == std::string::npos) pos2 = WXWIN.find("/", pos);
                ver = WXWIN.substr(pos + wxWidgets.length(), pos2 - pos - wxWidgets.length());
            }
        }
        if(ver != "" && ver.length() >= 5) {
            config[0].push_back(ver[0]);
            config[1].push_back(ver[2]);
            config[2].push_back(ver[4]);
            WXVER = ver.substr(0, ver.rfind("."));
        }
        // TOOLKIT
        // reading from WXCFG
        if(WXCFG != "") {
            std::string inc = "include";
            pos = WXCFG.find(inc);
            if(pos != std::string::npos) {
                pos2 = WXCFG.find("-", pos + inc.length());
                if(pos2 != std::string::npos)
                    config[7] += toUpperCase(
                        WXCFG.substr(pos + inc.length() + 1, pos2 - pos - inc.length() - 1));
            }
        }
        // read setup.h for other values
        std::ifstream file(setuph);
        if(!file.is_open()) {
            std::cerr << "Error: Could not open file " << setuph << std::endl;
            return;
        }
        while(std::getline(file, line)) {
            strstrip(line);
            // technically reading the first line, which is a comment
            if(ver == "" && line.find("\\wx\\setup.h") != std::string::npos) {
                pos = line.find("\\wx\\setup.h");
                pos2 = line.rfind("-", pos);
                WXVER = line.substr(pos2 + 1, pos - pos2 - 1);
                config[0].push_back(WXVER[0]);
                config[1].push_back(WXVER[2]);
                config[2].push_back('5'); // fallback value
                // set UNICODE value
            } else if(line.find("wxUSE_UNICODE ") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[6].push_back('1');
                } else {
                    config[6].push_back('0');
                }
                // set USE_EXCEPTIONS value
            } else if(line.find("wxUSE_EXCEPTIONS") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[16].push_back('1');
                } else {
                    config[16].push_back('0');
                }
            } else if(line.find("wxUSE_THREADS") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[18].push_back('1');
                } else {
                    config[18].push_back('0');
                }
            } else if(line.find("wxUSE_AUI") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[19].push_back('1');
                } else {
                    config[19].push_back('0');
                }
            } else if(line.find("wxUSE_GUI ") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[20].push_back('1');
                } else {
                    config[20].push_back('0');
                }
            } else if(line.find("wxUSE_HTML") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[21].push_back('1');
                } else {
                    config[21].push_back('0');
                }
            } else if(line.find("wxUSE_MEDIACTRL") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[22].push_back('1');
                } else {
                    config[22].push_back('0');
                }
            } else if(line.find("wxUSE_OPENGL") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[23].push_back('1');
                } else {
                    config[23].push_back('0');
                }
            } else if(line.find("wxUSE_PROPGRID") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[25].push_back('1');
                } else {
                    config[25].push_back('0');
                }
            } else if(line.find("wxUSE_RIBBON") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[26].push_back('1');
                } else {
                    config[26].push_back('0');
                }
            } else if(line.find("wxUSE_RICHTEXT") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[27].push_back('1');
                } else {
                    config[27].push_back('0');
                }
            } else if(line.find("wxUSE_STC") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[28].push_back('1');
                } else {
                    config[28].push_back('0');
                }
            } else if(line.find("wxUSE_WEBVIEW ") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[29].push_back('1');
                } else {
                    config[29].push_back('0');
                }
            } else if(line.find("wxUSE_XRC") != std::string::npos) {
                if(line[line.length() - 1] == '1') {
                    config[30].push_back('1');
                } else {
                    config[30].push_back('0');
                }
            }
        }
        file.close();
        // build the cfg
        std::ofstream outfile(filename);

        // Check if the file was opened successfully
        if(!outfile) {
            std::cerr << "Error opening file for writing!" << std::endl;
            return;
        }

        // Write the data to the file
        for(std::string item : config) outfile << item << std::endl; 
        // Close the file
        outfile.close();
    }
}

// fix the include directories
std::string fixInclude(std::string str) {
    std::string inc = "/include";
    size_t pos = str.find(inc), pos2;

    // find all includes
    while(pos != std::string::npos) {
        // if the string isn't "/include/"
        if(str[pos + inc.length()] != '/') {
            // reverse find another '/'
            size_t bgn = str.rfind("/", pos - 1);
            if(bgn != std::string::npos &&
               str.substr(bgn, pos - bgn).find("wxWidgets") == std::string::npos) {
                str = str.substr(0, bgn) + str.substr(pos, std::string::npos);
            }
        }
        // try again
        pos = str.find(inc, pos + inc.length());
    }
    // replace dynamic flags (lazy for a separate function)
    std::string dyn = "-DWXUSINGDLL",
                rep = "-DWX_STATIC"; // I have no idea if the new value does anything
    pos = str.find(dyn);
    if(pos != std::string::npos) {
        pos2 = str.find(" ", pos);
        str = str.substr(0, pos) + rep + str.substr(pos2, std::string::npos);
    }
    dyn = "--define WXUSINGDLL";
    pos = str.find(dyn);
    if(pos != std::string::npos) {
        rep = "--define WX_STATIC"; // I have no idea if the new value does anything
        pos2 = str.find(" ", pos + dyn.length());
        str = str.substr(0, pos) + rep + str.substr(pos2, std::string::npos);
    }
    return str;
}

// read setup.h for library options and add libraries accordingly
// edit your setup.h to change the list of additions
std::string addSetupHLibs() {
    std::string temp = "", filename = WXWIN + "/lib/" + WXCFG + "/wx/setup.h", line = "";
    std::ifstream file(filename);
    if(!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return "";
    }
    // read setup.h
    while(std::getline(file, line)) {
        strstrip(line);
        // start with wxWidgets libraries
        if(line.find("wxUSE_AUI") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwx_mswu_aui-" + WXVER + " ";
        } else if(line.find("wxUSE_MEDIA") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwx_mswu_media-" + WXVER + " ";
        } else if(line.find("wxUSE_OPENGL") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwx_mswu_gl-" + WXVER + " ";
        } else if(line.find("wxUSE_PROPGRID") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwx_mswu_propgrid-" + WXVER + " ";
        } else if(line.find("wxUSE_RIBBON") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwx_mswu_ribbon-" + WXVER + " ";
        } else if(line.find("wxUSE_RICHTEXT") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwx_mswu_richtext-" + WXVER + " ";
        } else if(line.find("wxUSE_STC") != std::string::npos) {
            if(line[line.length() - 1] == '1') {
                temp += "-lwx_mswu_stc-" + WXVER + " ";
                temp += "-lwxscintilla-" + WXVER + " ";
            }
            // add non-wx libraries
        } else if(line.find("wxUSE_EXPAT") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lexpat ";
        } else if(
            // this isn't the correct flag, but it is referred for value
            // also there are 2 instances of this, either of them being 1 will add the flag
            line.find("wxUSE_GRAPHICS_CONTEXT ") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lgdiplus ";
        } else if(line.find("wxUSE_LIBJPEG") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-ljpeg ";
        } else if(line.find("wxUSE_LIBPNG") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lpng ";
        } else if(line.find("wxUSE_LIBTIFF") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-ltiff ";
        } else if(line.find("wxUSE_OLE") != std::string::npos) {
            if(line[line.length() - 1] == '1') {
                temp += "-lole32 ";
                temp += "-loleaut32 ";
                // it is recommended to only add this if necessary
                // it causes compilation to fail because it is not found
                // temp += "-lole2w32 ";
            }
        } else if(line.find("wxUSE_SOCKETS") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lwsock32 ";
        } else if(line.find("wxUSE_ZLIB") != std::string::npos) {
            if(line[line.length() - 1] == '1') temp += "-lz ";
        }
    }
    return temp;
}

std::string addFlaglessLibs(bool custom) {
    // these are simply always (supposed to be) added by wx-config
    // if you wish to fine-tune these additions manually, use "--wxflagsCUSTOM
    if(!custom) {
        return "-lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm "
               "-lshell32 -lcomctl32 -lversion -lshlwapi -luxtheme -loleacc "
               "-luuid -lrpcrt4 -ladvapi32 ";
    }
    return "";
}

// add setup.h libraries
std::string addLibs(bool custom) {
    // read setup.h in $(BUILDDIR)\lib\wx\include\msw-unicode-static-WXVER\wx
    return addSetupHLibs() + addFlaglessLibs(custom);
}

// fix library names
std::string fixLibNames(std::string str) {
    // fix msw libraries
    std::string sub = "", fixed = "lwx_mswu", ver = "-" + WXVER + "";
    if(str.find("lwxmsw32ud") != std::string::npos) {
        sub = "lwxmsw32ud";
    } else if(str.find("lwxmsw32u") != std::string::npos) {
        sub = "lwxmsw32u";
    } else if(str.find("lwxmsw32d") != std::string::npos) {
        sub = "lwxmsw32d";
        fixed = "lwx_msw";
    } else if(str.find("lwxmsw32") != std::string::npos) {
        sub = "lwxmsw32";
        fixed = "lwx_msw";
    } else {
        return str;
    }
    size_t pos = str.find(sub), pos2;
    while(pos != std::string::npos) {
        pos2 = str.find(" ", pos);
        str = str.substr(0, pos) + fixed +
              str.substr(pos + sub.length(), pos2 - pos - sub.length()) + ver +
              str.substr(pos2, std::string::npos);
        pos = str.find(sub, pos2);
    }
    // fix base libraries
    sub = "", fixed = "lwx_baseu";
    if(str.find("lwxbase32ud") != std::string::npos) {
        sub = "lwxbase32ud";
    } else if(str.find("lwxbase32u") != std::string::npos) {
        sub = "lwxbase32u";
    } else if(str.find("lwxbase32d") != std::string::npos) {
        sub = "lwxbase32d";
        fixed = "lwx_base";
    } else if(str.find("lwxbase32") != std::string::npos) {
        sub = "lwxbase32";
        fixed = "lwx_base";
    } else {
        return str;
    }
    pos = str.find(sub);
    while(pos != std::string::npos) {
        pos2 = str.find(" ", pos);
        str = str.substr(0, pos) + fixed +
              str.substr(pos + sub.length(), pos2 - pos - sub.length()) + ver +
              str.substr(pos2, std::string::npos);
        pos = str.find(sub, pos2);
    }
    // Fix scintilla library
    sub = "lwxscintilla";
    if(str.find(sub) != std::string::npos) {
        str = str.substr(0, pos + sub.length()) + ver +
              str.substr(pos + sub.length(), std::string::npos);
    }

    return str;
}

// fix the library flags
std::string fixLibs(std::string str) {
    // first fix directories
    std::string inc = "lib/wx";
    size_t pos = str.find(inc);
    while(pos != std::string::npos) {
        // if the string isn't "lib/wx/"
        if(str[pos + inc.length()] != '/') {
            // remove "/wx"
            str = str.substr(0, pos + 3) + str.substr(pos + 6, std::string::npos);
        }
        // try again
        pos = str.find(inc, pos + inc.length());
    }
    return fixLibNames(str);
}

bool isDynamic(int argc, char *argv[]) {
    std::string str_dynamic = "--wxflagsDYNAMIC";
    for(int i = 0; i < argc; i++) {
        if(argv[i] == str_dynamic) return true;
    }
    return false;
}

bool isCustom(int argc, char *argv[]) {
    std::string str_dynamic = "--wxflagsCUSTOM";
    for(int i = 0; i < argc; i++) {
        if(argv[i] == str_dynamic) return true;
    }
    return false;
}

bool isLibs(int argc, char *argv[]) {
    std::string str_dynamic = "--libs";
    for(int i = 0; i < argc; i++) {
        if(argv[i] == str_dynamic) return true;
    }
    return false;
}

bool isCFlags(int argc, char *argv[]) {
    std::string str_dynamic = "--cflags";
    for(int i = 0; i < argc; i++) {
        if(argv[i] == str_dynamic) return true;
    }
    return false;
}

bool isRcFlags(int argc, char *argv[]) {
    std::string str_dynamic = "--rcflags";
    for(int i = 0; i < argc; i++) {
        if(argv[i] == str_dynamic) return true;
    }
    return false;
}

std::string createCommand(int argc, char *argv[]) {
    std::string temp = "wx-config", str_dynamic = "--wxflagsDYNAMIC",
                str_custom = "--wxflagsCUSTOM", str_prefix = "--prefix=", str_wxcfg = "--wxcfg=";
    for(int i = 1; i < argc; ++i) {
        if(argv[i] != str_dynamic && argv[i] != str_custom) {
            temp += " ";
            temp += argv[i];
        }
        // set environment variables if provided as flags
        // this one takes priority
        if(std::string(argv[i]).find(str_prefix) != std::string::npos) {
            WXWIN = std::string(argv[i]).substr(str_prefix.length(), std::string::npos);
        }
        if(std::string(argv[i]).find(str_wxcfg) != std::string::npos) {
            WXCFG = std::string(argv[i]).substr(str_wxcfg.length(), std::string::npos);
        }
    }
    return temp;
}

std::string wxconfig(const std::string &command) {

    // Create a pipe for the child process's output
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

    if(!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        std::cerr << "Error creating pipe" << std::endl;
        return "";
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if(!SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Error setting handle information" << std::endl;
        return "";
    }

    // Set up the STARTUPINFO structure.
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;

    // Set up the PROCESS_INFORMATION structure.
    PROCESS_INFORMATION pi;

    // Create the child process.
    if(!CreateProcess(NULL, const_cast<char *>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL,
                      &si, &pi)) {
        std::cerr << "Error creating process" << std::endl;
        return "";
    }

    // Close the write end of the pipe before reading from the read end of the pipe.
    CloseHandle(hPipeWrite);

    // Read the output from the pipe.
    CHAR buffer[4096];
    DWORD bytesRead;
    std::string output;

    while(ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    // Close handles
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return output;
}

int main(int argc, char **argv) {
    // create the wx-config command
    std::string command = createCommand(argc, argv);

    // create build.cfg
    createBuildCfg();

    // if dynamic flag is used
    if(isDynamic(argc, argv)) {
        // run wx-config with no modifications
        std::cout << wxconfig(command);
    } else {
        std::string output = wxconfig(command);
        if(isLibs(argc, argv)) {
            // It's not possible to speficy these in different places anymore.
            if(WXWIN == "" || WXCFG == "") {
                if(!getWXvars()) {
                    std::cerr << "Missing either WXWIN or WXCFG or both. Please "
                                 "specify either these environment variables or use "
                                 "--prefix= and --wxcfg= flags. ";
                }
            }
            // fix the output of wx-config
            output = fixLibs(output);
            // remove the newline at the end of the output
            strstrip(output);
            // add the libraries wx-config couldn't
            // by first reading from the setup.h file
            output += " " + addLibs(isCustom(argc, argv)) + "\n";
        } else if(isCFlags(argc, argv)) {
            output = fixInclude(output);
        } else if(isRcFlags(argc, argv)) {
            output = fixInclude(output);
        }
        std::cout << output << std::endl;
    }
    return 0;
}
