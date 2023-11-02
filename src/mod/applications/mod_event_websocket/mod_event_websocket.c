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
SWITCH_MODULE_RUNTIME_FUNCTION(mod_event_websocket_runtime)
{
	while(looping)
	{
		switch_cond_next();
	}
	return SWITCH_STATUS_TERM;
}
*/

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
