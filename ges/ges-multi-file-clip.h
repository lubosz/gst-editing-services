/* GStreamer Editing Services
 * Copyright (C) 2009 Brandon Lewis <brandon.lewis@collabora.co.uk>
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

#ifndef _GES_MULTI_FILE_CLIP
#define _GES_MULTI_FILE_CLIP

#include <glib-object.h>
#include <ges/ges-enums.h>
#include <ges/ges-types.h>
#include <ges/ges-source-clip.h>
#include <ges/ges-track.h>

G_BEGIN_DECLS

#define GES_TYPE_MULTI_FILE_CLIP ges_multi_file_clip_get_type()

#define GES_MULTI_FILE_CLIP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GES_TYPE_MULTI_FILE_CLIP, GESMultiFileClip))

#define GES_MULTI_FILE_CLIP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GES_TYPE_MULTI_FILE_CLIP, GESMultiFileClipClass))

#define GES_IS_MULTI_FILE_CLIP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GES_TYPE_MULTI_FILE_CLIP))

#define GES_IS_MULTI_FILE_CLIP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GES_TYPE_MULTI_FILE_CLIP))

#define GES_MULTI_FILE_CLIP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GES_TYPE_MULTI_FILE_CLIP, GESMultiFileClipClass))

typedef struct _GESMultiFileClipPrivate GESMultiFileClipPrivate;

/**
 * GESMultiFileClip:
 * 
 */

struct _GESMultiFileClip {

  GESSourceClip parent;

  /*< private >*/
  GESMultiFileClipPrivate *priv;

  /* Padding for API extension */
  gpointer _ges_reserved[GES_PADDING];
};

/**
 * GESMultiFileClipClass:
 */

struct _GESMultiFileClipClass {
  /*< private >*/
  GESSourceClipClass parent_class;

  /* Padding for API extension */
  gpointer _ges_reserved[GES_PADDING];
};

GType ges_multi_file_clip_get_type (void);

void
ges_multi_file_clip_set_mute (GESMultiFileClip * self, gboolean mute);

void
ges_multi_file_clip_set_vpattern (GESMultiFileClip * self,
    GESVideoTestPattern vpattern);

void
ges_multi_file_clip_set_frequency (GESMultiFileClip * self, gdouble freq);

void
ges_multi_file_clip_set_volume (GESMultiFileClip * self,
    gdouble volume);


GESVideoTestPattern
ges_multi_file_clip_get_vpattern (GESMultiFileClip * self);

gboolean ges_multi_file_clip_is_muted (GESMultiFileClip * self);
gdouble ges_multi_file_clip_get_frequency (GESMultiFileClip * self);
gdouble ges_multi_file_clip_get_volume (GESMultiFileClip * self);

GESMultiFileClip* ges_multi_file_clip_new (void);
GESMultiFileClip* ges_multi_file_clip_new_from_location(gchar * location);

G_END_DECLS

#endif /* _GES_MULTI_FILE_CLIP */

