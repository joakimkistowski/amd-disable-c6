/*
 * MIT License
 *
 * Copyright (c) 2018 Jóakim von Kistowski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credit goes to r4m0n for creating the original zenstates.py at https://github.com/r4m0n/ZenStates-Linux/blob/master/zenstates.py
 */

#include <stdint.h>
#include <iostream>
#include <cstdio>
#include <dirent.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <unistd.h>
#include <fstream>
#include <stdexcept>

const std::string FLAG_HELP0 = "-h";
const std::string FLAG_HELP1 = "--help";
const std::string FLAG_DISABLE = "--disable_c6";
const std::string FLAG_DISABLE_RETRY = "--disable_c6_retry";
const std::string FLAG_ENABLE = "--enable_c6";
const std::string FLAG_LIST0 = "-l";
const std::string FLAG_LIST1 = "--list";

const unsigned int MAX_RETRIES = 4;
const unsigned int RETRY_SLEEP_DURATION_S = 4;

const long int PACKAGE_MSR_ADDR = 0xC0010292;
const long int CORE_MSR_ADDR = 0xC0010296;

const uint64_t PACKAGE_MSR_ENABLED = (1UL << 32);
const uint64_t PACKAGE_MSR_DISABLED = ~PACKAGE_MSR_ENABLED;
const uint64_t CORE_MSR_ENABLED = ((1UL << 22) | (1UL << 14) | (1UL << 6));
const uint64_t CORE_MSR_DISABLED = ~CORE_MSR_ENABLED;

enum op_mode {DISABLE, RETRYDISABLE, ENABLE, LIST, NONE, ERROR};

void get_cpu_ids(std::vector<int> &ids) {
    const std::string DOT = std::string(".");
    const std::string MICROCODE = std::string("microcode");
    DIR * dir = opendir("/dev/cpu/");
    if (!dir) {
        std::cerr << "Failed reading CPU ids" << std::endl;
        return;
    }
    struct dirent * ent;
    while ((ent = readdir(dir)) != NULL) {
            std::string name(ent->d_name);
            if (!name.compare(0, DOT.length(), DOT) == 0 && name != MICROCODE) {
                try {
                    ids.push_back(std::stoi(std::string(ent->d_name)));
                } catch (const std::invalid_argument& ia) {
                    std::cerr << "Invalid CPU ID: " << ent->d_name << std::endl;
                }
            }
            
    }
    closedir(dir);
}
    
unsigned int write_msr(long int msr_addr, uint64_t value, int cpu_id = -1) {
    if (cpu_id < 0) {
        unsigned int ret = 0;
        std::vector<int> ids;
        get_cpu_ids(ids);
        for (const int& id : ids) {
            if (id > 0) {
                ret = ret | write_msr(msr_addr, value, id);
            }
        }
        return ret;
    } else {
        std::string path = std::string("/dev/cpu/") + std::to_string(cpu_id) + std::string("/msr");
        FILE * f = fopen(path.c_str(), "w");
        if (!f) {
            std::cerr << "Failed writing of C6 state; msr module is not loaded in kernel" << std::endl;
            return 1;
        }
        int err = fseek (f, msr_addr, SEEK_SET);
        if (err) {
            std::cerr << "Failed writing of C6 state; error writing to msr" << std::endl;
            fclose(f);
            return 1;
        }
        fwrite(&value, 8, 1, f);
        fclose(f);
    }
    return 0;
}
    
uint64_t read_msr(long int msr_addr, unsigned int cpu_id = 0) {
    std::string path = std::string("/dev/cpu/") + std::to_string(cpu_id) + std::string("/msr");
    FILE * f = fopen(path.c_str(), "r");
    if (!f) {
        std::cerr << "Failed reading of C6 state; msr module is not loaded in kernel" << std::endl;
        return 0;
    }
    int err = fseek (f, msr_addr, SEEK_SET);
    if (err) {
        std::cerr << "Failed reading of C6 state; error reading msr" << std::endl;
        fclose(f);
        return 0;
    }
    uint64_t msr_value = 0;
    fread(&msr_value, 8, 1, f);
    fclose(f);
    return msr_value;
}
    
void print_help() {
    std::cout << "Changes and reads the C6 state on AMD Zen (Epyc/Ryzen) processors." << std::endl;
    std::cout << std::endl <<  "Options:" << std::endl;
    std::cout << FLAG_HELP0 << " or " << FLAG_HELP1 << ": Print this help message." << std::endl;
    std::cout << FLAG_LIST0 << " or " << FLAG_LIST1 << ": List current C6 states." << std::endl;
    std::cout << FLAG_DISABLE << ": Disable C6 states. This is the default behavior." << std::endl;
    std::cout << FLAG_DISABLE_RETRY << ": Disable C6 states. On failure, retry up to "
        << MAX_RETRIES << " times and sleep " << RETRY_SLEEP_DURATION_S << " s between trys." << std::endl;
    std::cout << FLAG_ENABLE << ": Enable C6 states." << std::endl;
}

op_mode parse_op_mode(const int argc, const char *argv[]) {
    if (argc <= 1) {
        std::cout << "Disabling C6 state. Run with \"-h\" to display other options." << std::endl;
        return DISABLE;
    }
    std::string argument(argv[1]);
    if (argument == FLAG_HELP0 || argument == FLAG_HELP1) {
        print_help();
        return NONE;
    } else if (argument == FLAG_LIST0 || argument == FLAG_LIST1) {
        return LIST;
    } else if (argument == FLAG_DISABLE) {
        return DISABLE;
    } else if (argument == FLAG_DISABLE_RETRY) {
        return RETRYDISABLE;
    } else if (argument == FLAG_ENABLE) {
        return ENABLE;
    }
    std::cerr << "Error, unknown parameter: " << argument << std::endl;
    return ERROR;
}

bool has_amd_cpu() {
    const std::string VENDOR_PREFIX = std::string("vendor_id");
    const std::string VENDOR_REQUIRED = std::string(": AuthenticAMD");
    bool has_amd = false;
    std::string line;
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        while (getline(cpuinfo,line)) {
            if (line.compare(0, VENDOR_PREFIX.length(), VENDOR_PREFIX) == 0
                && line.find(VENDOR_REQUIRED) != std::string::npos) {
                has_amd = true;
                break;
            }
        }
        cpuinfo.close();
  }
  return has_amd;
}

int direct_execution_main(const op_mode &mode) {
    switch(mode) {
        case ERROR:
            return 1;
        case NONE:
            return 0;
        default:
            uint64_t core_msr_value = read_msr(CORE_MSR_ADDR);
            uint64_t package_msr_value = read_msr(PACKAGE_MSR_ADDR);
            if (!core_msr_value || !package_msr_value) {
                std::cerr << "Error reading msr; got invalid value" << std::endl;
                return 1;
            }
            if (mode == ENABLE) {
                    unsigned int err = write_msr(PACKAGE_MSR_ADDR, package_msr_value | PACKAGE_MSR_ENABLED);
                    err = err | write_msr(CORE_MSR_ADDR, core_msr_value | CORE_MSR_ENABLED);
                    if (err) {
                        return 1;
                    }
                    std::cout << "Enabled C6 state" << std::endl;
                    return 0;
            } else if (mode == DISABLE) {
                    unsigned int err = write_msr(PACKAGE_MSR_ADDR, package_msr_value & PACKAGE_MSR_DISABLED);
                    err = err | write_msr(CORE_MSR_ADDR, core_msr_value & CORE_MSR_DISABLED);
                    if (err) {
                        return 1;
                    }
                    std::cout << "Disabled C6 state" << std::endl;
                    return 0;
            } else if (mode == LIST) {
                    std::cout << "C6 State - Package : ";
                    if (package_msr_value & PACKAGE_MSR_ENABLED) {
                        std:: cout << "Enabled" << std::endl;
                    } else {
                        std:: cout << "Disabled" << std::endl;
                    }
                    std::cout << "C6 State - Core    : ";
                    if ((core_msr_value & CORE_MSR_ENABLED) == CORE_MSR_ENABLED) {
                        std:: cout << "Enabled" << std::endl;
                    } else {
                        std:: cout << "Disabled" << std::endl;
                    }
                    return 0;
            }
    }
    return 0;
}

int retry_execution_main() {
    if (direct_execution_main(DISABLE)) {
        for (unsigned int i = 0; i < MAX_RETRIES; i++) {
            std::cout << "Disabling C6 state failed; retrying in " << RETRY_SLEEP_DURATION_S 
                << " seconds" << std::endl;
            sleep(RETRY_SLEEP_DURATION_S);
            if (!direct_execution_main(DISABLE)) {
                return 0;
            }
        }
    }
    std::cerr << "Failed disabling C6 state on retry #" << MAX_RETRIES << "; accepting failure" << std::endl;
    return 1;
}

int main(const int argc, const char *argv[]) {
    if (!has_amd_cpu()) {
        std::cerr << "Failed disabling of C6 state; no AMD CPU found" << std::endl;
        return 1;
    }
    op_mode mode = parse_op_mode(argc, argv);
    if (mode == RETRYDISABLE) {
        return retry_execution_main();
    } else {
        return direct_execution_main(mode);
    }
}
