#include <kernel/kernel.h>
#include <kernel/rsrc.h>
#include <kernel/objreg.h>
#include <config.h>
#include <SID.h>
#include <System.h>
#include <status.h>

inherit rsrc API_RSRC;
inherit oreg API_OBJREG;

static void create()
{
	string *owners;
	int sz;

	rsrc::create();
	oreg::create();

	owners = query_owners();

	for (sz = sizeof(owners); --sz >= 0; ) {
		object first;
		string owner;

		owner = owners[sz];

		first = first_link(owners[sz]);

		call_out("scan_owner", 0, owner, first, first);
	}
}

private void check_thing(object obj)
{
	if (!obj->query_object_name()) {
		obj->painful_death((float)random(1000 * 30) * 0.001);
	}
}

private void check_object(object obj)
{
	string program;

	program = object_name(obj);
	sscanf(program, "%s#", program);

	if (sscanf(program, "/usr/SkotOS/merry/%*s")) {
		destruct_object(program);
		return;
	}

	switch(program) {
	case "/base/obj/thing":
		check_thing(obj);
		break;
	}
}

static void scan_owner(string owner, object first, object obj)
{
	object next;

	if (first_link(owner) != first) {
		DRIVER->message((owner ? owner : "Ecru") + " list changed, aborting scan.\n");
		return;
	}

	if (!obj) {
		DRIVER->message((owner ? owner : "Ecru") + ": lost iterator, aborting scan.\n");
		return;
	}

	next = next_link(obj);

	check_object(obj);

	if (next != first) {
		call_out("scan_owner", 0, owner, first, next);
	} else {
		DRIVER->message((owner ? owner : "Ecru") + " list preened.\n");
	}
}
