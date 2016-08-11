void broadcast(string event, mixed args...);
void action(string action, varargs mapping args, int flags);

static void insert_action(string action, varargs mapping args, int flags);
static void repeat_action(varargs string new_state);

# define ObArg(s)	(args[s])
# define StrArg(s)	(args[s])
# define NRefArg(s)	(args[s])
# define ArrArg(s)	(args[s])
# define MapArg(s)	(args[s])
# define Arg(s)		(args[s])
# define IntArg(s)	(args[s])


/* macros for the result object */

# define Add_Error(a, b, c)	{ 	\
      result->add_to_error(a,b,c);	\
}

# define Set_Error(a, b, c)	{ 	\
      result->set_error(a,b,c);		\
}
      
# define Clear_Error()		{ 	\
      result->clear_error();		\
}
