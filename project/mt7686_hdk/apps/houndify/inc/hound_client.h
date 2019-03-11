#ifndef __HOUND_CLIENT_H__
#define __HOUND_CLIENT_H__

#include <time.h>

const char *hound_lib_version(void);
void hound_client(const char *client_id, const char *client_key, const char *user_id);
void hound_set_time(time_t time_diff);
void hound_query_start(void);

#endif
