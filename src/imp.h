#ifndef MR_IMP_H
#define MR_IMP_H

#define local static

struct mascara;
struct mr_token;

struct mr_imp {
   void (*set_text)(struct mascara *, const unsigned char *, size_t, size_t);
   size_t (*next)(struct mascara *, struct mr_token **);
   void (*fini)(struct mascara *);  /* Can be = 0. */
};

struct mascara {
   const struct mr_imp *imp;
};

#endif
