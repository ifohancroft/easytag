/* musicbrainz_dialog.h - 2014/05/05 */
/*
 *  EasyTAG - Tag editor for MP3 and Ogg Vorbis files
 *  Copyright (C) 2000-2014  Abhinav Jangda <abhijangda@hotmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#ifndef __MUSICBRAINZ_DIALOG_H__
#define __MUSICBRAINZ_DIALOG_H__

#include "config.h"

#ifdef ENABLE_libmusicbrainz

G_BEGIN_DECLS

#define ET_MUSICBRAINZ_DIALOG_TYPE (et_musicbrainz_dialog_get_type ())
#define ET_MUSICBRAINZ_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                                                ET_MUSICBRAINZ_DIALOG_TYPE, \
                                                                EtMusicBrainzDialog))

#define ET_MUSICBRAINZ_DIALOG_CLASS(klass) (G_TYPE_CHECK_INSTANCE_CAST ((klass), \
                                                                        ET_MUSICBRAINZ_DIALOG_TYPE, \
                                                                        EtMusicBrainzDialog))

#define IS_ET_MUSICBRAINZ_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                                                   ET_MUSICBRAINZ_DIALOG_TYPE))

#define IS_ET_MUSICBRAINZ_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                                                        ET_MUSICBRAINZ_DIALOG_TYPE))
/****************
 * Declarations *
 ****************/

typedef struct _EtMusicBrainzDialogPrivate EtMusicBrainzDialogPrivate;

/*
 * EtMbEntityView:
 * @vbox: GtkBox, parent class of EtMbEntityView
 *
 * This widget is used to show data recieved from music brainz and helps to
 * navigate in it using breadcrumb widget.
 */
typedef struct
{
    GtkDialog dialog;
    EtMusicBrainzDialogPrivate *priv;
} EtMusicBrainzDialog;

/*
 * EtMbEntityViewClass:
 * @parent: GtkBoxClass, parent class of EtMbEntityViewClass
 *
 * Class of EtMbEntityView.
 */
typedef struct
{
    GtkDialogClass parent;
} EtMusicBrainzDialogClass;


/**************
 * Prototypes *
 **************/

GType
et_musicbrainz_dialog_get_type (void);
GtkWidget *
et_musicbrainz_dialog_new (void);
void
et_open_musicbrainz_dialog (void);
void
mb5_search_error_callback (GObject *source, GAsyncResult *res,
                           gpointer user_data);
void
et_show_status_msg_in_idle (gchar *message);
void
et_music_brainz_dialog_stop_set_sensitive (gboolean sensitive);
void
et_music_brainz_dialog_destroy (GtkWidget *widget);
gboolean
et_music_brainz_get_exit_on_complete (void);
void
et_music_brainz_dialog_set_statusbar_message (gchar *message);
void
et_music_brainz_dialog_set_response (GtkResponseType response);
G_END_DECLS
#endif /* __MUSICBRAINZ_DIALOG_H__ */
#endif /* ENABLE_libmusicbrainz */