#include <kernel/kernel.h>
#include <config.h>
#include <SID.h>
#include <SkotOS.h>
#include <UserAPI.h>
#include <System.h>
#include <status.h>

static void create()
{
	call_out("die", 0);

	UDATD->patch();
	SYS_MERRY->patch();
	"~TextIF/main"->patch();
}

static void die()
{
	destruct_object(this_object());
}
