#define AUDIO_BUF_SIZE		(1048576)	//1024 * 1024

static char rsp_audio_buf[AUDIO_BUF_SIZE];
static unsigned long int cur_idx;

void audio_buffer_reset(void)
{
	cur_idx = 0;
}

char *get_audio_buffer(void)
{
	return rsp_audio_buf;
}

void add_audio_buffer(char data)
{
	if (cur_idx >= AUDIO_BUF_SIZE)
		return;

	rsp_audio_buf[cur_idx++] = data;
	if (cur_idx >= AUDIO_BUF_SIZE)
		rsp_audio_buf[AUDIO_BUF_SIZE - 1] = 0;
}

unsigned long int get_buffer_size(void)
{
	return AUDIO_BUF_SIZE;
}
