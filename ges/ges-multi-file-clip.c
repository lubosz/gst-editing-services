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
#include "ges-video-test-source.h"
#include "ges-audio-test-source.h"
#include <string.h>

G_DEFINE_TYPE (GESMultiFileClip, ges_multi_file_clip, GES_TYPE_SOURCE_CLIP);

#define DEFAULT_VOLUME 1.0
#define DEFAULT_VPATTERN GES_VIDEO_TEST_PATTERN_SMPTE

struct _GESMultiFileClipPrivate
{
  gchar *location;
  //gboolean mute;
  //GESVideoTestPattern vpattern;
  //gdouble freq;
  //gdouble volume;
};

enum
{
  PROP_0,
  PROP_MUTE,
  PROP_VPATTERN,
  PROP_FREQ,
  PROP_VOLUME,
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
    case PROP_MUTE:
      g_value_set_boolean (value, priv->mute);
      break;
    case PROP_VPATTERN:
      g_value_set_enum (value, priv->vpattern);
      break;
    case PROP_FREQ:
      g_value_set_double (value, priv->freq);
      break;
    case PROP_VOLUME:
      g_value_set_double (value, priv->volume);
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
    case PROP_MUTE:
      ges_multi_file_clip_set_mute (uriclip, g_value_get_boolean (value));
      break;
    case PROP_VPATTERN:
      ges_multi_file_clip_set_vpattern (uriclip, g_value_get_enum (value));
      break;
    case PROP_FREQ:
      ges_multi_file_clip_set_frequency (uriclip, g_value_get_double (value));
      break;
    case PROP_VOLUME:
      ges_multi_file_clip_set_volume (uriclip, g_value_get_double (value));
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
   * GESMultiFileClip:vpattern:
   *
   * Video pattern to display in video track elements.
   */
  g_object_class_install_property (object_class, PROP_VPATTERN,
      g_param_spec_enum ("vpattern", "VPattern",
          "Which video pattern to display. See videotestsrc element",
          GES_VIDEO_TEST_PATTERN_TYPE,
          DEFAULT_VPATTERN, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  /**
   * GESMultiFileClip:freq:
   *
   * The frequency to generate for audio track elements.
   */
  g_object_class_install_property (object_class, PROP_FREQ,
      g_param_spec_double ("freq", "Audio Frequency",
          "The frequency to generate. See audiotestsrc element",
          0, 20000, 440, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  /**
   * GESMultiFileClip:volume:
   *
   * The volume for the audio track elements.
   */
  g_object_class_install_property (object_class, PROP_VOLUME,
      g_param_spec_double ("volume", "Audio Volume",
          "The volume of the test audio signal.",
          0, 1, DEFAULT_VOLUME, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));


  /**
   * GESMultiFileClip:mute:
   *
   * Whether the sound will be played or not.
   */
  g_object_class_install_property (object_class, PROP_MUTE,
      g_param_spec_boolean ("mute", "Mute", "Mute audio track",
          FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  timobj_class->create_track_element = ges_multi_file_clip_create_track_element;
}

static void
ges_multi_file_clip_init (GESMultiFileClip * self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      GES_TYPE_MULTI_FILE_CLIP, GESMultiFileClipPrivate);

  self->priv->freq = 0;
  self->priv->volume = 0;
  GES_TIMELINE_ELEMENT (self)->duration = 0;
}

/**
 * ges_multi_file_clip_set_mute:
 * @self: the #GESMultiFileClip on which to mute or unmute the audio track
 * @mute: %TRUE to mute the audio track, %FALSE to unmute it
 *
 * Sets whether the audio track of this clip is muted or not.
 *
 */
void
ges_multi_file_clip_set_mute (GESMultiFileClip * self, gboolean mute)
{
  GList *tmp;

  GST_DEBUG ("self:%p, mute:%d", self, mute);

  self->priv->mute = mute;

  /* Go over tracked objects, and update 'active' status on all audio objects */
  for (tmp = GES_CONTAINER_CHILDREN (self); tmp; tmp = tmp->next) {
    GESTrackElement *trackelement = (GESTrackElement *) tmp->data;

    if (ges_track_element_get_track (trackelement)->type ==
        GES_TRACK_TYPE_AUDIO)
      ges_track_element_set_active (trackelement, !mute);
  }
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
ges_multi_file_clip_set_vpattern (GESMultiFileClip * self,
    GESVideoTestPattern vpattern)
{
  GList *tmp;

  self->priv->vpattern = vpattern;

  for (tmp = GES_CONTAINER_CHILDREN (self); tmp; tmp = tmp->next) {
    GESTrackElement *trackelement = (GESTrackElement *) tmp->data;
    if (GES_IS_MULTI_FILE_SOURCE (trackelement))
      print ("foo");
    //ges_multi_file_source_set_pattern (
    //    (GESMultiFileSource *) trackelement, vpattern);
  }
}

/**
 * ges_multi_file_clip_set_frequency:
 * @self: the #GESMultiFileClip to set the frequency on
 * @freq: the frequency you want to use on @self
 *
 * Sets the frequency to generate. See audiotestsrc element.
 *
 */
void
ges_multi_file_clip_set_frequency (GESMultiFileClip * self, gdouble freq)
{
  GList *tmp;

  self->priv->freq = freq;

  for (tmp = GES_CONTAINER_CHILDREN (self); tmp; tmp = tmp->next) {
    GESTrackElement *trackelement = (GESTrackElement *) tmp->data;
    if (GES_IS_AUDIO_TEST_SOURCE (trackelement))
      ges_audio_test_source_set_freq (
          (GESAudioTestSource *) trackelement, freq);
  }
}

/**
 * ges_multi_file_clip_set_volume:
 * @self: the #GESMultiFileClip to set the volume on
 * @volume: the volume of the audio signal you want to use on @self
 *
 * Sets the volume of the test audio signal.
 *
 */
void
ges_multi_file_clip_set_volume (GESMultiFileClip * self, gdouble volume)
{
  GList *tmp;

  self->priv->volume = volume;

  for (tmp = GES_CONTAINER_CHILDREN (self); tmp; tmp = tmp->next) {
    GESTrackElement *trackelement = (GESTrackElement *) tmp->data;
    if (GES_IS_AUDIO_TEST_SOURCE (trackelement))
      ges_audio_test_source_set_volume (
          (GESAudioTestSource *) trackelement, volume);
  }
}

/**
 * ges_multi_file_clip_get_vpattern:
 * @self: a #GESMultiFileClip
 *
 * Get the #GESVideoTestPattern which is applied on @self.
 *
 * Returns: The #GESVideoTestPattern which is applied on @self.
 */
GESVideoTestPattern
ges_multi_file_clip_get_vpattern (GESMultiFileClip * self)
{
  return self->priv->vpattern;
}

/**
 * ges_multi_file_clip_is_muted:
 * @self: a #GESMultiFileClip
 *
 * Let you know if the audio track of @self is muted or not.
 *
 * Returns: Whether the audio track of @self is muted or not.
 */
gboolean
ges_multi_file_clip_is_muted (GESMultiFileClip * self)
{
  return self->priv->mute;
}

/**
 * ges_multi_file_clip_get_frequency:
 * @self: a #GESMultiFileClip
 *
 * Get the frequency @self generates.
 *
 * Returns: The frequency @self generates. See audiotestsrc element.
 */
gdouble
ges_multi_file_clip_get_frequency (GESMultiFileClip * self)
{
  return self->priv->freq;
}

/**
 * ges_multi_file_clip_get_volume:
 * @self: a #GESMultiFileClip
 *
 * Get the volume of the test audio signal applied on @self.
 *
 * Returns: The volume of the test audio signal applied on @self.
 */
gdouble
ges_multi_file_clip_get_volume (GESMultiFileClip * self)
{
  return self->priv->volume;
}

static GESTrackElement *
ges_multi_file_clip_create_track_element (GESClip * clip, GESTrackType type)
{
  GESMultiFileClipPrivate *priv = GES_MULTI_FILE_CLIP (clip)->priv;
  GESTrackElement *res = NULL;

  GST_DEBUG ("Creating a GESTrackTestSource for type: %s",
      ges_track_type_name (type));

  if (type == GES_TRACK_TYPE_VIDEO) {
    res = (GESTrackElement *) ges_video_test_source_new ();
    ges_video_test_source_set_pattern (
        (GESVideoTestSource *) res, priv->vpattern);
  } else if (type == GES_TRACK_TYPE_AUDIO) {
    res = (GESTrackElement *) ges_audio_test_source_new ();

    if (priv->mute)
      ges_track_element_set_active (res, FALSE);

    ges_audio_test_source_set_freq ((GESAudioTestSource *) res, priv->freq);
    ges_audio_test_source_set_volume ((GESAudioTestSource *) res, priv->volume);
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
ges_multi_file_clip_new_for_nick (gchar * nick)
{
  GEnumValue *value;
  GEnumClass *klass;
  GESMultiFileClip *ret = NULL;

  klass = G_ENUM_CLASS (g_type_class_ref (GES_VIDEO_TEST_PATTERN_TYPE));
  if (!klass)
    return NULL;

  value = g_enum_get_value_by_nick (klass, nick);
  if (value) {
    ret = ges_multi_file_clip_new ();
    ges_multi_file_clip_set_vpattern (ret, value->value);
  }

  g_type_class_unref (klass);
  return ret;
}
