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

BOOL Os_getMAC(UCHAR mac[6])
{
#ifdef _WIN32
	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus == ERROR_SUCCESS)
	{
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		do
		{
			memcpy(mac, pAdapterInfo->Address, 6);
			if (Std_sizeCHAR(mac))
				return TRUE;
			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
	}
	return FALSE;
#else
	struct ifreq s;
	memset(&s, 0, sizeof(s));

	int fd = socket(AF_INET, SOCK_DGRAM, 0);

	s.ifr_addr.sa_family = AF_INET;
	strncpy(s.ifr_name, "enp0s3", IFNAMSIZ - 1);
	//strcpy(s.ifr_name, "eth0");
	BOOL ok = !ioctl(fd, SIOCGIFHWADDR, &s);
	if (ok)
	{
		memcpy(mac, s.ifr_hwaddr.sa_data, 6);
		//printf("Mac address: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	return ok;
#endif
}

UBIG Os_getUID(void)
{
	UCHAR uid[6 + 6];
	if (Os_getMAC(uid))
	{
		memcpy(uid + 6, "SkyAlt", 6);

		OsCryptoSha2 sha;
		OsCryptoSha2_exe(uid, sizeof(uid), &sha);
		return *(UBIG*)&sha;
	}
	return 0;
}

void OsWeb_openWebBrowser(const char* webAddress)
{
	if (!webAddress)
		return;

#ifdef _WIN32
	ShellExecute(NULL, "open", webAddress, NULL, NULL, SW_SHOWNORMAL);
	return;
#else
#ifdef __APPLE__
	const char* cmdS = "open ";
	const char* cmdE = "";
#else
	const char* cmdS = "x-www-browser ";
	const char* cmdE = " &";	//new process
#endif
	char* path = malloc(Std_sizeCHAR(cmdS) + Std_sizeCHAR(webAddress) + Std_sizeCHAR(cmdE) + 1);
	strcpy(path, cmdS);
	strcat(path, webAddress);	//add address
	strcat(path, cmdE);

	if (system(path) < 0)	//go
		printf("Warning: system() web_browser\n");
	free(path);
#endif
}

char* OsWeb_generateEmailText(const char* dstEmail, const char* subject)
{
	if (!dstEmail)
		return 0;

#ifdef _WIN32
	const char* cmdS = "mailto:";
#elif __APPLE__
	const char* cmdS = " ";//...
#else
	const char* cmdS = "xdg-open mailto:";
#endif

	const char* cmdS2 = subject ? "?subject=" : 0;

	char* path = malloc(Std_sizeCHAR(cmdS) + Std_sizeCHAR(dstEmail) + Std_sizeCHAR(cmdS2) + Std_sizeCHAR(subject) + 1);
	strcpy(path, cmdS);
	strcat(path, dstEmail);	//add address
	if (cmdS2)
	{
		strcat(path, cmdS2);
		strcat(path, subject);	//add address
	}

	return path;
}

void OsWeb_openEmail(const char* dstEmail, const char* subject)
{
	char* path = OsWeb_generateEmailText(dstEmail, subject);
	if (!path)
		return;

#ifdef _WIN32
	ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
	//...
#else
	if (system(path) < 0)	//go
		printf("Warning: system() email\n");
#endif

	free(path);
}

const char* OsNet_init()
{
#ifdef WIN32
	struct WSAData wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) == SOCKET_ERROR)
		return "WSAStartup failed";
#endif
	return 0;
}

void OsNet_free()
{
#ifdef WIN32
	WSACleanup();
#endif
}

char* OsNet_getIpFromDomain(char* domain)
{
	char* out_ip = 0;

	struct addrinfo* result;

	//get IPv6
	//struct addrinfo hints;
	//memset(&hints, 0, sizeof(hints));
	//hints.ai_family = AF_INET6;
	//hints.ai_socktype = SOCK_STREAM;
	//int error = getaddrinfo(domain, NULL, &hints, &result);

	int error = getaddrinfo(domain, NULL, NULL, &result);
	if (!error)
	{
		struct addrinfo* res;
		for (res = result; res != NULL; res = res->ai_next)
		{
			char hostname[NI_MAXHOST];
			error = getnameinfo(res->ai_addr, (int)res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
			if (error)
				continue;

			if (*hostname != '\0')
			{
				out_ip = Std_newCHAR(hostname);
				break;
			}
		}

		freeaddrinfo(result);
	}

	return out_ip;
}

OsNetSocket OsNetSocket_initClosed(void)
{
	OsNetSocket self;
	self.m_socket = -1;
	return self;
}
void OsNetSocket_free(OsNetSocket* self)
{
#ifdef WIN32
	closesocket(self->m_socket);
#else
	close(self->m_socket);
#endif
	self->m_socket = -1;
}

void OsNetSocket_stop(OsNetSocket* self)
{
#ifdef WIN32
	shutdown(self->m_socket, SD_BOTH);
#else
	shutdown(self->m_socket, SHUT_RDWR);
#endif
}

BOOL OsNetSocket_initUDP(OsNetSocket* self, USHORT port, BOOL blocking)
{
	*self = OsNetSocket_initClosed();

	self->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (self->m_socket == SOCKET_ERROR)
	{
		return FALSE;
	}

	u_long iMode = blocking ? 0 : 1;
#ifdef _WIN32
	if (ioctlsocket(self->m_socket, FIONBIO, &iMode) != NO_ERROR)
#elif __linux__
	if (ioctl(self->m_socket, FIONBIO, &iMode) != 0)
#endif
	{
		return FALSE;
	}

	//linux
	//fcntl(self->m_socket, F_GETFL);
	//flags |= O_NONBLOCK;
	//fcntl(sock, F_SETFL, flags);

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(self->m_socket, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR)
	{
		OsNetSocket_free(self);
		return FALSE;
	}

	return TRUE;
}

BOOL OsNetSocket_is(OsNetSocket* self)
{
	return (self->m_socket > SOCKET_ERROR);
}

//non-blocking for linux - flags = MSG_DONTWAIT ...
int OsNetSocket_recvDatagram(OsNetSocket* self, UCHAR* data, const int max_size, struct sockaddr_storage* clientInfo, UINT addrlen)
{
	int nbyte;
	//SOCKADDR_IN clientInfo = {0};
	//UINT addrlen = sizeof(clientInfo);

	nbyte = recvfrom(self->m_socket, data, max_size, 0, (struct sockaddr*)clientInfo, &addrlen);

	//*out_port = ntohs(clientInfo.sin_port);
	//*out_ip = inet_ntoa(clientInfo.sin_addr);

	return nbyte == SOCKET_ERROR ? -1 : nbyte;
}

int OsNetSocket_sendDatagramTo(OsNetSocket* self, const UCHAR* data, const int size, struct sockaddr_storage* clientInfo, UINT addrlen)
{
	int nbyte;
	//SOCKADDR_IN clientInfo = {0};
	//int addrlen = sizeof(clientInfo);

	//clientInfo.sin_family = AF_INET;
	//clientInfo.sin_port = htons(port);
	//clientInfo.sin_addr = *out_ip;
	//if(!inet_aton(ip, &clientInfo.sin_addr))
	//	return -1;

	nbyte = sendto(self->m_socket, data, size, 0, (struct sockaddr*)clientInfo, addrlen);

	return nbyte == SOCKET_ERROR ? -1 : nbyte;
}

OsNetLimit OsNetLimit_initBytes(const int max_bytes)
{
	OsNetLimit self;
	self.max_bytes = max_bytes;
	self.act_bytes = 0;
	self.reset_time = Os_time();
	return self;
}

OsNetLimit OsNetLimit_initMegaBits(const float max_megabits)
{
	return OsNetLimit_initBytes((int)(max_megabits * 1000000 / 8));
}

OsNetLimit OsNetLimit_initInactive()
{
	return OsNetLimit_initBytes(0);
}

void OsNetLimit_free(OsNetLimit* self)
{
	memset(self, 0, sizeof(OsNetLimit));
}

void OsNetLimit_reset(OsNetLimit* self)
{
	self->act_bytes = 0;
	self->reset_time = Os_time();
}

BOOL OsNetLimit_isActive(OsNetLimit* self)
{
	return self->max_bytes > 0;
}

void OsNetLimit_addBytes(OsNetLimit* self, const int bytes)
{
	self->act_bytes += bytes;
}

void OsNetLimit_limit(OsNetLimit* self)
{
	const double dt = Os_time() - self->reset_time;

	if (dt >= 1)
	{
		OsNetLimit_reset(self);
	}

	if (OsNetLimit_isActive(self) && self->act_bytes > self->max_bytes)
	{
		OsThread_sleep((int)((1.0 - dt) * 1000));
		OsNetLimit_reset(self);
	}
}

OsHTTPS OsHTTPS_initEmpty(void)
{
	OsHTTPS self;
	self.server_addr = 0;
	self.m_curl = 0;
	return self;
}
void OsHTTPS_initGlobal(void)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
}
void OsHTTPS_freeGlobal(void)
{
	curl_global_cleanup();
}

BOOL OsHTTPS_init(OsHTTPS* self, const char* server_adrr)
{
	BOOL ok = TRUE;
	self->server_addr = 0;

	self->m_curl = curl_easy_init();
	ok = (self->m_curl != 0);

	if (ok)
	{
		self->server_addr = malloc(Std_sizeCHAR(server_adrr) + 1);
		strcpy(self->server_addr, server_adrr);
	}
	return ok;
}

void OsHTTPS_free(OsHTTPS* self)
{
	curl_easy_cleanup(self->m_curl);
	self->m_curl = 0;
	free(self->server_addr);
}

typedef struct OsHTTPSData_s
{
	char* data;
	UBIG size;

	volatile float* out_done;
	volatile BOOL* running;
	UBIG finalSize;
} OsHTTPSData;
BOOL OsHTTPSData_isRunning(const OsHTTPSData* self)
{
	if (self->running)
		return *self->running;
	else
		return StdProgress_is();
}
static UBIG _OsHTTPS_write(void* contents, size_t size, size_t nmemb, void* userp)
{
	OsHTTPSData* output = (OsHTTPSData*)userp;

	UBIG addSize = 0;
	if (OsHTTPSData_isRunning(output))
	{
		addSize = size * nmemb;
		output->data = realloc(output->data, output->size + addSize + 1);
		memcpy(&output->data[output->size], contents, addSize);
		output->size += addSize;

		output->data[output->size] = 0;	//only to be sure(to_UNI() or print())

		float done = output->size / (float)output->finalSize;
		if (output->out_done)
			*output->out_done = done;
		else
			StdProgress_set("DOWNLOADING", done);
	}

	return addSize;
}

static void _OsHTTPS_resetAndSetUrl(OsHTTPS* self, const char* url)
{
	curl_easy_reset(self->m_curl);
	curl_easy_setopt(self->m_curl, CURLOPT_USERAGENT, NET_USER_AGENT);
	curl_easy_setopt(self->m_curl, CURLOPT_URL, url);
}

BIG OsHTTPS_get(OsHTTPS* self, const char* sub_addr, char** output)
{
	BIG size = -1;
	*output = 0;

	char* path = malloc(Std_sizeCHAR(self->server_addr) + Std_sizeCHAR(sub_addr) + 1);
	strcpy(path, self->server_addr);
	strcat(path, sub_addr);

	_OsHTTPS_resetAndSetUrl(self, path);
	curl_easy_setopt(self->m_curl, CURLOPT_WRITEFUNCTION, _OsHTTPS_write);

	OsHTTPSData st;
	st.data = 0;
	st.size = 0;
	st.out_done = 0;
	st.running = 0;
	st.finalSize = 1;	//no divide 0
	curl_easy_setopt(self->m_curl, CURLOPT_WRITEDATA, (void*)&st);
	if (curl_easy_perform(self->m_curl) == CURLE_OK)
	{
		*output = st.data;
		size = st.size;
	}
	else
	{
		free(st.data);
	}
	free(path);

	return size;
}

BIG OsHTTPS_getSize(OsHTTPS* self, const char* sub_addr)
{
	char* path = malloc(Std_sizeCHAR(self->server_addr) + Std_sizeCHAR(sub_addr) + 1);
	strcpy(path, self->server_addr);
	strcat(path, sub_addr);

	_OsHTTPS_resetAndSetUrl(self, path);
	curl_easy_setopt(self->m_curl, CURLOPT_NOBODY, 1L);	//don't download body(file)

	CURLcode err = curl_easy_perform(self->m_curl);

	free(path);
	if (err == CURLE_OK)
	{
		double filesize = 0;
		CURLcode res = curl_easy_getinfo(self->m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
		return (res == CURLE_OK) ? (BIG)filesize : -1;
	}

	return -2;
}

BIG OsHTTPS_downloadWithStatus(const char* path, volatile float* out_done, volatile BOOL* running, char** output)
{
	BIG size = -2;

	OsHTTPS self;
	if (OsHTTPS_init(&self, path))
	{
		size = OsHTTPS_getSize(&self, "");
		//if(size > 0)
		{
			_OsHTTPS_resetAndSetUrl(&self, path);
			curl_easy_setopt(self.m_curl, CURLOPT_WRITEFUNCTION, _OsHTTPS_write);

			OsHTTPSData st;
			st.data = 0;
			st.size = 0;
			st.out_done = out_done;
			st.running = running;
			st.finalSize = Std_abs(size);
			curl_easy_setopt(self.m_curl, CURLOPT_WRITEDATA, (void*)&st);

			CURLcode err = curl_easy_perform(self.m_curl);
			if (err == CURLE_OK && OsHTTPSData_isRunning(&st))
			{
				*output = st.data;
				size = st.size;
			}
			else
			{
				free(st.data);
				size = -2;
			}
		}

		OsHTTPS_free(&self);
	}

	return size;
}
