/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-03-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

typedef struct OsAudioSpeaker_s
{
#ifdef _WIN32
	HWAVEOUT m_waveOut;
	WAVEHDR m_hdr[2];
#elif __linux__
	snd_pcm_t* pcm_handle;
	snd_pcm_hw_params_t* params;
	//snd_pcm_uframes_t frames;
	char* buff;
	int channels;
#elif __APPLE__
#endif
	int m_buf_size;
	int m_bitsPerSample;
} OsAudioSpeaker;

static BOOL OsAudioSpeaker_init(OsAudioSpeaker* a, int buf_size, UINT samplesPerSec, int bitsPerSample, int channels)
{
	a->m_buf_size = buf_size;
	a->m_bitsPerSample = bitsPerSample;

#ifdef _WIN32
	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = samplesPerSec;
	wfx.wBitsPerSample = bitsPerSample;
	wfx.nChannels = channels;
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3)* wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if (waveOutOpen(&a->m_waveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		printf("Error: waveOutOpen");
		return FALSE;
	}

	int i;
	for (i = 0; i < 2; i++)
	{
		a->m_hdr[i].lpData = (char*)malloc(buf_size);
		a->m_hdr[i].dwBufferLength = 0;
		a->m_hdr[i].dwBytesRecorded = 0;
		a->m_hdr[i].dwUser = 0;
		a->m_hdr[i].dwFlags = WHDR_DONE; //getFreeBlock()!
		a->m_hdr[i].dwLoops = 0;
		a->m_hdr[i].lpNext = NULL;
		a->m_hdr[i].reserved = 0;
	}

#elif __linux__
	a->channels = channels;

	if (snd_pcm_open(&a->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
		return FALSE;

	snd_pcm_hw_params_alloca(&a->params);
	snd_pcm_hw_params_any(a->pcm_handle, a->params);

	if (snd_pcm_hw_params_set_access(a->pcm_handle, a->params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		return FALSE;

	if (bitsPerSample == 8) if (snd_pcm_hw_params_set_format(a->pcm_handle, a->params, SND_PCM_FORMAT_S8) < 0) return FALSE;
	if (bitsPerSample == 16) if (snd_pcm_hw_params_set_format(a->pcm_handle, a->params, SND_PCM_FORMAT_S16_LE) < 0) return FALSE;
	if (bitsPerSample == 32) if (snd_pcm_hw_params_set_format(a->pcm_handle, a->params, SND_PCM_FORMAT_S32_LE) < 0) return FALSE;

	//printf("pause: %d\n", snd_pcm_hw_params_can_pause(a->params));

	if (snd_pcm_hw_params_set_channels(a->pcm_handle, a->params, channels) < 0)
		return FALSE;
	if (snd_pcm_hw_params_set_rate_near(a->pcm_handle, a->params, &samplesPerSec, 0) < 0)
		return FALSE;

	snd_pcm_uframes_t frames = (a->m_buf_size / channels / (bitsPerSample / 8));
	if (snd_pcm_hw_params_set_period_size(a->pcm_handle, a->params, frames, 0) < 0)
		return FALSE;

	if (snd_pcm_hw_params_set_periods(a->pcm_handle, a->params, 4, 0) < 0)
		return FALSE;

	if (snd_pcm_hw_params(a->pcm_handle, a->params) < 0)
		return FALSE;

	if (snd_pcm_prepare(a->pcm_handle) < 0)
		return FALSE;

	a->buff = calloc(1, a->m_buf_size);

	snd_pcm_writei(a->pcm_handle, a->buff, 10);
	snd_pcm_prepare(a->pcm_handle);

#elif __APPLE__
#endif

	return TRUE;
}

static void OsAudioSpeaker_waitFree(OsAudioSpeaker* a)
{
#ifdef _WIN32
	while (!(a->m_hdr[0].dwFlags & WHDR_DONE)) Sleep(1);
	while (!(a->m_hdr[1].dwFlags & WHDR_DONE)) Sleep(1);
#elif __linux__
#elif __APPLE__
#endif
}

static void OsAudioSpeaker_free(OsAudioSpeaker* a)
{
	OsAudioSpeaker_waitFree(a);

#ifdef _WIN32
	int i;
	for (i = 0; i < 2; i++)
	{
		free(a->m_hdr[i].lpData);

		if (a->m_hdr[i].dwFlags & WHDR_PREPARED)
		{
			if (waveOutUnprepareHeader(a->m_waveOut, a->m_hdr + i, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				printf("Error: waveOutUnprepareHeader");
		}
	}
	waveOutClose(a->m_waveOut);
#elif __linux__
	//snd_pcm_drain(a->pcm_handle);
	snd_pcm_close(a->pcm_handle);
	free(a->buff);
#elif __APPLE__
#endif
}

OsAudioSpeaker* OsAudioSpeaker_new(int buf_size, int samplesPerSec, int bitsPerSample, int channels)
{
	OsAudioSpeaker* a = malloc(sizeof(OsAudioSpeaker));
	if (!OsAudioSpeaker_init(a, buf_size, samplesPerSec, bitsPerSample, channels))
	{
		free(a);
		a = 0;
	}
	return a;
}

void OsAudioSpeaker_delete(OsAudioSpeaker* a)
{
	OsAudioSpeaker_free(a);
	free(a);
}

#ifdef _WIN32

WAVEHDR* getFreeBlock(OsAudioSpeaker* a)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		if (a->m_hdr[i].dwFlags & WHDR_DONE)
			return a->m_hdr + i;
	}
	return 0;
}
#elif __linux__
#elif __APPLE__
#endif

static void _OsAudioSpeaker_copy(char* dst, char* src, int size, float volume, int bitsPerSample) //volume<0,1>
{
	if (volume == 1.0f)
	{
		memcpy(dst, src, size);
	}
	else
	{
		volume = volume * volume; //yeah

		int i;
		if (bitsPerSample == 8)
		{
			char* d = (char*)dst;
			char* s = (char*)src;
			const int N = size / sizeof(char);
			for (i = 0; i < N; i++)
				d[i] = (char)(s[i] * volume);
		}
		else
			if (bitsPerSample == 16)
			{
				short* d = (short*)dst;
				short* s = (short*)src;
				const int N = size / sizeof(short);
				for (i = 0; i < N; i++)
					d[i] = (short)(s[i] * volume);
			}
			else
				if (bitsPerSample == 32)
				{
					float* d = (float*)dst;
					float* s = (float*)src;
					const int N = size / sizeof(float);
					for (i = 0; i < N; i++)
						d[i] = (float)(s[i] * volume);
				}
	}
}

int OsAudioSpeaker_write(OsAudioSpeaker* a, short* data, int data_size, float volume)
{
#ifdef _WIN32
	WAVEHDR* bl;
	while (!(bl = getFreeBlock(a))) //block it
		Sleep(1);

	if (bl->dwFlags & WHDR_PREPARED)
	{
		if (waveOutUnprepareHeader(a->m_waveOut, bl, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			printf("Error: waveOutUnprepareHeader");
			return 0;
		}
	}

	if (data_size > a->m_buf_size)
		data_size = a->m_buf_size;
	_OsAudioSpeaker_copy(bl->lpData, (char*)data, data_size, volume, a->m_bitsPerSample);
	bl->dwBufferLength = data_size;

	if (waveOutPrepareHeader(a->m_waveOut, bl, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("Error: waveOutPrepareHeader");
		return 0;
	}

	if (waveOutWrite(a->m_waveOut, bl, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		printf("Error: waveOutWrite");
		return 0;
	}

#elif __linux__

	int err;
	if ((err = snd_pcm_wait(a->pcm_handle, 100)) < 0) //1000 = 1second max. timeout
	{
		//printf ("poll failed (%s)\n", strerror (errno));
		return 0;
	}

	snd_pcm_uframes_t frames = (data_size / a->channels / 2); //2=16bits

	snd_pcm_uframes_t n;
	if ((n = snd_pcm_avail_update(a->pcm_handle)) < frames)
		return 0;
	//printf("(%d-%d)\n", n, frames);

	_OsAudioSpeaker_copy(a->buff, (char*)data, data_size, volume, a->m_bitsPerSample);

	if (snd_pcm_writei(a->pcm_handle, a->buff, frames) == -EPIPE)
	{
		snd_pcm_prepare(a->pcm_handle);
	}
	else
		return 0;
#elif __APPLE__
#endif
	return data_size;
}

void OsAudioSpeaker_pause(OsAudioSpeaker* a, BOOL pause)
{
#ifdef _WIN32
	//...
#elif __linux__
	snd_pcm_pause(a->pcm_handle, pause);
#elif __APPLE__

#endif
}
