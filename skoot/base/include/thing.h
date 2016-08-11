# include <nref.h>

nomask int move(object dest, varargs NRef prox);
nomask NRef prime_reference(varargs object looker);

object *query_souls();

object query_disconnect_room();
void set_disconnect_room(object ob);
void clear_disconnect_room();

float query_amount();

object query_possessor();
object query_possessee();
atomic void possess_body(object body);
atomic void break_possession();
