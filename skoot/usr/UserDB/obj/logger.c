# include <kernel/user.h>

inherit "/usr/System/lib/output_buffer";

static void create(int clone) {
    ::create();
}

void set_remote(string host, int port) {
    if (previous_program() == "/usr/UserDB/sys/ctld") {
    }
}

int login(string name) {
   if (previous_program() == LIB_CONN) {
      connection(previous_object());
      DEBUG("logger:login()");
      call_out("commit_suicide", 60);
      return MODE_LINE;
   }
}

void logout(int dest) {
   if (previous_program() == LIB_CONN) {
      DEBUG("logger:logout()");
      destruct_object();
   }
}

static void commit_suicide() {
    DEBUG("logger:commit_suicide()");
    disconnect();
    destruct_object();
}

int receive_message(string line) {
    if (previous_program() == LIB_CONN) {
	return call_limited("receive_line", line);
    }
}

static int receive_line(string line) {
    DEBUG("logger:receive_line:" + dump_value(line));
    return MODE_NOCHANGE;
}
