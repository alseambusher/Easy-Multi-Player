#ifndef wanderinghorse_net_nosjob_allocator_HPP_INCLUDED
#define wanderinghorse_net_nosjob_allocator_HPP_INCLUDED 1
/** @def NOSJOB_USE_WHALLOC_PAGER

   If NOSJOB_USE_WHALLOC_PAGER is true then we use a paging-based
   allocator for most internal types. If false, we simply use
   new/delete for the allocations. Setting this to a true value
   drastically cuts the number of real allocations performed by the
   nosjob API and has no notable performance impact. In my simple
   tests, runtimes were within a few thousandths of a second of each
   other for both de/allocation policies.

   Some valgrind numbers from my basic test app (i think this was on a
   32-bit machine):

   With paged allocator: 392 allocs, 392 frees, 43,604 bytes allocated

   With new/delete: 735 allocs, 735 frees, 46,293 bytes allocated

   This value _must_ be the same when the library and client code is
   compiled, and thus it should be set in this header and not come in
   from project-specific arguments, i.e. compiling with -D....

   
   BUGS:

   - The paging allocator makes no guarantees about alignment, which
   may be a problem on some platforms or particular use cases.


*/
#define NOSJOB_USE_WHALLOC_PAGER 0

#if NOSJOB_USE_WHALLOC_PAGER
#  include <wh/whalloc/whalloc_amalgamation.h>
// TODO: remove the artificial whalloc_pager/whalloc_bt dependency and only include whalloc_pager from here.
// Requires re-organizing of the whalloc headers.
#  if 0 && !defined(CERR)
#    include <iostream> /* only for debuggering */
#    define CERR std::cerr << __FILE__ << ":" << std::dec << __LINE__ << " : "
#  endif
#endif

#include "Mutex.hpp"

namespace nosjob {

#if NOSJOB_USE_WHALLOC_PAGER
    namespace Detail {
        /**
           Helper for locking/unlocking mutex objects.
           LockingT must be a type implementing lock()
           and unlock() methods.
        */
        template <typename LockingT>
        struct Locker
        {
        private:
            LockingT & mutex;
        public:
            /**
               Calls m.lock(). m must outlive this object.
             */
            Locker( LockingT & m ) : mutex(m)
            {
                m.lock();
            }
            /**
               Calls m.unlock() on the object passed to the ctor.
            */
            ~Locker()
            {
                mutex.unlock();
            }
        };
    }
    /**
       An allocator which uses WHALLOC_API(book) to manage pools of
       objects. Intended for use in allocating internal data for
       concrete Atom implementations.

       This allocator uses malloc() and free() to manage pages of
       memory, with a number of objects per page.

       To provide mutex locking for the underlying allocator, subclass
       this type and override the lock() and unlock() members. The
       underlying pager will then lock/unlock via those
       functions. They should report locking errors via exceptions,
       which this class will translate to error codes for the
       underlying C code (which cannot handle exceptions).
       
       Limitations:

       - T must be Default Constructable. This type allocates the
       memory for it and then calls placement new to initialize each
       object.
       
       - It can only instantiate objects of a single concrete type,
       and not subclasses of that type.
       
       - The allocator is stateful, so it cannot be used as a
       std::allocator.

       - Each instance can only dole out memory in chunks of sizeof(T)
       (which also prohibits it from being used as a std::allocator).

       - It cannot allocate arrays, because arrays may require
       expanding beyond the bounds of an internal page. The underlying
       pager does not guaranty that subsequent allocations made from a
       page come from adjacent memory (though they are likely to be
       close by in memory).

       - If objects allocated by this class are not properly
       deallocated, their memory will be released when the
       WhallocBookAllocator is destructed BUT the destructors for
       those objects will NOT be called. That is because this type
       doesn't have enough information to be able to do that (it does
       not have enough access to the lower-level memory pages).
    */
    template <typename T>
    class WhallocBookAllocator
    {
    private:
        typedef Detail::Locker<WhallocBookAllocator> Locker;
        const uint16_t m_opp;
        const int m_vacuumAfter;
        int m_counter;
        WHALLOC_API(book) m_book;
        /**
           WHALLOC_API(mutex)::lock() impl.  state MUST be a
           WhallocBookAllocator. Exceptions in lock() are translated
           to a non-0 return code.
         */
        static int book_lock( void * state ) throw()
        {
            try
            {
                static_cast<WhallocBookAllocator*>(state)->lock();
                return 0;
            }
            catch(...)
            {
                return -1;
            }
        }
        /**
           WHALLOC_API(mutex)::unlock() impl.  state MUST be a
           WhallocBookAllocator. Exceptions in unlock() are translated
           to a non-0 return code.
         */
        static int book_unlock( void * state ) throw()
        {
            try
            {
                static_cast<WhallocBookAllocator*>(state)->unlock();
                return 0;
            }
            catch(...)
            {
                return -1;
            }
        }
        void open()
        {
            {
#if 0
                Locker const locker(*this)
                    /**
                       A lock is actually useless here, because open()
                       is called during construction, before the
                       derived type is fully constructed, which means
                       (if i understand correctly), that we'll call
                       the base (no-op) lock/unlock() instead of
                       the real locking implementations.
                     */;
#endif
                if( 0 != m_book.mutex.state ) return
                    /* Already initialized. Theoretically this condition
                       can never happen (a ctor on one object cannot be
                       called twice in parallel threads, can it?), but
                       this is the world of multi-threading, so i'm
                       cautiously following the philosophy that whatever
                       is most unexpected is in fact the most likely to
                       happen.
                    */
                    ;
                WHALLOC_API(book) const * x =
                    WHALLOC_API(book_open)( m_opp, sizeof(T),
                                            WHALLOC_API(book_open_alloc_reuse), &this->m_book,
                                            NULL, NULL );
                if( ! x )
                {
                    throw AllocError(__FILE__,__LINE__);
                }
                assert( x == &this->m_book )
                    /* This is a side-effect of our using WHALLOC_API(book_open_alloc_reuse) */
                    ;
                this->m_book.mutex.lock = book_lock;
                this->m_book.mutex.unlock = book_unlock;
                this->m_book.mutex.state = this;
            }
            if( 0 == this->m_vacuumAfter )
            { // achtung: this must not be locked via the above MutexSentry, else deadlock.
                WHALLOC_API(book_vacuum_auto)(&this->m_book,true);
            }
        }

    public:
        /**
           Initializes this object to use the given number of objects
           per memory page. It must be greater than 0.

           If VaccuumAfter is 0 then auto-vacuuming is turned on for
           the pager, which means it deallocates empty pages
           immediately when they become empty.  If it is less than 0
           then the value of objectsPerPage is used. If it is greater
           than 0 then a vacuuming run is made each time that many
           DEallocations have occurred. e.g. if set to 1, it will
           check for free pages after every deallocation. That said, a
           value smaller than objectsPerPage is not practical, and 2-3
           times that number is probably a good value for moderately
           active applications.

           Throws a std::exception if initialization of the underlying
           paging allocator fails.

           FIXME: optimize the (1==VaccuumAfter) case to simply turn on
           auto-vacuuming at the WHALLOC_API(book) level.
        */
        WhallocBookAllocator(uint16_t objectsPerPage,int VacuumAfter = 0)
            : m_opp(objectsPerPage),
              m_vacuumAfter(VacuumAfter>=0 ? VacuumAfter : objectsPerPage),
              m_counter(0),
              m_book(WHALLOC_API(book_empty))
        {
            this->open();
        }
        /**
           Initializes this object to use some rather arbitrary number
           of objects per page. Throws a std::exception if
           initialization of the underlying paging allocator fails.
        */
        WhallocBookAllocator()
            : m_opp(1024*4/sizeof(Type)/*ARBITRARY!*/),
              m_vacuumAfter(m_opp),              
              m_counter(0),
              m_book(WHALLOC_API(book_empty))
        {
            if( m_opp < 5/*arbitrary!*/ ) {
                throw std::range_error("Type size is too large for the default "
                                       "paging settings. Use the non-default ctor!");
            }
            this->open();
        }
        /**
           Frees up all memory in the underlying pager, BUT DOES NOT
           CALL DESTRUCTORS for the objects in it. Thus all objects
           must have been properly destructed by the time this is
           called, or leaks will ensue if destructors are required for
           proper cleanup behaviours. The main problem here is that we
           do not have enough access to the underlying pages in order
           to iterate over them and destruct the ojects.
        */
        virtual ~WhallocBookAllocator()
        {
            WHALLOC_API(book_close)( &this->m_book );
        }
        typedef T Type;
        typedef Type * PointerType;
        /**
           Allocates memory for a new T object, calls its default
           constructor (via placement new), and returns it. If the
           ctor throws then this function deallocates the memory then
           passes on the exception.
        */
        PointerType alloc()
        {
            void * val = WHALLOC_API(book_alloc)( &this->m_book );
            if( ! val ) {
                throw AllocError(__FILE__,__LINE__);
            }
            try {
                // We must manually call the ctor in order to initialize, so...
                return new (val) Type; // my first-ever placement-new()!
            }
            catch(...) {
                WHALLOC_API(book_free)( &this->m_book, val );
                throw;
            }
        }

        /**
           Calls v->~Type(), then deallocates v. Results are undefined
           (possibly disastrous) if v was not previously allocated
           from this->alloc(). If v is null then this function is a
           no-op.
        */
        void dealloc( PointerType v )
        {
            if( v )
            {
                //CERR << "Deallocating "<<m_book.chunkSize<<" bytes @"<<v<<" from book.\n";
                v->~Type();
                /** ACHTUNG: we cannot lock until after the dtor, because the dtor might
                    (and often does) trigger deallocations which come through this function,
                    and that can cause a deadlock.
                */
                WHALLOC_API(book_free)( &this->m_book, v );
                if( m_vacuumAfter > 0 )
                {
                    if( 0 == (++m_counter % m_vacuumAfter) )
                    {
                        m_counter = 0;
                        this->vacuum();
                    }
                }
            }
        }

        /**
           "Vacuums" the memory pages. This frees up any pages which
           have become empty. It is not normally necessary to call
           this from client code.
        */
        void vacuum()
        {
            WHALLOC_API(book_vacuum)(&this->m_book);
        }

        virtual void lock()
        {}
        virtual void unlock()
        {}
    };

    /**
       Functionally identical to WhallocBookAllocator<T>, but overrides
       the lock() and unlock() operations to lock a Mutex object.
    */
    template <typename T>
    class WhallocLockingBookAllocator : public WhallocBookAllocator<T>
    {
    private:
        Mutex m_utex;
    public:
        WhallocLockingBookAllocator(uint16_t objectsPerPage,int vacuumAfter = 0)
            : WhallocBookAllocator<T>(objectsPerPage, vacuumAfter)
        {
        }
        virtual ~WhallocLockingBookAllocator()
        {}
        virtual void lock()
        {
            m_utex.lock();
        }
        virtual void unlock()
        {
            m_utex.unlock();
        }
    };
    
#endif // NOSJOB_USE_WHALLOC_PAGER

//     template <typename T>
//     struct TrivialAllocator
//     {
//         typedef T Type;
//         typedef T * PointerType;

//         PointerType alloc() const
//         {
//             return new T;
//         }

//         void dealloc( PointerType v )
//         {
//             if( v ) delete v;
//         }
//     };
//     template <typename T>
//     struct TrivialAllocator< Detail::RcValue<T> >
//     {
//         typedef Detail::RcValue<T> Type;
//         typedef Type * PointerType;

//         PointerType alloc() const
//         {
//             return new Type;
//         }

//         void dealloc( PointerType v ) const
//         {
//             if( ! v ) return;
//             else if( v->rc() > 1 ) v->decr();
//             else delete v;
//         }
//     };
    
    /**
       A simple allocator which allocates Detail::RcValue<X> objects
       via a WhallocBookAllocator<RcType> allocator.

       This type takes over the management of RcType's lifetime,
       rather than leaving it to the RcType::decr() to decide.

       RcType must be a Detail::RcValue<> type.
    */
    template <typename RcType>
    struct RcValuePagedAllocator
    {
    public:
#if NOSJOB_USE_WHALLOC_PAGER
        typedef WhallocLockingBookAllocator< RcType > ProxyType;
        typedef typename ProxyType::Type Type;
        typedef typename ProxyType::PointerType PointerType;
    private:
        mutable ProxyType impl;
        // Default value. See ProxyType docs.
        static const int ObjPerPage = 64;
        // Default value. See ProxyType docs.
        static const int VacuumHowOften = 64;
    public:
        /**
           Initializes this object with some arbitrary number of
           objects per page which periodically frees up clean internal
           memory pages.
        */
        RcValuePagedAllocator()
            : impl(ObjPerPage,VacuumHowOften)
        {}
        /**
           Initializes the allocator to manage the given number of
           objects per page. objsPerPage must be greater than 0, and a
           value of 1 makes no sense at all.

           See the ProxyType ctor for info about the vacuumHowOften
           parameter.
        */
        RcValuePagedAllocator(uint16_t objsPerPage,
                              int vacuumHowOften = VacuumHowOften)
            : impl(objsPerPage,vacuumHowOften)
        {}
        /**
           Returns a freshly-initialized object which must eventually
           be freed by passing it to dealloc().

           Clients may call incr() on the returned object but must not
           call decr() - instead they should pass the object to
           dealloc(), which will deallocate the object from the
           internal paging mechanism rather than letting decr() delete
           the object.
        */
        PointerType alloc() const
        {
            return impl.alloc();
        }
        /**
           Decrements v's reference count by 1. If v is the last
           reference to itself then it is deallocated. v must have
           been been allocated via alloc().
        */
        void dealloc( PointerType v ) const
        {
            if( ! v ) return;
            else if( v->rc() > 1/* yes! we manage the ==1 case below.*/ ) {
                v->decr();
            }
            else {
                impl.dealloc(v);
            }
        }
#else
        typedef RcType Type;
        typedef Type * PointerType;
    public:
        RcValuePagedAllocator(uint16_t/*ignored*/ = 0,int/*ignored*/ = 0)
//             : impl()
        {}
        PointerType alloc() const
        {
            return new Type;
        }
        void dealloc( PointerType v ) const
        {
            if( v ) v->decr();
        }
#endif /* NOSJOB_USE_WHALLOC_PAGER */
    };

//     template <>
//     struct RcAllocatorPolicy< Detail::RcValue<double> >
//         : RcValuePagedAllocator< Detail::RcValue<double> >
//     {
//         typedef RcValuePagedAllocator< Detail::RcValue<double> > ParentType;
//         RcAllocatorPolicy()
//             : ParentType(128)
//         {}
//         RcAllocatorPolicy(uint16_t objsPerPage,
//                           int vacuumHowOften)
//             : ParentType(objsPerPage,(vacuumHowOften >= 0 ? vacuumHowOften : objsPerPage))
//         {}
//     };

}

#endif /* wanderinghorse_net_nosjob_allocator_HPP_INCLUDED */
