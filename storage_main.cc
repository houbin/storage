#include <unistd.h>
#include <string>
#include "util/config.h"
#include "util/logger.h"

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

	fprintf(stdout, "config file is %s\n", config_file.c_str());

	Config config(config_file.c_str());
	
	Config::mapci iter = config.m_Contents.begin();
	for (iter = config.m_Contents.begin(); iter != config.m_Contents.end(); iter++)
	{
		cout << "key is " << iter->first << ", value is " << iter->second << endl;
	}



	return 0;
}

