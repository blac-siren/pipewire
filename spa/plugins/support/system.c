/* Spa
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

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>

#include <spa/support/log.h>
#include <spa/support/system.h>
#include <spa/support/plugin.h>
#include <spa/utils/type.h>

#define NAME "system"

struct impl {
	struct spa_handle handle;
	struct spa_system system;

        struct spa_log *log;
};

static ssize_t impl_read(void *object, int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

static ssize_t impl_write(void *object, int fd, const void *buf, size_t count)
{
	return write(fd, buf, count);
}

static int impl_ioctl(void *object, int fd, unsigned long request, ...)
{
	int res;
	va_list ap;
	long arg;

	va_start(ap, request);
	arg = va_arg(ap, long);
	res = ioctl(fd, request, arg);
	va_end(ap);

	return res;
}

static int impl_close(void *object, int fd)
{
	return close(fd);
}

/* clock */
static int impl_clock_gettime(void *object,
			int clockid, struct timespec *value)
{
	return clock_gettime(clockid, value);
}

static int impl_clock_getres(void *object,
			int clockid, struct timespec *res)
{
	return clock_getres(clockid, res);
}

/* timers */
static int impl_timerfd_create(void *object, int clockid, int flags)
{
	int fl = 0;
	if (flags & SPA_FD_CLOEXEC)
		fl |= TFD_CLOEXEC;
	if (flags & SPA_FD_NONBLOCK)
		fl |= TFD_NONBLOCK;
	return timerfd_create(clockid, fl);
}

static int impl_timerfd_settime(void *object,
			int fd, int flags,
			const struct itimerspec *new_value,
			struct itimerspec *old_value)
{
	int fl = 0;
	if (flags & SPA_FD_TIMER_ABSTIME)
		fl |= TFD_TIMER_ABSTIME;
	if (flags & SPA_FD_TIMER_CANCEL_ON_SET)
		fl |= TFD_TIMER_CANCEL_ON_SET;
	return timerfd_settime(fd, fl, new_value, old_value);
}

static int impl_timerfd_gettime(void *object,
			int fd, struct itimerspec *curr_value)
{
	return timerfd_gettime(fd, curr_value);

}
static int impl_timerfd_read(void *object, int fd, uint64_t *expirations)
{
	int res;
	if ((res = read(fd, expirations, sizeof(uint64_t))) != sizeof(uint64_t))
		return -errno;
	return 0;
}

/* events */
static int impl_eventfd_create(void *object, int flags)
{
	int fl = 0;
	if (flags & SPA_FD_CLOEXEC)
		fl |= EFD_CLOEXEC;
	if (flags & SPA_FD_NONBLOCK)
		fl |= EFD_NONBLOCK;
	if (flags & SPA_FD_EVENT_SEMAPHORE)
		fl |= EFD_SEMAPHORE;
	return eventfd(0, fl);
}

static int impl_eventfd_write(void *object, int fd, uint64_t count)
{
	if (write(fd, &count, sizeof(uint64_t)) != sizeof(uint64_t))
		return -errno;
	return 0;
}

static int impl_eventfd_read(void *object, int fd, uint64_t *count)
{
	if (read(fd, count, sizeof(uint64_t)) != sizeof(uint64_t))
		return -errno;
	return 0;
}

/* signals */
static int impl_signalfd_create(void *object, int signal, int flags)
{
	sigset_t mask;
	int res, fl = 0;

	if (flags & SPA_FD_CLOEXEC)
		fl |= SFD_CLOEXEC;
	if (flags & SPA_FD_NONBLOCK)
		fl |= SFD_NONBLOCK;

	sigemptyset(&mask);
	sigaddset(&mask, signal);
	res = signalfd(-1, &mask, fl);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	return res;
}

static int impl_signalfd_read(void *object, int fd, int *signal)
{
	struct signalfd_siginfo signal_info;
	int len;

	len = read(fd, &signal_info, sizeof signal_info);
	if (!(len == -1 && errno == EAGAIN) && len != sizeof signal_info)
		return -errno;

	*signal = signal_info.ssi_signo;

	return 0;
}

static const struct spa_system_methods impl_system = {
	SPA_VERSION_SYSTEM_METHODS,
	.read = impl_read,
	.write = impl_write,
	.ioctl = impl_ioctl,
	.close = impl_close,
	.clock_gettime = impl_clock_gettime,
	.clock_getres = impl_clock_getres,
	.timerfd_create = impl_timerfd_create,
	.timerfd_settime = impl_timerfd_settime,
	.timerfd_gettime = impl_timerfd_gettime,
	.timerfd_read = impl_timerfd_read,
	.eventfd_create = impl_eventfd_create,
	.eventfd_write = impl_eventfd_write,
	.eventfd_read = impl_eventfd_read,
	.signalfd_create = impl_signalfd_create,
	.signalfd_read = impl_signalfd_read,
};

static int impl_get_interface(struct spa_handle *handle, uint32_t type, void **interface)
{
	struct impl *impl;

	spa_return_val_if_fail(handle != NULL, -EINVAL);
	spa_return_val_if_fail(interface != NULL, -EINVAL);

	impl = (struct impl *) handle;

	switch (type) {
	case SPA_TYPE_INTERFACE_System:
		*interface = &impl->system;
		break;
	default:
		return -ENOENT;
	}
	return 0;
}

static int impl_clear(struct spa_handle *handle)
{
	spa_return_val_if_fail(handle != NULL, -EINVAL);
	return 0;
}

static size_t
impl_get_size(const struct spa_handle_factory *factory,
	      const struct spa_dict *params)
{
	return sizeof(struct impl);
}

static int
impl_init(const struct spa_handle_factory *factory,
	  struct spa_handle *handle,
	  const struct spa_dict *info,
	  const struct spa_support *support,
	  uint32_t n_support)
{
	struct impl *impl;
	uint32_t i;

	spa_return_val_if_fail(factory != NULL, -EINVAL);
	spa_return_val_if_fail(handle != NULL, -EINVAL);

	handle->get_interface = impl_get_interface;
	handle->clear = impl_clear;

	impl = (struct impl *) handle;
	impl->system.iface = SPA_INTERFACE_INIT(
			SPA_TYPE_INTERFACE_System,
			SPA_VERSION_SYSTEM,
			&impl_system, impl);

	for (i = 0; i < n_support; i++) {
		if (support[i].type == SPA_TYPE_INTERFACE_Log)
			impl->log = support[i].data;
	}

	spa_log_debug(impl->log, NAME " %p: initialized", impl);

	return 0;
}

static const struct spa_interface_info impl_interfaces[] = {
	{SPA_TYPE_INTERFACE_System,},
};

static int
impl_enum_interface_info(const struct spa_handle_factory *factory,
			 const struct spa_interface_info **info,
			 uint32_t *index)
{
	spa_return_val_if_fail(factory != NULL, -EINVAL);
	spa_return_val_if_fail(info != NULL, -EINVAL);
	spa_return_val_if_fail(index != NULL, -EINVAL);

	if (*index >= SPA_N_ELEMENTS(impl_interfaces))
		return 0;

	*info = &impl_interfaces[(*index)++];
	return 1;
}

const struct spa_handle_factory spa_support_system_factory = {
	SPA_VERSION_HANDLE_FACTORY,
	NAME,
	NULL,
	impl_get_size,
	impl_init,
	impl_enum_interface_info
};