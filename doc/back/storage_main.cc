#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <getopt.h>
#include "util/config_options.h"
#include "util/logger.h"
#include "util/errcode.h"
#include "src/discovery.h"
#include "src/vmsc_service.h"
#include "src/udp_service.h"
#include "src/stream_op_handler.h"
#include "src/stream_transfer_client_manager.h"
#include "src/index_file.h"

using namespace std;
using namespace util;
using namespace storage;

void Usage(char *arg)
{
    fprintf(stderr, "%s: -c [config file]\n", arg);
    return;
}

StreamOpHandler *stream_op_handler = NULL;
IndexFileManager *index_file_manager = NULL;
StreamTransferClientManager *transfer_client_manager = NULL;
Logger *logger = NULL;

int main(int argc, char *argv[])
{
    int opt;
    int32_t ret;
    string config_file;
    struct sockaddr_in in_addr = {0};
    struct sockaddr_in out_addr = {0};

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
        config_file.assign("/etc/jovision/storage.conf");
    }
    
    ConfigOption *config_option = new ConfigOption(config_file);
    config_option->Init();

    string log_dir(config_option->log_dir_);
    ret = NewLogger(log_dir.c_str(), &logger);
    if (ret != OK)
    {
        fprintf(stderr, "NewLogger error, log dir is %s, ret is %d\n", log_dir.c_str(), -ret);
        assert(ret != 0);
    }

    /* discovery */
#define DISCOVERY_RECV_PORT 9001
#define DISCOVERY_SEND_PORT 9002
    memset(&in_addr, 0, sizeof(struct sockaddr_in));
    memset(&out_addr, 0, sizeof(struct sockaddr_in));
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(DISCOVERY_RECV_PORT);
    in_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    out_addr.sin_family = AF_INET;
    out_addr.sin_port = htons(DISCOVERY_SEND_PORT);
    out_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    Discovery discovery(logger, in_addr, out_addr);

    /* stream transfer client manager */
    transfer_client_manager = new StreamTransferClientManager(logger);
    assert(transfer_client_manager != NULL);
    transfer_client_manager->Init();

    FreeFileTable free_file_table(transfer_client_manager);

    index_file_manager = new IndexFileManager(logger, transfer_client_manager, free_file_table);
    index_file_manager->Init();

    /* stream op handle */
    stream_op_handler = new StreamOpHandler(logger, transfer_client_manager);

    /* vms client service */
#define VMSCSERVICE_RECV_PORT 9003
    memset(&in_addr, 0, sizeof(struct sockaddr_in));
    memset(&out_addr, 0, sizeof(struct sockaddr_in)); /* 使用全0的out_addr，内部处理为sendto到recv的addr */
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(VMSCSERVICE_RECV_PORT);
    in_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    VmscService vmsc_service(logger, in_addr, out_addr, &stream_op_handler);
    
    /* bind */
    discovery.Bind();
    vmsc_service.Bind();

    /* thread start */
    discovery.Start();
    stream_op_handler->Start();
    vmsc_service.Start();

    /* wait for join */
    discovery.Join();
    vmsc_service.Join();
    stream_op_handler->Join();

    /* shutdown */
    free_file_table.Shutdown();
    transfer_client_manager->Shutdown();
    index_file_manager->Shutdown();

    delete config_option;
    delete stream_op_handler;
    delete transfer_client_manager;
    
    return 0;
}

