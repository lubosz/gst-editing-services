/* GStreamer Editing Services
 * Copyright (C) 2009 Edward Hervey <bilboed@bilboed.com>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _GES_TRACK
#define _GES_TRACK

#include <glib-object.h>
#include <gst/gst.h>
#include <ges/ges-types.h>

G_BEGIN_DECLS

#define GES_TYPE_TRACK ges_track_get_type()

#define GES_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GES_TYPE_TRACK, GESTrack))

#define GES_TRACK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GES_TYPE_TRACK, GESTrackClass))

#define GES_IS_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GES_TYPE_TRACK))

#define GES_IS_TRACK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GES_TYPE_TRACK))

#define GES_TRACK_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GES_TYPE_TRACK, GESTrackClass))

struct _GESTrack {
  GstBin parent;

  GESTimeline * timeline;

  GstElement * composition;	/* The composition associated with this track */
};

struct _GESTrackClass {
  GstBinClass parent_class;
};

GType ges_track_get_type (void);

GESTrack* ges_track_new (void);

void ges_track_set_timeline (GESTrack * track, GESTimeline *timeline);

gboolean ges_track_add_object (GESTrack * track, GESTrackObject * object);
gboolean ges_track_remove_object (GESTrack * track, GESTrackObject * object);

G_END_DECLS

#endif /* _GES_TRACK */

