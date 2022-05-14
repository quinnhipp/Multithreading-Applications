# Multithreading-Applications
Implementation of POSIX-Pthreads to multi-thread Quicksort for sorting huge lists.

Project Requirements:
// project2 -n SIZE
// [-s THRESHOLD]
// [-a ALTERNATE]
// [-r SEED]
// [-m MULTITHREAD]
// [-p PIECES]
// [-t MAXTHREADS]
// [-m3 MEDIAN]
//
// where:
// SIZE is the number of items to sort (must be a positive 32-bit integer)
//
// THRESHOLD is an optional parameter that represents the threshold point (the size of the
// segment) at which you will switch from recursive Quicksort to the alternate
// sort. If the size of the segment is less than or equal THRESHOLD, use the
// alternate algorithm. This implies that if you start the program with the same
// value for SIZE and THRESHOLD, it will make one call into Quicksort, which
// will then sort the entire array using the alternate sort algorithm. At the
// other end of that spectrum, if THRESHOLD is set to zero, it will ALWAYS
// recursively Quicksort the entire array (all the way down to single items).
// If THRESHOLD is omitted, the default value is 10.
//
// ALTERNATE is an optional parameter that specifies what the alternate algorithm will be
// The default is 's' (Shell sort). The user can specify S, s, I, or i (I is for
// Insertion Sort).
//
// SEED is an optional (integer) parameter. If omitted, the random number generator
// is not seeded, and you run with whatever sequence it generates (note: it will
// generate the same sequence every time, so there will be the appearance of
// randomness WITHIN a run of the program, but no randomness BETWEEN runs.
// If SEED is specified (but is not -1), you will see the random number
// generator with the supplied value. If SEED is specified as -1, you will seed
// it with clock().
//
// MULTITHREAD is also optional, and will be a single character, used to determine whether
// or not to multithread the solution. If it is 'n' or 'N', simply call
// Quicksort on a single thread; otherwise, run it multithreaded. The default
// value is 'y'.
//
// PIECES is another optional parameter. PIECES Specifies how many partitions to
// divide the original list into. The default value is ten. If MULTITHREAD is
// 'n' or 'N' (or not specified), then setting PIECES has no effect
//
// MAXTHREADS is another optional parameter, which applies only if MULTITHREAD is 'y'. It
// specifies gives the number of threads to attempt to run at once, but
// MAXTHREADS must be no more than PIECES (we can't run 3 pieces on 4 threads).
// The default value for MAXTRHEADS is 4.
//
// MEDIAN is another optional y/n parameter, which determines whether each segment
// will be partitioned using the MEDIAN-OF-THREE technique. The default is 'n'.
//
The command-line arguments can appear in any order, but they must be valid before your program can
continue. All parameters have integer arguments, except MULTITHREAD and MEDIAN, which must be one
of {Y,y,N,n}. If any parameter is invalid, display an error message and exit immediately
