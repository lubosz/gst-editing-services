/* GStreamer Editing Services
 * Copyright (C) 2013 Lubosz Sarnecki <lubosz@gmail.com>
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

#include <ges/ges.h>

/* A image sequence test */
int
main (int argc, gchar ** argv)
{
  GESPipeline *pipeline;
  GESTimeline *timeline;
  GESMultiFileClip *clip;
  //GESTestClip *clip;
  GESLayer *layer;
  GMainLoop *mainloop;
  gchar *path;
  GESTrack *track;

  gst_init (&argc, &argv);
  ges_init ();

  timeline = ges_timeline_new ();
  track = GES_TRACK (ges_video_track_new ());
  ges_timeline_add_track (timeline, track);

  layer = ges_layer_new ();
  if (!ges_timeline_add_layer (timeline, layer))
    return -1;

  path =
      g_strconcat
      ("/home/bmonkey/workspace/ges/data/transparent/blender-cube/png/%04d.png",
      NULL);

  clip = ges_multi_file_clip_new_from_location (path);
/*
  clip = ges_test_clip_new();
*/

  g_object_set (clip, "duration", 4 * GST_SECOND, NULL);

  ges_layer_add_clip (layer, GES_CLIP (clip));

  pipeline = ges_pipeline_new ();

  if (!ges_pipeline_add_timeline (pipeline, timeline))
    return -1;

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
      GST_DEBUG_GRAPH_SHOW_ALL, "multi");

  mainloop = g_main_loop_new (NULL, FALSE);

  g_timeout_add_seconds (4, (GSourceFunc) g_main_loop_quit, mainloop);
  g_main_loop_run (mainloop);

  GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
      GST_DEBUG_GRAPH_SHOW_ALL, "multi2");

  return 0;
}
