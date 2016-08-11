# define COMPAT_CODE
# define USR			"/usr"	/* default user directory */
# define USR_DIR		"/usr"	/* default user directory */
# define INHERITABLE_SUBDIR	"/lib/"
# define CLONABLE_SUBDIR	"/obj/"
# define LIGHTWEIGHT_SUBDIR	"/data/"

# define SYS_CONTINUOUS		/* off by default */ /* but on for us */
# define SYS_PERSISTENT		/* new name for SYS_CONTINUOUS */
# define SYS_DATAGRAMS		/* off by default */ /* but on for us */
# define TLS_SIZE	2	/* thread local storage */

# define SYS_NETWORKING		/* ichat server has networking capabilities */

# define CALLOUTRSRC	TRUE	/* do have callouts as a resource */

# ifdef SYS_NETWORKING
#  define TELNET_PORT	6047	/* default telnet port */
#  define BINARY_PORT	6048	/* default binary port */
# endif

# define CREATOR	"_F_sys_create"
