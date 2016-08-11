/*
 * ~Tool/sys/generic.c
 *
 * Parser for misc +tool commands.
 *
 * Copyright Skotos Tech Inc 2002, 2005.
 */

# include "~/include/tool.h"
# include <TextIF.h>

private inherit "/lib/string";
        inherit "~/lib/generic";

static void
create()
{
   SYS_TOOL->register_handler_ob(this_object());
}

mixed **
query_grammar()
{
   return ({
      ({ "quote",                                 "cmd_quote"                      }),
      ({ "quote to <text>",                       "cmd_quote", "2"                 }),
      ({ "quote <string>",                        "cmd_quote_string", "1"          }),

      ({ "ide <object>",                          "cmd_ide", "1"                   }),
      ({ "popup",                                 "cmd_popup",                     }),
      ({ "popup <object>",                        "cmd_popup_object", "1" }),
      ({ "popup <object> <text>",                 "cmd_popup_property", "1", "2" }),
      ({ "profiler",                              "cmd_profiler_info"              }),
      ({ "profiler commands",                     "cmd_profiler_textif"            }),

      ({ "profiler merry",                        "cmd_profiler_merry"             }),
      ({ "profiler merry daily",                  "cmd_profiler_merry", 0, 1       }),

      ({ "profiler merry <number>",               "cmd_profiler_merry", "2"        }),
      ({ "profiler merry daily <number>",         "cmd_profiler_merry", "3", 1     }),

      ({ "profiler merry archives",               "cmd_profiler_archives"          }),
      ({ "profiler merry daily archives",         "cmd_profiler_archives", 1       }),

      ({ "profiler merry archive <number>",       "cmd_profiler_archive", "3"      }),
      ({ "profiler merry daily archive <number>", "cmd_profiler_archive", "4", 1   }),

      ({ "profiler merry <object>",               "cmd_profiler_object", "2"       }),
      ({ "profiler merry daily <object>",         "cmd_profiler_object", "3", 1    }),

      ({ "profiler socials",                      "cmd_profiler_socials", 0        }),
      ({ "profiler socials <number>",             "cmd_profiler_socials", 0, "2"   }),

      ({ "profiler socials hourly",               "cmd_profiler_socials", 1        }),
      ({ "profiler socials hourly <number>",      "cmd_profiler_socials", 1, "3"   }),

      ({ "profiler socials daily",                "cmd_profiler_socials", 2        }),
      ({ "profiler socials daily <number>",       "cmd_profiler_socials", 2, "3"   }),

      ({ "status",                                "cmd_status"                     }),
      ({ "status <object>",                       "cmd_status_object", "1"         }),

      ({ "convert",                               "cmd_convert"                    }),
      ({ "convert <anything>",                    "cmd_convert", "1"               }),
   });
}
