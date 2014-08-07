/* mb_search.c - 2014/05/05 */
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

#include "config.h"

#ifdef ENABLE_libmusicbrainz

#include <glib/gi18n.h>

#include "mb_search.h"
#include "musicbrainz_dialog.h"

/****************
 * Declarations *
 ****************/

static gchar *server = NULL;
static int port = 0;

#define USER_AGENT PACKAGE_NAME"/"PACKAGE_VERSION" ( "PACKAGE_URL" ) "
#define CHECK_CANCELLED(cancellable) if (g_cancellable_is_cancelled (cancellable))\
                                     {\
                                           g_set_error (error, ET_MB5_SEARCH_ERROR,\
                                                        ET_MB5_SEARCH_ERROR_CANCELLED,\
                                                        _("Operation cancelled by user"));\
                                           g_assert (error == NULL || *error != NULL);\
                                           goto cancel;\
                                      }

/**************
 * Prototypes *
 **************/
static void
et_musicbrainz_set_error_from_query (Mb5Query query, tQueryResult result, 
                                     GError **error);
static void
free_mb_node_children (GNode *node);

/*************
 * Functions *
 *************/

/*
 * et_mb5_search_error_quark:
 *
 * To get EtMB5SearchError domain.
 *
 * Returns: GQuark for EtMB5SearchError domain
 */
GQuark
et_mb5_search_error_quark (void)
{
    return g_quark_from_static_string ("et-mb5-search-error-quark");
}

/*
 * et_mb5_recording_get_artists_names:
 * @recording: Mb5Recording
 *
 * Returns: String containing all the artist names of recording
 * Get all the artist names of recording.
 */
gchar *
et_mb5_recording_get_artists_names (Mb5Recording recording)
{
    GString *artist;
    Mb5ArtistCredit artist_credit;

    artist_credit = mb5_recording_get_artistcredit (recording);
    artist = g_string_new ("");

    if (artist_credit)
    {
        Mb5NameCreditList name_list;
        int i;
        int size;
        gchar title[NAME_MAX_SIZE];

        name_list = mb5_artistcredit_get_namecreditlist (artist_credit);

        for (i = 0; i < mb5_namecredit_list_size (name_list); i++)
        {
            Mb5NameCredit name_credit;
            Mb5Artist name_credit_artist;

            name_credit = mb5_namecredit_list_item (name_list, i);
            name_credit_artist = mb5_namecredit_get_artist (name_credit);
            size = mb5_artist_get_name (name_credit_artist, title,
                                        sizeof (title));
            g_string_append_len (artist, title, size);

            if (i + 1 < mb5_namecredit_list_size (name_list))
            {
                g_string_append_len (artist, ", ", 2);
            }
        }
    }

    return g_string_free (artist, FALSE);
}

/*
 * et_mb5_release_get_artists_names:
 * @release: Mb5Release
 *
 * Returns: String containing all artist names of release
 * Get all the artist names of recording.
 */
gchar *
et_mb5_release_get_artists_names (Mb5Release release)
{
    GString *album_artist;
    Mb5ArtistCredit artist_credit;

    album_artist = g_string_new ("");
    artist_credit = mb5_release_get_artistcredit (release);

    if (artist_credit)
    {
        Mb5NameCreditList name_list;
        gchar temp[NAME_MAX_SIZE];
        int i;

        name_list = mb5_artistcredit_get_namecreditlist (artist_credit);

        for (i = 0; i < mb5_namecredit_list_size (name_list); i++)
        {
            Mb5NameCredit name_credit;
            Mb5Artist name_credit_artist;
            int size;

            name_credit = mb5_namecredit_list_item (name_list, i);
            name_credit_artist = mb5_namecredit_get_artist (name_credit);
            size = mb5_artist_get_name (name_credit_artist, temp,
                                        sizeof (temp));
            g_string_append_len (album_artist, temp, size);

            if (i + 1 < mb5_namecredit_list_size (name_list))
            {
                g_string_append_len (album_artist, ", ", 2);
            }
        }
    }

    return g_string_free (album_artist, FALSE);
}

/*
 * et_musicbrainz_search_set_server_port:
 * @_server: Address of server
 * @_port: Port
 *
 * Set a musicbrainz server address and port.
 */
void
et_musicbrainz_search_set_server_port (gchar *_server, int _port)
{
    if (server)
    {
        g_free (server);
    }

    server = g_strdup (_server);
    port = _port;
}

/*
 * et_musicbrainz_search_in_entity:
 * @child_type: Type of the children to get.
 * @parent_type: Type of the parent.
 * @parent_mbid: MBID of parent entity.
 * @root: GNode of parent.
 * @error: GError
 *
 * To retrieve children entities of a parent entity.
 *
 * Returns: TRUE if successful, FALSE if not.
 */
gboolean
et_musicbrainz_search_in_entity (MbEntityKind child_type,
                                 MbEntityKind parent_type,
                                 gchar *parent_mbid, GNode *root,
                                 GError **error, GCancellable *cancellable)
{
    Mb5Query query;
    Mb5Metadata metadata;
    Mb5Metadata metadata_entity;
    tQueryResult result;
    char *param_values[1];
    char *param_names[1];

    param_names[0] = "inc";
    query = mb5_query_new (USER_AGENT, server, port);
    metadata = NULL;
    metadata_entity = NULL;
    CHECK_CANCELLED(cancellable);

    if (child_type == MB_ENTITY_KIND_ALBUM &&
        parent_type == MB_ENTITY_KIND_ARTIST)
    {
        param_values[0] = "releases";
        metadata = mb5_query_query (query, "artist", parent_mbid, "", 1,
                                    param_names, param_values);
        result = mb5_query_get_lastresult (query);

        if (result == eQuery_Success)
        {
            if (metadata)
            {
                int i;
                Mb5ReleaseList list;
                Mb5Artist artist;
                gchar *message;

                CHECK_CANCELLED(cancellable);
                artist = mb5_metadata_get_artist (metadata);
                list = mb5_artist_get_releaselist (artist);
                param_values[0] = "artists release-groups";
                message = g_strdup_printf (ngettext (_("Found %d Album"),
                                           _("Found %d Albums"),
                                           mb5_release_list_size (list)),
                                           mb5_release_list_size (list));
#ifndef TEST
                et_show_status_msg_in_idle (message);
#endif
                g_free (message);

                for (i = 0; i < mb5_release_list_size (list); i++)
                {
                    Mb5Release release;
                    release = mb5_release_list_item (list, i);

                    if (release)
                    {
                        gchar buf[NAME_MAX_SIZE];
                        GNode *node;
                        EtMbEntity *entity;
                        int size;

                        CHECK_CANCELLED(cancellable);
                        size = mb5_release_get_title ((Mb5Release)release, buf,
                                                      sizeof (buf));
                        buf[size] = '\0';
                        message = g_strdup_printf (_("Retrieving %s (%d/%d)"),
                                                   buf, i+1,
                                                   mb5_release_list_size (list));
#ifndef TEST
                        et_show_status_msg_in_idle (message);
#endif
                        g_free (message);

                        size = mb5_release_get_id ((Mb5Release)release,
                                                   buf,
                                                   sizeof (buf));
                        buf[size] = '\0';
                        metadata_entity = mb5_query_query (query, "release",
                                                           buf, "",
                                                           1, param_names,
                                                           param_values);
                        result = mb5_query_get_lastresult (query);

                        if (result == eQuery_Success)
                        {
                            if (metadata_entity)
                            {
                                CHECK_CANCELLED(cancellable);
                                entity = g_slice_new (EtMbEntity);
                                entity->entity = mb5_release_clone (mb5_metadata_get_release (metadata_entity));
                                entity->type = MB_ENTITY_KIND_ALBUM;
                                entity->is_red_line = FALSE;
                                node = g_node_new (entity);
                                g_node_append (root, node);
                                mb5_metadata_delete (metadata_entity);
                            }
                        }
                        else
                        {
                            free_mb_node_children (root);
                            goto err;
                        }
                    }
                }
            }

            mb5_metadata_delete (metadata);
        }
        else
        {
            goto err;
        }
    }
    else if (child_type == MB_ENTITY_KIND_TRACK &&
             parent_type == MB_ENTITY_KIND_ALBUM)
    {
        param_values[0] = "recordings";
        CHECK_CANCELLED(cancellable);
        metadata = mb5_query_query (query, "release", parent_mbid, "", 1,
                                    param_names, param_values);
        CHECK_CANCELLED(cancellable);
        result = mb5_query_get_lastresult (query);

        if (result == eQuery_Success)
        {
            if (metadata)
            {
                int i;
                Mb5MediumList list;
                Mb5Release release;
                release = mb5_metadata_get_release (metadata);
                list = mb5_release_get_mediumlist (release);
                param_values[0] = "releases artists artist-credits release-groups";

                for (i = 0; i < mb5_medium_list_size (list); i++)
                {
                    Mb5Medium medium;
                    medium = mb5_medium_list_item (list, i);

                    if (medium)
                    {
                        gchar buf[NAME_MAX_SIZE];
                        GNode *node;
                        EtMbEntity *entity;
                        Mb5TrackList track_list;
                        int j;
                        int size;
                        gchar *message;

                        track_list = mb5_medium_get_tracklist (medium);
                        message = g_strdup_printf (ngettext (_("Found %d Track"),
                                                   _("Found %d Tracks"),
                                                   mb5_track_list_size (list)),
                                                   mb5_track_list_size (list));
#ifndef TEST
                        et_show_status_msg_in_idle (message);
#endif
                        g_free (message);

                        for (j = 0; j < mb5_track_list_size (track_list); j++)
                        {
                            Mb5Recording recording;


                            CHECK_CANCELLED(cancellable);
                            recording = mb5_track_get_recording (mb5_track_list_item (track_list, j));
                            size = mb5_recording_get_title (recording, buf,
                                                            sizeof (buf));
                            buf[size] = '\0';
                            message = g_strdup_printf (_("Retrieving %s (%d/%d)"),
                                                       buf, j,
                                                       mb5_track_list_size (track_list));
#ifndef TEST
                            et_show_status_msg_in_idle (message);
#endif
                            g_free (message);

                            size = mb5_recording_get_id (recording,
                                                         buf,
                                                         sizeof (buf));
                            CHECK_CANCELLED(cancellable);
                            metadata_entity = mb5_query_query (query,
                                                               "recording",
                                                               buf, "", 1,
                                                               param_names,
                                                               param_values);
                            result = mb5_query_get_lastresult (query);

                            if (result == eQuery_Success)
                            {
                                if (metadata_entity)
                                {
                                    CHECK_CANCELLED(cancellable);
                                    entity = g_slice_new (EtMbEntity);
                                    entity->entity = mb5_recording_clone (mb5_metadata_get_recording (metadata_entity));
                                    entity->type = MB_ENTITY_KIND_TRACK;
                                    entity->is_red_line = FALSE;
                                    node = g_node_new (entity);
                                    g_node_append (root, node);
                                    mb5_metadata_delete (metadata_entity);
                                }
                            }
                            else
                            {
                                free_mb_node_children (root);
                                goto err;
                            }
                        }
                    }
                }
            }

            mb5_metadata_delete (metadata);
        }
        else
        {
            goto err;
        }
    }
    else if (child_type == MB_ENTITY_KIND_ALBUM &&
             parent_type == MB_ENTITY_KIND_FREEDBID)
    {
        mb5_query_delete (query);

        return et_musicbrainz_search (parent_mbid, child_type, root, 0, error,
                                      cancellable);
    }

    CHECK_CANCELLED(cancellable);
    mb5_query_delete (query);

    return TRUE;

    err:
    
    if (metadata_entity)
    {
        mb5_metadata_delete (metadata_entity);
    }

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    et_musicbrainz_set_error_from_query (query, result, error);
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);
    
    return FALSE;

    cancel:

    if (metadata_entity)
    {
        mb5_metadata_delete (metadata_entity);
    }

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
}

/*
 * et_musicbrainz_set_error_from_query:
 * @query: Query
 * @result: Result of the Query
 * @error: GError to set
 *
 * Sets GError on the basis of tQueryResult.
 */
static void
et_musicbrainz_set_error_from_query (Mb5Query query, tQueryResult result, 
                                     GError **error)
{
    char error_message[256];

    g_return_if_fail (error == NULL || *error == NULL);
    mb5_query_get_lasterrormessage (query, error_message,
                                    sizeof(error_message));

    switch (result)
    {
        case eQuery_ConnectionError:
            g_set_error (error, ET_MB5_SEARCH_ERROR,
                         ET_MB5_SEARCH_ERROR_CONNECTION, error_message);
            break;

        case eQuery_Timeout:
            g_set_error (error, ET_MB5_SEARCH_ERROR,
                         ET_MB5_SEARCH_ERROR_TIMEOUT, error_message);
            break;

        case eQuery_AuthenticationError:
            g_set_error (error, ET_MB5_SEARCH_ERROR,
                         ET_MB5_SEARCH_ERROR_AUTHENTICATION, error_message);
            break;

        case eQuery_FetchError:
            g_set_error (error, ET_MB5_SEARCH_ERROR,
                         ET_MB5_SEARCH_ERROR_FETCH, error_message);
            break;
 
        case eQuery_RequestError:
            g_set_error (error, ET_MB5_SEARCH_ERROR,
                         ET_MB5_SEARCH_ERROR_REQUEST, error_message);
            break;
 
        case eQuery_ResourceNotFound:
            g_set_error (error, ET_MB5_SEARCH_ERROR,
                         ET_MB5_SEARCH_ERROR_RESOURCE_NOT_FOUND,
                         error_message);
            break;

        default:
            break;
    }

    g_assert (error == NULL || *error != NULL);
}

static void
free_mb_node_children (GNode *node)
{
    GNode *child;

    child = g_node_first_child (node);

    while (child)
    {
        GNode *child1;
        child1 = g_node_next_sibling (child);
        free_mb_tree (&child);
        child = child1;
    }
}

EtMbEntity *
et_mb_entity_copy (EtMbEntity *etentity)
{
    EtMbEntity *entity;
    
    if (!etentity)
    {
        return NULL;
    }

    entity = g_slice_new (EtMbEntity);
    entity->type = etentity->type;

    switch (entity->type)
    {
        case MB_ENTITY_KIND_ARTIST:
            entity->entity = mb5_artist_clone (etentity->entity);
            break;

        case MB_ENTITY_KIND_ALBUM:
            entity->entity = mb5_release_clone (etentity->entity);
            break;

        case MB_ENTITY_KIND_TRACK:
            entity->entity = mb5_recording_clone (etentity->entity);
            break;

        case MB_ENTITY_KIND_FREEDBID:
            entity->entity = mb5_freedbdisc_clone (etentity->entity);
            break;

        case MB_ENTITY_KIND_DISCID:
            entity->entity = mb5_disc_clone (etentity->entity);
            break;

        default:
            g_slice_free (EtMbEntity, entity);
            return NULL;
    }

    return entity;
}

/*
 * et_musicbrainz_search_artist:
 * @string: String to search
 * @root: Root Node
 * @error: GErorr
 * @cancellable: GCancellable
 *
 * Returns: TRUE if successfull otherwise FALSE
 *
 * Search for Artists with name as @string
 */
static gboolean
et_musicbrainz_search_artist (gchar *string, GNode *root, int offset, 
                              GError **error, GCancellable *cancellable)
{
    Mb5Query query;
    Mb5Metadata metadata;
    tQueryResult result;
    char *param_values[3];
    char *param_names[3];

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    param_names[0] = "query";
    param_names[1] = "limit";
    param_values[1] = SEARCH_LIMIT_STR;
    param_names[2] = "offset";
    param_values[2] = g_strdup_printf ("%d", offset);
    query = mb5_query_new (USER_AGENT, server, port);
    CHECK_CANCELLED(cancellable);
    param_values[0] = g_strconcat ("artist:", string, NULL);
    metadata = mb5_query_query (query, "artist", "", "", 3, param_names,
                                param_values);
    g_free (param_values[0]);
    g_free (param_values[2]);
    result = mb5_query_get_lastresult (query);

    if (result == eQuery_Success)
    {
        if (metadata)
        {
            int i;
            Mb5ArtistList list;
            list = mb5_metadata_get_artistlist (metadata);
 
            for (i = 0; i < mb5_artist_list_size (list); i++)
            {
                Mb5Artist artist;

                CHECK_CANCELLED(cancellable);
                artist = mb5_artist_list_item (list, i);

                if (artist)
                {
                    GNode *node;
                    EtMbEntity *entity;
                    entity = g_slice_new (EtMbEntity);
                    entity->entity = mb5_artist_clone (artist);
                    entity->type = MB_ENTITY_KIND_ARTIST;
                    entity->is_red_line = FALSE;
                    node = g_node_new (entity);
                    g_node_append (root, node);
                }
            }
        }

        mb5_metadata_delete (metadata);
    }
    else
    {
        goto err;
    }

    CHECK_CANCELLED(cancellable);
    mb5_query_delete (query);

    return TRUE;

    err:
    et_musicbrainz_set_error_from_query (query, result, error);
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
    
    cancel:
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
}

/*
 * et_musicbrainz_search_album:
 * @string: String to search
 * @root: Root Node
 * @error: GErorr
 * @cancellable: GCancellable
 *
 * Returns: TRUE if successfull otherwise FALSE
 *
 * Search for Albums with name as @string
 */
static gboolean
et_musicbrainz_search_album (gchar *string, GNode *root, int offset, 
                             GError **error, GCancellable *cancellable)
{
    Mb5Query query;
    Mb5Metadata metadata;
    Mb5Metadata metadata_release;
    tQueryResult result;
    char *param_values[3];
    char *param_names[3];

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    metadata = NULL;
    metadata_release = NULL;
    param_names[0] = "query";
    param_names[1] = "limit";
    param_names[2] = "offset";
    param_values[2] = g_strdup_printf ("%d", offset);
    param_values[1] = SEARCH_LIMIT_STR;
    query = mb5_query_new (USER_AGENT, server, port);
    param_values[0] = g_strconcat ("release:", string, NULL);
    CHECK_CANCELLED(cancellable);
    metadata = mb5_query_query (query, "release", "", "", 3, param_names,
                                param_values);
    result = mb5_query_get_lastresult (query);
    g_free (param_values[0]);
    g_free (param_values[2]);

    if (result == eQuery_Success)
    {
        if (metadata)
        {
            int i;
            Mb5ReleaseList list;
            gchar *message;

            list = mb5_metadata_get_releaselist (metadata);
            param_names[0] = "inc";
            param_values[0] = "artists release-groups";
            message = g_strdup_printf (ngettext (_("Found %d Albums"),
                                       _("Found %d Albums"), 
                                       mb5_release_list_size (list)),
                                       mb5_release_list_size (list));
#ifndef TEST
            et_show_status_msg_in_idle (message);
#endif
            g_free (message);

            for (i = 0; i < mb5_release_list_size (list); i++)
            {
                Mb5Release release;

                CHECK_CANCELLED(cancellable);
                release = mb5_release_list_item (list, i);

                if (release)
                {
                    gchar buf[NAME_MAX_SIZE];
                    GNode *node;
                    EtMbEntity *entity;
                    int size;

                    size = mb5_release_get_title ((Mb5Release)release,
                                                  buf, sizeof (buf));
                    buf[size] = '\0';
                    message = g_strdup_printf (_("Retrieving %s (%d/%d)"),
                                               buf, i,
                                               mb5_release_list_size (list));
#ifndef TEST
                    et_show_status_msg_in_idle (message);
#endif
                    g_free (message);

                    mb5_release_get_id ((Mb5Release)release,
                                        buf,
                                        sizeof (buf));
                    CHECK_CANCELLED(cancellable);
                    metadata_release = mb5_query_query (query, "release",
                                                        buf, "",
                                                        1, param_names,
                                                        param_values);
                    result = mb5_query_get_lastresult (query);

                    if (result == eQuery_Success)
                    {
                        if (metadata_release)
                        {
                            CHECK_CANCELLED(cancellable);
                            entity = g_slice_new (EtMbEntity);
                            entity->entity = mb5_release_clone (mb5_metadata_get_release (metadata_release));
                            entity->type = MB_ENTITY_KIND_ALBUM;
                            entity->is_red_line = FALSE;
                            node = g_node_new (entity);
                            g_node_append (root, node);
                            mb5_metadata_delete (metadata_release);
                        }
                    }
                    else
                    {
                        goto err;
                    }
                }
            }
        }

        mb5_metadata_delete (metadata);
    }
    else
    {
        goto err;
    }

    CHECK_CANCELLED(cancellable);
    mb5_query_delete (query);

    return TRUE;

    err:
    
    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    et_musicbrainz_set_error_from_query (query, result, error);
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
    
    cancel:
    
    if (metadata_release)
    {
        mb5_metadata_delete (metadata_release);
    }

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
}

/*
 * et_musicbrainz_search_track:
 * @string: String to search
 * @root: Root Node
 * @error: GErorr
 * @cancellable: GCancellable
 *
 * Returns: TRUE if successfull otherwise FALSE
 *
 * Search for Tracks with name as @string
 */
static gboolean
et_musicbrainz_search_track (gchar *string, GNode *root, int offset, 
                             GError **error, GCancellable *cancellable)
{
    Mb5Query query;
    Mb5Metadata metadata;
    Mb5Metadata metadata_recording;
    tQueryResult result;
    char *param_values[3];
    char *param_names[3];

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    metadata = NULL;
    param_names[0] = "query";
    param_names[1] = "limit";
    param_names[2] = "offset";
    param_values[2] = g_strdup_printf ("%d", offset);
    param_values[1] = SEARCH_LIMIT_STR;
    metadata_recording = NULL;
    query = mb5_query_new (USER_AGENT, server, port);
    param_values[0] = g_strconcat ("recordings:", string, NULL);
    CHECK_CANCELLED(cancellable);
    metadata = mb5_query_query (query, "recording", "", "", 3,
                                param_names, param_values);
    result = mb5_query_get_lastresult (query);
    g_free (param_values[0]);
    g_free (param_values[2]);

    if (result == eQuery_Success)
    {
        if (metadata)
        {
            int i;
            Mb5RecordingList list;
            gchar *message;

            list = mb5_metadata_get_recordinglist (metadata);
            param_names[0] = "inc";
            param_values[0] = "releases artists artist-credits release-groups";
            message = g_strdup_printf (ngettext (_("Found %d Track"),
                                       _("Found %d Tracks"),
                                       mb5_recording_list_size (list)),
                                       mb5_recording_list_size (list));
#ifndef TEST
            et_show_status_msg_in_idle (message);
#endif
            g_free (message);

            for (i = 0; i < mb5_recording_list_size (list); i++)
            {
                Mb5Recording recording;
                gchar buf[NAME_MAX_SIZE];
                GNode *node;
                EtMbEntity *entity;
                int size;

                CHECK_CANCELLED(cancellable);
                recording = mb5_recording_list_item (list, i);
                size = mb5_recording_get_title (recording, buf, sizeof (buf));
                buf[size] = '\0';
                message = g_strdup_printf (_("Retrieving %s (%d/%d)"),
                                           buf, i,
                                           mb5_track_list_size (list));
#ifndef TEST
                et_show_status_msg_in_idle (message);
#endif
                g_free (message);

                mb5_recording_get_id (recording,
                                      buf,
                                      sizeof (buf));
                metadata_recording = mb5_query_query (query, "recording",
                                                      buf, "",
                                                      1, param_names,
                                                      param_values);
                result = mb5_query_get_lastresult (query);

                if (result == eQuery_Success)
                {
                    if (metadata_recording)
                    {
                        CHECK_CANCELLED(cancellable);
                        entity = g_slice_new (EtMbEntity);
                        entity->entity = mb5_recording_clone (mb5_metadata_get_recording (metadata_recording));
                        entity->type = MB_ENTITY_KIND_TRACK;
                        entity->is_red_line = FALSE;
                        node = g_node_new (entity);
                        g_node_append (root, node);
                        mb5_metadata_delete (metadata_recording);
                    }
                }
                else
                {
                    goto err;
                }
            }
        }

        mb5_metadata_delete (metadata);
    }
    else
    {
        goto err;
    }

    CHECK_CANCELLED(cancellable);
    mb5_query_delete (query);

    return TRUE;

    err:

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    et_musicbrainz_set_error_from_query (query, result, error);
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
    
    cancel:
    
    if (metadata_recording)
    {
        mb5_metadata_delete (metadata_recording);
    }

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
}

/*
 * et_musicbrainz_search_discid:
 * @string: String to search
 * @root: Root Node
 * @error: GErorr
 * @cancellable: GCancellable
 *
 * Returns: TRUE if successfull otherwise FALSE
 *
 * Search for DiscID with value as @string
 */
static gboolean
et_musicbrainz_search_discid (gchar *string, GNode *root, GError **error,
                              GCancellable *cancellable)
{
    Mb5Query query;
    Mb5Metadata metadata;
    tQueryResult result;
    char *param_values[2];
    char *param_names[2];

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    param_names[0] = "query";
    param_names[1] = "limit";
    param_values[1] = SEARCH_LIMIT_STR;
    CHECK_CANCELLED(cancellable);
    query = mb5_query_new (USER_AGENT, server, port);
    param_names[0] = "toc";
    param_values[0] = "";
    metadata = mb5_query_query (query, "discid", string, "", 1, param_names,
                                param_values);
    result = mb5_query_get_lastresult (query);

    if (result == eQuery_Success)
    {
        if (metadata)
        {
            int i;
            Mb5ReleaseList list;
            gchar *message;
            Mb5Disc disc;

            disc = mb5_metadata_get_disc (metadata);
            list = mb5_disc_get_releaselist (disc);
            param_names[0] = "inc";
            param_values[0] = "artists release-groups";
            message = g_strdup_printf (ngettext (_("Found %d Album"),
                                       _("Found %d Albums"),
                                       mb5_release_list_size (list)),
                                       mb5_release_list_size (list));
#ifndef TEST
            et_show_status_msg_in_idle (message);
#endif
            g_free (message);

            for (i = 0; i < mb5_release_list_size (list); i++)
            {
                Mb5Release release;

                CHECK_CANCELLED(cancellable);
                release = mb5_release_list_item (list, i);

                if (release)
                {
                    Mb5Metadata metadata_release;
                    gchar buf[NAME_MAX_SIZE];
                    GNode *node;
                    EtMbEntity *entity;
                    int size;

                    size = mb5_release_get_title ((Mb5Release)release, buf,
                                                  sizeof (buf));
                    buf[size] = '\0';
                    message = g_strdup_printf (_("Retrieving %s (%d/%d)"),
                                               buf, i,
                                               mb5_release_list_size (list));
#ifndef TEST
                    et_show_status_msg_in_idle (message);
#endif
                    g_free (message);
                    mb5_release_get_id ((Mb5Release)release,
                                        buf,
                                        sizeof (buf));
                    metadata_release = mb5_query_query (query, "release",
                                                        buf, "",
                                                        1, param_names,
                                                        param_values);
                    result = mb5_query_get_lastresult (query);

                    if (result == eQuery_Success)
                    {
                        if (metadata_release)
                        {
                            CHECK_CANCELLED(cancellable);
                            entity = g_slice_new (EtMbEntity);
                            entity->entity = mb5_release_clone (mb5_metadata_get_release (metadata_release));
                            entity->type = MB_ENTITY_KIND_ALBUM;
                            entity->is_red_line = FALSE;
                            node = g_node_new (entity);
                            g_node_append (root, node);
                            mb5_metadata_delete (metadata_release);
                        }
                    }
                    else
                    {
                        goto err;
                    }
                }
            }
        }

        mb5_metadata_delete (metadata);
    }
    else
    {
        goto err;
    }

    CHECK_CANCELLED(cancellable);
    mb5_query_delete (query);

    return TRUE;
        
    err:

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    et_musicbrainz_set_error_from_query (query, result, error);
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
    
    cancel:
    
    return FALSE;
}

/*
 * et_musicbrainz_search_freedbid:
 * @string: String to search
 * @root: Root Node
 * @error: GErorr
 * @cancellable: GCancellable
 *
 * Returns: TRUE if successfull otherwise FALSE
 *
 * Search for FreeDBID with name as @string
 */
static gboolean
et_musicbrainz_search_freedbid (gchar *string, GNode *root, GError **error,
                                GCancellable *cancellable)
{
    Mb5Query query;
    Mb5Metadata metadata;
    tQueryResult result;
    char *param_values[2];
    char *param_names[2];

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    param_names[0] = "query";
    param_names[1] = "limit";
    param_values[1] = SEARCH_LIMIT_STR;
    CHECK_CANCELLED(cancellable);
    query = mb5_query_new (USER_AGENT, server, port);
    param_values[0] = g_strconcat ("discid:", string, NULL);
    metadata = mb5_query_query (query, "freedb", "", "", 2, param_names,
                                param_values);
    result = mb5_query_get_lastresult (query);
    g_free (param_values[0]);

    if (result == eQuery_Success)
    {
        if (metadata)
        {
            int i;
            Mb5FreeDBDiscList list;
            gchar *message;

            list = mb5_metadata_get_freedbdisclist (metadata);
            message = g_strdup_printf (ngettext (_("Found %d Result"),
                                       _("Found %d Results"),
                                       mb5_freedbdisc_list_size (list)), 
                                       mb5_freedbdisc_list_size (list));
#ifndef TEST
            et_show_status_msg_in_idle (message);
#endif
            g_free (message);

            for (i = 0; i < mb5_freedbdisc_list_size (list); i++)
            {
                Mb5FreeDBDisc freedbdisc;

                CHECK_CANCELLED(cancellable);
                freedbdisc = mb5_freedbdisc_list_item (list, i);

                if (freedbdisc)
                {
                    gchar buf[NAME_MAX_SIZE];
                    GNode *node;
                    EtMbEntity *entity;
                    int size;

                    size = mb5_freedbdisc_get_title (freedbdisc,
                                                     buf, sizeof (buf));
                    buf[size] = '\0';
                    entity = g_slice_new (EtMbEntity);
                    entity->entity = mb5_freedbdisc_clone (freedbdisc);
                    entity->type = MB_ENTITY_KIND_FREEDBID;
                    entity->is_red_line = FALSE;
                    node = g_node_new (entity);
                    g_node_append (root, node);
                }
            }
        }

        mb5_metadata_delete (metadata);
    }
    else
    {
        goto err;
    }

    CHECK_CANCELLED(cancellable);
    mb5_query_delete (query);

    return TRUE;

    err:

    if (metadata)
    {
        mb5_metadata_delete (metadata);
    }

    et_musicbrainz_set_error_from_query (query, result, error);
    mb5_query_delete (query);
    g_assert (error == NULL || *error != NULL);

    return FALSE;
    
    cancel:
    
    return FALSE;
}

/*
 * et_musicbrainz_search:
 * @string: String to search in MusicBrainz database.
 * @type: Type of entity to search.
 * @root: Root of the mbTree.
 * @error: GError.
 *
 * To search for an entity in MusicBrainz Database.
 *
 * Returns: TRUE if successfull, FALSE if not.
 */
gboolean
et_musicbrainz_search (gchar *string, MbEntityKind type, GNode *root, int offset,
                       GError **error, GCancellable *cancellable)
{
    switch (type)
    {
        case MB_ENTITY_KIND_ARTIST:
            return et_musicbrainz_search_artist (string, root, offset, error,
                                                 cancellable);
    

        case MB_ENTITY_KIND_ALBUM:
            return et_musicbrainz_search_album (string, root, offset, error,
                                                cancellable);

        case MB_ENTITY_KIND_TRACK:
            return et_musicbrainz_search_track (string, root, offset, error,
                                                cancellable);

        case MB_ENTITY_KIND_DISCID:
            return et_musicbrainz_search_discid (string, root, error,
                                                 cancellable);
        case MB_ENTITY_KIND_FREEDBID:
            return et_musicbrainz_search_freedbid (string, root, error,
                                                   cancellable);
        default:
            return FALSE;
    }
}

/*
 * free_mb_tree:
 * @node: Root of the tree to start freeing with.
 *
 * To free the memory occupied by the tree.
 */
void
free_mb_tree (GNode **_node)
{
    EtMbEntity *et_entity;
    GNode *child;
    GNode *node;

    node = *_node;

    if (!node)
    {
        return;
    }

    et_entity = (EtMbEntity *)node->data;

    if (et_entity)
    {
        if (et_entity->type == MB_ENTITY_KIND_ARTIST)
        {
            mb5_artist_delete ((Mb5Artist)et_entity->entity);
        }

        else if (et_entity->type == MB_ENTITY_KIND_ALBUM)
        {
            mb5_release_delete ((Mb5Release)et_entity->entity);
        }

        else if (et_entity->type == MB_ENTITY_KIND_TRACK)
        {
            mb5_recording_delete ((Mb5Recording)et_entity->entity);
        }
    }

    g_slice_free (EtMbEntity, et_entity);
    g_node_unlink (node);
    child = g_node_first_child (node);

    while (child)
    {
        GNode *child1;
        child1 = g_node_next_sibling (child);
        free_mb_tree (&child);
        child = child1;
    }

    g_node_destroy (node);
    *_node = NULL;
}
#endif /* ENABLE_libmusicbrainz */