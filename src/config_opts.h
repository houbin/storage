#ifndef STORAGE_CONFIG_OPTS_H_
#define STORAGE_CONFIG_OPTS_H_

const static char log_dir[] = "/var/log/storage.log";

#define STREAM_KEY_INFO_SIZE 64

#define MAX_WRITE_STREAM_COUNTS 300
#define MAX_READ_STREAM_COUNTS 1700

const static uint32_t kRecordFileSize = 256 * 1024 * 1024;
const static uint32_t kBlockSize = 1024 * 1024;
const static uint32_t kStripeCount = kRecordFileSize / kBlockSize;


const static uint32_t kMagicCode = 0x4A4F5649;

/* magic code + type + frame time + stamp + frame size */
const static kHeaderSize = 4 + 4 + 8 + 8 + 4;

#endif
