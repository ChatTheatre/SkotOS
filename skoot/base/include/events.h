# include <nref.h>

object get_result_object();
object query_result_object();
void flush_result_object();
void clear_result_object();

int pre_signal(string signal, mapping a, varargs mapping b, mapping c);
int desc_signal(string signal, mapping a, varargs mapping b, mapping c);
int prime_signal(string signal, mapping a, varargs mapping b, mapping c);
int new_signal(string signal, mapping a, varargs mapping b, mapping c);
void post_signal(string signal, mapping a, varargs mapping b, mapping c);
int run_signal(string signal, varargs NRef target, mapping args);
int special_signal(string namespace, string signal, varargs mapping args);

void broadcast_event(string event, object body, mixed args...);
void broadcast_locally(string event, object body, mixed args...);

void broadcast(string event, mixed args...);
void occur(string event, mixed args...);
