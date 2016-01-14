private inherit "/lib/loader";

static
void create() {
   compile_object("/usr/SAM/sys/sam");
   compile_object("/usr/SAM/data/unsam");
   compile_object("/usr/SAM/data/context");

   load_programs("/usr/SAM/samtags");
   load_programs("/usr/SAM/sys");
}
