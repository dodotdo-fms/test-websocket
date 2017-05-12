#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "libwebsockets.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>


static struct lws*	wsi;
static 	char	tmp1[5120];
static	char*	tmp2[2];
static char   namespace_buf[512];

static int send_count = 0;

int _pv1_enc(char* out, const char* event, const char* data, size_t len)
{
	char*	p1	= out;
	strcpy(p1, event);	p1	+= strlen(event);
	strcat(p1, " ");	p1	++;
	memcpy(p1, data, len);
	return	strlen(event) +1+ len;
}
int _pv1_dec(char** out, char* data, size_t len)
{
	char*	p1	= strchr(data, ' ');
	if (p1 == NULL)
		return	-1;

	*p1		= 0;
	out[0]	= data;
	out[1]	= p1+1;
	return	0;
}

static int
dodotdo_distro_v1(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	switch (reason) {
	case LWS_CALLBACK_CLOSED:
		fprintf(stderr, "LWS_CALLBACK_CLOSED on %p\n", (void *)wsi);
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			lws_callback_on_writable(wsi);

			char*d1	= "{\"userid\":\"staff2\",\"passwd\":\"1111\"}";
			int len	= _pv1_enc(tmp1, "/login", d1, strlen(d1));
			printf("SEND   %d, %s\n", len, tmp1);
			lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);
		}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		_pv1_dec(tmp2, in, len);
		printf("RECVD  event=%s data=%s\n", tmp2[0], tmp2[1]);

		snprintf(namespace_buf, 511, "%s", in);
		printf("name space is [%s]\n", namespace_buf);
		lws_callback_on_writable(wsi);

		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		printf("LWS_CALLBACK_CLIENT_WRITEABLE: namespace [%s]\n", namespace_buf);
		if (strncmp(namespace_buf, "/channel/listd", 14) == 0)
		{
			char*d2	= "{\"id\":2}";
			lws_callback_on_writable(wsi);

			int	len	= _pv1_enc(tmp1, "/channel/join", d2, sizeof(d2));
			tmp1[len] = '\0';
			printf("SEND  %d, %s\n",len, tmp1);
			snprintf(namespace_buf, 511, "/channel/join");
			lws_callback_on_writable(wsi);

			lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);
		}
#if 0
		else if (strncmp(namespace_buf, "/channel/joind", 14) == 0)
		{
			char*d3	= "{\"channelId\":2}";
			lws_callback_on_writable(wsi);

			int	len	= _pv1_enc(tmp1, "/stream/lock", d3, sizeof(d3)+7);
			tmp1[len] = '\0';
			printf("SEND  %d, %s\n",len, tmp1);
			snprintf(namespace_buf, 511, "/stream/lock");
			lws_callback_on_writable(wsi);

			lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);

		}
#endif
#if 0
		else if (strncmp(namespace_buf, "/stream/lockd", 14) == 0 || strncmp(namespace_buf, "/stream/data", 12) == 0)
		{
			if (send_count < 20)
			{
				lws_callback_on_writable(wsi);

				char*data3	= "gjygjyggygjhgj";

				int	len	= _pv1_enc(tmp1, "/stream/data", data3, strlen(data3));
				tmp1[len] = '\0';
				printf("SEND  %d, %s\n",len, tmp1);
				snprintf(namespace_buf, 511, "/stream/data");
				lws_callback_on_writable(wsi);

				lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);
				send_count++;

			}
			else
			{
				send_count = 0;
				lws_callback_on_writable(wsi);

				int	len	= _pv1_enc(tmp1, "/stream/release", NULL, 0);
				tmp1[len] = '\0';
				printf("SEND  %d, %s\n",len, tmp1);
				snprintf(namespace_buf, 511, "/stream/release");
				lws_callback_on_writable(wsi);

				lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);
			}
		}

		break;
#endif

	default:
		break;
	}

	return 0;
}


/* list of supported protocols and callbacks */
static struct lws_protocols protocols[] = {

	{
		"dodotdo-distro-v1",
		dodotdo_distro_v1,
		5120,
	},
	{
		NULL, NULL, 0/* end of list */
	}
};

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_no_context_takeover; client_max_window_bits"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL }
};


int main(int argc, char **argv)
{
	char	ads_port[128];
	struct	lws_context *context;

	int		ietf_version = -1;
	struct	lws_context_creation_info info	= {0};
	struct	lws_client_connect_info i		= {0};


	info.port		= CONTEXT_PORT_NO_LISTEN;
	info.protocols	= protocols;
	info.extensions	= exts;
	info.gid		= -1;
	info.uid		= -1;

	context = lws_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "Creating libwebsocket context failed\n");
		return 1;
	}

	i.context	= context;
	i.address	= "52.183.80.183";
	i.port		= 80;

	lws_snprintf(ads_port, sizeof(ads_port), "%s:%u", i.address, i.port & 65535);
	lwsl_notice("Connecting to ...\n");
	i.path		= "/distro";
	i.host		= ads_port;
	i.origin	= ads_port;
	i.protocol	= protocols[0].name;
	i.ssl_connection	= 0;
	i.ietf_version_or_minus_one = ietf_version;

	wsi	= lws_client_connect_via_info(&i);

	/* service loop */

	int	n = 0;
	while (n >= 0)
	{
		n = lws_service(context, 10);
	}


	lws_context_destroy(context);

	return 0;
}
