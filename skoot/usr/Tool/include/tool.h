/*
 * ~Tool/include/tool.h
 *
 * Copyright Skotos Tech Inc 2002, 2003.
 */

# ifndef __TOOL_H
# define __TOOL_H

/*** From /usr/TextIF: ***/

# define HOST_PARSER   "/usr/TextIF/sys/parser/host"

/*** Local Tool defines: ***/

# define DEFAULT_WIDTH 76

# define SYS_TOOL      "/usr/Tool/sys/tool"
# define EDITOR_OBJ    "/usr/System/obj/editor"
# define SCRATCH_PATH  "/usr/Tool/tmp/"

# define WOE_HELP_DATA "Tool:Data:Help"

/*** For syntax colouring: ***/

# define CC_COMMENT(_str) ({ "comment", _str })
# define CC_ERROR(_str)   ({ "error",   _str })
# define CC_KEYWORD(_str) ({ "keyword", _str })
# define CC_LINENR(_str)  ({ "linenr",  _str })
# define CC_STRING(_str)  ({ "string",  _str })
# define CC_TYPE(_str)    ({ "type",    _str })
# define CC_CPP(_str)     ({ "cpp",     _str })

# endif
