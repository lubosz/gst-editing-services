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
#include <string.h>

G_DEFINE_TYPE (GESMultiFileClip, ges_multi_file_clip, GES_TYPE_SOURCE_CLIP);

struct _GESMultiFileClipPrivate
{
  gchar *location;
  guint fps;
};

enum
{
  PROP_0,
  PROP_LOCATION,
  PROP_FPS,
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
    case PROP_FPS:
      g_value_set_enum (value, priv->fps);
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
    case PROP_FPS:
      ges_multi_file_clip_set_fps (uriclip, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
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

  /**
   * GESMultiFileClip:fps:
   *
   * The fps of the sequence.
   */
  g_object_class_install_property (object_class, PROP_FPS,
      g_param_spec_int ("fps", "Frames Per Second",
          "FPS Caps", 1, 200, 25, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


  timobj_class->create_track_element = ges_multi_file_clip_create_track_element;
}

static void
ges_multi_file_clip_init (GESMultiFileClip * self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      GES_TYPE_MULTI_FILE_CLIP, GESMultiFileClipPrivate);

  self->priv->fps = 0;
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
  //GList *tmp;

  self->priv->location = location;

/*
  for (tmp = GES_CONTAINER_CHILDREN (self); tmp; tmp = tmp->next) {
    //GESTrackElement *trackelement = (GESTrackElement *) tmp->data;
    //if (GES_IS_MULTI_FILE_SOURCE (trackelement))
    //g_object_set ((GESVideoTestSource *) trackelement, "location",
    //    location, NULL);
  }
*/
}

/**
 * ges_multi_file_clip_set_vpattern:
 * @self: the #GESMultiFileClip to set the pattern on
 * @vpattern: the #GESVideoTestPattern to use on @self
 *
 * Sets which video pattern to display on @self.
 *
 */
void
ges_multi_file_clip_set_fps (GESMultiFileClip * self, guint fps)
{
  //GList *tmp;

  self->priv->fps = fps;
/*
  for (tmp = GES_CONTAINER_CHILDREN (self); tmp; tmp = tmp->next) {
    GESTrackElement *trackelement = (GESTrackElement *) tmp->data;
    if (GES_IS_MULTI_FILE_SOURCE (trackelement))
    //ges_multi_file_source_set_pattern (
    //    (GESMultiFileSource *) trackelement, vpattern);
  }
*/
}



/**
 * ges_multi_file_clip_get_vpattern:
 * @self: a #GESMultiFileClip
 *
 * Get the #GESVideoTestPattern which is applied on @self.
 *
 * Returns: The #GESVideoTestPattern which is applied on @self.
 */
guint
ges_multi_file_clip_get_fps (GESMultiFileClip * self)
{
  return self->priv->fps;
}

/**
 * ges_multi_file_clip_get_frequency:
 * @self: a #GESMultiFileClip
 *
 * Get the frequency @self generates.
 *
 * Returns: The frequency @self generates. See audiotestsrc element.
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

    //g_object_set ((GESMultiFileSource *) res, "location", priv->location, NULL);

    //ges_multi_file_source_set_fps (
    //    (GESVideoTestSource *) res, priv->fps);
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
/*
  GEnumValue *value;
  GEnumClass *klass;
  GESMultiFileClip *ret = NULL;

  klass = G_ENUM_CLASS (g_type_class_ref (GES_VIDEO_TEST_PATTERN_TYPE));
  if (!klass)
    return NULL;

  value = g_enum_get_value_by_nick (klass, location);
  if (value) {
    ret = ges_multi_file_clip_new ();
    //ges_multi_file_clip_set_vpattern (ret, value->value);
  }

  g_type_class_unref (klass);
  return ret;
*/
}
