#ifndef STORAGE_CONFIG_OPTIONS_H
#define STORAGE_CONFIG_OPTIONS_H

#include <stdint.h>
#include <string>
#include "config.h"

using namespace std;
namespace util
{

class ConfigOption
{
private:
    string config_file_path_;
    Config *config_;

public:
    string log_dir_;
    string mount_point_parent_dir_;

    ConfigOption(string &path);
    ~ConfigOption();
    int32_t Init();
};

}

#endif
