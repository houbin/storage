#ifndef STORAGE_INCLUDE_C_
#define STORAGE_INCLUDE_C_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 描述自1970年1月1日00:00:00经过的秒数和纳秒数 */
typedef struct utime
{
    uint32_t seconds;   
    uint32_t nseconds;    
}UTIME_T;

/* 描述某一视频流对应的录像段信息 */
typedef struct fragment_info
{
    UTIME_T start_time;
    UTIME_T end_time;
}FRAGMENT_INFO_T;

typedef struct frame_info
{
    uint32_t type;      // 帧类型
    UTIME_T frame_time; // 绝对时间，用于表示帧的时间
    uint64_t stamp;     // 相对时间戳，用于控制帧的播放
    uint32_t size;      // 帧长度
    char *buffer;       // 帧数据
}FRAME_INFO_T;

typedef struct disk_info
{
    char name[32];
    uint32_t capacity;
    char status[32];
}DISK_INFO_T;

/* 功能： 获取磁盘信息
 * 输入： disk 磁盘名称
 *
 * 输出： disk_info 获取的磁盘信息
 * 返回： 成功返回0，失败返回负数
 */
extern int32_t storage_get_disk_info(char *disk, DISK_INFO_T *disk_info);

/* 功能： 格式化一块磁盘
 * 输入： disk  磁盘名称
 *
 * 输出： 无
 * 返回： 成功返回0，失败返回负数
 */
extern int32_t storage_formate_disk(char *disk);

/* 功能： 初始化storage模块 
 * 输入：   无
 *
 * 输出：   无
 * 返回：   无
 */
extern void storage_init();

/* 功能： 通过stream info和读写标志获取视频流操作id。读取视频流操作可以打开多次，写视频流操作只能打开一次
 * 输入：   stream_info     视频流信息
 *          size            视频流信息长度，最长不超过64个字节
 *          flag            打开流的标志（读或者写）
 *                          0 读 
 *                          1 写
 *
 * 输出：   id              视频流操作id
 *          
 * 返回： 成功返回0，失败返回负数
 */
extern int32_t storage_open(char *stream_info, uint32_t size, int flags, uint32_t *id);

/* 功能： 通过id写入数据，
 * 输入：   id          操作的流id
 *          frame_info  视频数据内容
 *
 * 返回： 成功返回0，失败返回负数
 */
extern int32_t storage_write(const uint32_t id, FRAME_INFO_T *frame_info);

/* 功能： 获取指定时间段的录像段信息列表
 * 输入：   id          操作的视频流id
 *          start       起始时间
 *          end         结束时间
 *          frag_info   录像段数组头指针
 *          
 * 输出：   frag_info   获取的录像段数组头指针
 *          count       录像段个数
 *
 * 返回： 成功返回0，失败返回负数
 */
extern int32_t storage_list_record_fragments(const uint32_t id, const UTIME_T *start, const UTIME_T *end, FRAGMENT_INFO_T **frag_info, uint32_t *count);

/* 功能： 释放录像段内存
 * 输入：   frag_info   录像段数组头指针
 *
 * 返回： 成功返回0，失败返回负数
 */
extern void storage_free_record_fragments(FRAGMENT_INFO_T *frag_info);

/* 功能： 读取录像数据时，seek到某一时刻
 * 输入：   id      操作的视频流id
 *          stamp   需要seek到的时刻
 *
 * 返回： 成功返回0，失败返回负数
 */
extern int32_t storage_seek(const uint32_t id, const UTIME_T *stamp);

/* 功能： 读取视频流数据
 * 输入：   id      操作的视频流id
 *          
 * 输出：   frame_info  读取的视频帧数据，所有用到的内存由调用者申请
 *           
 * 返回： 读取的字节数（0表示没有读到数据），失败返回负数
 */
extern int32_t storage_read(const uint32_t id, FRAME_INFO_T *frame_info);

/* 功能： 关闭视频流id
 * 输入：   id 需要关闭的视频流操作id
 *
 * 输出：   无
 * 返回：   无
 */
extern void storage_close(const uint32_t id);

/* 功能： 关闭storage模块
 * 输入：
 *
 * 输出：   无
 * 返回：   无
 */
extern void storage_shutdown();


#ifdef __cplusplus
}
#endif

#endif
