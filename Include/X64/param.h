#define DEV_BSHIFT 9
#define DEV_BSIZE (1 << DEV_BSHIFT)
#define dbtob(x)        ((x) << DEV_BSHIFT)
#define btodb(x)        ((x) >> DEV_BSHIFT)
#define NBBY	8
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)      ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)      (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)