#ifndef INCLUDE_ERRCODE_H_
#define INCLUDE_ERRCODE_H_

#include <errno.h>

#define ERR_SELF_DEFINE_START       2000
#define ERR_INVALID_INPUT_PARA      2001
#define ERR_FILE_NOT_EXIST          2002
#define ERR_OPEN_FILE               2003
#define ERR_REACH_MAX_STREAM        2004
#define ERR_STREAM_NOT_FOUND        2005
#define ERR_MALLOC_FAILED           2006
#define ERR_ENTRY_NOT_FOUND         2007

#define ERR_CREATE_SOCKET           2008
#define ERR_TIMEOUT                 2009

#define ERR_RECODE_NOT_FOUND        2010
#define ERR_DOUBLE_WRITE            2011
#define ERR_SHUTDOWN                2012

#define ERR_ITEM_NOT_FOUND          3000
#define ERR_REACH_WRITE_THREHOLD    3001
#define ERR_REACH_READ_THREHOLD     3002
#define ERR_RECORD_WRITE_OFFSET_ZERO 3003
#define ERR_NO_MAGIC_CODE           3004
#define ERR_SEEK_ERROR              3005


#endif
