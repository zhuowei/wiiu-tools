#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEBUG_PRINT printf
#define HANDLER_BUFSIZE 0x10000

struct derpybuf {
	size_t size;
	char thebuf[HANDLER_BUFSIZE];
};

static int readInt(char** buf) {
	int retval = htonl(*((uint32_t*) *buf));
	*buf += 4;
	return retval;
}

static char readChar(char** buf) {
	char ret = **buf;
	*buf += 1;
	return ret;
}

static char* readStr(char** buf) {
	char* ret = *buf;
	int len = strlen(ret);
	*buf += (len + 1);
	return ret;
}

static void platform_curl_setopt(CURL* curl) {
}

static ssize_t writeToBuf_handler(void* ptr, size_t size, size_t nmemb, struct derpybuf* derpybuf) {
	DEBUG_PRINT("%s\n", ptr);
	size_t totsize = size * nmemb;
	size_t newoff = derpybuf->size + totsize;
	if (newoff > sizeof(derpybuf->thebuf)) {
		newoff = sizeof(derpybuf->thebuf);
		totsize = newoff - derpybuf->size;
		if (!totsize) return 0;
	}
	DEBUG_PRINT("%x %x\n", derpybuf->size, totsize);
	memcpy(&derpybuf->thebuf[derpybuf->size], ptr, totsize);
	derpybuf->size = newoff;
	return totsize;
}

static void handleBuf(struct derpybuf* derpybuf, struct derpybuf* outbuf) {
	char* b = derpybuf->thebuf;
	int i;
	for (i = 0; i < HANDLER_BUFSIZE; i++) {
		if (b[i] == '\n') b[i] = '\0';
	}
	char method = readChar(&b);
	char* url = readStr(&b);
	char* data = method == 'p'? readStr(&b) : NULL;
	DEBUG_PRINT("%c %s\n", method, url);
	memset(outbuf, 0, sizeof(*outbuf));

	// actual curl stuff starts here
	CURL* curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		if (method == 'p') {
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		}
		//curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeToBuf_handler);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToBuf_handler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, outbuf);
		platform_curl_setopt(curl);
		curl_easy_perform(curl);
		DEBUG_PRINT("Performed\n");
		curl_easy_cleanup(curl);
	}
	DEBUG_PRINT("Done\n");
}

static void handleOne(int sock) {
	DEBUG_PRINT("Connected %d\n", sock);
	struct derpybuf derpybuf;
	struct derpybuf derpybuf2;
	char* buf = derpybuf.thebuf;
	memset(buf, 0, HANDLER_BUFSIZE);
	ssize_t len;
	while ((len = read(sock, buf, HANDLER_BUFSIZE)) != 0) {
		//printf("%s", buf);
		handleBuf(&derpybuf, &derpybuf2);
		DEBUG_PRINT("%s", derpybuf2.thebuf);
		write(sock, derpybuf2.thebuf, derpybuf2.size);
		break;
		//memset(buf, 0, HANDLER_BUFSIZE);
	}
	close(sock);
}

static void handleLoop(int port) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = 0;
	int listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int yep = 1;
	setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &yep, sizeof(yep));
	bind(listensock, (struct sockaddr*) &addr, sizeof(addr));
	listen(listensock, 0x1000);
	while (true) {
		int accepted = accept(listensock, NULL, NULL);
		handleOne(accepted);
	}
}

int main() {
	handleLoop(12333);
	return 0;
}
