/* GStreamer Editing Services
 * Copyright (C) 2009 Edward Hervey <edward.hervey@collabora.co.uk>
 *               2009 Nokia Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/**
 * SECTION:ges-multi-file-clip
 * @short_description: Render video from an image sequence
 * #GESLayer
 *
 * Useful for loading animations with an alpha channel.
 *
 * You can use the ges_asset_request_simple API to create a Asset
 * capable of extractinf GESMultiFileClip-s
 */

#include "ges-internal.h"
#include "ges-multi-file-clip.h"
#include "ges-source-clip.h"
#include "ges-track-element.h"
#include "ges-multi-file-source.h"
#include "ges-extractable.h"
#include <string.h>

static void ges_extractable_interface_init (GESExtractableInterface * iface);

#define parent_class ges_multi_file_clip_parent_class

G_DEFINE_TYPE_WITH_CODE (GESMultiFileClip, ges_multi_file_clip,
    GES_TYPE_SOURCE_CLIP,
    G_IMPLEMENT_INTERFACE (GES_TYPE_EXTRACTABLE,
        ges_extractable_interface_init));

GESExtractableInterface *parent_extractable_iface;

struct _GESMultiFileClipPrivate
{
  gchar *location;
};

enum
{
  PROP_0,
  PROP_LOCATION,
};

static GESTrackElement
    * ges_multi_file_clip_create_track_element (GESClip * clip,
    GESTrackType type);

static void
ges_multi_file_clip_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GESMultiFileClipPrivate *priv = GES_MULTI_FILE_CLIP (object)->priv;

  switch (property_id) {
    case PROP_LOCATION:
      g_value_set_string (value, priv->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
ges_multi_file_clip_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GESMultiFileClip *uriclip = GES_MULTI_FILE_CLIP (object);

  switch (property_id) {
    case PROP_LOCATION:
      ges_multi_file_clip_set_location (uriclip, g_value_dup_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static gchar *
extractable_check_id (GType type, const gchar * id)
{
/*
  const gchar *testing_directory;

  testing_directory = g_getenv ("GES_TESTING_ASSETS_DIRECTORY");

  // Testing purposes, user can specify a directory to look up for script
  if (testing_directory != NULL) {
    gchar **tokens;
    gchar *location = NULL;
    guint i;

    GST_DEBUG ("Checking if the testing directory contains needed media");

    tokens = g_strsplit (id, "media", 2);
    for (i = 0; tokens[i]; i++)
      if (i == 1)
        location = tokens[1];

    if (location == NULL)
      GST_WARNING ("The provided id doesn't have a media subdirectory");
    else {
      gchar *actual_id =
          g_strconcat ("file://", testing_directory, "/media/", location, NULL);

      if (gst_uri_is_valid (actual_id)) {
        GST_DEBUG ("Returning new id %s instead of id %s", actual_id, id);
        g_strfreev (tokens);
        return (actual_id);
      } else
        GST_WARNING ("The constructed id %s was not valid, trying %s anyway",
            actual_id, id);

      g_free (actual_id);
    }

    g_strfreev (tokens);
  }

  if (gst_uri_is_valid (id))
    return g_strdup (id);
*/
  return NULL;
}

static GParameter *
extractable_get_parameters_from_id (const gchar * id, guint * n_params)
{
  GParameter *params = g_new0 (GParameter, 2);

  params[0].name = "uri";
  g_value_init (&params[0].value, G_TYPE_STRING);
  g_value_set_string (&params[0].value, id);

  *n_params = 1;

  return params;
}

static gchar *
extractable_get_id (GESExtractable * self)
{
  return g_strdup (GES_MULTI_FILE_CLIP (self)->priv->location);
}

/*
static void
extractable_set_asset (GESExtractable * self, GESAsset * asset)
{
  GESMultiFileClip *multifileclip = GES_MULTI_FILE_CLIP (self);
  GESMultiFileClipAsset *filesource_asset = GES_MULTI_FILE_CLIP_ASSET (asset);
  GESClip *clip = GES_CLIP (self);

  if (GST_CLOCK_TIME_IS_VALID (GES_TIMELINE_ELEMENT_DURATION (clip)) == FALSE)
    _set_duration0 (GES_TIMELINE_ELEMENT (multifileclip),
        ges_multi_file_clip_asset_get_duration (filesource_asset));

  ges_timeline_element_set_max_duration (GES_TIMELINE_ELEMENT (multifileclip),
      ges_multi_file_clip_asset_get_duration (filesource_asset));

  ges_uri_clip_set_is_image (multifileclip,
      ges_multi_file_clip_asset_is_image (filesource_asset));

  if (ges_clip_get_supported_formats (clip) == GES_TRACK_TYPE_UNKNOWN) {

    ges_clip_set_supported_formats (clip,
        ges_clip_asset_get_supported_formats
        (GES_CLIP_ASSET (filesource_asset)));
  }

  GES_TIMELINE_ELEMENT (multifileclip)->asset = asset;
}
*/

static void
ges_extractable_interface_init (GESExtractableInterface * iface)
{
  //iface->asset_type = GES_TYPE_URI_CLIP_ASSET;
  iface->check_id = (GESExtractableCheckId) extractable_check_id;
  iface->get_parameters_from_id = extractable_get_parameters_from_id;
  iface->get_id = extractable_get_id;
  //iface->set_asset = extractable_set_asset;
}


static void
ges_multi_file_clip_class_init (GESMultiFileClipClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GESClipClass *timobj_class = GES_CLIP_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GESMultiFileClipPrivate));

  object_class->get_property = ges_multi_file_clip_get_property;
  object_class->set_property = ges_multi_file_clip_set_property;

  /**
   * GESMultiFileClip:location:
   *
   * Location of the files.
   */
  g_object_class_install_property (object_class, PROP_LOCATION,
      g_param_spec_string ("location", "Files Location",
          "Location of the Sequence", NULL,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  timobj_class->create_track_element = ges_multi_file_clip_create_track_element;
}

static void
ges_multi_file_clip_init (GESMultiFileClip * self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      GES_TYPE_MULTI_FILE_CLIP, GESMultiFileClipPrivate);

  GES_TIMELINE_ELEMENT (self)->duration = 0;
}

/**
 * ges_multi_file_clip_set_location:
 * @self: the #GESMultiFileClip on which to set the location
 * @location: Location of the files as path
 *
 * Sets the location.
 *
 */
void
ges_multi_file_clip_set_location (GESMultiFileClip * self, gchar * location)
{
  if (GES_CONTAINER_CHILDREN (self)) {
    /* FIXME handle this case properly */
    GST_WARNING_OBJECT (self, "Can not change location when already"
        "containing TrackElements");
    return;
  }

  self->priv->location = location;
}

/**
 * ges_multi_file_clip_get_location:
 * @self: a #GESMultiFileClip
 *
 * Get the location @self generates.
 *
 * Returns: The location @self generates.
 */
char *
ges_multi_file_clip_get_location (GESMultiFileClip * self)
{
  return self->priv->location;
}

static GESTrackElement *
ges_multi_file_clip_create_track_element (GESClip * clip, GESTrackType type)
{
  GESMultiFileClipPrivate *priv = GES_MULTI_FILE_CLIP (clip)->priv;
  GESTrackElement *res = NULL;

  GST_DEBUG ("Creating a GESTrackTestSource for type: %s",
      ges_track_type_name (type));

  if (type == GES_TRACK_TYPE_VIDEO) {
    res = (GESTrackElement *) ges_multi_file_source_new (priv->location);
  }

  return res;
}

/**
 * ges_multi_file_clip_new:
 *
 * Creates a new #GESMultiFileClip.
 *
 * Returns: The newly created #GESMultiFileClip, or NULL if there was an
 * error.
 */
GESMultiFileClip *
ges_multi_file_clip_new (void)
{
  GESMultiFileClip *new_clip;
  GESAsset *asset = ges_asset_request (GES_TYPE_MULTI_FILE_CLIP, NULL, NULL);

  new_clip = GES_MULTI_FILE_CLIP (ges_asset_extract (asset, NULL));
  gst_object_unref (asset);

  return new_clip;
}

/**
 * ges_multi_file_clip_new_for_nick:
 * @nick: the nickname for which to create the #GESMultiFileClip
 *
 * Creates a new #GESMultiFileClip for the provided @nick.
 *
 * Returns: The newly created #GESMultiFileClip, or NULL if there was an
 * error.
 */
GESMultiFileClip *
ges_multi_file_clip_new_from_location (gchar * location)
{
  return g_object_new (GES_TYPE_MULTI_FILE_CLIP, "location", location, NULL);
}
