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

// TODO: internationalization

#include <pidgin.h>
#include <libpurple/version.h>
#include <libpurple/purple.h>
#include <libpurple/account.h>
#include <pidgin/gtkplugin.h>
#include <pidgin/gtkconv.h>

#define PLUGIN_ID "gtk-rutsky-check_before_sending"

#define WATCHER_OBJECT_KEY "check_before_sending"

/* Tag that marks misspelled words in GtkSpell */
/* HACK: This is from GtkSpell internals, not available in public interface. */
#define GTK_SPELL_MISSPELLED_TAG "gtkspell-misspelled"

PurplePlugin *plugin_handle = NULL;

struct _watcher {
  GtkTextView *view;
  gint last_misspelled; /* index of first letter of last misspelled word,
                           -1 if not defined. */
};

typedef struct _watcher watcher;

/* Returns TRUE if tag found */
static gboolean
forward_to_tag_begin(GtkTextIter *iter, GtkTextTag *tag)
{
  if (gtk_text_iter_forward_to_tag_toggle(iter, tag))
  {
    if (!gtk_text_iter_begins_tag(iter, tag))
    {
      /* Moved to tag end. Moving further */
      return gtk_text_iter_forward_to_tag_toggle(iter, tag);
    }
    else
      return TRUE;
  }
  else
    return FALSE;
}

static void
message_send_cb(GtkWidget *widget, watcher *wtchr)
{
  GtkTextBuffer *buffer;
  GtkTextIter iter, start;
  GtkTextTagTable *tagtable;
  GtkTextTag *misspelled_tag;

  buffer = gtk_text_view_get_buffer(wtchr->view);
  tagtable = gtk_text_buffer_get_tag_table(buffer);
  misspelled_tag = gtk_text_tag_table_lookup(tagtable, GTK_SPELL_MISSPELLED_TAG);

  if (wtchr->last_misspelled >= 0)
  {
    /* Move text cursor to next misspelled word, and if this is last
     * word - send message */

    gtk_text_buffer_get_iter_at_offset(buffer, &iter, wtchr->last_misspelled);

    if (forward_to_tag_begin(&iter, misspelled_tag))
    {
      /* Cursor moved to next misspelled word - stop message sending */

      wtchr->last_misspelled = gtk_text_iter_get_offset(&iter);
      gtk_text_buffer_place_cursor(buffer, &iter);

      g_signal_stop_emission_by_name(widget, "message_send");
    }
    else
    {
      /* This was last misspelled word and it was ignored by user - send message */
    }
  }
  else
  {
    gtk_text_buffer_get_start_iter(buffer, &start);

    /* Force spell checking for current word by navigating to begin of line */
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
    gtk_text_buffer_place_cursor(buffer, &start);
    gtk_text_buffer_place_cursor(buffer, &iter);

    if (gtk_text_iter_begins_tag(&start, misspelled_tag) ||
        gtk_text_iter_forward_to_tag_toggle(&start, misspelled_tag))
    {
      /* Misspelled words found.
       * Move cursor to first misspelled word and stop message sending */

      wtchr->last_misspelled = gtk_text_iter_get_offset(&start);
      gtk_text_buffer_place_cursor(buffer, &start);

      g_signal_stop_emission_by_name(widget, "message_send");
    }
    else
    {
      /* Text is ok, send it */
    }
  }
}

static
void
on_buffer_changed(GtkTextBuffer *buffer, watcher *wtchr)
{
  wtchr->last_misspelled = -1;
}

static void
watcher_free(watcher *wtchr)
{
  GtkTextBuffer *buffer;

  g_return_if_fail(wtchr != NULL);

  buffer = gtk_text_view_get_buffer(wtchr->view);

  g_signal_handlers_disconnect_matched(buffer,
      G_SIGNAL_MATCH_DATA,
      0, 0, NULL, NULL,
      wtchr);

  g_free(wtchr);
}

static void
attach_to_conversation(PurpleConversation *conv)
{
  watcher *wtchr;
  PidginConversation *gtkconv;
  GtkTextView *view;
  GtkTextBuffer *buffer;

  gtkconv = PIDGIN_CONVERSATION(conv);

  view = GTK_TEXT_VIEW(gtkconv->entry);

  wtchr = g_object_get_data(G_OBJECT(view), WATCHER_OBJECT_KEY);
  if (wtchr != NULL)
    return;

  /* attach to the widget */
  wtchr = g_new0(watcher, 1);
  wtchr->view = view;
  wtchr->last_misspelled = -1;

  g_object_set_data_full(G_OBJECT(view), WATCHER_OBJECT_KEY, wtchr,
      (GDestroyNotify)watcher_free);

  buffer = gtk_text_view_get_buffer(wtchr->view);

  g_signal_connect_after(G_OBJECT(buffer),
      "changed",
      G_CALLBACK(on_buffer_changed), wtchr);

  g_signal_connect(G_OBJECT(gtkconv->entry), "message_send",
                   G_CALLBACK(message_send_cb), wtchr);
  return;
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
  void *conv_handle = purple_conversations_get_handle();
  GList *convs;

  plugin_handle = plugin;

  /* Attach to existing conversations */
  for (convs = purple_get_conversations(); convs != NULL; convs = convs->next)
  {
    attach_to_conversation((PurpleConversation *)convs->data);
  }

  purple_signal_connect(conv_handle, "conversation-created",
      plugin, PURPLE_CALLBACK(attach_to_conversation), NULL);

  return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
  GList *convs;

  /* Detach from existing conversations */
  for (convs = purple_get_conversations(); convs != NULL; convs = convs->next)
  {
      PidginConversation *gtkconv = PIDGIN_CONVERSATION((PurpleConversation *)convs->data);
      watcher *wtchr = g_object_get_data(G_OBJECT(gtkconv->entry), WATCHER_OBJECT_KEY);

      g_signal_handlers_disconnect_by_func(gtkconv->entry, message_send_cb, wtchr);
      g_object_set_data(G_OBJECT(gtkconv->entry), WATCHER_OBJECT_KEY, NULL);
  }

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
  PLUGIN_ID,
  "Check Before Sending",
  "0.1",
  "Check Spelling Before Sending Plugin",
  "Piding plugin forcing user to recheck message containing spell "
    "check errors before sending it.",
  "Vladimir Rutsky <altsysrq@gmail.com>",
  "http://github.com/rutsky/check_before_sending/",
  plugin_load,
  plugin_unload,
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
