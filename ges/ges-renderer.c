/*

  Author: Lubosz Sarnecki
  2013

*/

#include "ges-renderer.h"

#include <stdlib.h>

static GESPipeline *pipeline = NULL;
static GstClockTime duration;
static gboolean wasError = FALSE;

char *
replace (char *s, char old, char replacement)
{
  char *p = s;

  while (*p) {
    if (*p == old)
      *p = replacement;
    ++p;
  }
  return s;
}

void
ges_renderer_init (void)
{
  duration = 0;
}

gchar *
ges_renderer_get_data_uri (void)
{
  gchar *data_path;
  gchar directory[1024];
  getcwd (directory, 1024);
#ifdef PLATTFORM_WINDOWS
  char *replaced = replace (directory, '\\', '/');
  data_path = g_strconcat ("file:///", replaced, "/data/", NULL);
#else
  data_path = g_strconcat ("file://", &directory, "/data/", NULL);
#endif
  return data_path;
}

gchar *
ges_renderer_get_absolute_path_win_multifile (const char *rel_path)
{
  gchar *data_path;
  char *replaced;
  gchar directory[1024];
  getcwd (directory, 1024);
  replaced = replace (directory, '\\', '/');
  data_path = g_strconcat ("file://", replaced, "/data/", NULL);

  return g_strconcat (data_path, rel_path, NULL);
}

gchar *
ges_renderer_get_absolute_path (const char *rel_path)
{
  return g_strconcat (ges_renderer_get_data_uri (), rel_path, NULL);
}

gint
ges_asset_get_structure_int (GESUriClipAsset * asset, const char *name)
{
  GstDiscovererInfo *info = ges_uri_clip_asset_get_info (asset);
  GstDiscovererStreamInfo *stream_info =
      gst_discoverer_info_get_stream_info (info);
  GstCaps *caps = gst_discoverer_stream_info_get_caps (stream_info);
  GstStructure *structure = gst_caps_get_structure (caps, 0);
  gint value;
  gst_structure_get_int (structure, name, &value);
  return value;
}

gint
ges_asset_get_width (GESUriClipAsset * asset)
{
  return ges_asset_get_structure_int (asset, "width");
}

gint
ges_asset_get_height (GESUriClipAsset * asset)
{
  return ges_asset_get_structure_int (asset, "height");
}

GESClip *
ges_clip_unknown_from_rel_path (const gchar * path, GESLayer * layer,
    gint start, gint in, gint dur)
{
  return ges_clip_from_rel_path (path, layer, start, in, dur,
      GES_TRACK_TYPE_UNKNOWN);
}

GESClip *
ges_multi_clip_from_path (const gchar * rel_path, GESLayer * layer,
    gint start, gint in, gint dur, gboolean absolute_paths)
{
  const gchar *path = NULL;
  gchar *multi_path;

  if (absolute_paths) {
    path = rel_path;
  } else {
#ifdef PLATTFORM_WINDOWS
    path = ges_renderer_get_absolute_path_win_multifile (rel_path);
#else
    path = ges_renderer_get_absolute_path (rel_path);
#endif
  }

  multi_path = g_strconcat ("multi", path, NULL);

  return ges_clip_from_path (multi_path, layer, start, in, dur,
      GES_TRACK_TYPE_VIDEO);
}

void
ges_timeline_save_xges (GESTimeline * timeline, const gchar * path)
{
  GESAsset *formatter_asset =
      ges_asset_request (GES_TYPE_XML_FORMATTER, NULL, NULL);
  GError *error = NULL;

  ges_timeline_save_to_uri (timeline, path, formatter_asset, TRUE, &error);

  if (error != NULL) {
    fprintf (stderr, "Unable to save xges to %s: %s\n", path, error->message);
    g_error_free (error);
    exit (0);
  }
}


GESClip *
ges_clip_from_rel_path (const gchar * rel_path, GESLayer * layer, gint start,
    gint in, gint dur, GESTrackType tt)
{
  return ges_clip_from_path (ges_renderer_get_absolute_path (rel_path), layer,
      start, in, dur, tt);
}

GESClip *
ges_clip_from_path (const gchar * path, GESLayer * layer, gint start, gint in,
    gint dur, GESTrackType tt)
{
  GError *error = NULL;
  GESUriClipAsset *asset;
  GESClip *clip;

  asset = ges_uri_clip_asset_request_sync (path, &error);

  if (error != NULL) {
    fprintf (stderr, "Unable to read asset: %s\n", error->message);
    g_print ("path %s\n", path);
    g_error_free (error);
    exit (0);
  }

  clip = ges_layer_add_asset (layer, GES_ASSET (asset),
      start * GST_SECOND, in * GST_SECOND, dur * GST_SECOND, tt);

  gst_object_unref (asset);

  return clip;
}

void
bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop)
{
  GError *err = NULL;
  gchar *dbg_info = NULL;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      //TODO: Log error message to file
      wasError = TRUE;

      gst_message_parse_error (message, &err, &dbg_info);
      g_printerr ("\n\nERROR from element %s: %s\n",
          GST_OBJECT_NAME (message->src), err->message);
      g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
      g_error_free (err);
      g_free (dbg_info);

      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
          GST_DEBUG_GRAPH_SHOW_ALL, "ges-renderer-error");

      g_main_loop_quit (mainloop);
      break;
    }
    case GST_MESSAGE_EOS:{
      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
          GST_DEBUG_GRAPH_SHOW_ALL, "ges-renderer-eos");
      g_print ("\nDone\n");
      g_main_loop_quit (mainloop);
      break;
    }
    default:
      break;
  }
}

GstCaps *
gst_caps_from_renderer_profile (GESRendererProfile * profile)
{
  GstCaps *caps;
  char capsstring[50];
  sprintf (capsstring,
      "video/x-raw,width=%d,height=%d,framerate=%d/1",
      profile->width, profile->height, profile->fps);

  caps = gst_caps_from_string (capsstring);

  if (profile->format != NULL) {
    gchar *capsstring_format =
        g_strconcat (capsstring, ",format=", profile->format, NULL);
    caps = gst_caps_from_string (capsstring_format);
    g_print ("format caps %s\n", capsstring_format);
  }


  return caps;
}

void
ges_renderer_profile_print (GESRendererProfile * profile)
{
  EncodingProfile type = profile->profile;
  g_print ("Format: %s\n  Container: %s\n  Video:     %s \n  Audio:     %s\n",
      profiles[type][3],
      profiles[type][0], profiles[type][2], profiles[type][1]);
}

GstEncodingProfile *
ges_renderer_profile_get_encoding_profile (GESRendererProfile * profile)
{
  GstEncodingContainerProfile *prof;
  GstCaps *caps;
  GstCaps *settings;
  EncodingProfile type = profile->profile;

  ges_renderer_profile_print (profile);

  caps = gst_caps_from_string (profiles[type][0]);
  prof =
      gst_encoding_container_profile_new ("Profile", "A web video profile",
      caps, NULL);
  gst_caps_unref (caps);

  caps = gst_caps_from_string (profiles[type][2]);

  settings = gst_caps_from_renderer_profile (profile);

  gst_encoding_container_profile_add_profile (prof,
      (GstEncodingProfile *) gst_encoding_video_profile_new (caps, NULL,
          settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  caps = gst_caps_from_string (profiles[type][1]);
  settings = gst_caps_from_string ("audio/x-raw");
  gst_encoding_container_profile_add_profile (prof,
      (GstEncodingProfile *) gst_encoding_audio_profile_new (caps, NULL,
          settings, 0));
  gst_caps_unref (caps);
  gst_caps_unref (settings);

  return (GstEncodingProfile *) prof;
}

gboolean
ges_renderer_print_progress (void)
{
  gint64 position = 0;
  float percent;
  float positionSec;
  float durationSec;

  gst_element_query_position (GST_ELEMENT (pipeline),
      GST_FORMAT_TIME, &position);

  percent = (float) position *100 / (float) duration;

  //TODO: error when pos > dur

  positionSec = (float) position / GST_SECOND;
  durationSec = (float) duration / GST_SECOND;

  if (position > 0)
    g_print ("\r%.2f%% %.2f/%.2fs", percent, positionSec, durationSec);

  return TRUE;
}

void
ges_pipeline_setup_rendering (GESPipeline * pipeline,
    const gchar * name, GESRendererProfile * profile, gboolean absolute_paths)
{
  EncodingProfile type = profile->profile;
  GstEncodingProfile *gst_profile;

  const gchar *fileName;

  if (absolute_paths == TRUE) {
    fileName = name;
  } else {
    fileName =
        g_strconcat (ges_renderer_get_data_uri (), "export/", name, ".",
        profiles[type][3], NULL);
  }
  g_print ("Rendering %s\n", fileName);

  gst_profile = ges_renderer_profile_get_encoding_profile (profile);
  ges_pipeline_set_render_settings (pipeline, fileName, gst_profile);
  ges_pipeline_set_mode (pipeline, GES_PIPELINE_MODE_RENDER);
}

GESPipeline *
ges_pipeline_from_timeline (GESTimeline * timeline)
{
  GESPipeline *pipeline;
  pipeline = ges_pipeline_new ();
  ges_pipeline_set_timeline (pipeline, timeline);

  duration = ges_timeline_get_duration (timeline);

  return pipeline;
}

GESTimeline *
ges_timeline_new_video (void)
{
  GESTrack *trackv;
  GESTimeline *timeline;
  timeline = ges_timeline_new ();
  trackv = GES_TRACK (ges_video_track_new ());
  ges_timeline_add_track (timeline, trackv);
  return timeline;
}

void
ges_renderer_run_job (GESTimeline * timeline, const gchar * name,
    GESRendererProfile * profile, gboolean absolute_paths)
{
  GstBus *bus;
  GMainLoop *mainloop = g_main_loop_new (NULL, FALSE);

  //set restriction caps
  GList *tracks = ges_timeline_get_tracks (timeline);
  GESTrack *trackv = g_list_first (tracks)->data;

  if (GES_IS_VIDEO_TRACK (trackv)) {
    GstCaps *caps = gst_caps_from_renderer_profile (profile);
    ges_track_set_restriction_caps (trackv, caps);
    g_print ("restiction caps: %s\n", gst_caps_to_string (caps));
  } else {
    g_print ("Not a video track");
  }

  pipeline = ges_pipeline_from_timeline (timeline);

  if (name != NULL) {
    ges_pipeline_setup_rendering (pipeline, name, profile, absolute_paths);
  } else {
    ges_pipeline_set_mode (pipeline, GES_PIPELINE_MODE_PREVIEW_VIDEO);
    g_timeout_add_seconds (duration, (GSourceFunc) g_main_loop_quit, mainloop);
  }

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  g_signal_connect (bus, "message", (GCallback) bus_message_cb, mainloop);
  g_timeout_add (100, (GSourceFunc) ges_renderer_print_progress, NULL);
  gst_bus_add_signal_watch (bus);
  gst_object_unref (bus);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (pipeline),
      GST_DEBUG_GRAPH_SHOW_ALL, "ges-renderer");

  g_main_loop_run (mainloop);
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  g_main_loop_unref (mainloop);


  gst_object_unref (pipeline);
}

void
ges_renderer_play (GESTimeline * timeline)
{
  gchar *name = NULL;
  GESRendererProfile profile = { 0, 0, 0, PROFILE_NONE };
  ges_renderer_run_job (timeline, name, &profile, FALSE);
}

void
ges_renderer_render (GESTimeline * timeline, const gchar * name,
    GESRendererProfile * profile, gboolean absolute_paths)
{
  const gchar *exitStatus = "Rendering";
  float now = (float) g_get_monotonic_time () / (float) GST_MSECOND;
  float then;
  float dur;

  g_print ("\n====\n");
  ges_renderer_run_job (timeline, name, profile, absolute_paths);
  then = (float) g_get_monotonic_time () / (float) GST_MSECOND;
  dur = then - now;
  g_print ("\n====\n");

  if (wasError)
    exitStatus = "Error";

  g_print ("%s took %.2fs\n", exitStatus, dur);
  g_print ("====\n");
}
