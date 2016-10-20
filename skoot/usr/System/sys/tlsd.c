#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/tls.h>

private inherit API_TLS;

static void create()
{
	::create();
}

mixed query_tls_value(string domain, string key)
{
	mapping heap;
	mapping dmap;

	heap = get_tlvar(3);

	if (!heap) {
		return nil;
	}

	dmap = heap[domain];

	if (!dmap) {
		return nil;
	}

	return dmap[key];
}

void set_tls_value(string domain, string key, mixed value)
{
	mapping heap;
	mapping dmap;

	heap = get_tlvar(3);

	if (!heap) {
		if (value == nil) {
			return;
		} else {
			heap = ([ ]);
		}
	}

	dmap = heap[domain];

	if (!dmap) {
		if (value == nil) {
			return;
		} else {
			dmap = ([ ]);
		}
	}

	dmap[key] = value;

	if (map_sizeof(dmap) == 0) {
		dmap = nil;
	}

	heap[domain] = dmap;

	if (map_sizeof(heap) == 0) {
		heap = nil;
	}

	set_tlvar(3, heap);
}
