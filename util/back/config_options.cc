#include "config_options.h"

namespace util
{

ConfigOption::ConfigOption(string &path)
: log_dir_("/var/log/storage.log"), mount_point_parent_dir_("/jovision")
{
    config_file_path_.assign(path);
    config_ = new Config(config_file_path_.c_str());
}

int32_t ConfigOption::Init()
{
    log_dir_= config_->Read("log_dir", log_dir_);
    mount_point_parent_dir_ = config_->Read("mount_point_parent_dir", mount_point_parent_dir_);

    return 0;
}

ConfigOption::~ConfigOption()
{
    delete config_;
    config_ = NULL;
}

}
