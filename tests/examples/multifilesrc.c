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
  GESLayer *layer;
  GMainLoop *mainloop;
  gchar *path;

  gst_init (&argc, &argv);
  ges_init ();

  timeline = ges_timeline_new_audio_video ();

  layer = ges_layer_new ();
  if (!ges_timeline_add_layer (timeline, layer))
    return -1;

  path =
      g_strconcat
      ("/home/bmonkey/workspace/ges/data/transparent/blender-cube/png/%04d.png",
      NULL);

  clip = ges_multi_file_clip_new_from_location (path);

  g_object_set (clip, "duration", 4 * GST_SECOND, NULL);

  ges_layer_add_clip (layer, GES_CLIP (clip));

  pipeline = ges_pipeline_new ();

  if (!ges_pipeline_add_timeline (pipeline, timeline))
    return -1;

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  mainloop = g_main_loop_new (NULL, FALSE);

  g_timeout_add_seconds (4, (GSourceFunc) g_main_loop_quit, mainloop);
  g_main_loop_run (mainloop);

  return 0;
}
