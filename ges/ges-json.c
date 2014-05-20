
#ifdef PLATTFORM_WINDOWS
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ges-json.h"
#include "ges-renderer.h"

const gchar *
getString (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  const char *value = json_reader_get_string_value (reader);
  json_reader_end_member (reader);
  return value;
}

const gdouble
getDouble (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  double value = json_reader_get_double_value (reader);
  json_reader_end_member (reader);
  return value;
}

const int
getInt (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  int value = json_reader_get_int_value (reader);
  json_reader_end_member (reader);
  return value;
}

const gboolean
getBool (JsonReader * reader, const gchar * member_name)
{
  json_reader_read_member (reader, member_name);
  gboolean value = json_reader_get_boolean_value (reader);
  json_reader_end_member (reader);
  return value;
}

void
getClips (JsonReader * reader, GESLayer * layer, GESTrackType type,
    gboolean absolute_paths)
{
  int i;
  json_reader_read_member (reader, "clips");

  g_print ("= clips =\n");

  for (i = 0; i < json_reader_count_elements (reader); i++) {
    json_reader_read_element (reader, i);
    const char *src = getString (reader, "src");
    int start = getInt (reader, "start");
    int in = getInt (reader, "in");
    int dur = getInt (reader, "dur");
    g_print ("Clip: %s (start: %d, in: %d, dur: %d)\n", src, start, in, dur);

    GESClip *clip;

    if (is_in_members (reader, "multi") && getBool (reader, "multi")) {
      g_print ("multi on.\n");
      clip =
          ges_multi_clip_from_path (src, layer, start, in, dur, absolute_paths);
    } else {
      const char *path;
      if (absolute_paths == TRUE) {
        path = src;
      } else {
        path = ges_renderer_get_absolute_path (src);
      }
      clip = ges_clip_from_path (path, layer, start, in, dur, type);
    }

    GESTimeline *tl = ges_layer_get_timeline (layer);
    GList *tracks = ges_timeline_get_tracks (tl);
    GESTrack *trackv = g_list_first (tracks)->data;
    GESTrack *tracka = g_list_last (tracks)->data;

    if (is_in_members (reader, "volume")) {
      double volume = getDouble (reader, "volume");
      GESTrackElement *audioElement =
          ges_clip_find_track_element (clip, tracka, G_TYPE_NONE);
      if (audioElement != NULL) {
        ges_track_element_set_child_properties (audioElement, "volume", volume,
            NULL);
      }
    }

    GESTrackElement *videoElement =
        ges_clip_find_track_element (clip, trackv, G_TYPE_NONE);

    if (videoElement != NULL) {
      if (is_in_members (reader, "x")) {
        int x = getInt (reader, "x");
        ges_track_element_set_child_properties (videoElement, "posx", x, NULL);
      }
      if (is_in_members (reader, "y")) {
        int y = getInt (reader, "y");
        ges_track_element_set_child_properties (videoElement, "posy", y, NULL);
      }
      if (is_in_members (reader, "alpha")) {
        gdouble alpha = getDouble (reader, "alpha");
        ges_track_element_set_child_properties (videoElement, "alpha", alpha,
            NULL);
      }

      if (is_in_members (reader, "size")) {
        gdouble size = getDouble (reader, "size");
        GESUriClipAsset *asset =
            GES_URI_CLIP_ASSET (ges_extractable_get_asset (GES_EXTRACTABLE
                (clip)));
        guint width = ges_asset_get_width (asset);
        guint height = ges_asset_get_height (asset);

        if (width != 0 && height != 0) {
          double dw = width * size;
          double dh = height * size;
          g_print ("%dx%d => * %f => %dx%d\n", width, height, size, (int) dw,
              (int) dh);
          ges_track_element_set_child_properties (videoElement,
              "width", (int) dw, "height", (int) dh, NULL);
        }
      }

      if (is_in_members (reader, "effect")) {
        const char *effect_str = getString (reader, "effect");
        if (strcmp (effect_str, "") != 0) {
          g_print ("Using effect %s", effect_str);
          GESEffect *effect = ges_effect_new (effect_str);
          ges_container_add (GES_CONTAINER (clip),
              GES_TIMELINE_ELEMENT (effect));
        }
      }
    }

    json_reader_end_element (reader);
  }
  json_reader_end_member (reader);
}

gboolean
is_in_members (JsonReader * reader, const char *member)
{
  gchar **members = json_reader_list_members (reader);
  int member_size = json_reader_count_members (reader);

  for (int i = 0; i < member_size; i++) {
    if (strcmp (members[i], member) == 0) {
      //g_print("found member: %s = %s\n", members[i], member);
      return TRUE;
    }

  }
  return FALSE;
}

void
render_json (const char *filename)
{
  JsonParser *parser;
  JsonNode *root;
  GError *error;
  parser = json_parser_new ();

  error = NULL;
  json_parser_load_from_file (parser, filename, &error);
  if (error) {
    g_print ("Parsing error `%s':\n %s\n", filename, error->message);
    g_error_free (error);
    g_object_unref (parser);
    exit (0);
  }

  root = json_parser_get_root (parser);

  JsonReader *reader = json_reader_new (root);
  GESTimeline *timeline;

  json_reader_read_member (reader, "composition");

  // comp strings
  const char *name = getString (reader, "name");
  //const char *src_dir = getString (reader, "src-dir");
  //g_print ("Source Directory: %s\nName: %s\n", src_dir, name);

  // comp ints
  int width = getInt (reader, "width");
  int height = getInt (reader, "height");
  int fps = getInt (reader, "fps");

  gboolean transparency = TRUE;

  if (is_in_members (reader, "transparency")) {
    transparency = getBool (reader, "transparency");
  }

  gboolean absolute_paths = FALSE;

  if (is_in_members (reader, "absolute_paths")) {
    absolute_paths = getBool (reader, "absolute_paths");
  }

  g_print ("Resolution: %dx%d, FPS: %d\n", width, height, fps);

  timeline = ges_timeline_new_audio_video ();

  int i;
  json_reader_read_member (reader, "layers");

  for (i = 0; i < json_reader_count_elements (reader); i++) {
    json_reader_read_element (reader, i);

    GESLayer *layer = ges_layer_new ();

    g_object_set (layer, "priority", i, NULL);

    if (is_in_members (reader, "autotransition")) {
      gboolean autotransition = getBool (reader, "autotransition");
      if (autotransition)
        g_print ("Auto Transitions on.\n");
      g_object_set (layer, "auto-transition", autotransition, NULL);
    }

    ges_timeline_add_layer (timeline, layer);

    getClips (reader, layer, GES_TRACK_TYPE_UNKNOWN, absolute_paths);

    json_reader_end_element (reader);
  }
  json_reader_end_member (reader);

  ges_timeline_commit (timeline);

  const gchar *xges_path = g_strconcat ("file://", filename, ".xges", NULL);

  ges_timeline_save_xges (timeline, xges_path);

  //free(xges_path);

  // formats
  GESRendererProfile res =
      { width, height, fps, PROFILE_AAC_H264_QUICKTIME, NULL };
  if (!transparency) {
    g_print ("Deactivating transparency\n");
    res.format = "I420";
  }

  json_reader_read_member (reader, "formats");
  for (i = 0; i < json_reader_count_elements (reader); i++) {
    json_reader_read_element (reader, i);
    const char *format = json_reader_get_string_value (reader);
    json_reader_end_element (reader);
    g_print ("format: %s\n", format);
    EncodingProfile prof = PROFILE_AAC_H264_QUICKTIME;
    if (strcmp (format, "webm") == 0) {
      prof = PROFILE_VORBIS_VP8_WEBM;
    } else if (strcmp (format, "mkv") == 0) {
      prof = PROFILE_VORBIS_H264_MATROSKA;
    } else if (strcmp (format, "mp4") == 0) {
      prof = PROFILE_AAC_H264_QUICKTIME;
    } else if (strcmp (format, "ogg") == 0) {
      prof = PROFILE_VORBIS_THEORA_OGG;
    }
    res.profile = prof;
    ges_renderer_render (timeline, name, &res, absolute_paths);
  }
  json_reader_end_member (reader);

  json_reader_end_member (reader);

  g_object_unref (reader);
  g_object_unref (parser);
}

int
main (int argc, char *argv[])
{
#ifdef PLATTFORM_WINDOWS
  LoadLibrary ("exchndl.dll");
#endif
  if (argc < 2) {
    g_print ("Usage: ./ges-json <filename.json>\n");
    return EXIT_FAILURE;
  }
  gst_init (&argc, &argv);
  ges_init ();

  ges_renderer_init ();

  render_json (argv[1]);

  return EXIT_SUCCESS;
}
