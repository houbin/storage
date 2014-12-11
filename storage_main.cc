#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <getopt.h>
#include "util/config_options.h"
#include "util/logger.h"
#include "util/errcode.h"

using namespace std;
using namespace util;

void Usage(char *arg)
{
	fprintf(stderr, "%s: -c [config file]\n", arg);
	return;
}

int main(int argc, char *argv[])
{
	int opt;
	int32_t ret;
	string config_file;

	struct option longopts[] = {
		{"config", 1, NULL, 'c'},
		{"help", 0, NULL, 'h'},
		{0,0,0,0}};

	while ((opt = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1)
	{
		switch(opt)
		{
			case 'h':
				Usage(argv[0]);
				break;

			case 'c':
				config_file.assign(optarg);
				break;

			case ':':
				fprintf(stderr, "options need a value\n");
				return -1;
			
			case '?':
				fprintf(stderr, "unknown option: %c\n", optopt);
				return -1;
			
			default:
				Usage(argv[0]);
				return -1;
		}
	}

	if (config_file == "")
	{
		fprintf(stdout, "config file is NULL, so use default config: /etc/jovision/storage.conf\n");
		config_file.assign("/etc/jovision/storage.conf");
	}
	
	ConfigOption *config_option = new ConfigOption(config_file);
	config_option->Init();

	string log_dir(config_option->log_dir_);
	Logger *logger = NULL;
	ret = NewLogger(log_dir.c_str(), &logger);
	if (ret != OK)
	{
		fprintf(stderr, "NewLogger error, log dir is %s, ret is %d\n", log_dir.c_str(), -ret);
		assert(ret != 0);
	}

	cout << "log dir is " << log_dir.c_str() << endl;

	
	return 0;
}

