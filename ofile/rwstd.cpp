// This is only needed for linking multi-threaded Rogue Wave STL.
//New for 1.4
#include "odefs.h"
#include "ofile.h"

#ifdef RWSTD_MULTI_THREAD
RWSTDMutex __stl_tree_mutex;
#endif
