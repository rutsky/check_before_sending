/*
 * Piding plugin forcing user to recheck message containing spell check 
 * errors before sending it.
 *
 * Copyright (C) 2012  Vladimir Rutsky <altsysrq@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02111-1301  USA.
 */

/* vim: set ts=2 sw=2 et: */

#include <pidgin.h>
#include <libpurple/version.h>
#include <libpurple/purple.h>
#include <libpurple/account.h>
#include <pidgin/gtkplugin.h>

PurplePlugin *plugin_handle = NULL;

static gboolean 
outgoing_msg_cb(PurpleAccount *account, const char *who, char **message,
                PurpleConversation *conv, PurpleMessageFlags flags, 
                gpointer null)
{
  printf("Trying to send: '%s'", *message);
  
  // TODO
  
  return TRUE; // TODO
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
  void *handle = purple_conversations_get_handle();
  plugin_handle = plugin;
  purple_signal_connect(handle, "sending-im-msg", plugin,
      PURPLE_CALLBACK(outgoing_msg_cb), NULL);

  return TRUE;
}


static PurplePluginInfo info =
{
  PURPLE_PLUGIN_MAGIC,
  PURPLE_MAJOR_VERSION,
  PURPLE_MINOR_VERSION,
  PURPLE_PLUGIN_STANDARD,
  NULL,
  0,
  NULL,
  PURPLE_PRIORITY_DEFAULT,
  "check_before_sending",
  "Check Before Sending",
  "0.1",
  "Check Spelling Before Sending Plugin",
  "Piding plugin forcing user to recheck message containing spell "
    "check errors before sending it.",
  "Vladimir Rutsky <altsysrq@gmail.com>",
  "http://github.com/rutsky/check_before_sending/",
  plugin_load,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  /* padding */
  NULL,
  NULL,
  NULL,
  NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(check_before_sending, init_plugin, info)
