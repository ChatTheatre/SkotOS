object npcbody;

void die() {
   destruct_object();
}

void patch() {
   if (sscanf(ur_name(this_object()), "%*s#") && npcbody) {
      npcbody->possess(find_object("/usr/SkotOS/sys/npc-bodies"));
   }
   call_out("die", random(2));
}

string query_describer() { return "/usr/SkotOS/sys/npc-bodies"; }
