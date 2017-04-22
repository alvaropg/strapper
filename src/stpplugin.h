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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __STP_PLUGIN_H__
#define __STP_PLUGIN_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define STP_TYPE_PLUGIN (stp_plugin_get_type())
#define STP_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),STP_TYPE_PLUGIN,StpPlugin))
#define STP_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),STP_TYPE_PLUGIN,StpPluginClass))
#define STP_IS_PLUGIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),STP_TYPE_PLUGIN))
#define STP_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),STP_TYPE_PLUGIN))

typedef struct _StpPlugin      StpPlugin;
typedef struct _StpPluginClass StpPluginClass;

struct _StpPlugin
{
	GstElement element;
	GstPad *sinkpad, *srcpad;

	gchar *filename;
};

struct _StpPluginClass 
{
	GstElementClass parent_class;
};

GType stp_plugin_get_type (void);

G_END_DECLS

#endif /* __STP_PLUGIN_H__ */
