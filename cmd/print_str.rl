/* Drops excessive whitespace inside multi-word tokens. */

%%{

machine print_str;

include whitespace "../src/fsm/whitespace.rl";

alphtype unsigned char;

main := (whitespace+ @ { putchar(' '); } | any @ { putchar(fc); })*;

}%%

%% write data noerror nofinal;

static void print_str(const unsigned char *p, size_t len)
{
   int cs;
   const unsigned char *const pe = &p[len];
   const unsigned char *const eof = pe;

   %% write init;
   %% write exec;
   
   (void)eof;
}
