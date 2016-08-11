# include <System.h>

# ifndef LOGNAME
#  define LOGNAME	nil
# endif

/* The different priorities */
# define LOG_DEBUG    1
# define LOG_INFO     2
# define LOG_WARNING  3
# define LOG_ERROR    4
# define LOG_CRITICAL 5

/* The basic functionality. */
# define SYSLOG(priority,_str) (SYSLOGD->syslogd((priority), (_str)))

/* The derived functionality. */
# define INFO(_str)     SYSLOG(LOG_INFO, (_str))
# define DEBUG(_str)    SYSLOG(LOG_DEBUG, (_str))
# define WARNING(_str)  SYSLOG(LOG_WARNING, (_str))
# define ERROR(_str)    SYSLOG(LOG_ERROR, (_str))
# define CRITICAL(_str) SYSLOG(LOG_CRITICAL, (_str))

/* Backward compatibility. */
# define SysLog(_str)   DEBUG(_str)
# define D(_str,_n)     if (_n <= SYSLOGD->query_my_debug_level()) { \
                            SYSLOGD->syslogd_debug(LOGNAME, (_str)); \
                        }
# define Debug(_str)	D((_str), 1)
# define XDebug(_str)	D((_str), 2)
