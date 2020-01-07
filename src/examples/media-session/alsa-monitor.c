/* PipeWire
 *
 * Copyright © 2019 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "config.h"

#include <alsa/asoundlib.h>

#include <spa/monitor/device.h>
#include <spa/node/node.h>
#include <spa/node/keys.h>
#include <spa/utils/result.h>
#include <spa/utils/hook.h>
#include <spa/utils/names.h>
#include <spa/utils/keys.h>
#include <spa/param/props.h>
#include <spa/pod/builder.h>
#include <spa/debug/dict.h>
#include <spa/debug/pod.h>
#include <spa/support/dbus.h>

#include <pipewire/pipewire.h>
#include <pipewire/main-loop.h>
#include <extensions/session-manager.h>

#include "media-session.h"

#include "reserve.c"

#define DEFAULT_JACK_SECONDS	1

struct node {
	struct impl *impl;
	enum pw_direction direction;
	struct device *device;
	struct spa_list link;
	uint32_t id;

	struct pw_properties *props;

	struct spa_node *node;

	struct sm_node *snode;
};

struct device {
	struct impl *impl;
	struct spa_list link;
	uint32_t id;
	uint32_t device_id;

	struct rd_device *reserve;
	struct spa_hook sync_listener;
	int seq;
	int priority;

	int profile;

	struct pw_properties *props;

	struct spa_handle *handle;
	struct spa_device *device;
	struct spa_hook device_listener;

	struct sm_device *sdevice;
	struct spa_hook listener;

	unsigned int first:1;
	unsigned int appeared:1;
	struct spa_list node_list;
};

struct impl {
	struct sm_media_session *session;
	struct spa_hook session_listener;

	DBusConnection *conn;

	struct spa_handle *handle;

	struct spa_device *monitor;
	struct spa_hook listener;

	struct spa_list device_list;

	struct spa_source *jack_timeout;
	struct pw_proxy *jack_device;
};

#undef NAME
#define NAME "alsa-monitor"

static struct node *alsa_find_node(struct device *device, uint32_t id)
{
	struct node *node;

	spa_list_for_each(node, &device->node_list, link) {
		if (node->id == id)
			return node;
	}
	return NULL;
}

static void alsa_update_node(struct device *device, struct node *node,
		const struct spa_device_object_info *info)
{
	pw_log_debug("update node %u", node->id);

	if (pw_log_level_enabled(SPA_LOG_LEVEL_DEBUG))
		spa_debug_dict(0, info->props);

	pw_properties_update(node->props, info->props);
}

static struct node *alsa_create_node(struct device *device, uint32_t id,
		const struct spa_device_object_info *info)
{
	struct node *node;
	struct impl *impl = device->impl;
	int res;
	const char *dev, *subdev, *stream;
	int priority;

	pw_log_debug("new node %u", id);

	if (strcmp(info->type, SPA_TYPE_INTERFACE_Node) != 0) {
		errno = EINVAL;
		return NULL;
	}
	node = calloc(1, sizeof(*node));
	if (node == NULL) {
		res = -errno;
		goto exit;
	}

	node->props = pw_properties_new_dict(info->props);

	pw_properties_setf(node->props, PW_KEY_DEVICE_ID, "%d", device->device_id);

	pw_properties_set(node->props, "factory.name", info->factory_name);

	if ((dev = pw_properties_get(node->props, SPA_KEY_API_ALSA_PCM_DEVICE)) == NULL)
		dev = "0";
	if ((subdev = pw_properties_get(node->props, SPA_KEY_API_ALSA_PCM_SUBDEVICE)) == NULL)
		subdev = "0";
	if ((stream = pw_properties_get(node->props, SPA_KEY_API_ALSA_PCM_STREAM)) == NULL)
		stream = "unknown";

	if (!strcmp(stream, "capture"))
		node->direction = PW_DIRECTION_OUTPUT;
	else
		node->direction = PW_DIRECTION_INPUT;

	if (device->first) {
		if (atol(dev) != 0)
			device->priority -= 256;
		device->first = false;
	}

	priority = device->priority;
	if (!strcmp(stream, "capture"))
		priority += 1000;
	priority -= atol(dev) * 16;
	priority -= atol(subdev);

	if (pw_properties_get(node->props, PW_KEY_PRIORITY_MASTER) == NULL) {
		pw_properties_setf(node->props, PW_KEY_PRIORITY_MASTER, "%d", priority);
		pw_properties_setf(node->props, PW_KEY_PRIORITY_SESSION, "%d", priority);
	}

	if (pw_properties_get(node->props, SPA_KEY_MEDIA_CLASS) == NULL) {
		if (node->direction == PW_DIRECTION_OUTPUT)
			pw_properties_setf(node->props, SPA_KEY_MEDIA_CLASS, "Audio/Source");
		else
			pw_properties_setf(node->props, SPA_KEY_MEDIA_CLASS, "Audio/Sink");
	}
	if (pw_properties_get(node->props, SPA_KEY_NODE_NAME) == NULL) {
		const char *devname;
		if ((devname = pw_properties_get(device->props, SPA_KEY_DEVICE_NAME)) == NULL)
			devname = "unknown";
		pw_properties_setf(node->props, SPA_KEY_NODE_NAME, "%s.%s.%s.%s",
				devname, stream, dev, subdev);
	}
	if (pw_properties_get(node->props, PW_KEY_NODE_DESCRIPTION) == NULL) {
		const char *desc, *name = NULL;

		if ((desc = pw_properties_get(device->props, SPA_KEY_DEVICE_DESCRIPTION)) == NULL)
			desc = "unknown";

		name = pw_properties_get(node->props, SPA_KEY_API_ALSA_PCM_NAME);
		if (name == NULL)
			name = pw_properties_get(node->props, SPA_KEY_API_ALSA_PCM_ID);
		if (name == NULL)
			name = dev;

		if (strcmp(subdev, "0")) {
			pw_properties_setf(node->props, PW_KEY_NODE_DESCRIPTION, "%s (%s %s)",
					desc, name, subdev);
		} else if (strcmp(dev, "0")) {
			pw_properties_setf(node->props, PW_KEY_NODE_DESCRIPTION, "%s (%s)",
					desc, name);
		} else {
			pw_properties_setf(node->props, PW_KEY_NODE_DESCRIPTION, "%s",
					desc);
		}
	}

	node->impl = impl;
	node->device = device;
	node->id = id;
	node->snode = sm_media_session_create_node(impl->session,
				"adapter",
				&node->props->dict);
	if (node->snode == NULL) {
		res = -errno;
		goto clean_node;
	}

	spa_list_append(&device->node_list, &node->link);

	return node;

clean_node:
	pw_properties_free(node->props);
	free(node);
exit:
	errno = -res;
	return NULL;
}

static void alsa_remove_node(struct device *device, struct node *node)
{
	pw_log_debug("remove node %u", node->id);
	spa_list_remove(&node->link);
	sm_object_destroy(&node->snode->obj);
	pw_properties_free(node->props);
	free(node);
}

static void alsa_device_info(void *data, const struct spa_device_info *info)
{
	struct device *device = data;

	if (pw_log_level_enabled(SPA_LOG_LEVEL_DEBUG))
		spa_debug_dict(0, info->props);

	pw_properties_update(device->props, info->props);
}

static void alsa_device_object_info(void *data, uint32_t id,
                const struct spa_device_object_info *info)
{
	struct device *device = data;
	struct node *node;

	node = alsa_find_node(device, id);

	if (info == NULL) {
		if (node == NULL) {
			pw_log_warn("device %p: unknown node %u", device, id);
			return;
		}
		alsa_remove_node(device, node);
	} else if (node == NULL) {
		alsa_create_node(device, id, info);
	} else {
		alsa_update_node(device, node, info);
	}
}

static const struct spa_device_events alsa_device_events = {
	SPA_VERSION_DEVICE_EVENTS,
	.info = alsa_device_info,
	.object_info = alsa_device_object_info
};

static struct device *alsa_find_device(struct impl *impl, uint32_t id)
{
	struct device *device;

	spa_list_for_each(device, &impl->device_list, link) {
		if (device->id == id)
			return device;
	}
	return NULL;
}

static void alsa_update_device(struct impl *impl, struct device *device,
		const struct spa_device_object_info *info)
{
	pw_log_debug("update device %u", device->id);

	if (pw_log_level_enabled(SPA_LOG_LEVEL_DEBUG))
		spa_debug_dict(0, info->props);

	pw_properties_update(device->props, info->props);
}

static int update_device_props(struct device *device)
{
	struct pw_properties *p = device->props;
	const char *s, *d;
	char temp[32];

	if ((s = pw_properties_get(p, SPA_KEY_DEVICE_NAME)) == NULL) {
		if ((s = pw_properties_get(p, SPA_KEY_DEVICE_BUS_ID)) == NULL) {
			if ((s = pw_properties_get(p, SPA_KEY_DEVICE_BUS_PATH)) == NULL) {
				snprintf(temp, sizeof(temp), "%d", device->id);
				s = temp;
			}
		}
	}
	pw_properties_setf(p, PW_KEY_DEVICE_NAME, "alsa_card.%s", s);

	if (pw_properties_get(p, PW_KEY_DEVICE_DESCRIPTION) == NULL) {
		d = NULL;

		if ((s = pw_properties_get(p, PW_KEY_DEVICE_FORM_FACTOR)))
			if (strcmp(s, "internal") == 0)
				d = "Built-in Audio";
		if (!d)
			if ((s = pw_properties_get(p, PW_KEY_DEVICE_CLASS)))
				if (strcmp(s, "modem") == 0)
					d = "Modem";
		if (!d)
			d = pw_properties_get(p, PW_KEY_DEVICE_PRODUCT_NAME);

		if (!d)
			d = "Unknown device";

		pw_properties_set(p, PW_KEY_DEVICE_DESCRIPTION, d);
	}

	if (pw_properties_get(p, PW_KEY_DEVICE_ICON_NAME) == NULL) {
		d = NULL;

		if ((s = pw_properties_get(p, PW_KEY_DEVICE_FORM_FACTOR))) {
			if (strcmp(s, "microphone") == 0)
				d = "audio-input-microphone";
			else if (strcmp(s, "webcam") == 0)
				d = "camera-web";
			else if (strcmp(s, "computer") == 0)
				d = "computer";
			else if (strcmp(s, "handset") == 0)
				d = "phone";
			else if (strcmp(s, "portable") == 0)
				d = "multimedia-player";
			else if (strcmp(s, "tv") == 0)
				d = "video-display";
			else if (strcmp(s, "headset") == 0)
				d = "audio-headset";
			else if (strcmp(s, "headphone") == 0)
				d = "audio-headphones";
			else if (strcmp(s, "speaker") == 0)
				d = "audio-speakers";
			else if (strcmp(s, "hands-free") == 0)
				d = "audio-handsfree";
		}
		if (!d)
			if ((s = pw_properties_get(p, PW_KEY_DEVICE_CLASS)))
				if (strcmp(s, "modem") == 0)
					d = "modem";

		if (!d)
			d = "audio-card";

		s = pw_properties_get(p, PW_KEY_DEVICE_BUS);

		pw_properties_setf(p, PW_KEY_DEVICE_ICON_NAME,
				"%s-analog%s%s", d, s ? "-" : "", s);
	}
	return 1;
}

static void set_jack_profile(struct impl *impl, int index)
{
	char buf[1024];
	struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buf, sizeof(buf));

	if (impl->jack_device == NULL)
		return;

	pw_device_set_param((struct pw_device*)impl->jack_device,
			SPA_PARAM_Profile, 0,
			spa_pod_builder_add_object(&b,
				SPA_TYPE_OBJECT_ParamProfile, SPA_PARAM_Profile,
				SPA_PARAM_PROFILE_index,   SPA_POD_Int(index)));
}

static void set_profile(struct device *device, int index)
{
	char buf[1024];
	struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buf, sizeof(buf));

	pw_log_debug("%p: set profile %d id:%d", device, index, device->device_id);

	device->profile = index;
	if (device->device_id != 0) {
		spa_device_set_param(device->device,
				SPA_PARAM_Profile, 0,
				spa_pod_builder_add_object(&b,
					SPA_TYPE_OBJECT_ParamProfile, SPA_PARAM_Profile,
					SPA_PARAM_PROFILE_index,   SPA_POD_Int(index)));
	}
}

static void remove_jack_timeout(struct impl *impl)
{
	struct pw_loop *main_loop = impl->session->loop;

	if (impl->jack_timeout) {
		pw_loop_destroy_source(main_loop, impl->jack_timeout);
		impl->jack_timeout = NULL;
	}
}

static void jack_timeout(void *data, uint64_t expirations)
{
	struct impl *impl = data;
	remove_jack_timeout(impl);
	set_jack_profile(impl, 1);
}

static void add_jack_timeout(struct impl *impl)
{
	struct timespec value;
	struct pw_loop *main_loop = impl->session->loop;

	if (impl->jack_timeout == NULL)
		impl->jack_timeout = pw_loop_add_timer(main_loop, jack_timeout, impl);

	value.tv_sec = DEFAULT_JACK_SECONDS;
	value.tv_nsec = 0;
	pw_loop_update_timer(main_loop, impl->jack_timeout, &value, NULL, false);
}

static void reserve_acquired(void *data, struct rd_device *d)
{
	struct device *device = data;
	struct impl *impl = device->impl;

	pw_log_debug("%p: reserve acquired", device);

	remove_jack_timeout(impl);
	set_jack_profile(impl, 0);
	set_profile(device, 1);
}

static void sync_complete_done(void *data, int seq)
{
	struct device *device = data;
	struct impl *impl = device->impl;

	pw_log_debug("%d %d", device->seq, seq);
	if (seq != device->seq)
		return;

	spa_hook_remove(&device->sync_listener);
	device->seq = 0;

	rd_device_complete_release(device->reserve, true);

	add_jack_timeout(impl);
}

static void sync_destroy(void *data)
{
	struct device *device = data;
	if (device->seq != 0)
		sync_complete_done(data, device->seq);
}

static const struct pw_proxy_events sync_complete_release = {
	PW_VERSION_PROXY_EVENTS,
	.destroy = sync_destroy,
	.done = sync_complete_done
};

static void reserve_release(void *data, struct rd_device *d, int forced)
{
	struct device *device = data;
	struct impl *impl = device->impl;

	pw_log_debug("%p: reserve release", device);

	remove_jack_timeout(impl);
	set_profile(device, 0);

	if (device->seq == 0)
		pw_proxy_add_listener(device->sdevice->obj.proxy,
				&device->sync_listener,
				&sync_complete_release, device);
	device->seq = pw_proxy_sync(device->sdevice->obj.proxy, 0);
}

static const struct rd_device_callbacks reserve_callbacks = {
	.acquired = reserve_acquired,
	.release = reserve_release,
};

static void device_destroy(void *data)
{
	struct device *device = data;
	struct node *node;

	pw_log_debug("device %p destroy", device);

	spa_list_consume(node, &device->node_list, link)
		alsa_remove_node(device, node);
}

static void device_update(void *data)
{
	struct device *device = data;

	pw_log_debug("device %p appeared %d %d", device, device->appeared, device->profile);

	if (device->appeared)
		return;

	device->device_id = device->sdevice->obj.id;
	device->appeared = true;

	spa_device_add_listener(device->device,
		&device->device_listener,
		&alsa_device_events, device);

	set_profile(device, device->profile);
	sm_object_sync_update(&device->sdevice->obj);
}

static const struct sm_object_events device_events = {
	SM_VERSION_OBJECT_EVENTS,
        .destroy = device_destroy,
        .update = device_update,
};

static struct device *alsa_create_device(struct impl *impl, uint32_t id,
		const struct spa_device_object_info *info)
{
	struct pw_context *context = impl->session->context;
	struct device *device;
	struct spa_handle *handle;
	int res;
	void *iface;
	const char *card;

	pw_log_debug("new device %u", id);

	if (strcmp(info->type, SPA_TYPE_INTERFACE_Device) != 0) {
		errno = EINVAL;
		return NULL;
	}

	handle = pw_context_load_spa_handle(context,
			info->factory_name,
			info->props);
	if (handle == NULL) {
		res = -errno;
		pw_log_error("can't make factory instance: %m");
		goto exit;
	}

	if ((res = spa_handle_get_interface(handle, info->type, &iface)) < 0) {
		pw_log_error("can't get %s interface: %s", info->type, spa_strerror(res));
		goto unload_handle;
	}

	device = calloc(1, sizeof(*device));
	if (device == NULL) {
		res = -errno;
		goto unload_handle;
	}

	device->impl = impl;
	device->id = id;
	device->handle = handle;
	device->device = iface;
	device->props = pw_properties_new_dict(info->props);
	device->priority = 1000;
	update_device_props(device);

	device->sdevice = sm_media_session_export_device(impl->session,
			&device->props->dict, device->device);
	if (device->sdevice == NULL) {
		res = -errno;
		goto clean_device;
	}

	sm_object_add_listener(&device->sdevice->obj,
			&device->listener,
			&device_events, device);

	if ((card = spa_dict_lookup(info->props, SPA_KEY_API_ALSA_CARD)) != NULL) {
		const char *reserve;

		pw_properties_setf(device->props, "api.dbus.ReserveDevice1", "Audio%s", card);
		reserve = pw_properties_get(device->props, "api.dbus.ReserveDevice1");

		device->reserve = rd_device_new(impl->conn, reserve,
				"PipeWire", 10,
				&reserve_callbacks, device);

		if (device->reserve == NULL) {
			pw_log_warn("can't create device reserve for %s: %m", reserve);
		} else {
			rd_device_set_application_device_name(device->reserve,
				spa_dict_lookup(info->props, SPA_KEY_API_ALSA_PATH));
		}
		device->priority -= atol(card) * 64;
	}

	/* no device reservation, activate device right now */
	if (device->reserve == NULL)
		set_profile(device, 1);

	device->first = true;
	spa_list_init(&device->node_list);
	spa_list_append(&impl->device_list, &device->link);

	return device;

clean_device:
	free(device);
unload_handle:
	pw_unload_spa_handle(handle);
exit:
	errno = -res;
	return NULL;
}

static void alsa_remove_device(struct impl *impl, struct device *device)
{
	pw_log_debug("remove device %u", device->id);
	spa_list_remove(&device->link);
	if (device->appeared)
		spa_hook_remove(&device->device_listener);
	if (device->seq != 0)
		spa_hook_remove(&device->sync_listener);
	if (device->reserve)
		rd_device_destroy(device->reserve);
	if (device->sdevice)
		sm_object_destroy(&device->sdevice->obj);
	spa_hook_remove(&device->listener);
	pw_unload_spa_handle(device->handle);
	pw_properties_free(device->props);
	free(device);
}

static void alsa_udev_object_info(void *data, uint32_t id,
                const struct spa_device_object_info *info)
{
	struct impl *impl = data;
	struct device *device;

	device = alsa_find_device(impl, id);

	if (info == NULL) {
		if (device == NULL)
			return;
		alsa_remove_device(impl, device);
	} else if (device == NULL) {
		if ((device = alsa_create_device(impl, id, info)) == NULL)
			return;
	} else {
		alsa_update_device(impl, device, info);
	}
}

static const struct spa_device_events alsa_udev_events =
{
	SPA_VERSION_DEVICE_EVENTS,
	.object_info = alsa_udev_object_info,
};

static int alsa_start_jack_device(struct impl *impl)
{
	struct pw_properties *props;
	int res = 0;

	props = pw_properties_new(
			SPA_KEY_FACTORY_NAME, SPA_NAME_API_JACK_DEVICE,
			SPA_KEY_NODE_NAME, "JACK-Device",
			NULL);

	impl->jack_device = sm_media_session_create_object(impl->session,
				"spa-device-factory",
				PW_TYPE_INTERFACE_Device,
				PW_VERSION_DEVICE,
				&props->dict,
                                0);

	if (impl->jack_device == NULL)
		res = -errno;

	pw_properties_free(props);

	return res;
}

static void session_destroy(void *data)
{
	struct impl *impl = data;
	spa_hook_remove(&impl->session_listener);
	spa_hook_remove(&impl->listener);
	pw_proxy_destroy(impl->jack_device);
	pw_unload_spa_handle(impl->handle);
	free(impl);
}

static const struct sm_media_session_events session_events = {
	SM_VERSION_MEDIA_SESSION_EVENTS,
	.destroy = session_destroy,
};

int sm_alsa_monitor_start(struct sm_media_session *session)
{
	struct pw_context *context = session->context;
	struct impl *impl;
	void *iface;
	int res;

	impl = calloc(1, sizeof(struct impl));
	if (impl == NULL)
		return -errno;

	impl->session = session;

	if (session->dbus_connection)
		impl->conn = spa_dbus_connection_get(session->dbus_connection);
	if (impl->conn == NULL)
		pw_log_warn("no dbus connection, device reservation disabled");
	else
		pw_log_debug("got dbus connection %p", impl->conn);

	impl->handle = pw_context_load_spa_handle(context, SPA_NAME_API_ALSA_ENUM_UDEV, NULL);
	if (impl->handle == NULL) {
		res = -errno;
		goto out_free;
	}

	if ((res = spa_handle_get_interface(impl->handle, SPA_TYPE_INTERFACE_Device, &iface)) < 0) {
		pw_log_error("can't get udev Device interface: %d", res);
		goto out_unload;
	}
	impl->monitor = iface;
	spa_list_init(&impl->device_list);
	spa_device_add_listener(impl->monitor, &impl->listener, &alsa_udev_events, impl);

	if ((res = alsa_start_jack_device(impl)) < 0)
		goto out_unload;

	sm_media_session_add_listener(session, &impl->session_listener, &session_events, impl);

	return 0;

out_unload:
	pw_unload_spa_handle(impl->handle);
out_free:
	free(impl);
	return res;
}
