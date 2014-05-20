/*

  Author: Lubosz Sarnecki
  2013

*/

#ifndef GES_RENDERER_H
#define GES_RENDERER_H

#include <unistd.h>
#include <stdio.h>
#include <gst/gst.h>
#include <ges/ges.h>
#include <gst/pbutils/encoding-profile.h>
#include <gst/pbutils/encoding-target.h>

static const char *const profiles[][4] = {
  {"application/ogg", "audio/x-vorbis", "video/x-theora", "ogv"},
  {"video/webm", "audio/x-vorbis", "video/x-vp8", "webm"},
  {"video/quicktime,variant=iso", "audio/mpeg,mpegversion=1,layer=3",
      "video/x-h264", "mp4"},
  {"video/x-matroska", "audio/x-vorbis", "video/x-h264", "mkv"},
};

typedef enum
{
  PROFILE_NONE = -1,
  PROFILE_VORBIS_THEORA_OGG,
  PROFILE_VORBIS_VP8_WEBM,
  PROFILE_AAC_H264_QUICKTIME,
  PROFILE_VORBIS_H264_MATROSKA
} EncodingProfile;

typedef struct GESRendererProfile GESRendererProfile;
struct GESRendererProfile
{
  gint width;
  gint height;
  gint fps;
  EncodingProfile profile;
  const gchar* format;
};

void ges_timeline_save_xges(GESTimeline * timeline, const gchar * path);

gchar *ges_renderer_get_absolute_path (const char *rel_path);
gchar *ges_renderer_get_data_uri (void);
gchar *ges_renderer_get_absolute_path_win_multifile (const char *rel_path);

char *replace (char *s, char old, char replacement);

GESClip *ges_clip_unknown_from_rel_path (const gchar * get_path,
    GESLayer * layer, gint start, gint in, gint dur);
GESClip *ges_clip_from_path (const gchar * ges_renderer_get_absolute_path,
    GESLayer * layer, gint start, gint in, gint dur, GESTrackType tt);
GESClip *ges_clip_from_rel_path (const gchar * rel_path, GESLayer * layer,
    gint start, gint in, gint dur, GESTrackType tt);
GESClip *ges_multi_clip_from_path (const gchar * rel_path, GESLayer * layer,
    gint start, gint in, gint dur, gboolean absolute_paths);

void bus_message_cb (GstBus * bus, GstMessage * message, GMainLoop * mainloop);

GstEncodingProfile
    *ges_renderer_profile_get_encoding_profile (GESRendererProfile * profile);

gboolean ges_renderer_print_progress (void);

void ges_pipeline_setup_rendering (GESPipeline * pipeline,
    const gchar * name, GESRendererProfile * profile, gboolean absolute_paths);

GESPipeline *ges_pipeline_from_timeline (GESTimeline * timeline);
void ges_renderer_run_job (GESTimeline * timeline, const gchar * name,
    GESRendererProfile * profile, gboolean absolute_paths);

void ges_renderer_play (GESTimeline * timeline);
void ges_renderer_render (GESTimeline * timeline, const gchar * name,
    GESRendererProfile * profile, gboolean absolute_paths);

void ges_renderer_profile_print (GESRendererProfile * profile);

GstCaps *gst_caps_from_renderer_profile (GESRendererProfile * profile);

GESTimeline * ges_timeline_new_video(void);

gint ges_asset_get_structure_int (GESUriClipAsset * asset, const char *name);
gint ges_asset_get_width (GESUriClipAsset * asset);
gint ges_asset_get_height (GESUriClipAsset * asset);

void ges_renderer_init (void);

#endif // GES_RENDERER_H
