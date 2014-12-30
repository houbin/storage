#ifndef STORAGE_ERRCODE_H_
#define STORAGE_ERRCODE_H_

#include <errno.h>

#define safe_free(ptr) do{if(ptr){free(ptr);ptr = NULL;}}while(0)


#define OK                          0

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

#endif
