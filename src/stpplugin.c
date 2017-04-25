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

#include "stpplugin.h"

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
					     "Plugin",
					     "FIXME:Generic",
					     "FIXME:Generic Template Element",
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

	filter->filename = NULL;
}

static void
stp_plugin_set_property (GObject      *object,
			 guint         prop_id,
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
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static gboolean
stp_plugin_sink_event (GstPad    *pad,
		       GstObject *parent,
		       GstEvent  *event)
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

	filter = STP_PLUGIN (parent);

	/* just push out the incoming buffer without touching it */
	return gst_pad_push (filter->srcpad, buf);
}


static gboolean
plugin_init (GstPlugin * plugin)
{
	GST_DEBUG_CATEGORY_INIT (stp_plugin_debug, "plugin", 0, "Strapper plugin");

	return gst_element_register (plugin, "plugin", GST_RANK_NONE, STP_TYPE_PLUGIN);
}


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   plugin,
		   "Strapper plugin",
		   plugin_init,
		   VERSION,
		   "LGPL",
		   "GStreamer",
		   "http://gstreamer.net/")
