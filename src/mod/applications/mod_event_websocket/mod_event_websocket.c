/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2014, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Anthony Minessale II <anthm@freeswitch.org>
 * Neal Horman <neal at wanlink dot com>
 *
 *
 * mod_event_websocket.c -- Framework Demo Module
 *
 */
#include <libwebsockets.h>
#include <switch.h>

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_event_websocket_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_event_websocket_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_event_websocket_load);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime)
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_event_websocket, mod_event_websocket_load, mod_event_websocket_shutdown, NULL);

// #define _switch_stun_packet_next_attribute(attribute, end) (attribute && (attribute = (switch_stun_packet_attribute_t
// *) (attribute->value +  _switch_stun_attribute_padded_length(attribute))) && ((void *)attribute < end) && ((void
// *)(attribute +  _switch_stun_attribute_padded_length(attribute)) < end))
SWITCH_STANDARD_API(websocket_function) { return SWITCH_STATUS_SUCCESS; }

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
	case LWS_CALLBACK_HTTP:
		lws_serve_http_file(wsi, "example.html", "text/html", NULL, 0);
		break;
	default:
		break;
	}

	return 0;
}

#define EXAMPLE_RX_BUFFER_BYTES (10)
struct payload {
	unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + EXAMPLE_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
	size_t len;
} received_payload;

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
	case LWS_CALLBACK_HTTP:
		lws_serve_http_file(wsi, "example.html", "text/html", NULL, 0);
		break;
	default:
		break;
	}

	return 0;
}
static int callback_example(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
	case LWS_CALLBACK_RECEIVE:
		memcpy(&received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], in, len);
		received_payload.len = len;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Len send Data [%d]", len);

		lws_callback_on_writable_all_protocol(lws_get_context(wsi), lws_get_protocol(wsi));
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		lws_write(wsi, &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], received_payload.len, LWS_WRITE_TEXT);
		break;

	default:
		break;
	}

	return 0;
}

enum protocols { PROTOCOL_HTTP = 0, PROTOCOL_EXAMPLE, PROTOCOL_COUNT };

static struct lws_protocols protocols[] = {
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,			   /* No per session data. */
		0,			   /* max frame size / rx buffer */
	},
	{
		"example-protocol",
		callback_example,
		0,
		EXAMPLE_RX_BUFFER_BYTES,
	},
	{NULL, NULL, 0, 0} /* terminator */
};

SWITCH_MODULE_LOAD_FUNCTION(mod_event_websocket_load)
{
	switch_api_interface_t *api_interface;
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Hello World Kiennt!\n");
	SWITCH_ADD_API(api_interface, "websocket", "Websocket API", websocket_function, "syntax");

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_event_websocket_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_event_websocket_shutdown)
{
	/* Cleanup dynamically allocated config settings */
	return SWITCH_STATUS_SUCCESS;
}

/*
  If it exists, this is called in it's own thread when the module-load completes
  If it returns anything but SWITCH_STATUS_TERM it will be called again automatically
  Macro expands to: switch_status_t mod_event_websocket_runtime()
*/
SWITCH_MODULE_RUNTIME_FUNCTION(mod_event_websocket_runtime)
{
	// while (looping) { switch_cond_next(); }
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	struct lws_context *context = lws_create_context(&info);

	while (1) { lws_service(context, /* timeout_ms = */ 1000000); }

	lws_context_destroy(context);
	return SWITCH_STATUS_TERM;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet
 */
