#ifndef wanderinghorse_net_nosjob_MUTEX_HPP_INCLUDED
#define wanderinghorse_net_nosjob_MUTEX_HPP_INCLUDED

// #if ! defined(nosjob_CONFIG_SINGLE_THREADED) && !defined(nosjob_CONFIG_HAVE_PTHREADS)
// #  define nosjob_CONFIG_SINGLE_THREADED 1
// #endif

// #if ! defined(nosjob_CONFIG_HAVE_PTHREADS)
// #  define nosjob_CONFIG_HAVE_PTHREADS 0
// #endif

namespace nosjob {

	/**
	   Represents a Mutex used for locking threads.

	   Notes about the underlying Mutex implementation(s):

	   - On single-threaded builds this class has no underlying
	   native Mutex and does nothing.
	   
	   - On pthreads builds, the underlying native Mutex is a
	   per-instance pthread Mutex.

	   - On Win32, per-instance critical sections are used.

	   To simulate a global Mutex, create a shared instance of
	   this type, e.g., via a myApp::getGlobalMutex() function.

           To build with pthreads support, define
           nosjob_CONFIG_HAVE_PTHREADS to a true value while compiling
           these bits of the library. On Win32, its native support is
           automatically selected. To explicitly turn off pthreads on
           platforms which support it define
           nosjob_CONFIG_HAVE_PTHREADS to 0 during compilation. Note
           that the mutex will then be a no-op, performing no locking.
        */
	class Mutex
	{
	public:
		/**
		   Initializes this Mutex.
		*/
		Mutex();
		/**
		   Closes the Mutex.
		*/
		~Mutex() throw();
		/**
		   Locks this Mutex. Trying to lock it a second (or
		   subsequent) time will cause this function to wait
		   until the previous lock(s) is (are) released.

		   Recursive locks are not supported, which means that
		   if you call lock() twice on a Mutex without an
		   intervening unlock(), you will deadlock.  Code
		   using this Mutex must be careful to avoid this case
		   (see the MutexSentry class, which is one
		   solution).

		   Returns a reference to this object mainly to allow this call
		   to be used in ctor member initialization lists, but maybe
		   that will have some other use eventually.
		*/
		Mutex & lock();
		/**
		   Unlocks this Mutex.
		*/
		void unlock();

		/**
		   Copying a Mutex is a no-op. It does nothing but the
		   operators are supported to enable the
		   implementation of client classes which want
		   per-instance Mutex members.
		*/
		Mutex & operator=( Mutex const & );
		/**
		   This ctor ignores its argument and behaves identically
		   to the default ctor. See operator=().
		*/
		Mutex( Mutex const & );
	private:
		struct impl; // opaque internal type
		impl * m_p;
	};

        /**
           This sentry class locks a Mutex on construction and unlocks
           in on destruction. The intended usage is to instantiate it
           at the start of a routine which needs a lock. The
           instantiation will not return until the lock is acquired or
           the locking function throws an exception. Upon destruction
	   of this object, the Mutex will be unlocked.

	   Note that these objects are not copyable.
        */
        class MutexSentry
        {
        public:
		/** Calls mx.lock(). mx must outlive
		    this object.
		*/
                explicit MutexSentry( Mutex & mx );
		/** Unlocks the Mutex we locked in the ctor. */
                ~MutexSentry() throw();
	private:
		MutexSentry & operator=( MutexSentry const & ); // unimplemented!
		MutexSentry( MutexSentry const & ); // unimplemented!
		Mutex & mx;
        };

} // namespace

#endif // wanderinghorse_net_nosjob_MUTEX_HPP_INCLUDED
