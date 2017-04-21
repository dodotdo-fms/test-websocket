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
	char	tmp1[5120];
	char*	tmp2[2];

	switch (reason) {
	case LWS_CALLBACK_CLOSED:
		fprintf(stderr, "LWS_CALLBACK_CLOSED on %p\n", (void *)wsi);
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			lws_callback_on_writable(wsi);

			char*d1	= "{\"userid\":\"root\",\"passwd\":\"1234\"}";
			int len	= _pv1_enc(tmp1, "/login", d1, strlen(d1));
			printf("SEND   %d, %s\n", len, tmp1);
			lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);
		}
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		_pv1_dec(tmp2, in, len);
		printf("RECVD  event=%s data=%s\n", tmp2[0], tmp2[1]);

		if (strcmp(tmp2[0], "/logind") == 0)
		{
			char dat[]	= {1,2,3,4,5};
			int	len	= _pv1_enc(tmp1, "/test1", (char*)dat, sizeof(dat));
			lws_write(wsi, (unsigned char*)tmp1, len, LWS_WRITE_TEXT);
		}
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		break;

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
