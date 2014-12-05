#include <unistd.h>
#include <string>
#include "util/config.h"
#include "util/logger.h"
#include "util/errcode.h"

using namespace storage;
using namespace std;

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

	if (argc < 2)
	{
		Usage(argv[0]);
		return -1;
	}

	while ((opt = getopt(argc, argv, "hc:")) != -1)
	{
		switch(opt)
		{
			case 'h':
				Usage(argv[0]);
				break;

			case 'c':
				config_file.assign(optarg);
				break;
			
			default:
				Usage(argv[0]);
				return -1;
		}
	}

	Config config(config_file.c_str());

	string log_dir;
	log_dir = config.Read("log_dir", log_dir);
	if (log_dir.c_str() == "")
	{
		log_dir.assign("/var/log/storage.log");
	}

	Logger *logger = NULL;
	ret = NewLogger(log_dir.c_str(), &logger);
	if (ret != OK)
	{
		fprintf(stderr, "NewLogger error, log dir is %s\n", log_dir.c_str());
		return -1;
	}

	char mount_dir[] = "/jovision";






	return 0;
}

