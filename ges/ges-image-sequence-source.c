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
 * SECTION:ges-image-sequence-source
 * @short_description: outputs the video stream from a sequence of images.
 * 
 * Outputs the video stream from a given image sequence. The start frame
 * chosen will be determined by the in-point property on the track element.
 */

#include "ges-internal.h"
#include "ges-track-element.h"
#include "ges-image-sequence-source.h"

G_DEFINE_TYPE (GESImageSequenceSource, ges_image_sequence_source,
    GES_TYPE_VIDEO_SOURCE);

struct _GESImageSequenceSourcePrivate
{
  /*  Dummy variable */
  void *nothing;
};

enum
{
  PROP_0,
  PROP_URI
};

static void
ges_image_sequence_source_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GESImageSequenceSource *uriclip = GES_IMAGE_SEQUENCE_SOURCE (object);

  switch (property_id) {
    case PROP_URI:
      g_value_set_string (value, uriclip->location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
ges_image_sequence_source_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GESImageSequenceSource *uriclip = GES_IMAGE_SEQUENCE_SOURCE (object);

  switch (property_id) {
    case PROP_URI:
      uriclip->location = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
ges_image_sequence_source_dispose (GObject * object)
{
  GESImageSequenceSource *uriclip = GES_IMAGE_SEQUENCE_SOURCE (object);

  if (uriclip->location)
    g_free (uriclip->location);

  G_OBJECT_CLASS (ges_image_sequence_source_parent_class)->dispose (object);
}

static void
pad_added_cb (GstElement * timeline, GstPad * pad, GstElement * scale)
{
  GstPad *sinkpad;
  GstPadLinkReturn ret;

  sinkpad = gst_element_get_static_pad (scale, "sink");
  if (sinkpad) {
    GST_DEBUG ("got sink pad, trying to link");

    ret = gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
    if (GST_PAD_LINK_SUCCESSFUL (ret)) {
      GST_DEBUG ("linked ok, returning");
      return;
    }
  }

  GST_DEBUG ("pad failed to link properly");
}

static GstElement *
ges_image_sequence_source_create_source (GESTrackElement * track_element)
{
  GstElement *bin, *source, *decodebin, *iconv;
  GstPad *src, *target;

  bin = GST_ELEMENT (gst_bin_new ("still-image-bin"));
  source = gst_element_factory_make ("multifilesrc", NULL);
  decodebin = gst_element_factory_make ("decodebin", NULL);
  iconv = gst_element_factory_make ("videoconvert", NULL);

  gst_bin_add_many (GST_BIN (bin), source, decodebin, iconv, NULL);

  gst_element_link_pads_full (decodebin, "src", iconv, "sink",
      GST_PAD_LINK_CHECK_NOTHING);

  /* FIXME: add capsfilter here with sink caps (see 626518) */

  target = gst_element_get_static_pad (iconv, "src");

  src = gst_ghost_pad_new ("src", target);
  gst_element_add_pad (bin, src);
  gst_object_unref (target);

  g_object_set (source, "location",
      ((GESImageSequenceSource *) track_element)->location, NULL);

  g_signal_connect (G_OBJECT (source), "pad-added",
      G_CALLBACK (pad_added_cb), iconv);

  return bin;
}

static void
ges_image_sequence_source_class_init (GESImageSequenceSourceClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GESVideoSourceClass *source_class = GES_VIDEO_SOURCE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GESImageSequenceSourcePrivate));

  object_class->get_property = ges_image_sequence_source_get_property;
  object_class->set_property = ges_image_sequence_source_set_property;
  object_class->dispose = ges_image_sequence_source_dispose;

  /**
   * GESImageSequenceSource:location:
   *
   * The location of the file/resource to use.
   */
  g_object_class_install_property (object_class, PROP_URI,
      g_param_spec_string ("location", "LOCATION", "location of the resource",
          NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  source_class->create_source = ges_image_sequence_source_create_source;
}

static void
ges_image_sequence_source_init (GESImageSequenceSource * self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      GES_TYPE_IMAGE_SEQUENCE_SOURCE, GESImageSequenceSourcePrivate);
}

/**
 * ges_image_sequence_source_new:
 * @location: the URI the source should control
 *
 * Creates a new #GESImageSequenceSource for the provided @location.
 *
 * Returns: A new #GESImageSequenceSource.
 */
GESImageSequenceSource *
ges_image_sequence_source_new (gchar * location)
{
  return g_object_new (GES_TYPE_IMAGE_SEQUENCE_SOURCE, "location", location,
      "track-type", GES_TRACK_TYPE_VIDEO, NULL);
}
