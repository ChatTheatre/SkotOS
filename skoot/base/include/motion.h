# include <nref.h>

nomask object	query_environment();
nomask NRef	query_proximity();
nomask int	move(object ob, varargs NRef prox);
nomask int	contained_by(object container);
nomask object	query_holder();
