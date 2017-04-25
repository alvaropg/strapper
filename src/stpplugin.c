/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Strapper Plugin
 * Copyright (C) 2017 Álvaro Peña <alvaropg@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:strapper-plugin
 *
 * FIXME:Unzip a file.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! plugin ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <string.h>

#include "stpplugin.h"

#define CHUNK 16*1024

GST_DEBUG_CATEGORY_STATIC (stp_plugin_debug);
#define GST_CAT_DEFAULT stp_plugin_debug

enum
{
	LAST_SIGNAL
};

enum
{
	PROP_0
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
								    GST_PAD_SINK,
								    GST_PAD_ALWAYS,
								    GST_STATIC_CAPS ("ANY")
								    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
								   GST_PAD_SRC,
								   GST_PAD_ALWAYS,
								   GST_STATIC_CAPS ("ANY")
								   );

#define stp_plugin_parent_class parent_class
G_DEFINE_TYPE (StpPlugin, stp_plugin, GST_TYPE_ELEMENT);

static void stp_plugin_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void stp_plugin_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static gboolean	     stp_plugin_sink_event (GstPad *pad, GstObject *parent, GstEvent *event);
static GstFlowReturn stp_plugin_chain	   (GstPad *pad, GstObject *parent, GstBuffer *buf);

static void
stp_plugin_class_init (StpPluginClass *klass)
{
	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;

	gobject_class = (GObjectClass *) klass;
	gstelement_class = (GstElementClass *) klass;

	gobject_class->set_property = stp_plugin_set_property;
	gobject_class->get_property = stp_plugin_get_property;

	gst_element_class_set_details_simple(gstelement_class,
					     "Strapper",
					     "Decoder",
					     "Uncompress gzip files",
					     "Álvaro Peña <alvaropg@gmail.com>");

	gst_element_class_add_pad_template (gstelement_class,
					    gst_static_pad_template_get (&src_factory));
	gst_element_class_add_pad_template (gstelement_class,
					    gst_static_pad_template_get (&sink_factory));
}


static void
stp_plugin_init (StpPlugin *filter)
{
	filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
	gst_pad_set_event_function (filter->sinkpad, GST_DEBUG_FUNCPTR(stp_plugin_sink_event));
	gst_pad_set_chain_function (filter->sinkpad, GST_DEBUG_FUNCPTR(stp_plugin_chain));
	GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
	gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

	filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
	GST_PAD_SET_PROXY_CAPS (filter->srcpad);
	gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

	filter->stream = g_new0 (z_stream, 1);
	filter->stream->zalloc = Z_NULL;
	filter->stream->zfree = Z_NULL;
	filter->stream->opaque = Z_NULL;
	filter->stream->avail_in = 0;
	filter->stream->next_in = Z_NULL;
	inflateInit2 (filter->stream, 15 + 32);
}

static void
stp_plugin_set_property (GObject      *object,
			 guint	       prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
stp_plugin_get_property (GObject    *object,
			 guint	     prop_id,
			 GValue	    *value,
			 GParamSpec *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static gboolean
stp_plugin_sink_event (GstPad	 *pad,
		       GstObject *parent,
		       GstEvent	 *event)
{
	StpPlugin *filter;
	gboolean ret;

	filter = STP_PLUGIN (parent);

	GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT, GST_EVENT_TYPE_NAME (event), event);

	switch (GST_EVENT_TYPE (event)) {
	case GST_EVENT_CAPS:
		{
			GstCaps * caps;

			gst_event_parse_caps (event, &caps);
			/* do something with the caps */

			/* and forward */
			ret = gst_pad_event_default (pad, parent, event);
			break;
		}
	default:
		ret = gst_pad_event_default (pad, parent, event);
		break;
	}
	return ret;
}


static GstFlowReturn
stp_plugin_chain (GstPad    *pad,
		  GstObject *parent,
		  GstBuffer *buf)
{
	StpPlugin *filter;
	int ret;
	GstMapInfo map;
	GstMapInfo info;
	GstFlowReturn gst_return;
	guchar out[CHUNK];
	guint out_size = sizeof(out);
	guint have;
	GstBuffer *buffer_out;
	GstMemory *memory;

	gst_return = GST_FLOW_OK;

	filter = STP_PLUGIN (parent);

	if (!gst_buffer_map (buf, &map, GST_MAP_READ)) {
		GST_ERROR ("Error mapping buffer for read access: %" GST_PTR_FORMAT, buf);
		gst_return = GST_FLOW_ERROR;
		goto finish;
	}

	filter->stream->avail_in = map.size;
	filter->stream->next_in = map.data;
	while (filter->stream->avail_in) {
		filter->stream->avail_out = out_size;
		filter->stream->next_out = out;
		ret = inflate (filter->stream, Z_SYNC_FLUSH);
		if (ret == Z_STREAM_ERROR) {
			GST_ERROR("Error inflating");
			gst_return = GST_FLOW_ERROR;
			goto finish;
		}
		switch (ret) {
		case Z_NEED_DICT:
			GST_WARNING("Z_NEED_DICT");
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			GST_WARNING("Memory error");
			(void) inflateEnd (filter->stream);
			gst_return = GST_FLOW_ERROR;
			goto finish;
		}

		have = out_size - filter->stream->avail_out;

		buffer_out = gst_buffer_new ();
		memory = gst_allocator_alloc (NULL, have, NULL);
		gst_buffer_append_memory (buffer_out, memory);
		gst_buffer_map (buffer_out, &info, GST_MAP_WRITE);
		memcpy (info.data, out, have >= info.size ? info.size : have);
		gst_buffer_unmap (buffer_out, &info);

		gst_return = gst_pad_push (filter->srcpad, buffer_out);

	} while (filter->stream->avail_out == 0);

 finish:
	gst_buffer_unmap (buf, &map);
	return gst_return;
}


static gboolean
plugin_init (GstPlugin * plugin)
{
	GST_DEBUG_CATEGORY_INIT (stp_plugin_debug, "gzdec", 0, "Strapper plugin");

	return gst_element_register (plugin, "gzdec", GST_RANK_NONE, STP_TYPE_PLUGIN);
}


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   gzdec,
		   "Strapper plugin",
		   plugin_init,
		   VERSION,
		   "LGPL",
		   "GStreamer",
		   "http://gstreamer.net/")
