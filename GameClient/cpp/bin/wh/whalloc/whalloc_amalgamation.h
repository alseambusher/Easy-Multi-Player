/* auto-generated on Sat Nov 27 12:05:38 CET 2010. Do not edit! */
#define WHALLOC_AMALGAMATION_BUILD 1
#if ! defined __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#if defined(__cplusplus) && !defined(restrict)
#define restrict
#endif
/* begin file whalloc_namespace.h */
#line 8 "whalloc_namespace.h"
#ifndef WANDERINGHORSE_NET_WHALLOC_NAMESPACE_H_INCLUDED
#define WANDERINGHORSE_NET_WHALLOC_NAMESPACE_H_INCLUDED 1
/** @def WHALLOC_API

    WHALLOC_API must be a macro taking a single token which is suitable
    as part of a function or type name. It is used to construct the
    type/function names used by this API because i have several use cases
    in mind where i want to compile different variants with different
    names.

    The use of this construct is probably going to confuse doxygen :/.
    To filter the API while doxygen is processing it, add this line to
    your Doxyfile:

    @code
    INPUT_FILTER = "perl -pe 's|WHALLOC_API\(([^)]+)\)|TOKEN_$1|g;'"
    @endcode

    Where "TOKEN_" is the prefix defined by this macro.
    
*/
#define WHALLOC_API(X) whalloc_ ## X

#endif /* WANDERINGHORSE_NET_WHALLOC_NAMESPACE_H_INCLUDED */
/* end file whalloc_namespace.h */
/* begin file whalloc.h */
#line 8 "whalloc.h"
#ifndef WANDERINGHORSE_NET_WHALLOC_POOL_H_INCLUDED
#define WANDERINGHORSE_NET_WHALLOC_POOL_H_INCLUDED 1

#if defined(__cplusplus) &&  !defined(__STDC_FORMAT_MACROS)
#  define __STDC_FORMAT_MACROS /* used by C++ for enabling PRIxNN defines. */
#endif


#include <stdint.h> /* fixed-size int types. */
#include <stdio.h> /* FILE*/
#include <inttypes.h> /* PRIxNN printf-specifier macros for the standard int types. */
#ifdef __cplusplus
extern "C" {
#endif
    /** @page whalloc_page_main whalloc memory allocation API

    The whalloc API is home to several "alternative memory allocators"
    for C. That is, it provides functions similar to malloc() and
    free(), but which use a client-provided memory range for all
    allocations. It is particularly intended to be used for routines
    which need to allocate lots of small objects, but where calling
    malloc() dozens (or hundreds) of times is not appropriate.

    It was originally based off of code found here:

    http://pastebin.com/f207f6232

    which was PRESUMABLY originally written by Chris M. Thomasson, as his
    name was on the pastebin.com post and the code had no credits.

    This variant was written by Stephan Beal
    (http://wanderinghorse.net/home/stephan), with input from Amir
    "Ashran" Mohammadkhani (http://modme.info), and has been extended
    in many ways (see below).

    License: This code is Public Domain in jurisdictions which
    acknowledge Public Domain property. For other jurisdictions it is
    released under the terms of the MIT Licenses (which is about as
    close to Public Domain as a license can get). The MIT License is
    compatible with the GPL and with commercial licenses.

    The more signiciant classes are:

    - WHALLOC_API(bt): an almost-general-purpose allocator,
    particularly useful for apps which want to place a hard cap on the
    amount of memory they can allocate.

    - WHALLOC_API(page) and WHALLOC_API(book): special-purpose
    allocators for doling out memory blocks of a fixed size and
    re-using deallocated objects.

    The WHALLOC_API(ht) class, prevelantly seen in the API docs, is
    considered obsoleted by the WHALLOC_API(bt) class, and might be
    removed from this API.
    */

    /** @page whalloc_page_portability whalloc Portability Notes


    Code Portability:

    The core whalloc library (not necessarily the test code) compiles
    cleaning in either C89/C90 or C99 modes, but it requires the
    C99-standard stdint.h and inttypes.h headers for the standard
    fixed-size integers and their corresponding standardized printf
    format specifiers. If those are not available, the user will have
    to hack a bit to remove the offending headers and provide
    equivalent typedefs. Much of this code relies in part on
    fixed-sized integer types.

    Memory Alignment:

    Most of the API does not do anything special to support "proper"
    alignment of memory allocated via this API. This may prove
    problematic on platforms which care about objects being aligned on
    some boundary.

    The following API members provide at least some way to guaranty a
    specific alignment:

    - See WHALLOC_API(bt_init)() for how to initialize a WHALLOC_API(bt)
    such that it always returns properly aligned pointers.

    - The WHALLOC_API(region) _can_ provide alignment guarantees IF
    the client initializes it using a properly aligned buffer AND all
    allocations are of a size which is a multiple of the platform's
    alignment.
    */
    
    /** @def WHALLOC_BITNESS

        WHALLOC_BITNESS must be set to one of 8, 16, or 32. It
        determines the overall memory layout. 16 bits is a good
        trade-off for space efficiency in the internal tables.
    */

    /** @def WHALLOC_BITNESS_OVERRIDE

        WHALLOC_BITNESS_OVERRIDE is an internal-use only macro to
        allow us to build seperate bitnesses of the library for
        testing purpose without changing this file. It should NOT be
        used by clients unless they are rebuilding this library along
        with their application/library build.
    */

  /** @def WHALLOC_MASK

      WHALLOC_MASK must be a bitmask where the right-most
      WHALLOC_BITNESS bits are on and all other bits (if there are
      more bits) are off. This is used by some of the allocators
      in constructing hashing masks.
   */

  /** @def WHALLOC_SIZE_T_PFMT

      WHALLOC_SIZE_T_PFMT is the equivalent of one of the standard
      PRIuNNN macros from <inttypes.h>, where NNN is the same as
      WHALLOC_BITNESS. It should be used instead of a hard-coded
      specifier in printf-like strings so that the code will compile
      cleanly on machines with different word sizes. e.g.  do not use
      percent-u directly, use this instead. If you do not, compilation
      in certain environments will warn about the width of the type
      vis-a-vis the expected format specifier. Particularly
      problematic in this regard are size_t objects, as size_t does
      not have a fixed size - a format specifier which compiles
      cleanly on 32-bit machines might fail loudly on 64-bit machines
      (and vice versa). Thus the fixed-size integers (and their
      standard format specifier macros) should generally be preferred
      over size_t (and other non-fixed-size integer types).
   */
#if defined(WHALLOC_BITNESS_OVERRIDE)
#    define WHALLOC_BITNESS (WHALLOC_BITNESS_OVERRIDE)
#else
#    define WHALLOC_BITNESS_OVERRIDE 0 /* only to keep doxygen happy. */
#    define WHALLOC_BITNESS 16
#endif
#if 8 == WHALLOC_BITNESS
    typedef uint8_t WHALLOC_API(size_t);
#   define WHALLOC_MASK 0x00ff
#   define WHALLOC_SIZE_T_PFMT PRIu8
#elif 16 == WHALLOC_BITNESS
    typedef uint16_t WHALLOC_API(size_t);
#   define WHALLOC_MASK 0x0000ffff
#   define WHALLOC_SIZE_T_PFMT PRIu16
#elif 32 == WHALLOC_BITNESS
    typedef uint32_t WHALLOC_API(size_t);
#   define WHALLOC_MASK 0xffffffff
#   define WHALLOC_SIZE_T_PFMT PRIu32
#elif 64 == WHALLOC_BITNESS
    typedef uint64_t WHALLOC_API(size_t);
#   define WHALLOC_MASK ((WHALLOC_API(size_t))0xffffffffffffffff)
#   define WHALLOC_SIZE_T_PFMT PRIu64
#else
#  error "WHALLOC_BITNESS must be one of (8,16,32)!"
#endif
    /**
       Routines in this API which return an int value will almost
       always return a value described in terms of this type's
       members. They are accessed via the WHALLOC_API(rc) object.

       All
       of them have unique, but unspecified, non-zero values except:

       -  OK must be 0.

       - HashCodeError is ((WHALLOC_API(size_t))-1).

       - None of the integer members may have the value -1, to allow
       us to distinguish error codes from some system-level routines
       which return -1 and update errno on error.
    */
    struct WHALLOC_API(rc_t)
    {
        /** Always == 0. */
        int OK;
        /** Signals that some value was out of range. */
        int RangeError;
        /** Signals that an argument was of an unexpected value. */
        int ArgError;
        /** Signals that an internal error was found. */
        int InternalError;
        /** Signals an error related to the internal hashtable. */
        int HashingError;
        /** Signals a resource allocation error. */
        int AllocError;
        /** Signals a usage error. */
        int UsageError;
        /** Signals that some sort of data or state consistency
            problem was discovered. */
        int ConsistencyError;
        /** Signals that some form of mutex un/locking error was encountered.*/
        int LockingError;

        /** Always == ((WHALLOC_API(size_t))-1), but the *exact* value of
            -1 differs depending on the size of WHALLOC_API(size_t).
        */
        WHALLOC_API(size_t) HashCodeError;
    };
    typedef struct WHALLOC_API(rc_t) WHALLOC_API(rc_t);
    /**
       Functions in the whalloc API which return an integer
       error code will return one of the values defined in
       WHALLOC_API(rc_t). Clients can use this object to get
       those values.
    */
    extern const WHALLOC_API(rc_t) WHALLOC_API(rc);

    /**
       A mutex class used by some allocator classes.

       To activate the mutex for a class which contains one of these
       object, assign all of the members to appropriate values.
       You must either set neither or both of the lock() and unlock()
       members. The state member may be null if the lock/unlock functions
       do not use it.
    */
    struct WHALLOC_API(mutex)
    {
        /**
           Intended to lock a mutex. This interface does not require
           that it be a recursive lock, but it may need to be so if
           the client uses the same mutex for outer-level locking
           regarding a given allocator. It is passed the state member
           as its only parameter.

           Must return 0 on success, non-zero on error.

           API routines for allocators which support a mutex should
           return an error code if the mutex cannot be locked.
        */
        int (*lock)( void * );
        /**
           Intended to unlock the mutex locked by lock(). It is passed
           the state member as its only parameter.

           If lock is non-null then unluck must also be non-null and
           complementary to the lock function.

           Must return 0 on success, non-zero on error.
           
           BUGS: most (or all) of the allocator code ignores the
           return code from the unlock routine because they cannot
           sensibly "undo" the result of a re/de/allocation if
           unlocking fails.
        */
        int (*unlock)( void * );
        /** Arbitrary state data which should be passed to lock() and unlock(),
            e.g. a platform-specific mutex type.
        */
        void * state;
    };
    typedef struct WHALLOC_API(mutex) WHALLOC_API(mutex);

    /**
       An empty WHALLOC_API(mutex) initialization object.
    */
    extern const WHALLOC_API(mutex) WHALLOC_API(mutex_empty);

    /** @def whalloc_mutex_empty_m

       An empty WHALLOC_API(mutex) initialization object, for use when
       static allocation is necessary.
    */
#define whalloc_mutex_empty_m {0/*lock*/,0/*unlock*/,0/*data*/}

    /**
       WHALLOC_API(realloc_f) is a typedef for de/re/allocation
       routines semantically compatible with realloc(), but with the
       addition of a third parameter which can be used to pass
       allocator-specific state.

       This interface is used for allocating memory for use by the
       WHALLOC_API(page) and WHALLOC_API(book) classes. The intention
       is to allow clients to use their own allocators to de/allocate
       WHALLOC_API(page) and WHALLOC_API(book) objects.

       The allocState parameter should hold any state needed for the
       custom de/re/allocation algorithm. Ownership of that state object
       is completely dependent on the allocator and its rules, but it is
       typically a pointer to an allocator object with arbitrary
       allocator-internal data (e.g. a WHALLOC_API(bt) object, but any
       custom allocator which can support the overall realloc(3)
       interface/semantics would suffice).

       By default, most of the API which uses this type has a default
       implementation which simply forwards all calls to realloc(3)
       and ignores the third argument.

       An interesting ACHTUNG: realloc( someObject, 0 ) is apparently not
       guaranteed to return NULL, but may also return "some value suitable
       for passing to free()." Thus it is philosophically impossible to
       know if realloc(obj,0) ever really succeeds in freeing the memory
       because we have no constant to compare the return value against
       except NULL. Anyway... just something to be aware of.

       Rules for implementations:

       1) If passed (NULL,non-0,anything) they must behave like a malloc()
       request.

       2) If passed (anything,0,anything) they must behave like free()
       and return void. It may legally assert() (or similar) if passed
       memory it does not own/manage (this is common behaviour in
       standard free(3) implementations).

       3) If passed (non-NULL,non-0,anything), they have the following
       options:

       3a) If they support a realloc operation, it must be
       semantically compatible with realloc(3). That implies that
       conditions (1) and (2) are met.

       3b) If they do not support a realloc operation, they have two
       options:

       3b1) If the allocator knows that the new size does not require
       moving the memory (i.e. is equal to or smaller than the
       original size, or does not grow enough to move it into a new
       memory block), it may legally return the original pointer.

       3b2) Return NULL (failing the realloc).

       Provided that the third argument (allocState) is ignored,
       realloc(3) is considered a compliant implementation (indeed, it
       is the reference implementation) for all operations, meeting
       requirements 1, 2, and 3a.
    */
    typedef void * (*WHALLOC_API(realloc_f))( void * m, unsigned int n, void * allocState );

    /**
       An abstract interface for a class containing data for use
       with the WHALLOC_API(realloc_f)() interface.
    */
    struct WHALLOC_API(allocator)
    {
        /**
           The de/re/allocator function. Must conform to the
           WHALLOC_API(realloc_f)() interface specifications.
        */
        WHALLOC_API(realloc_f) realloc;
        /**
           State data for the realloc member, passed as the third
           argument to that function.
        */
        void * state;
    };
    /** Convenience typedef. */
    typedef struct WHALLOC_API(allocator) WHALLOC_API(allocator);

    /** A WHALLOC_API(allocator) object which is initialized to use
        realloc(3) as its memory source.
    */
    extern const WHALLOC_API(allocator) WHALLOC_API(allocator_realloc3);
    /**
       An empty-initialized WHALLOC_API(allocator) object.
    */
    extern const WHALLOC_API(allocator) WHALLOC_API(allocator_empty);

    /** @def whalloc_allocator_empty_m
        
       An empty WHALLOC_API(allocator) initialization object, for use
       when static allocation is necessary.
    */
#define whalloc_allocator_empty_m {NULL/*realloc*/,NULL/*reallocState*/}

    /** @deprecated The WHALLOC_API(bt) API is preferred.

       Metadata for a single WHALLOC_API(ht) entry.
    */
    struct WHALLOC_API(ht_entry)
    {
        /**
           Length of the record ((WHALLOC_BITNESS-1) low bits) plus 1
           is-used flag bit (the high bit).
        */
        WHALLOC_API(size_t) lenfl;
    };
    typedef struct WHALLOC_API(ht_entry) WHALLOC_API(ht_entry);

    /**
       An empty WHALLOC_API(ht_entry) initialization object.
    */
    extern const WHALLOC_API(ht_entry) WHALLOC_API(ht_entry_empty);

    /** @def whalloc_ht_entry_empty_m
       An empty WHALLOC_API(ht_entry) initialization object.
    */
#define whalloc_ht_entry_empty_m { 0/*lenfl*/ }

    /**
       WHALLOC_API(log_f)() is a printf()-compatible print function
       intended only for debugging purposes.
    */
    typedef int (*WHALLOC_API(log_f))( char const * fmt, ... );

    /** @deprecated The WHALLOC_API(allocator) interface is more flexible.

        This class abstracts "fallback allocators" for the
        WHALLOC_API(ht) and WHALLOC_API(bt) classes.

        TODO: remove this interface in favour of the
        WHALLOC_API(allocator) interface.
    */
    struct WHALLOC_API(fallback)
    {
        /** A allocation routine which should allocate n bytes and
            return a pointer to the first one, or NULL on error.

            Must be semantically compatible with realloc(3).

            Remember that realloc(NULL,n) behaves like malloc(n) and
            that realloc(ptr,0) behaves like free().
        */
        void * (*realloc)( void *, size_t  n );
        /**
           Must be semantically compatible with free(3) and be
           complementary to the alloc member.

           This member WILL GO AWAY once WHALLOC_API(ht_realloc)() is
           implemented, since free() can be implemented in terms of
           realloc().
        */
        void (*free)();
    };
    /** Convenience typedef. */
    typedef struct WHALLOC_API(fallback) WHALLOC_API(fallback);
    /**
       An empty WHALLOC_API(fallback) object for use in static
       initialization.
    */
#define whalloc_fallback_empty_m {0/*alloc*/,0/*free*/}
    /** An initialization WHALLOC_API(fallback) object which uses the C-standard
        malloc() and free.
    */
    extern const WHALLOC_API(fallback) WHALLOC_API(fallback_stdalloc);
    /** An initialization object for an empty WHALLOC_API(fallback). */
    extern const WHALLOC_API(fallback) WHALLOC_API(fallback_empty);

    /**
       A helper type which consolidates data required by some concrete
       allocator implementations. See WHALLOC_API(ht) and
       WHALLOC_API(bt) for classes which use this.
    */
    struct WHALLOC_API(allocator_base)
    {
        /** The start of the managed memory range. */
        unsigned char * mem;
        /** One-past-the-end of the managed memory range.
            That is, (mem+size).
        */
        unsigned char const * end;
        /**
           A pointer somewhere between [mem,end) which refers to the
           start of the memory which can be used for actual
           allocations, as opposed to allocator-internal data.
        */
        unsigned char * uspace;
        /** The size of the managed memory range, in bytes. */
        WHALLOC_API(size_t) size;
        /** The "usable" size of the managed memory range, in
            bytes. "Unusuable" space is used for the allocator's
            internals.
         */
        WHALLOC_API(size_t) usize;
        /** The block size used for partitioning memory. */
        WHALLOC_API(size_t) blockSize;
        /** The maximum number of items the pool can managed.
            This is a function of (size/blockSize), adjusted for
            memory taken up by the allocator.
        */
        WHALLOC_API(size_t) blockCount;
        /**
           A bitmask where the bitCount-most right values are set and
           all others are unset.
        */
        WHALLOC_API(size_t) hashMask;
        /**
           The number of active allocations. This is incremented
           on alloc() and decremented on free().
         */
        WHALLOC_API(size_t) allocCount;
        /**
           The number of blocks used by active allocations. This is
           incremented on alloc() and decremented on free().
         */
        WHALLOC_API(size_t) allocBlockCount;
        /**
           A hint to the allocator about where the next free
           memory might be. The value is an index into its
           internal table (i.e. a memory block ID).
        */
        WHALLOC_API(size_t) freeIndexHint;
        /**
           Allocators may optionally store their last error
           code here.
        */
        int errorCode;
        /** The number of bits needed for creating a minimal hashcode
            for doing memory-to-index mapping.  This is function of
            the maximum item count, and should be set to the N, where
            2^N is the smallest power of 2 necessary for holding the
            number in blockCount.

            This number is, not incidentally, the number of bits set
            in the hashMask member.
        */
        uint8_t bitCount;
        /**
           Optional mutex which can be configured by the client.  The
           allocator API will use this mutex if it is set, otherwise
           it does no locking.
        */
        WHALLOC_API(mutex) mutex;

        /**
           Optional memory allocation routines which may be supplied
           by the caller.

           If allocation cannot work and fallback.alloc is set, that
           function will be used to fetch the memory.  When memory is
           passed to the pool's free() routine and it is outside the
           range associated with the pool, if fallback.free is set
           then it is called to free the memory.

           If alloc is set then free MUST be set, or an assertion
           may be triggered in functions which use the fallback
           allocator.

           TODO: replace this with WHALLOC_API(allocator).
        */
        WHALLOC_API(fallback) fallback;
        /**
           Optional logging/debugging function. Set this to non-null
           to intercept (potentially lots of) debugging messages.
           This is typically only enabled if the library is built
           in debugging mode (i.e. NDEBUG is not defined).

           TODO: make this an object with a callback function and a
           state pointer.
        */
        WHALLOC_API(log_f) log;
    };
    /** Convenience typedef. */
    typedef struct WHALLOC_API(allocator_base) WHALLOC_API(allocator_base);
    /** Empty WHALLOC_API(allocator_base) initialization object. */
#define whalloc_allocator_base_empty_m {\
        0/*mem*/,                               \
            0/*end*/,                           \
            0/*uspace*/,                        \
            0/*size*/,                          \
            0/*usize*/,                         \
            8/*blockSize*/,                     \
            0/*blockCount*/,                      \
            0/*hashMask*/,                        \
            0/*allocCount*/,                      \
            0/*allocBlockCount*/,                 \
            0/*errorCode*/,                       \
            0/*bitCount*/,                        \
            0/*freeIndexHint*/,                   \
            whalloc_mutex_empty_m,                \
            whalloc_fallback_empty_m,           \
            0/*log*/                 \
    }
    /** Empty WHALLOC_API(allocator_base) initialization object. */
    extern const WHALLOC_API(allocator_base) WHALLOC_API(allocator_base_empty);
    /** @deprecated The WHALLOC_API(bt) API is preferred.

       A holder for allocator data for use with WHALLOC_API(ht_alloc)() and
       WHALLOC_API(ht_free)().  They can be created on the stack or with
       malloc(), but must be initialized with WHALLOC_API(ht_init)()
       before they are used.

       This class is not thread-safe by default. To enable locking,
       the client must setup the mutex member to use his lock/unlock
       routines of choice.

       Performance: for the average cases, if the allocation size is
       less than or equal to the pool's block size, this allocator
       performs O(1) for both allocation and deallocation. For certain
       usage patterns it would worst-case at near O(N), but that would
       require that one fills the whole list, deallocates the first
       and last items (in any order), then allocate two more
       items. Only that last allocation would be O(N). The others
       would be O(1) (or very, very close to it). When allocating
       larger than a single block, there are linear components. We
       have to check for a free range, and that requires traversing at
       least N hashtable records, where N is the number of blocks
       we're traversing. If the block span is not free, we have to
       search further, making this a relatively expensive operation
       compared to single-block allocations.

       This API can map no more than 2^WHALLOC_BITNESS bytes in one
       allocator, and no single allocation from an allocator can be
       more than (2^(WHALLOC_BITNESS-1)) bytes.
       
    */
    struct WHALLOC_API(ht)
    {
        /** Holds the "base-most" allocator data, which is common to multiple allocator
            implementations. */
        WHALLOC_API(allocator_base) base;
        /**
           Hashtable (lives within mem, before uspace).
        */
        struct Hashtable
        {
            /**
               Points to space somewhere in WHALLOC_API(ht)::mem.

               Will hold 'len' elements, but only [0,end) are valid.
            */
            WHALLOC_API(ht_entry) * head;
            /** The real number of entries in the hashtable. The hashtable
                might have more allocated space than are used. */
            WHALLOC_API(size_t) len;
            /** The size of the hashtable data, in bytes. */
            WHALLOC_API(size_t) sizeOf;
            /**
               Index of one-after-the-end entry.  This is also
               incidentally the true capacity of this object, and may
               actually be smaller than len.
            */
            WHALLOC_API(size_t) end;
        } ht;
    };
    typedef struct WHALLOC_API(ht) WHALLOC_API(ht);
    /**
       An empty WHALLOC_API(ht) initialialization object.
    */
    extern const WHALLOC_API(ht) WHALLOC_API(ht_empty);
    /** @def whalloc_ht_empty_m
       An empty WHALLOC_API(ht) initialialization object.
    */
#define whalloc_ht_empty_m {                                          \
        whalloc_allocator_base_empty_m, \
            {/*ht*/ 0/*head*/,0/*len*/,0/*sizeOf*/,0/*endHash*/} \
        }

    

    /** @deprecated The WHALLOC_API(bt) API is preferred.
       Initializes a WHALLOC_API(ht) for use, such that it can be used to
       "allocate" memory from a user-defined memory range. The
       intention is to save on calls to malloc() by using
       stack-allocated memory as an allocation source.

       self must be an empty object, e.g. created on the stack and
       assigned from the WHALLOC_API(ht_empty) initializer object.

       buffer is memory which will be used as storage by this allocator.

       size is the number of bytes in the buffer argument. If it is
       not a multiple of 2, it will be reduced by one for purposes of
       memory management. It must also fall within certain bounds: a
       minimum of 128 (arbitrarily chosen) and 2^(WHALLOC_BITNESS).

       blockSize is the size of each data page within the memory
       pool. Any allocation will "allocate" at least this many bytes
       internally. blockSize must be 2 or higher for this
       implementation, since the overhead for tracking size-1 blocks
       is so uselessly high.

       Some of the given buffer memory will be used for the internal
       allocation metadata, and thus the buffer will effectively
       become smaller than when the user passed it in. Host much
       smaller is a function of how long it is and the block size.  A
       high blockSize requires a smaller internal hashtable, and a
       small value will waste more space. Values of
       sizeof(WHALLOC_API(ht_entry)) or less won't work well. Values of 8
       or higher work fairly efficiently in terms of space management.

       It is not legal to manipulate the buffer as long as self is
       associated with it, except via pointers returned from
       WHALLOC_API(ht_alloc)().
   
       Returns WHALLOC_API(rc).OK on success. On error, one of the other
       integer values from WHALLOC_API(rc).

       When finished with the self object, call WHALLOC_API(ht_drain)() to
       wipe it clean (or just let it go out of scope). You can also call
       WHALLOC_API(ht_free)() to free individual elements, making their
       memory immediadately available for re-use.

       The buffer parameter is owned by the caller, but it must outlive
       self, and its memory position must not change (e.g. via realloc)
       during self's lifetime.

       After calling this, the client may assign self->mutex,
       self->log, and/or self->fallback to his preferred
       implementations. The mutex and fallback members must not be
       changed during self's lifetime, however. Doing so engenders
       Undefined Behaviour. The log can be set at will, as long as one
       locks the mutex (if set) before doing so. If the log member is
       set before calling this, error messages from the initialization
       process will be logged through it.

       After this call is complete, client code must never change any
       properties of self other than noted above. Doing so may corrupt
       the state of the allocator and lead to undefined behaviour. It
       is okay to read, but don't write.

       The maximum number of items which the pool will be able to allocate
       is calculated based on the size and blockSize.
       
       Example usage:

       @code
       WHALLOC_API(ht) pool = WHALLOC_API(ht_empty);
       enum { BufLen = 1024 * 8 };
       unsigned char buffer[BufLen];
       WHALLOC_API(size_t) blockSize = sizeof(my_type);
       int rc = WHALLOC_API(ht_init)(&pool, buffer, BufLen, blockSize );
       if( WHALLOC_API(rc).OK != rc ) { ... error ... }
       my_type * m = WHALLOC_API(ht_alloc)(&pool, sizeof(my_type));
       // ^^^ m now lives somewhere inside of buffer
       @endcode

       Design notes:

       Despite all the talk of "allocation" in these docs, this API uses no
       dynamic memory allocation. It does, however, abstractly allocate
       memory via a reserved memory area.

       The init routine double-checks its hashtable creation process
       to ensure that all hashcodes it generates are unique and that
       it does not generate too many (which would cause us to step out
       of bounds later on). Because of this consistency checking (A)
       subsequent operations can skimp on range checking (making them
       faster) and (B) the initialization is basically O(N), where N
       is the number of blocks.
       
       When WHALLOC_BITNESS is 16, this routine can (quite by happy
       accident) create a near-optimal hashtable for storing allocation
       metadata, without wasting much memory for small block sizes
       <4). The heuristics for 8- and 32-bit builds are not as optimized,
       but also don't waste too much.

       WHALLOC_API(ht_init)() is the only one of the public WHALLOC_API(ht) family of
       functions which does not check for self->base.mutex.lock. If the object
       must be locked for the initialization, it needs to be locked from
       the client code.
   
       @see WHALLOC_API(ht_alloc)()
       @see WHALLOC_API(ht_free)()
       @see WHALLOC_API(ht_drain)()
    */
    int WHALLOC_API(ht_init)( WHALLOC_API(ht) * const self, void* buffer, WHALLOC_API(size_t) size, WHALLOC_API(size_t) blockSize );

    /** @deprecated
       Tries to allocate an object of the given size from self's
       memory.

       On success a pointer is returned, owned by self. It may be freed
       using WHALLOC_API(ht_free)() or WHALLOC_API(ht_drain)(). That said, if
       the memory used by self is stack-allocated, and one never uses up
       the memory in the allocator, it is not necessary to clean it up -
       stack deallocation will do it when self goes out of scope.

       If size is zero, self->blockSize is assumed.

       The largest legal value for the size parameter is
       (2^(WHALLOC_BITNESS-1)) (that is, half the maximum value of a
       WHALLOC_API(size_t)). This is a side-effect of the internal
       memory-management mechanism, which saves lots of space by
       having this limitation.
       
       Error conditions include:

       - self is null

       - self is full
   
       - self has no more hashtable entries.

       - We cannot allocate enough contiguous blocks. If the list
       becomes fragmented due to unusual alloc/dealloc patterns then
       it might have enough memory, but not contiguous.
    */
    void * WHALLOC_API(ht_alloc)( WHALLOC_API(ht) * const self, WHALLOC_API(size_t) size );

    /** @deprecated
       If m is associated with self (was allocated via
       WHALLOC_API(ht_alloc)()) then it is marked as free for later
       allocation. Otherwise, if self->fallback.free is set then it is
       passed m. If self->fallback.free is set and self->fallback.alloc is
       NOT set then an assertion may be triggered (and if one is not,
       undefined behaviour will ensue unless you know exactly what you're
       doing).

       Returns WHALLOC_API(rc).OK on success, else some other whalloc_rc value.
       
       @see WHALLOC_API(ht_alloc)()
       @see WHALLOC_API(ht_init)()
    */
    int WHALLOC_API(ht_free)( WHALLOC_API(ht) * const self, void * m );

    /** @deprecated
       Requires that self be an initialized object (see WHALLOC_API(ht_init)()).
       This routine wipes out the allocation information for self,
       which semantically calls free() on all managed elements.
       That is, after calling this, any pointers which came from
       WHALLOC_API(ht_alloc)() are considered invalid/danging.

       This function does not accommodate any memory served by
       self->fallback, and if this function should not be used in conjunction
       with objects which have a fallback memory allocator then it
       should be called after WHALLOC_API(ht_free)() has been called for
       each managed element.

       After calling this, self can again be used with
       WHALLOC_API(ht_alloc)().

       It will only fail (returning non-zero) if !self or if the mutex
       locking is enabled but fails to lock.
       
       @see WHALLOC_API(ht_alloc)()
       @see WHALLOC_API(ht_free)()
       @see WHALLOC_API(ht_init)()
    */
    int WHALLOC_API(ht_drain)( WHALLOC_API(ht) * const self );


    /**
        A WHALLOC_API(mutex)::lock() implementation which may
        sends a debugging message to stderr.  It ignores its argument.
    */
    int WHALLOC_API(mutex_lock_trace)( void * );

    /**
       A WHALLOC_API(mutex)::unlock() implementation which sends a
       debugging message to stderr. It ignores its argument.
    */
    int WHALLOC_API(mutex_unlock_trace)( void * x );

    
    /**

        A WHALLOC_API(mutex) implementation which does no locking but sends
        a debugging message to stderr. The mutex functions ignore
        their arguments.
    */
    extern const WHALLOC_API(mutex) WHALLOC_API(mutex_trace);
    /** @def WHALLOC_API(mutex_trace_m)

        An inline-initialized form of WHALLOC_API(mutex_trace).
    */
#define whalloc_mutex_trace_m \
    {WHALLOC_API(mutex_lock_trace),WHALLOC_API(mutex_unlock_trace),0/*data*/}

    /**
       Returns WHALLOC_BITNESS. Can be used in applications to assert that
       they are linked against the proper whalloc library. Liking against
       one with a different bitness will result in undefined behaviour
       (been there, done that).
    */
    uint8_t WHALLOC_API(bitness)();

    /** @internal @deprecated

        Dumps debugging info about self to the given file. This can be used
        to see which hashtable nodes are linked to what.
    */
    int WHALLOC_API(ht_dump_debug)( WHALLOC_API(ht) const * const self, FILE * out );

    typedef enum {
        /**
           The length of the WHALLOC_API(bt)::bits::cache member, in bytes.
           It must be a positive multiple of 2. To enable it, set it
           to some multiple of (WHALLOC_BITNESS/2).  To disable use of
           this cache (reducing the sizeof(WHALLOC_API(bt)) a bit), set it
           to 0.

           Each byte of increase enables WHALLOC_API(bt) to hold tables
           of 4 entries larger. Thus 8 bytes allows 32 more entries
           before WHALLOC_API(bt) will reserve space from the client
           memory.
        */
    WHALLOC_API(bt_CacheLength) = (2*WHALLOC_BITNESS)
    } WHALLOC_API(config_options);
    
    /**
       Similar to WHALLOC_API(ht) but uses a different internal structure
       for memory management, which generally uses up less of the
       reserved memory than WHALLOC_API(ht) does.

       This class is intended to be used identically to WHALLOC_API(ht),
       but it internally uses a much different storage mechanism
       memory management data. Whereas WHALLOC_API(ht) builds a hashtable
       internally, this class builds up a bitmap of data requiring
       only 2 bits per memory block it manages (plus 2 padding bytes
       for internal reasons).

       A WHALLOC_API(bt) instance must not be used for allocation until
       WHALLOC_API(bt_init)() has succeeded on it.
       
       In theory this memory manager performs identically to
       WHALLOC_API(ht), as all of the underlying algorithms are the
       same. However, the internals of this API open it up to certain
       potential/future optimizations which aren't possible in
       WHALLOC_API(ht), such as checking multiple blocks for
       used/linked-ness via one bitmask operation.
       
       Many, many thanks go to Amir "Ashran" Mohammadkhani for his
       help on this. He came over to my place to discuss the
       WHALLOC_API(ht) implementation with me, and he had the insight that
       the allocator doesn't really need to remember the size of
       allocated blocks - it only needs to know if a given
       (re)allocation is within a block boundary. He combined that
       with a different block-linking mechanism and came up with the
       2-bits-per-block method to replace the WHALLOC_API(ht)'s hashtable.

       Here's an overview of how this memory manager works using
       only two bits of memory per managed memory block:

       First see the documentation for WHALLOC_API(bt_init)(). That
       explains how the managed memory is sliced up into blocks (which
       can be as small as 1 byte for certain pool
       sizes). Understanding that stuff...

       When the maximum number of blocks is known, we can calculate a
       bitmask which, when applied to any memory address in the
       managed range, will uniquely identify it with a hash value.
       The hash value of any pointer within that range is the index of
       a logical block within the managed memory (starting at index
       0), aligned on blockSize boundaries. The exact length of the
       hash mask depends on the maximum number of items the we can
       manage in that memory. The hash mask has a number of bits equal
       to 2^N, where N is smallest number of bits needed to hold the
       value of the number of managed blocks.  e.g. if a memory space
       has 100 blocks (regardless of their size), we need 7 bits
       (2^7=128, whereas 2^6=64 cannot map 100 entries) in the hash
       mask. Any other bits of the address range are irrelevant for
       purposes of uniquely mapping an address to a memory block
       index, and an index back to an address (with blockSize-boundary
       granularity). We also set aside, from the managed memory space,
       enough bytes to hold 2*B (B=block count) _bits_, which are
       explained below. Given any address within our managed range,
       even if it's not aligned at the start of the allocated block,
       will "hash back" to an index in our bit lists. Likewise, we can
       convert such an index index back to the address of its
       corresponding block boundary.

       The internals maintain two bitsets, each with a maximum length
       (in bits) equal to the number of managed blocks. e.g. if the
       pool has 100 blocks, it will reserve enough memory for 200 bits
       (plus potentially a padding byte). These bitsets hold two
       pieces of information:

       - The "is-in-use" flag tells us whether a given block is
       allocated or not.

       - The "is-linked" flag tells us if this block is part
       of an allocation chain. Each block which is followed
       by another block is marked as linked. The last block
       in a chain is not linked.

       Those two bits contain all the metadata for a memory block, and
       two bits gives us the following four possible states for any
       given memory block:

       0) !in-use and !is-linked: the block is available for
       allocation.

       1) in-use and is-linked: it is the start of a multi-block
       allocation chain.

       2) in-use and !is-linked: (2a) the only block in a single-block
       allocation, or (2b) this is final link in an allocation chain.

       3) !in-use and is-linked: this is a non-terminal link in an
       allocation chain.

       (Again, thanks to Amir for providing that insight.)

       When traversing block chains to see if we can allocate a given
       range, we must linearly scan each block by simply accessing the
       next bit in the in-use bitset. If a used/linked item is found
       in our proposed range, we move up the list and try again until
       we can find no range we can fit it. The bitsets are normally
       very small (e.g. pool size of 8000 with 16-byte blocks uses
       about 124 bytes for its bitsets).
       
       When traversing block chains to free them up, we can detect a
       mis-free by checking for the appropriate states. Given the
       address to free, we can hash that in O(1) time to find the
       index of the associated data block (including bounds/error
       checking, of course). On deallocation, at the start of a chain
       we require states (1) or (2a). Then we crawl the chain, looking
       for states (2b) or (3). Any other state at any time is an
       error, signaling an internal state error (probably corruption
       via buggy modification of interal state), which would cause
       WHALLOC_API(bt_free)() to return an error code and possibly
       emit a debugging/logging message via WHALLOC_API(bt)::base::log
       (if set). If built in debugging mode, it may assert() on a
       failed free() operation, as free(3) often does if passed an
       invalid address.
       
       The amount of space which we must reserve for the internal bits
       is most strongly affected by the block size. A block size of 1
       might work for relatively small pools (up to up to
       2^(WHAlLOC_BITNESS-1) in size), but it has a high overhead and
       not all blocks will be allocable if they are doled out in
       one-byte increments. Selecting a block size value involves a
       making trade-offs for the specific use case. As a general rule,
       try to set the block size as big as your average object size,
       because single-block de/allocation always out-performs
       multi-block, sometimes by several factors.
    */
    struct WHALLOC_API(bt)
    {
        /** Holds the "base-most" allocator data, which is common to multiple allocator
            implementations. */
        WHALLOC_API(allocator_base) base;
        /**
           Bitsets holding block metadata (they live within mem,
           before uspace).
        */
        struct WHALLOC_API(bt_bitset)
        {
            /**
               The bitset holding the is-used flags.
            */
            unsigned char * usage;
            /**
               The bitset holding the is-linked flags.
            */
            unsigned char * links;
            /**
               The number of bits used in each of the 'usage' and
               'links' members. i don't remember why we store this
               separately (it seems to always be the same as
               base.blockCount), and we may be able to remove this
               from the API (with appropriate changes in the impl
               code).
            */
            WHALLOC_API(size_t) end;
            /**
               The number of bytes from mem reserved for use by the
               in-use and is-linked bitsets. It will be 0 if
               the cache member is used for storing the bitsets.
            */
            WHALLOC_API(size_t) byteCount;
            /**
               If the bit cache can fit in this memory, it will be
               used instead of reserving memory from the pool. When
               this is the case, the full amount given by the client
               (minus perhaps a partial block at the end) will be
               available for allocation.
            */
            unsigned char cache[(WHALLOC_API(bt_CacheLength))
                                ?(WHALLOC_API(bt_CacheLength)+3)
                                /*^^^^^ reminder: +3 to pad the buffer
                                with 0s to avoid chain walks from
                                overrunning. The final +1 is a kludge
                                for the internal calculation when
                                we've got enough elements to get within 8 bits of over-filling
                                this cache, so that we can hold a couple
                                more elements before falling back to
                                using the client's memory.

                                Reminder #2: since we moved the bitsets to the
                                end of the managed memory we can get rid of the
                                second padding byte and we need to move
                                the 'usage' member up one byte.
                                */
                                :1];
        } bits;
    };
    typedef struct WHALLOC_API(bt) WHALLOC_API(bt);
    /**
       An empty WHALLOC_API(bt) initialialization object.
    */
    extern const WHALLOC_API(bt) WHALLOC_API(bt_empty);
    /** @def whalloc_bt_empty_m
       An empty WHALLOC_API(bt) initialialization object.
    */
#define whalloc_bt_empty_m {                                          \
        whalloc_allocator_base_empty_m,                                     \
            {/*bits*/ 0/*usage*/,0/*links*/,0/*end*/,0/*byteCount*/,{0/*cache*/}/*,0 endOfCache*/}, \
        }

    /**
       Works very similarly to WHALLOC_API(ht_init)(), but works
       on a WHALLOC_API(bt) storage manager.

       self must be an empty-initialized object (e.g assign it
       from WHALLOC_API(bt_empty) to do this).

       mem is pointer to size bytes of memory. The blockSize parameter
       defines how big each managed memory block is. size must be at
       least (2*blockSize) and blockSize may be any value from 1 to
       something smaller than size. A value of 0 causes some
       relatively small default value to be used (e.g. 8 bytes). A
       value equal to or larger than size causes WHALLOC_API(rc).ArgError
       to be returned. The mem parameter is owned by the caller but
       its contents should only be manipulated through pointers
       allocated with WHALLOC_API(bt_alloc)(). Any other access to that
       memory region while self is alive will lead to undefined
       behaviour.

       The blockSize parameter specifies how large each managed such
       of memory can be. If 0 is passed, a default value of 8 is used.
       The size and blockSize determine maximum number number of items
       which can be active in the allocator. Not all combinations will work.
       If a combination doesn't work, try adjusting the blockSize
       by a point or two in either direction. i'm still working on getting
       the calculation working properly :/.
       
       A portion of mem will be reserved by self for use in its memory
       management. Specifically, this needs 2 bits (not bytes) per
       managed block. In the worst case (blockSize==1), this will
       "reappropriate" about 18-19% of mem for internal management
       details. For larger block sizes, the percentage of memory
       dedicated to the internals drops rapidly: blockSize=2=(~11%),
       4=(~6%), 8=(~3%), ... 64=(~0.4%), 128=(~0.2%). Doubling the
       block size approximately halves the percentage of memory
       reserved for internal purposes. This also means, however, that
       by increasing blockSize, the maximum number of objects which
       can be allocated will be reduced, since there are fewer overall
       blocks. self->bits.end holds the maximum number of items the
       allocator can manage, which is roughly (size/blockSize), minus
       whatever memory we need for internal management.
       
       On success:

       WHALLOC_API(rc).OK is returned. mem is owned by the caller but is
       managed by self, and mem must outlive self.

       On error:

       A non-0 value is returned and self will be in an undefined
       state. self should not be used for allocation until
       WHALLOC_API(bt_init)() has succeeded on it.

       Note that self does not own any dynamic resources, so it does
       not explicitly need to be cleaned up. If the caller allocates
       the mem buffer using malloc(), he must free that up after he is
       done using the WHALLOC_API(bt) object associated with it.


       The init routine double-checks its lookup table creation process
       to ensure that all hashcodes it generates are unique and that
       it does not generate too many (which would cause us to step out
       of bounds later on). Because of this consistency checking (A)
       subsequent operations can skimp on range checking (making them
       faster) and (B) the initialization is basically O(N), where N
       is the number of blocks.

       To re-use the allocator, use WHALLOC_API(bt_drain)() to clear out
       the internal state, or call WHALLOC_API(bt_init)() again to
       completely re-build the internals (slower, but necessary if the
       memory or block sizes change).
       
       Example:

       @code
       enum { BufSize 1024 * 8 };
       unsigned char buf[BufSize];
       WHALLOC_API(bt) bm = WHALLOC_API(bt_empty);
       WHALLOC_API(size_t) blockSize = 4;
       // bm.base.log = printf; // generates tons of debugging/tracing messages.
       int rc = WHALLOC_API(bt_init)( &bm, buf, BufSize, blockSize );
       if( WHALLOC_API(rc).OK != rc ) { ... error ... }
       MyType * m = WHALLOC_API(bt_alloc)( &bm, sizeof(MyType) );
       // ^^^ the returned memory lives inside of buf
       ...
       WHALLOC_API(bt_free)(&bm, m);
       @endcode


       Regarding aligment of pointers allocated by the self object:

       If mem is properly aligned and blockSize is a multiple of the
       platform's alignment then pointers allocated via the self
       object are guaranteed (as of version 20100304) to be properly
       aligned. This function does not do any magic to ensure
       alignment - if mem is not aligned or if blockSize is not a
       multiple of the alignment then some or all returned objects
       will not be "properly" aligned.  All that said, x86 platforms
       apparently don't care about alignment (or at least don't crap
       out if a specific alignment is not used).
    */
    int WHALLOC_API(bt_init)( WHALLOC_API(bt) * const self, void * mem, WHALLOC_API(size_t) size, WHALLOC_API(size_t) blockSize );

    /**
       Tries to allocate n bytes from self's pool. On success it returns
       the memory address, which will abstractly be n bytes but may in fact
       be larger (depending on self->base.blockSize). On error it returns
       null.

       If self->base.fallback.alloc AND self->base.fallback.free are set,
       and self cannot fulfill the allocation, the result of calling
       self->base.fallback.alloc(n) will be returned.

       If self was initialized with a "properly" aligned memory buffer
       and a block size which is a multiple of the platform's
       alignment, the returned pointer will be "properly" aligned.
    */
    void * WHALLOC_API(bt_alloc)( WHALLOC_API(bt) * const self, WHALLOC_API(size_t) n  );

    /**
       Semantically equivalent to realloc(), but uses the given
       allocator to manage the underlying storage.

       If !m then this is equivalent to WHALLOC_API(bt_alloc)(self,n). If
       size is 0 and m is not NULL this is equivalent to
       WHALLOC_API(bt_free)(self,m) except that it returns NULL on success
       and m on error.

       If m does not refer to memory managed by self AND
       self->fallback is set up, then the fallback routine is used,
       otherwise 0 is returned and an assertion may be triggered (in
       debug builds).

       If the memory block(s) allocated to m is/are already large
       enough to hold n then the m will be returned but the internal
       size reserved for it may be reduced if fewer blocks are needed
       for the new size.

       m may have to be relocated for the reallocation, and calling
       this invalidates all previously-stored pointers to m.

       On success, a pointer to the newly-(re)allocated memory is
       returned, which must be freed using WHALLOC_API(bt_free)(),
       WHALLOC_API(bt_realloc)(self,mem,0), or WHALLOC_API(bt_drain)().  As with
       realloc(), do not directly assign the return value to the same
       pointer passed as the m argument: if realloc fails that would
       overwrite the m pointer with 0, effectively "losing" the
       pointer to that memory.

       Example usage:

       @code
       void * re = WHALLOC_API(bt_realloc)(&mybt, myMemory, 12);
       if( ! re ) { ... error ... }
       else { myMemory = re; }
       @endcode
    */
    void *  WHALLOC_API(bt_realloc)( WHALLOC_API(bt) * const self, void * m, WHALLOC_API(size_t) n );

    /**
       Deallocates memory from self allocated via WHALLOC_API(bt_alloc)().

       On success it returns WHALLOC_API(rc).OK, else it returns some
       other value described for the whalloc_rc object.

       If self->base.fallback.alloc AND self->base.fallback.free are
       set AND m is not in the address space managed by self,
       self->base.fallback.free(m) is called and 0 is returned.  If m
       is outside of self's managed memory and neither
       self->base.fallback.alloc nor self->base.fallback.free are set,
       an assertion may be triggered (if the library was built with
       them enabled) or a non-zero error code will be returned.
    */
    int WHALLOC_API(bt_free)( WHALLOC_API(bt) * const self, void * m );


    /**
       If neither b nor a are NULL then a is modified such that further
       calls to a->realloc() will use b as the memory source. If the client
       modifies either a->realloc or a->allocState after this call returns,
       undefined behaviour will result.

       On error (either b or a are NULL) it returns non-zero.
    */
    int WHALLOC_API(bt_allocator)( WHALLOC_API(bt) * b, WHALLOC_API(allocator) * a );

    /**
       Cleans up the contents of self, such that:

       - Pointers which were fetched via WHALLOC_API(bt_alloc)() are
       considered to be invalidated.

       - May be used with WHALLOC_API(bt_alloc)() after calling this,
       re-using any memory it may have previously doled out.

       It will only fail (returning non-zero) if !self or if the mutex
       locking is enabled but fails to lock.
    */
    int WHALLOC_API(bt_drain)( WHALLOC_API(bt) * const self );

    /** @internal

       Internal debuggering tool which dumps info about self to the
       given stream.
    */
    int WHALLOC_API(bt_dump_debug)( WHALLOC_API(bt) const * const self, FILE * out );    


    /**
       Calculates a hash mask for a memory block containing the given
       number of logical elements (i.e. data blocks).

       The number parameter is the number for which to calculate a
       hash mask, and should be the maximum possible number of items
       for which a hash value is needed. bits must be a non-null
       pointer to a uint8_t. mask may optionally be null.

       This function basically determines how many bits of information
       we need in order to be able to store the given number. This
       info is used for creating hashcodes for the various whalloc
       allocator implementations.

       On success, WHALLOC_API(rc).OK is returned and:

       - bits is set to the number of bits needed to hold the number
       "size".  If (bits > (WHALLOC_BITNESS-1)) then
       WHALLOC_API(rc).RangeError is returned.

       - if mask is not null then it is set to a bitmask where the lowest N bits
       are set, where N is the value assigned to the bits argument.

       On error, a non-OK value is returned describing the nature of
       the problem and none of the output parameters are modified.
    */
    int whalloc_calc_mask( WHALLOC_API(size_t) number,
                           uint8_t * bits,
                           WHALLOC_API(size_t) * mask );

    /**
       Works like whalloc_calc_mask(), but takes a memory buffer
       size and a logical block size. If size is not evenly dividable
       by blockSize then this function discards the "overflow" for purposes
       of the calculation.

       On success, the same behaviour as whalloc_calc_mask() applies,
       but additionally:

       - if blocks is non-null then it will be assigned to the number
       of logical blocks which can be managed using the given size and blockSize.

    */
    int whalloc_calc_mask2( WHALLOC_API(size_t) size,
                            WHALLOC_API(size_t) blockSize,
                            uint8_t * bits,
                            WHALLOC_API(size_t) * mask, 
                            WHALLOC_API(size_t) *blocks );


    /**
       A simple memory region allocator class. It supports only
       allocation, not deallocation, of individual areas. It does not
       manage blocks of memory, but simply doles out the next n bytes
       of memory on each subsequent request.  The only way to "free"
       the allocated items for re-use is to re-set the region.

       @see WHALLOC_API(region_init)()
       @see WHALLOC_API(region_alloc)()
       @see WHALLOC_API(region_free)()
    */
    struct WHALLOC_API(region)
    {
        /** Pointer to the managed memory. */
        unsigned char * mem;
        /** Current allocation position. */
        unsigned char * pos;
        /** One-past-the-end of the memory reason. */
        unsigned char * end;
    };
    /** Convenience typedef. */
    typedef struct WHALLOC_API(region) WHALLOC_API(region);
    /** Empty-initialized WHALLOC_API(region) object. */
    extern const WHALLOC_API(region) WHALLOC_API(region_empty);

    /**
       Initializes r to use length bytes of mem for memory allocation
       wipes all bytes of mem to 0. Technically, ownership of mem is
       not transfered, but semantically it is given to r. Modifying
       mem it from outside this API (aside from modifying via pointers
       allocated by it) may corrupt the expected state.

       Returns 0 on success, non-zero on error. The only error conditions are
       if !r, !m, or !length.

       If mem is properly aligned for the host platform AND all
       allocations are a multiple of the alignment, then the memory
       allocated via the r object will be properly aligned, otherwise
       there are no alignment guarantees. Note that when using
       allocation sizes which come directly from sizeof(), the result
       will (except in the case of char arrays) be guaranteed to be a
       multiple of the alignment IF the platform pads its structs to
       such alignments.

       TODO:

       - Consider adding an option to the region class which tells it
       to round all allocation requests to the next multiple of the
       alignment value.
    */
    int WHALLOC_API(region_init)( WHALLOC_API(region) * r, void * mem, WHALLOC_API(size_t) length );

    /**
       Tries to allocate n bytes from r. On success returns the new
       pointer, which lives in memory managed by r. On error (!r or
       not enough bytes left) NULL is returned.
    */
    void * WHALLOC_API(region_alloc)( WHALLOC_API(region) * r, WHALLOC_API(size_t) n );

    /**
       "Frees" all memory in r by re-setting the allocation pointer
       and memsetting all bytes to 0.  Any objects allocated by
       WHALLOC_API(region_alloc)() are semantically freed by this, and
       future calls to WHALLOC_API(region_alloc)() may return memory
       overloapping with previously-allocated objects.

       Returns 0 on success, and the only error condition is if
       r is NULL.

       Note that this function does not actually free r itself, as r's
       destruction depends on how it was created (e.g., on the stack,
       malloc(), or via one of the other whalloc allocators).
    */
    int WHALLOC_API(region_free)( WHALLOC_API(region) * r );
    
    /** @def WHALLOC_USE_ALIGN

        No longer used. Will go away.
    */


    /** @def WHALLOC_ENABLE_CONVENIENCE_API

        If WHALLOC_ENABLE_CONVENIENCE_API is defined to a true value
        before including this header, macros are installed which
        provide a "shorthand" form of the core whalloc API.

        Namely, the functions named whalloc_XX_func() are macroized
        to wha_XX_func(), where XX is one of (bt,ht).

        Additionally, the whalloc_XX types are typedef'd to wha_XX
        and whalloc_XX_empty is defined to to wha_XX_empty.

        And... WHALLOC_API(bt_XXX) is macroized to wha_XXX, since
        WHALLOC_API(bt) implementation is considered to be a better (more
        efficient) allocator than WHALLOC_API(ht), and should be the
        default choice.
    */
#if !defined(WHALLOC_ENABLE_CONVENIENCE_API)
#  define WHALLOC_ENABLE_CONVENIENCE_API 0
#endif
#if WHALLOC_ENABLE_CONVENIENCE_API
#  define wha_rc whalloc_rc
    typedef WHALLOC_API(bt) wha_b;
    static const wha_b wha_b_empty = whalloc_bt_empty_m;
#  define wha_b_init WHALLOC_API(bt_init)
#  define wha_b_alloc WHALLOC_API(bt_alloc)
#  define wha_b_realloc WHALLOC_API(bt_realloc)
#  define wha_b_free WHALLOC_API(bt_free)
#  define wha_b_drain WHALLOC_API(bt_drain)
#  define wha_b_dump WHALLOC_API(bt_dump_debug)
    typedef WHALLOC_API(ht) wha_h;
    static const wha_h wha_h_empty = whalloc_ht_empty_m;
#  define wha_h_empty_m whalloc_ht_empty_m
#  define wha_h_init WHALLOC_API(ht_init)
#  define wha_h_alloc WHALLOC_API(ht_alloc)
#  define wha_h_free WHALLOC_API(ht_free)
#  define wha_h_drain WHALLOC_API(ht_drain)
#  define wha_h_dump WHALLOC_API(ht_dump_debug)

    typedef WHALLOC_API(bt) wha_a;
    static const wha_a wha_a_empty = whalloc_bt_empty_m;
#  define wha_empty wha_b_empty_m
#  define wha_init wha_b_init
#  define wha_alloc wha_b_alloc
#  define wha_realloc wha_b_realloc
#  define wha_free wha_b_free
#  define wha_drain wha_b_drain
#  define wha_dump wha_b_dump

#endif /* WHALLOC_ENABLE_CONVENIENCE_API */
#undef WHALLOC_ENABLE_CONVENIENCE_API /* we don't need this anymore */


#define WHALLOC_USE_ALIGN 0
#if WHALLOC_USE_ALIGN
    /** @internal */
    enum whalloc_align_enum {
    WHALLOC_ALIGN_ENUM
    };

    /** @internal */
    struct whalloc_align_struct {
        char pad;
        double type;
    };

    /** @internal */
    union whalloc_align_max {
#if 0
        char char_;
        short int short_;
        int int_;
        long int long_;
        float float_;
        double double_;
        long double long_double_;
#else
        uint8_t u8_;
        uint16_t u16_;
        uint32_t u32_;
        uint64_t u64_;
        double double_;
        long double long_double_;
#endif
        void* ptr_;
        void* (*fptr_) (void*);
        enum whalloc_align_enum enum_;
        struct whalloc_align_struct struct_;
        size_t size_t_;
        ptrdiff_t ptrdiff_t;
    };

#  define WHALLOC_ALIGN_OF(mp_type)             \
    offsetof(                                   \
             struct {                           \
                 char pad_WHALLOC_ALIGN_OF;     \
                 mp_type type_WHALLOC_ALIGN_OF; \
             },                                 \
             type_WHALLOC_ALIGN_OF              \
             )

#  define WHALLOC_ALIGN_MAX WHALLOC_ALIGN_OF(union whalloc_align_max)

#  define WHALLOC_ALIGN_UP(mp_ptr, mp_align)            \
    ((void*)(                                           \
             (((uintptr_t)(mp_ptr)) + ((mp_align) - 1)) \
             & ~(((mp_align) - 1))                      \
             ))


#  define WHALLOC_ALIGN_ASSERT(mp_ptr, mp_align)                \
    (((void*)(mp_ptr)) == WHALLOC_ALIGN_UP(mp_ptr, mp_align))

#else/*!WHALLOC_USE_ALIGN*/

#  define WHALLOC_ALIGN_OF(mp_type) 0
#  define WHALLOC_ALIGN_UP(mp_ptr, mp_align) ((void*)(mp_ptr))
#  define WHALLOC_ALIGN_ASSERT(mp_ptr, mp_align) 1
#if 8 == WHALLOC_BITNESS
#  define WHALLOC_ALIGN_MAX 4
#elif 16 == WHALLOC_BITNESS
#  define WHALLOC_ALIGN_MAX 4
#elif 32 == WHALLOC_BITNESS
#  define WHALLOC_ALIGN_MAX 4
#elif 64 == WHALLOC_BITNESS
#  define WHALLOC_ALIGN_MAX 8
#else
#  error "WHALLOC_BITNESS must be one of (8,16,32,64)!"
#endif

#endif /*WHALLOC_USE_ALIGN*/
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* WANDERINGHORSE_NET_WHALLOC_POOL_H_INCLUDED */
/* end file whalloc.h */
/* begin file whalloc_pager.h */
#line 8 "whalloc_pager.h"
#if !defined(WANDERINGHORSE_NET_WHALLOC_PAGER_H_INCLUDED)
#define WANDERINGHORSE_NET_WHALLOC_PAGER_H_INCLUDED 1
/** @page whalloc_pager whalloc_page API

   A page-based memory allocation manager for C.

   Author: Stephan Beal (http://wanderinghorse.net/home/stephan/

   License: Public Domain

   Features:

   - Allocation of same-sized objects via a page-based memory pool,
   where each page manages a number of equal-sized memory chunks and
   can only dole out individual chunks (as opposed to blocks larger
   than one chunks).

   - The "book" class internally manages any number of pages,
   providing a simple memory de/allocation interface.

   - Allows the client to define how books and pages are allocated.
   e.g. a memory book and its pages can use WHALLOC_API(bt) for storage.

   - The book class optionally supports a mutex to lock all
   access. The page class does not because it is intended to be used
   within a book, and the page class is already fat enough as it is.

   Misfeatures:

   - This is not a general-purpose allocator. It is intended
   specifically for cases which allocate many of the same types of
   objects and wants to recycle the memory to avoid the overhead of
   malloc(). (The original use case was allocating 12-byte structs
   to record record locking information, where we have to create
   and destroy arbitrary numbers of them.)

   - A side-effect of the internal list reorganization (to help
   guaranty good performance for allocation) is that allocations made
   one after another are, in many cases, likely to live in different
   areas of memory (in different allocator-internal pages), and thus
   have poor locality of reference. If guaranteed good locality of
   reference is critical for your application then this code is not
   for you.
*/

#ifdef __cplusplus
extern "C" {
#endif
    

/**
   A class representing a "page" of memory blocks
   of equal size. The related API is responsible
   for managing the "allocation".

   This class is intended for "indirect" client use via the
   WHALLOC_API(book) class, but it can also be used as the basis for
   book-like classes which manage memory pages.

   This class is structured like a doubly-linked list to facilitate
   traversal and manipulation of multiple pages in page-managing
   algorithms. Whether or not the links may be circular depends
   on the managing class and its algorithms (the book class does
   not create a circular list, though it arguably should).

   These objects are created (indirectly) via the WHALLOC_API(book)
   API or directly by calling WHALLOC_API(page_new)(). They are freed
   using WHALLOC_API(page_finalize)().  Use WHALLOC_API(page_alloc)()
   WHALLOC_API(page_free)() to allocate and deallocate invididual
   objects within a page.

   TODOs:

   In the interest of memory use...
   
   - Consider removing the useCount member. i think we can do without
   it by relying on nextFree. It's only used as an optimization to
   figure out if the list is full.

   - Consider removing one of the page link pointers (prev/next
   members). i think the book class can get by with a singly-linked
   list just as well (except that it makes re-organization of the list
   more difficult).
*/
struct WHALLOC_API(page)
{
    /**
       The number of memory chunks for which this page reserves
       space. i.e.  the maximum number of objects it may
       allocate. This is set at initialization and must not be changed
       afterwards.
     */
    uint16_t length;

    /**
       The size of each memory chunk. This is set at initialization
       and must not be changed afterwards.
    */
    uint16_t chunkSize;

    /**
       The current number of "used" (allocated to the client)
       entries. This is modified by the de/allocation routines, and
       must not be modified by client code.

       It is primarily a performance optimization for checking
       is-the-page-empty. We could theoretically get rid of this and
       simply check if nextFree>=length for that case. Something to
       try.
    */
    uint16_t useCount;

    /** @internal

       Internal hint to speed up allocation - the index of the next
       (suspected) free chunk. A value here does not guaranty that
       that position is actually free - this is only a hint as to
       where to start looking for a free chunk. This is modified by
       the de/allocation routines, and is always set to be the
       lowest-offset free chunk.

       Setting this from client code can cause a page to allocate
       more slowly or even skip unused memory chunks (treating them
       as used though they are not). Don't do it.
    */
    uint16_t nextFree;

    /** @internal

        A number of flag bytes (enough to hold thisObject->length
        bits). Each chunk in the page requires one bit in this memory
        for marking the chunk as used or unused.

        Do not use this from client code.
        
        Maintenance reminder: this is (unsigned char*) instead of
       (void*) to facilitate access to specific offsets via addition.
    */
    unsigned char * flags;

    /** @internal

       Objects allocated in this page are stored somewhere in here.
       This space is off limits to client code. All access to the
       memory should be via WHALLOC_API(page_alloc)().

       Maintenance reminder: this is (unsigned char*) instead of
       (void*) to facilitate access to specific offsets via addition.
    */
    unsigned char * pool;

    /**
       A pointer to the next page in the list.
    */
    struct WHALLOC_API(page) * next;

    /**
       A pointer to the previous page in the list.
    */
    struct WHALLOC_API(page) * prev;

    /**
       If set to have non-null values during initialization, this
       object is used to allocate the page instance. In
       WHALLOC_API(page_finalize)(), this routine is used to free the
       object.

       This member must comply strictly to the WHALLOC_API(allocator)
       interface.

       A default implementation is used if the does not provide one to
       WHALLOC_API(page_new)(). alloc.realloc must not be NULL and
       must not change addresses after initialization.
    */
    WHALLOC_API(allocator) alloc;
};

/** Convenience typedef. */
typedef struct WHALLOC_API(page) WHALLOC_API(page);

/**
   Empty-initialized WHALLOC_API(page) object.
*/
extern const WHALLOC_API(page) WHALLOC_API(page_empty);

/**
   A book is a management object for a collection of memory page
   objects (see WHALLOC_API(page)). Each book manages pages of a
   certain length and memory chunks of a certain size. The number of
   pages is dynamic, and may grow or shrink over time.
*/
struct WHALLOC_API(book)
{
    /**
       The number of objects allocated per page. Must not be changed
       after initialization.
     */
    uint16_t pageLength;

    /**
       The size of each allocation unit. Must not be changed after
       initialization.
    */
    uint16_t chunkSize;

    /** @internal
        Internal state flags.
    */
    int8_t flags;
    
    /**
       The first page in the memory page list. The API may reorganize
       the list throughout the life of this object, so there is no
       guaranty that this pointer remains stable (in fact, it is
       guaranteed to be unstable if any de/allocation is done which
       when the book has more than one page).
    */
    WHALLOC_API(page) * page;
    /**
       Optional logging/debugging function. Set this to non-null
       to intercept debugging messages. Only used if built in
       debugging mode.
    */
    WHALLOC_API(log_f) log;

    /**
       If set to contain non-null values during initialization, this
       routine is used to de/allocate this book (but not its pages:
       see the pageAlloc member). e.g. WHALLOC_API(book_close)() uses
       routine to free the object.

       This member must comply strictly to the
       WHALLOC_API(allocator)() interface, must not have a NULL
       realloc member, and the members must not change addresses after
       initialization.

       WHALLOC_API(book_open)() assigns a default implementation if
       given a NULL allocator function. The default simply uses
       realloc(3).
    */
    WHALLOC_API(allocator) alloc;
    /**
       If set to contain non-null values during initialization, this
       routine is used to de/allocate this book's pages (but not the
       book itself: see the alloc member).

       This member must comply strictly to the
       WHALLOC_API(allocator)() interface, must not have a NULL
       realloc member, and the members must not change addresses after
       initialization.

       WHALLOC_API(book_open)() assigns a default implementation if
       give a NULL allocator function. The default simply uses
       realloc(3).
    */
    WHALLOC_API(allocator) pageAlloc;
    /**
       An optional mutex which should be used by this object
       to lock/unlock during all operations. It is used as
       described in the docs for the WHALLOC_API(mutex) type.
       If it contains NULL values then locking will not be
       used.

       Pedantic note: if the pageAlloc member uses the same underlying
       native mutex as this object then the mutex must be capable of
       recursive locks. If a book and allocator share a non-recursive
       mutex, allocating new pages in the book may cause a deadlock
       because the allocation must be locked from the book (because it
       uses book members) and then the allocator itself will trigger
       another lock.
    */
    WHALLOC_API(mutex) mutex;
};

/** Convenience typedef. */
typedef struct WHALLOC_API(book) WHALLOC_API(book);

/**
   Empty-initialized WHALLOC_API(book) object.
*/
extern const WHALLOC_API(book) WHALLOC_API(book_empty);

/**
   For a given number of objects (n) of a given size (chunkSize), this
   function returns the size of the memory needed to store a
   WHALLOC_API(page) object created with WHALLOC_API(page_new)() using
   those same parameters.

   If either value is 0 then 0 is returned.

   When using a custom allocator for page objects, this can be helpful
   in sizing the memory blocks of the custom allocator (so they can
   have the same size as the page objects they will host, for an
   optimum use of space).
*/
unsigned int WHALLOC_API(page_calc_size)( uint16_t n, uint16_t chunkSize );

/** @def WHALLOC_API_PAGE_CALC_SIZE

    WHALLOC_API_PAGE_CALC_SIZE is equivalent to the function
    WHALLOC_API(page_calc_size)(), but is provided because this value
    is sometimes required as a compile-time constant. (While C++ will
    let us initialize static const objects from function return
    values, C does not.)

    N is the number of objects to be stored in the page and ChunkSize
    is the size of each object. If either is 0 then this macro
    evaluates to 0 (which is not a legal page size).

    @see WHALLOC_API(page_calc_size)
*/
#define WHALLOC_API_PAGE_CALC_SIZE(N,ChunkSize) ((!(N) || !(ChunkSize)) ? 0 : (sizeof(WHALLOC_API(page)) + (/*pool*/(N) * (ChunkSize) + (N/8+1)/*flags*/)))


    /**
       Allocates a page object along with enough space to hold
       n objects of chunkSize bytes each.

       On success it returns a new book object which is owned by the
       caller and must eventually be cleaned up using
       WHALLOC_API(page_finalize)(). On error NULL is returned.

       The alloc parameter specifies an allocation routine which should be
       used to allocate and destroy the page object's memory. It may be
       null, in which case a realloc(3)-compatible implementation is used
       which ignores the 3rd argument
       
       The allocState parameter is used as the 3rd argument to the alloc()
       argument. It may be NULL if alloc is NULL or if the alloc
       implementation needs no private state (or the state is encapsulated
       with the alloc function itself).

       To succeed, this function has to be able to allocate
       WHALLOC_API(page_calc_size)(n,chunkSize) bytes using a single call to
       alloc() (or the default implementation, if the alloc argument is
       NULL).

       TODO (or to consider): ensure that the memory segment used for
       doling out objects is generically aligned. Currently there are no
       alignment guarantees. Even if we guaranty the initial position, a
       non-multiple-of-alignment chunkSize would ensure that all (or most)
       other allocations were not aligned. We could fudge that by rounding
       up chunkSize to a multiple of the max alignment. Hmmm.

       @see WHALLOC_API(page_new2)()
    */
WHALLOC_API(page) * WHALLOC_API(page_new)( uint16_t n, uint16_t chunkSize,
                                           WHALLOC_API(realloc_f) alloc, void * allocState);


/**
   Equivalent to WHALLOC_API(page_new)() but takes its allocator
   argument in a different form. See WHALLOC_API(page_new)() for
   the full details, and below for the differences...

   If alloc is NULL or alloc->realloc is NULL then a default allocator
   which uses realloc(3) is used in its place.

   If the allocator object is not NULL, is is SHALLOWLY COPIED for
   future use. Thus the alloc pointer itself may legally change after
   this call, but the alloc->state and alloc->realloc values must stay
   valid for the life of the returned object.

   @see WHALLOC_API(page_new)()
   @see WHALLOC_API(page_finalize)()
*/
WHALLOC_API(page) * WHALLOC_API(page_new2)( uint16_t n, uint16_t chunkSize,
                                            WHALLOC_API(allocator) const * alloc );



/**
   Frees all memory associated with p, invalidating p and any
   pointers to memory which were allocated via p. The function
   p->alloc.realloc() is used to do the actual deallocation.

   If p is NULL then this function is a no-op.

   If p is managed by a higher-level structure
   (e.g. WHALLOC_API(book)) then do not use
   WHALLOC_API(page_finalize)() on that page! Doing so may invalidate
   pointers used by the management structure.
*/
void WHALLOC_API(page_finalize)( WHALLOC_API(page) * p );

/**
   Returns the amount of memory which was initially allocated for p
   (assuming p was allocated via WHALLOC_API(page_new)), and is
   equivalent to WHALLOC_API(page_calc_size)(p->length,p->chunkSize).
   Note that this returns the _requested_ allocation size, and the
   underlying allocator might have allocated more (but that space is
   considered unusable).

   Returns 0 if !p.
 */
unsigned int WHALLOC_API(page_sizeof)( WHALLOC_API(page) const * p );

/**
   Searches for the next free memory chunk in p and
   returns it, or returns NULL on error (!p) or
   if p is full.

   Performance notes:

   This implementation performs O(1) for the average case, where
   objects are deallocated in reverse order of their allocation.
   The worst-case scenario is O(N), where N is slightly less than
   p->length, but hitting that case would require some really unusual
   de/allocation patterns.
*/
void * WHALLOC_API(page_alloc)( WHALLOC_API(page) * p );

/**
   Returns non-zero (true) if m is in the bounds managed by p (that
   is, if it appears that m was (or could become) allocated from
   p). Else it returns 0 (false).

   This is an O(1) operation.
*/
char WHALLOC_API(page_owns_mem)( WHALLOC_API(page) const * p, void const * m );

/**
   Semantically equivalent to free(mem), this returns ownership of
   mem, which must have been allocated by WHALLOC_API(page_alloc)(p),
   to p. Returns 0 on success or non-zero error( !p, !mem, or mem is
   not owned by p).

   This routine is O(1): it only has to perform a couple of
   address comparisons and potentially set a byte.

   To free the p object itself, use WHALLOC_API(page_finalize)().
   If the page object itself is managed by a higher-level structure
   (e.g. WHALLOC_API(book)) then do not use WHALLOC_API(page_finalize)()!
*/
int WHALLOC_API(page_free)( WHALLOC_API(page) * p, void * mem );

/**
   Erases the chunk-allocation state of p, such that all memory chunks
   in it are considered unused and ready for re-allocation. Any
   pointers to memory which were allocated via p are semantically
   invalidated by this call (and if used they might clobber or be
   clobbered by a future re-allocation of the chunk).

   p->next and p->prev are not changed.

   Returns 0 on success, non-zero on error. The only error condition
   is !p.


   This operation has effectively O(N) performance, where N is
   approximately (1+(p->length/8)) and is the number of bytes in the
   internal bitset. (Those bytes must be memset()'d to 0.)
*/
int WHALLOC_API(page_erase)( WHALLOC_API(page) * p );

/**
   Returns 0 (false) if p has more chunks available to allocate, else
   it returns non-zero (true).

   This is an O(1) operation.
*/
char WHALLOC_API(page_is_full)( WHALLOC_API(page) const * p );

/**
   Inserts p as after->next, re-linking after->next if necessary.

   Returns 0 on success or non-zero if:

   - !p or !after

   - Either of p->prev or p->next is non-NULL (use
   WHALLOC_API(page_snip) to remove it from a chain if you need to).
*/
int WHALLOC_API(page_insert_after)( WHALLOC_API(page) * p, WHALLOC_API(page) * after );


/**
   Identical to PAGE(page_insert_after), but inserts p before the
   second argument. It has the same error conditions and return codes
   as that function.
*/
int WHALLOC_API(page_insert_before)( WHALLOC_API(page) * p, WHALLOC_API(page) * before );

/**
   Snips p from its prev/next neighbors (if any) and links the
   neighbors together. On error (!p) it returns NULL, else it returns
   p->next (if it was set) or p->prev (if p->next is NULL). That is,
   it tries to return the right-side neighbor, but returns the left
   neighbor if there is nobody on the right. If there are no
   neighbors, NULL is returned (but this is not an error).

   On success, ownership of p is given to the caller. Ownership of the
   returned object does not change.
*/
WHALLOC_API(page)* WHALLOC_API(page_snip)( WHALLOC_API(page) * p );

/**
   Moves page p one step to the right in its chain. On success then 0
   is returned. If !p then non-zero (error) is returned. The case
   of !p->next is treated as a no-op success.
*/
int WHALLOC_API(page_move_right)( WHALLOC_API(page) * p );

/**
   The left-side counterpart to WHALLOC_API(page_move_right), this moves
   p one step to the left in its chain. It has the same error
   conditions and return codes as WHALLOC_API(page_move_right).
*/    
int WHALLOC_API(page_move_left)( WHALLOC_API(page) * p );

/**
   Allocates a book which serves memory in chunks of exactly chunkSize
   bytes. The n parameter is the number of chunks to store in each
   internal memory page (see WHALLOC_API(page)). Pages are added to
   the book as needed when allocation is requested.

   No pages are initially allocated - they are allocated on demand or
   via WHALLOC_API(book_add_page).

   The alloc parameter specifies an allocation routine which should be
   used to allocate and destroy the book object's memory. It may be
   null, in which case a realloc(3)-compatible implementation is used
   which ignores the 3rd argument.
   
   The allocState parameter is used as the 3rd argument to the alloc()
   argument. It may be NULL if alloc is NULL or if the alloc
   implementation needs no private state.

   The pageAlloc and pageAllocState parameters define the allocator
   data for pages allocated via the book, in the same way that th
   ealloc and allocState parameters define it for the book
   itself. pageAlloc may be NULL, in which case the same default
   allocator is used as described above for the book
   object. pageAllocState may or may not be allowed to be NULL,
   depending on the requirements of pageAlloc (the default
   implementation ignores the pageAllocState argument).
   pageAlloc/pageAllocState may be the same values as
   alloc/allocState unless the documentation for those functions
   prohibit it.

   On success it returns a new book object which is owned by the
   caller and must eventually be cleaned up using
   WHALLOC_API(book_close)(). On error NULL is returned.

   Why use two different allocators for the book and the pages? Most
   client cases would use the same for both. i have, however, a highly
   memory-optimized library where i can press out a hundred bytes or
   so by allocating the book from the general-purpose allocator and
   the pages from a page-size-optimized allocator. Pages are much
   larger (bigger allocated size) than books, so allocating pages from
   the GP allocator causes lots of memory block spans in the allocator
   (i.e. worse de/alloc performance). Likewise, allocating the book
   from a page-optimized allocator means the book allocation takes up
   way more slack space than it needs to. Pages have a runtime-defined
   size, which complicates pool size optimization a tiny
   bit. Anyway...


   @see WHALLOC_API(book_open2)()
   @see WHALLOC_API(book_close)()
*/
WHALLOC_API(book) * WHALLOC_API(book_open)( uint16_t n, uint16_t chunkSize,
                                            WHALLOC_API(realloc_f) alloc, void * allocState,
                                            WHALLOC_API(realloc_f) pageAlloc, void * pageAllocState
                                            );

/**
   Equivalent to WHALLOC_API(book_open)() but takes its allocator
   arguments in a different form. See WHALLOC_API(book_open)() for
   the full details, and below for the differences...

   If alloc or pageAlloc are NULL, a default allocator which uses
   realloc(3) is used in its place.

   alloc and pageAlloc may be the same object if the allocator's rules
   do not prohibit it. (e.g. WHALLOC_API(book_open_alloc_reuse)()
   explicitly prohibits it.)

   If the allocator objects are not NULL, they are SHALLOWLY COPIED
   for future use. Thus the alloc/pageAlloc pointers themselves may
   legally change after this call, but the alloc/pageAlloc->state and
   alloc/pageAlloc->realloc values must stay valid for the life of the
   returned book.

   @see WHALLOC_API(book_open)()
   @see WHALLOC_API(book_close)()
*/
WHALLOC_API(book) * WHALLOC_API(book_open2)( uint16_t n, uint16_t chunkSize,
                                             WHALLOC_API(allocator) const * alloc,
                                             WHALLOC_API(allocator) const * pageAlloc );

    
/**
   Implements WHALLOC_API(realloc_f)() in a highly unconventional
   manner:

   If n is not 0 (an allocation request) then allocState is
   returned. If n is 0 (a free() operation) then NULL is returned. In
   no case is any memory actually allocated or deallocated, and only
   the n parameter is actually evaluated.

   Why?

   This can be used as the book allocator parameter (NOT the page
   allocator parameter!) to WHALLOC_API(book_open)(), passing a
   pointer to an allocated WHALLOC_API(book) object as the allocator
   state parameter.  That will cause WHALLOC_API(book_open)() to use
   the client-supplied object. WHALLOC_API(book_close)() will then
   free the pages but not the book (we assume the client will clean it
   up, or that it was stack allocated and needs no further cleaning).

   BIG FAT HAIRY ACHTUNG: do NOT use this as the page allocation
   parameter to WHALLOC_API(book_open)()!!!! Doing so WILL corrupt
   memory!
*/
void * WHALLOC_API(book_open_alloc_reuse)( void * m, unsigned int n, void * allocState );


/**
   Similar to WHALLOC_API(book_open)(), but this function uses src as
   its allocator for de/re/allocating both the new book object and all
   page allocations the book makes.

   On success the caller owns the returned object and must eventually
   free it using WHALLOC_API(book_close)().

   The src object must outlive the returned object, or results are
   undefined.

   On error, NULL is returned.

   Mutex notes:

   The client may legally set the mutex member of the returned object
   to their own mutex if they do so before concurrent access to the
   returned book becomes possible. If the mutex is also used by the bt
   object then the mutex MUST be recursion-capable.  If the shared
   mutex is not capable of recursive locks then adding new pages to
   the book may cause a deadlock (or a deadlock avoidance in the guise
   of an out-of-memory error). A deadlock avoidance error is particularly
   messy in deallocation, and can lead to a leak.

   An optimization tip/hack for clients:

   If src is to be used soley by the returned object, instead of being
   shared for other purposes, it can be tweaked for optimal memory
   sizing by setting it up to have the same memory block size as the
   _allocated_ size of a WHALLOC_API(page) object (which depends on
   their length and chunk size). This setup ensures as little waste as
   possible in the memory pool (due to slack space) and better
   performance because we are guaranteed that the underlying
   de/allocation routines do not have to span blocks (which changes
   them from O(1) to O(N*M), where N=blocks needed for the
   de/allocation and M is a function of how fragmented the memory pool
   is).
   
   @code
   enum { objsPerPage = 10,
          objSize = sizeof(MyType),
          pageSize = WHALLOC_API_PAGE_CALC_SIZE( objsPerPage, objSize ),
          PoolSize = pageSize * 5//=number of pages in pool
   };
   unsigned char mem[PoolSize];
   WHALLOC_API(bt) bt = WHALLOC_API(bt_empty);
   int rc = WHALLOC_API(bt_init)( &bt, mem, PoolSize, pageSize );
   if( rc ) { ... error ... }
   WHALLOC_API(book) * b =
       WHALLOC_API(book_from_bt)( &bt, objsPerPage, objSize );
   @endcode

   HOWEVER: while this provides an optimal block size for the pages,
   the book is also allocated from such a block. Thus sizing this way
   causes the memory pool block used by the book itself to be way too
   large (a book object much smaller than a page, strangely
   enough). So some pool memory is wasted (in the form of slack space)
   there.
   
   In that configuration, the pager will fail with out-of-memory
   if the pool fills up and the book needs more memory. We can add
   a "fallback plan" which uses the standard realloc(3) for all
   de/re/allocation by adding the following line right after
   WHALLOC_API(bt_init)():

   @code
   bt.base.fallback = whalloc_fallback_stdalloc;
   @endcode
*/
WHALLOC_API(book) * WHALLOC_API(book_from_bt)( struct WHALLOC_API(bt) * src,
                                               uint16_t pageLength,
                                               uint16_t chunkSize );

    
/**
   Frees the memory associated with b, which must have been allocated
   using WHALLOC_API(book_open)(), including all pages in the book. This
   of course invalidates any pointers which refer to memory inside the
   book.

   If b is NULL then this function is a no-op.

   Returs non-zero on error if !b or if mutex locking fails.  On
   success, b is invalidated by this call. On error b is not modified.
*/
int WHALLOC_API(book_close)( WHALLOC_API(book) * b );

/**
   Adds a new page to b. Returns the new page, which is owned by b, on
   success. On error it returns NULL. The page is allocated using
   b->pageAlloc(), and will be deallocated by the book when the time
   comes. Thus, do not hold a pointer to the returned object, as the
   book may free it at any time during the alloc/dealloc routines.

   It is not normally necessary for client code to use this, as
   WHALLOC_API(book_alloc)() adds new pages on demand. The client can
   use this, however, to pre-allocate pages.

   The interface does not specify at what position of the internal
   page list the page must be added, and the implementation is free to
   place it anywhere in the page chain. Having said that: this
   implementation puts it at the start of the chain, since the general
   API policy is to move the least-full pages to the front to speed up
   allocation.
*/
WHALLOC_API(page) * WHALLOC_API(book_add_page)( WHALLOC_API(book) * b );

/**
   Allocates the next available memory slot from b.  On success it
   returns a pointer to that block, which should be treated as being
   b->chunkSize bytes long. On error NULL is returned.

   There is no guaranty whatsoever that objects returned from
   subsequent calls lie in adjecent memory, and the returned value
   should never be used as an array pointer.

   This function adds pages to b as needed, using
   WHALLOC_API(book_add_page)() to allocate the pages. It also may
   re-structure the internal list for (perceived) performance reasons.
   
   Note that re-ordering the internal page list does not actually
   change any addresses within a page, and therefore does not
   invalidate pointers allocated via a page in the book. It often
   does, however, invalidate any pointers to b->page, so _never_ hold
   a pointer to that member.

   Performance notes:

   Allocation is, for the average case, almost (but not quite) O(1).
   When a book de/allocates a block, it may re-position the containing
   page in its internal page list. The book class tries to keep the
   pages with the most free blocks at the front of the list, to help
   ensure fast allocation. The down-side of this is that it can (but
   doesn't necessarily) lead to the opposite effect for deallocation
   (which could become slower, but that depends on the exact de/alloc
   counts and orderings). When an object is deallocated (via
   WHALLOC_API(book_free)()) the book has to ask each page if it
   owns the memory.

   The more pages a book has, the slower average de/allocation
   performance will theoretically be, as the book has to ask each page
   if it owns a given memory block. The exact performance
   characteristics depend largely on the orderings of the individual
   de/allocations and the pageLength/chunkSize.

   Regarding allocation (but not deallocation):

   Since less-used pages are moved to the front, allocation
   performance is (generally) only affected when the front-most page
   fills up (which only happens if all other pages are full or one
   entry away from being full). In this case, the book has to iterate
   over each page (all of which are full now) in order to figure out
   that it is full.

   For peformance, a few long pages are better than more short
   pages, but client requirements may call for allocation in smaller
   chunks, and in such a case more, but smaller, pages might be more
   appropriate. Keep in mind that the memory allocated for a page via
   WHALLOC_API(page_new)() is larger than
   sizeof(WHALLOC_API(page)), as it also allocates memory for the
   paged objects.
*/
void * WHALLOC_API(book_alloc)( WHALLOC_API(book) * b );

/**
   Returns mem, which must have been allocated from b using
   WHALLOC_API(book_alloc)(), to b. Returns 0 on success, non-zero on
   error.

   Performance notes:

   Performance is theoretically O(N), where N is a function of the
   number of pages in b. This is because the book has to ask each page
   if it owns mem. The deallocation itself, once the owning page is
   found, is O(1). Because the internal page list gets moved around to
   optimize allocation speed, it is not possible to calculate the
   exact performance without knowing the exact de/allocation patterns
   used by the client of the book.
*/
int WHALLOC_API(book_free)( WHALLOC_API(book) * b, void * mem );

/**
   De-allocates any pages in b which no longer host any allocated
   items.

   Vacuuming is not a guaranty that any pages _will_ be released,
   because it is possible to have a situation where every page has at
   least one allocated chunk.
   
   If auto-vacuuming is on (see WHALLOC_API(book_vacuum_auto)()) then
   this function need never be called, but if a high chunk recycling
   rate is desired for b then auto-vacuuming is better left off.

   Returns the number of pages removed from b, or 0 if !b.

   This operation is essentially O(N)+O(M), where:

   N is the total number of pages (we have to query each to see if it
   is empty) and M the number of empty pages (which require
   deallocation, and therefore more time). The relative cost of one
   empty page is higher than a non-empty by an unspecified factor
   (performance relies on the underlying deallocator).
*/
unsigned int WHALLOC_API(book_vacuum)( WHALLOC_API(book) * b );

/**
   If autoVac is non-zero (true) then auto-vacuum mode is enabled for
   b, otherwise auto-vacuum is disabled. Autovacuuming means when a
   given page in b deallocates its last chunk, b will automatically
   deallocate the page.

   Returns 0 on success, non-zero on error. The only error cases are
   if !b or locking of b's mutex fails.

   In terms of performance, turning this option on is faster than
   periodically calling WHALLOC_API(book_vacuum)(), but should
   arguably be turned off unless know you won't be recycling most of
   the allocations.

   This is an O(1) operation (not counting the potential mutex lock)
   and does not trigger an immediate vacuum.

   TODO: allow this function to take a threshold value, meaning to
   vacuum if the given number of pages are empty. The problem with
   that is that it increases de/allocation time considerably (at least
   O(N), N=page count) because we would have to check all pages on
   each de/alloc (unless we add more optimizations to the book's
   internals to keep track of the page used/unused counts in the book
   itself). We do not currently have such optimizations because
   changes made to a book from outside of this API would immediately
   invalidate the accounting data.
*/
int WHALLOC_API(book_vacuum_auto)( WHALLOC_API(book) * b, char autoVac );
    
/**
   Erases the chunk-allocation state of b, such that all memory pages
   in it are considered empty and ready for re-allocation. Any
   pointers to memory which were allocated via b (or any of its pages)
   are invalidated by this call.

   Returns 0 on success, non-zero on error. The only error condition
   is !p.

   If alsoDeallocPages is non-zero (true) then all pages in b are
   deallocated, otherwise they are simply erased (see
   WHALLOC_API(page_erase)()) but not deallocated (and not
   deallocated regardless of auto-vacuum setting (see
   WHALLOC_API(book_vacuum_auto)())).
   
   This routine essentially has O(N)*M performance, where N is the
   number of pages in the book and M is the performance of
   WHALLOC_BOOK(page_erase)().
*/
int WHALLOC_API(book_erase)( WHALLOC_API(book) * p, char alsoDeallocPages );
    
    
#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* WANDERINGHORSE_NET_WHALLOC_PAGER_H_INCLUDED */
/* end file whalloc_pager.h */
