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
  gchar *location;

  if (gst_uri_is_valid (id)) {
    GST_WARNING
        ("MultiFileClip does not take uris as parameter, but location patterns: %s",
        id);
    location = gst_uri_get_location (id);

    if (location != NULL) {
      return location;
    } else {
      GST_WARNING ("No location found in uri: %s", id);
      return NULL;
    }
  }
  return g_strdup (id);
}

static GParameter *
extractable_get_parameters_from_id (const gchar * id, guint * n_params)
{
  GParameter *params = g_new0 (GParameter, 2);

  params[0].name = "location";
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

static void
ges_extractable_interface_init (GESExtractableInterface * iface)
{
  iface->check_id = (GESExtractableCheckId) extractable_check_id;
  iface->get_parameters_from_id = extractable_get_parameters_from_id;
  iface->get_id = extractable_get_id;
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
 * @location: the location from which to create the #GESMultiFileClip
 * Creates a new #GESMultiFileClip.
 *
 * Returns: The newly created #GESMultiFileClip, or NULL if there was an
 * error.
 */
GESMultiFileClip *
ges_multi_file_clip_new (gchar * location)
{
  GESMultiFileClip *new_clip;
  GESAsset *asset =
      ges_asset_request (GES_TYPE_MULTI_FILE_CLIP, location, NULL);

  new_clip = GES_MULTI_FILE_CLIP (ges_asset_extract (asset, NULL));
  gst_object_unref (asset);

  return new_clip;
}
