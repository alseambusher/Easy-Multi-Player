#ifndef wanderinghorse_net_nosjob_HPP_INCLUDED
#define wanderinghorse_net_nosjob_HPP_INCLUDED 1
// License is at the bottom of the file.
#include <cassert>
#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS 1 /* for inttypes.h PRIi32 and friends */
#endif
#include <inttypes.h>
#include <string>
#include <stdexcept>
#include <map>
#include <vector>
#include <list>
#include <stdint.h> // C99: fixed-size int types.
#include <cstdio> // sprintf() and friends.
#include <string.h> // memset()
#include <cstdlib> // abort()
/**

*/
namespace {} /* kludge for doxygen */
#include "utfcpp/utf8.h"
/* Fwd decl from JSON_parser lib. */
struct JSON_value_struct;
/* Fwd decl from JSON_parser lib. */
typedef struct JSON_value_struct JSON_value;

/**
   The nosjob namespace encapsulates an API for generating and
   consuming JSON data in C++.

   Primary features:

   - Represents the JSON-defined data types (e.g. numbers and
   strings), called Atoms, using an interface similar to that used by
   C++-based JavaScript engines. Atoms have value semantics and are
   cheap to copy.

   - Some degree of conversion between the types (erring in favour of
   compatibility with JavaScript conversions).

   - Support for ASCII, UTF8, and UTF16 strings.
   
   - Various templates for converting Atoms to UTF strings.

   - Reading JSON input from arbitrary sources using input
   iterators.

   - Writing JSON-format output to arbitrary destinations
   using output iterators.

   - "High-level" values can be "copied down" to a base Atom object,
   copied around in that form, and then "cast" back to their
   high-level type, without any data loss. Such copies use reference
   counting where possible, and are cheap in terms of memory and
   run-time. This is all done without virtual inhertiance or virtual
   functions. (None of the core data types use virtual inheritance.)
   
   This API is influenced heavily by 3rd-party JavaScript engine
   implementations, namely Qt's QtScript and the Google v8 engine.

   Primary misfeatures:

   - It is not designed to support an infinite set of types, nor is it
   designed to support the full range of JavaScript semantics. Only
   the features required by JSON are provided.

   - As a side-effect of the library's "non-virtual, yet still
   polymorphic" object copying policy, the internals of the library
   are significantly more convoluted than they would be if i had
   allowed the Atom class to use virtual inheritance. It also seems to
   be difficult, or damned near impossible, to do multiple levels of
   inheritance from the base Atom class this way.
   
   - It uses static_cast<>() quite a bit to cast beteen (void*) and
   internal data types. However, it only does so in certain
   private/internal contexts which shouldn't ever be [able to be]
   reached unless the (void*) being cast is not of the correct type.
   Maybe casts don't bug you, but they bug me.

   - Due to internal overhead, it is relatively memory-inefficient
   for booleans and numeric types. sizeof(Atom) on a 64-bit machine
   is 16 bytes (half that on 32-bit machines).

   - It allocates and frees memory very often (but always small
   amounts, except for large client-provided strings). It needs to
   integrate a memory pool for its internal allocations, but initial
   attempts failed when allocating objects which contain
   std::maps. e.g. the map iterator::operator--() segfaults when the
   map itself is allocated as part of an object with allocate from
   Loki::SmallObjAllocator or a custom memory pool. Weird.  LOL! 
   Running Loki's own SmallObjAllocator benchmarks shows that malloc()
   is (on my system) faster than all the benchmarked alternatives,
   including SmallObjAllocator, for the cases i would use.
   

   Significant TODOs:

   - The API contains a lot of evolutionary cruft which needs to be
   cleaned up.

   - The Object and Array implementations are nearly code-identical.
   Consolidate them where possible. Same goes for Utf8String and
   Utf16String.

   - Add TypeID::ClientString, and hooks to allow clients to plug in
   their own native string type by providing specializations of some
   core bits (and implementing the Atom/AtomAPI APIs). If we can
   consolidate the core of the Object/Atom implementations into a
   single template, then such extensions would be realatively simple.

*/
namespace nosjob {
    /**
       Holds the core type IDs used by this framework. They are based
       on the JavaScript variable/variant/value model.
    */
    struct TypeID
    {
        enum Types {
        Unspecialized = -1,
        /** Range-begin marker. */
        _AtomsBegin = 0,
        /** Analog to JavaScript's 'undefined' value. */
        Undefined = _AtomsBegin,
        /** Analog to JavaScript's 'null' value. */
        Null,
        /** Analog to JavaScript's boolean type. */
        Boolean,
        /** Analog to JavaScript integers. */
        Integer,
        //UInt,
        /** Analog to JavaScript doubles. Almost. (We probably have
         lower precision than JavaScript allows for.)
        */
        Double,
        /** A UTF-8 string class. */
        Utf8String,
        /** A UTF-16 string class. */
        Utf16String,
//         /** NYI: For eventual binding of arbitrary (void*) to Atoms.
//          */
//         External,
        /** Range-end marker. */
        _AtomsEnd,
        /** Range-begin marker. */
        _CompoundsBegin = _AtomsEnd,
        Object = _CompoundsBegin,
        Array,
        //Function,
        /** Range-end marker. */
        _CompoundsEnd
        };

        /**
           Returns true if id represents an Atom type.
        */
        inline static bool isAtom( Types id )
        {
            return (id >= TypeID::_AtomsBegin)
                && (id < TypeID::_AtomsEnd)
                ;
        }
        /**
           Returns true if id represents a Compound type.
        */
        inline static bool isCompound( Types id )
        {
            return (id >= TypeID::_CompoundsBegin)
                && (id < TypeID::_CompoundsEnd)
                ;
        }
        /**
           Returns true if id represents an Atom type.
        */
        inline static bool isAtomID( int id )
        {
            return (id >= int(TypeID::_AtomsBegin))
                && (id < int(TypeID::_AtomsEnd))
                ;
        }
        /**
           Returns true if id represents a Compound type.
        */
        inline static bool isCompoundID( int id )
        {
            return (id >= int(TypeID::_CompoundsBegin))
                && (id < int(TypeID::_CompoundsEnd))
                ;
        }
        inline static bool isTypeID( int id )
        {
            return isAtomID(id) || isCompoundID(id);
        }
        
    };


    /**
       The base exception type used by this library.
    */
    class Error : public std::exception
    {
    protected:
        Error() throw();
        void what(std::string const &msg)
        {
            this->m_what = msg;
        }
        
    public:
        /**
           Initializes the exception with the given informative message.
           The what string is copied, so this exception type should not be
           thrown on allocation errors (where we presumably have no memory
           left to copy to).
         */
        Error( char const * what );
        ~Error() throw();
        /**
           The exception's informational message.
         */
        char const * what() const throw();
    protected:
        std::string m_what;
    };

    /**
       An Error type which does not allocate any resources (e.g. a
       string), so it should be safe to throw for an allocation error.
    */
    class AllocError : public Error
    {
    public:
        AllocError() throw();
        /** Intended to be passed the __LINE__ and __FILE__ macros.
            The file ptr value is not copied and MUST be static, or
            results are undefined.
        */
        AllocError( char const * file, int line ) throw();
        ~AllocError() throw();
        char const * what() const throw();
        /** Returns the filename passed to the constructor,
            or some unspecified string (not null) if no name
            was provided.
        */
        char const * file() const throw();
        /** Returns the line number passed to the constructor,
            or a negative number if none was provided.
        */
        int line() const throw();
    private:
        char const * m_file;
        int m_line;
    };

    /**
       An exception specifying that some sort of type mismatch has
       occurred.
    */
    class TypeMismatchError : public Error
    {
    public:
        /**
           Sets the what() string to something along the lines of
           ("type mismatch: expected"+expected+" but got "+got).
        */
        TypeMismatchError( TypeID::Types expected, TypeID::Types got );
        ~TypeMismatchError() throw();
    };

    class Atom;

    /**
       AtomAPI defines the base interface required by Atom types. Each
       Atom has a pointer to a shared (per-type) AtomAPI object, and
       that object defines how type-specific various operations are
       performed on an Atom. This API allows any compliant Atom
       subclass to be copied to a base Atom object, copied around, and
       converted back without any data loss. Basically, it specifies
       the features needed for "polymorphic" copying of type-specific
       metadata.

       Possible future needs:

       - bool StrictEquals( Atom const & lhs, Atom const & rhs )

       StrictEquals() returns true only if lhs/rhs refer to the same
       value and the same native type, not performing any
       implicit/convenience type conversions.

       In JS-space:

       - (Number(3) === Number(3)) == true

       - (Number(3.33) === Number(3.33)) == true

       - (Number(3.33) === Number(3.333)) == false


       Reminders to self:

       - The current data copying mechanism works fine until we get to
       the cast() operations provided by subclasses, then we find that
       delayed instantiation does not play well with this approach.

       - This mechism does not directly support multiple levels
       of inheritance, which limits it greatly.

       TODOs:

       - Possibly refactor to use a virtual interface. Since there is
       only one instance of these per type (and they're static/const),
       this isn't a concern as it is with the Atom type (which doesn't
       use virtual inheritance).
    */
    class AtomAPI
    {
    public:
        /**
           Must clone self's data to dest and return dest's copy
           of the memory (which is owned by dest). This function
           does not ensure that dest's data destructor can
           handle that data - that is up to the caller.

           Note that this is a SHALLOW clone for purposes of non-POD
           types. That is, types which require a higher-order object
           to store their state should only copy a reference to the
           original here, instead of copying it outright.

           self and dest must both have the same typeID(), but the
           core Atom API will take care of setting that before calling
           this. Implementations are free to throw if that
           precondition is not met.

           Should return a pointer to dest's (possibly new) data.
        */
        typedef void * (*CopyValue)( Atom const & self, Atom & dest );
        /**
           Must disassociate self from its internal data. If must
           deallocate (if necessary) self's data and update self's
           state accordingly.
        */
        typedef void (*DataDtor)(Atom & self);
        /**
            Must return true if self evalues to a "true" value, else
            false.  The exact interpretation is class-dependent, but
            implementations are encouraged to follow JavaScript
            conventions for the data type. (e.g. a string is "true" if
            it has any non-empty value.)
        */
        typedef bool (*ToBool)(Atom const & self);


        /**
           Not yet used, and might never be.

           The plan is that this will provide the "empty value" which
           is used to initialize default-constructed atoms. In
           practice, most of the concrete Atom types which allocate
           memory use an internal shared object as the value for
           default-constructed instances of the Atom type. The purpose
           of this function would be to formalize where that "empty"
           object comes from.
        */
        typedef void * (*EmptyValue)( Atom const & self );
        
        /**
           A function for comparing atoms using memcmp()/strcmp()
           semantics. Returns less than zero if lhs is "less than"
           rhs, 0 if they are equal, and greater than 0 if lhs is
           "greater than" rhs. Interpretations are of course
           type-dependent.

           Suggestions for implementations:

           - If the TypeIDs match, or are semantically compatible
           for comparison purposes (e.g. Integer and Double), then
           do the appropriate comparison.

           - First compare TypeIDs. If they are not the same, it will
           not always be possible to compare, in which case return
           non-0. The convention is to return a comparison of each
           type's TypeID value. This way like-types will group
           together in sorting operations which use operator<().
        */
        typedef int (*Compare)( Atom const & lhs, Atom const & rhs );

        /**
           Not yet used. Might never be.
        
           Must return true if atom is of a type which can, by this
           AtomAPI instance, be converted to the given AtomAPI type
           via a CopyValue operation.
        */
        typedef bool (*CanConvertTo)( Atom const & atom, AtomAPI const * toAPI );
        
        /**
           A concrete DataDtor() impl which does nothing.
        */
        static void DataDtor_noop(Atom & self);

        /** A ToBool() impl which always returns false. */
        static bool ToBool_false(Atom const & self);

        /**
           A ToBool() impl which returns true if self's internal
           data is not a null pointer.
        */
        static bool ToBool_voidptr(Atom const & self);

        /**
           A CopyValue() impl which copies self's internal data
           pointer's value to dest. Must not be used for types data
           pointers point to dynamically-allocated memory, as ownership
           of that memory then becomes unpredictable.
        */
        static void * CopyValue_copy_ptr( Atom const & self, Atom & dest );

    private:
        friend class Atom;
        const TypeID::Types typeID;
        const CopyValue cloneData;
        const DataDtor dtor;
        const ToBool toBool;
        const Compare compare;
    protected:
        AtomAPI(TypeID::Types tid,
                CopyValue cloneData,
                DataDtor dtor,
                ToBool toBool,
                Compare cmp)
            : typeID(tid),
              cloneData(cloneData),
              dtor(dtor),
              toBool(toBool),
              compare(cmp)
        {}
        ~AtomAPI()
        {
        }
    public:
        /**
           A shared instance which implements the special "null" type.
         */
        static AtomAPI const & Null();
        /**
           A shared instance which implements the special "undefined" type.
         */
        static AtomAPI const & Undefined();
    };

    namespace Detail
    {
        /** @internal

        A template which provides access to Atom internals. It must be
           specialized.
        */
        template <bool ReadWrite>
        struct AtomAccess;
        /** @internal
            Private impl detail. Defined internally.
        */
        template <> struct AtomAccess<false>;
        /** @internal
            Private impl detail. Defined internally.
        */
        template <> struct AtomAccess<true>;

        /**
           An internally-used referencing-counting type. It is NOT
           full-featured. For the rc features to work it must be
           created on the heap and the incr()/decr() members used
           to change the reference count.

           ValueT must be default-constructable, copy-constructable,
           and have value semantics.

           FIXME? Adding full value semantics, as that might simplify
           some of the Atom implementations by a small amount.
        */
        template <typename ValueT>
        class RcValue
        {
        private:
            mutable size_t m_rc;
            RcValue & operator=(RcValue const &);
            RcValue( RcValue const & );
        public:
            ValueT value;
            typedef ValueT ValueType;
            explicit RcValue(ValueType const &val)
                : m_rc(1),
                  value(val)
            {}
            RcValue() : m_rc(1),value() {}
            ~RcValue() {}
//             ValueType & value() { return m_val; }
//             ValueType const & value() const { return m_val; }
            /** Returns the current reference count. */
            size_t rc() const { return m_rc; }
            /**
               Increases the reference count by 1.
             */
            void incr() const
            {
                ++m_rc;
            }
            /**
               Decreases reference count by 1. If it reaches 0 then
               (delete this) is called. Returns the current
               (pos-decrement) count.
            */
            size_t decr() const
            {
                --m_rc;
                if( 0 == m_rc ) {
                    delete this;
                    return 0;
                }
                else return m_rc;
            }
        };
    }

    
    /**
       An Atom respresents a basic atomic value. It is not
       atomic in the concurrency sense of the word, but in
       the value sense of the word.

       Most Atoms are immutable, in the sense that they provide no
       APIs, other than their constructors and copy operators, for
       setting their values. This allows them to share the underlying
       value data where it makes sense to do so. Their copy operators
       must work such that re-assigning the value does not invalidate
       other handles to that object's data (such handles keep a
       reference to the old value).

       Some peculiarities of this model...
       
       Subclasses of Atom must provide/implement an AtomAPI object
       which conforms to the AtomAPI interface, and each instance of
       that class must point its internal m_api member to that
       instance.

       It is possible to copy any Atom subclass to a base Atom object,
       then convert it back, without data loss. e.g.

       @code
       Atom av = Utf8String("Hi, world!");
       @endcode

       That will create a temporary string object, copy its internal
       data to Atom, and change the internal state of Atom such that
       it can later be converted back to a string with:

       @code
       Utf8String s( Utf8String::cast( av ) );
       std::cout << s.value() << std::endl; // "Hi, world!"
       @endocde

       The various ConcreteAtomType::cast() functions throw if the
       argument they are given is not of the exact same type (or
       something which they can sanely convert). Thus the following
       would throw an exception:

       @code
       Integer i( Integer::cast(av) ); // throws because av is-a Utf8String.
       @endcode

       The ability to copy atom-internal data this way requires that
       subclasses store all data in a way which the base Atom type can
       work with (to some degree) without having to know any
       type-specific details. They also must define, without using
       virtual functions, how to interpret and manipulate that data.

       The base Atom class holds a (void*) for holding such
       data. Subclasses which store values which are the same size as,
       or smaller than, (void*) can store their data directly in that
       pointer without requiring any memory allocation. Types which
       must store larger data types, or must store higher-level
       objects, must dynamically allocate it and point their internal
       void pointer at it. Implementations are strongly encouraged to
       use reference counting to keep from re-allocating copies of the
       same data when atoms are passed around.

       Each Atom instance holds an AtomAPI pointer which describes how
       that internal void pointer is used. The AtomAPI interface
       describes those "virtual" functions necessary for basic Atom
       operations, and concrete AtomAPI instances provide support for
       a given interpretation of the internal data. All instances of a
       given Atom type/subtype share the same underlying AtomAPI
       instance, as this is much cheaper (in terms of memory) than
       having each atom instance hold copies of the required function
       pointers. Copying Atoms "polymorphically", as demonstrated
       above, requires that we modify the internal API pointer of the
       target Atom, so that it knows (indirectly) how to work
       with/interpret its new data pointer value. In a sense we are
       partially replacing its "vtbl" pointer, but restricting the
       change to the parts which are specific to a given underlying
       data type.

       Subclasses cannot, if we are to support the above copying
       features, provide their extra functionality via
       subclass-specific private member data or virtual functions. The
       problem is that the virtualness (or internal private data) is
       lost when copying from a derived type to a base Atom value.
    */
    class Atom
    {
    private:
        /** AtomAccess<true> provides "back-door" read/write access to
            the Atom internals. */
        friend class Detail::AtomAccess<true>;
        /** AtomAccess<false> provides "back-door" read-only access to
            the Atom internals. */
        friend class Detail::AtomAccess<false>;
         /* Friendship unfortunately required by some of the static AtomAPI members. */
        friend class AtomAPI;

    protected:
        /**
           This object's concrete AtomAPI implementation. All
           instances of a specific logical type must have the same
           m_api value. It is illegal for this value to be 0.
        */
        AtomAPI const * m_api;

        /**
           The internal data pointer. Interpretation of
           this value is up to the m_api object. It is mutable
           so that:

           1) it can be used to hold, e.g., a reference count inside
           the allocated object and still be const-legal for
           Atom-level assignment operations.

           2) It can be lazily instantiated if desired.
        */
        mutable void * m_data;

        /** Returns this->m_data. */
        inline void * dataPtr() { return this->m_data; }

        /**
           Assigns this->m_data, but does not destroy any existing
           data (which can lead to a leak if not used properly). The
           caller must call this->destroyData() if that needs to be
           done before re-assignment.

           This is not actually const - it modifies a mutable member.
        */
        inline void * dataPtr( void * v ) const { return this->m_data = v; }
        /** Initializes the atom with the given type info. Throws
            Throws if !TypeID::isAtom(api->typeID) and
            !TypeID::isCompound(api->typeID).
        */
        explicit Atom( AtomAPI const * api );

        /**
           Copies rhs's state into this object, possibly
           changing its type (i.e. its m_api value).

           Whether or not this is a deep or shallow copy depends
           on the Atom implementation. Those types which use
           reference counting (e.g. strings and Objects) will simply
           copy a pointer, whereas POD types which do not need to
           allocate will simply copy a single value.

           If this->api()->cloneData is not 0 then it is used
           to clone the data, otherwise the data pointer is copied,
           under the assumption that the atom's data is encoded
           directly in the data pointer.

           This function re-assigns this object to be of the same
           virtual type (the same typeID() value) as rhs.

           This function may propagate an exception on error.
        */
        Atom & copyOp( Atom const & rhs );

        /**
           Calls this->api()->dataDtor( this->m_data ) if none of
           m_api, m_api->dataDtor, or m_data are 0. That function is
           responsible for cleaning up any type-specific internal
           resources.

           Before re-assigning values, this function should be called
           to clean up any state (if needed by the particular atom
           type).
        */
        void destroyData();
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Undefined;
        /**
           Initializes this object with the type TypeID::Undefined.
        */
        Atom();
        /**
           Calls this->destroyData().
        */
        ~Atom();
        /**
           Copies all internal state of the given Atom, which must be
           either an Atom or a fully-compliant subclass. If subclasses
           implementat reference counting and such, this routine
           may or may not need to allocate anything.

           See the protected copyOp() function.
        */
        Atom & operator=( Atom const & );
        /**
           Equivalent to operator=(rhs).
        */
        Atom( Atom const & rhs );

        /** The type ID of the current value. */
        TypeID::Types typeID() const;

        /**
           Returns true if this atom "evaluates to true", but what
           exactly that means depends on the concrete Atom
           subclass. The base/default implementation returns true if
           the internal data pointer has a non-zero value.
        */
        bool boolValue() const;

        /**
           Returns this object's internal data pointer. The meaning
           depends on its exact type. Any non-const access to this
           atom can invalidate or change the underlying pointer,
           invalidating the returned value. This pointer must never be
           held - it is only for short-term reference in very specific
           cases.

           This function is only in the public API so that concrete
           Atom implementations can do "certain things" with it when
           passed an Atom argument (as opposed to an argument of their
           own concrete type). "Certain things" includes checking
           whether the pointer is of a specific type.
        */
        void const * dataPtr() const { return this->m_data; }

        /**
           Equivalent to (this->typeID() == id).
        */
        bool isA( TypeID::Types id ) const;
        /**
           Equivalent to isA(other.typeID()).
        */
        bool isA( const Atom & other ) const
        {
            return isA( other.typeID() );
        }

        /**
           A shared "null" value.
        */
        static const Atom Null;

        /**
           A shared "undefined" value.
        */
        static const Atom Undefined;

        /**
           Returns true if this object has the special Undefined value.
         */
        bool isUndefined() const;

        /**
           Returns true if this object has the special Null value.
         */
        bool isNull() const;

        /**
           Returns true if this value is neither isNull() nor isUndefined().
         */
        bool hasValue() const;

        /**
           Returns the API object associated with this object, or
           throws if it is NULL.
        */
        AtomAPI const * api() const;

        /**
           Comparess this object's value to rhs' value
           using memcmp() semantics. Ordering is unspecified
           when comparing non-comparable types (e.g. comparing
           an Array and a String).
        */
        int compare( Atom const & rhs ) const
        {
            return this->m_api->compare(*this, rhs);
        }
        bool operator<( Atom const & rhs ) const
        {
            return 0 > this->compare(rhs);
        }
        bool operator>( Atom const & rhs ) const
        {
            return 0 < this->compare(rhs);
        }
        bool operator==( Atom const & rhs ) const
        {
            return 0 == this->compare(rhs);
        }
        bool operator!=( Atom const & rhs ) const
        {
            return 0 != this->compare(rhs);
        }

        /**
           Returns true if this object and rhs are of the same type
           and have equivalent values.
        */
        bool strictEquals( Atom const & rhs ) const
        {
            return (rhs.m_api == this->m_api)
                && (*this == rhs);
        }
        
    };
    
    /**
       An Atom type representing immutable boolean value.

       Peculiarities of the implementation:

       The Atom::compare() implementation will use boolValue() to get
       the other atom's value for comparison purposes. This might or
       might not be philosophically correct vis-a-vis comparison with
       Objects, Arrays, and Strings.
    */
    class Boolean : public Atom
    {
    private:
        /** Internal helper to set the value. */
        void setValue( bool b );
        class APIImpl;
        friend class APIImpl;
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Boolean;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Boolean;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();

        typedef bool ValueType;
        
        /** Initializes this object with a false value. */
        Boolean();
        /** Initializes this object with the given value. */
        explicit Boolean( ValueType v );
        /** Copies the state of v. */
        Boolean( Boolean const & v );
        /** Initializes this object with the value v.boolValue(). */
        explicit Boolean( Atom const & v );
        ~Boolean();
        /** Copies the state of v. */
        Boolean & operator=( Boolean const & v );
        /** Initializes this object with the given value. */
        Boolean & operator=( bool v );
        /** Alias for this->boolValue(). */
        
        ValueType value() const;

        /**
           Returns true if a is-a Boolean. This isn't normally
           necessary, as all Atoms are required to support a
           boolValue() operation, so they can all be evaluated in a
           boolean context.
        */
        static bool isBoolean( Atom const & a );

        /**
           Returns Boolean(v.boolValue()).
        */
        static Boolean cast( Atom const & v );

        /**
           A shared "true" value.
        */
        static const Boolean True;
        /**
           A shared "false" value.
        */
        static const Boolean False;

    };

    namespace Detail
    {
        template <bool B>
        struct IntSizeChooser
        {
            typedef int32_t Type;
        };
        template <>
        struct IntSizeChooser<true>
        {
            typedef int64_t Type;
        };
    }
    /**
       An Atom type representing an immutable integral value.

       The size of its integer type depends on the platform:

       - If (sizeof(void*)>=sizeof(int64_t)) then 64-bit integers
       are enabled.

       - Otherwise 32-bit.

       The reason for this distinction is because if we want to
       support 64-bit values on 32-bit plaforms then we would need to
       dynamically allocate the integer values. On platforms where an
       int is small enough (or a (void*) big enough), we simply store
       the integer value in the underlying data pointer.

       TODO: use 64-bit int for all cases, to prevent that a 32-bit
       client cannot read large values written by a 64-bit client.

       Peculiarities of the implementation:

       The Atom::compare() implementation will behave as expected
       for Boolean, Integer, and Double arguments, but for other types
       it only compares the type IDs of the two atoms.
    */
    class Integer : public Atom
    {
    private:
        /** Internal AtomAPI impl. */
        class APIImpl;
        friend class APIImpl;
        /**
           Template function which determines whether (void*) is big
           enough to hold an int64_t value.
        */
        typedef Detail::IntSizeChooser<sizeof(void*)>=sizeof(int64_t)> IntSizeChooser;
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Integer;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();
        /**
           The underlying native value type. Either int32_t or
           int64_t, depending upon sizeof(void*).
        */
        typedef IntSizeChooser::Type ValueType;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Integer;
        /**
           The number of bits in ValueType.
        */
        static const uint8_t BitCount = sizeof(ValueType)*8;
    private:
        void init( ValueType v );
//         template <typename IntT>
//         void init( IntT v )
//         {
//             this->m_api = Integer::API;
//             this->m_data = reinterpret_cast<void*>(ValueType(v));
//         }
    public:
        Integer();
//         explicit Integer( int8_t v );
//         explicit Integer( int16_t v );
//         explicit Integer( int32_t v );
//         explicit Integer( uint8_t v );
//         explicit Integer( uint16_t v );
//         explicit Integer( uint32_t v );
        explicit Integer( ValueType v );
        ~Integer();

        /**
           Returns this->value() converted to NumT.  NumT must be a
           numeric type. This function does not check for overflow,
           underflow, the flow of time, or any sort of other flow.
        */
        template <typename NumT>
        NumT numberValue() const
        {
            return NumT(this->value());
        }
        /**
           Returns this object's integral value.
        */
        ValueType value() const;

        /**
           Returns true if atom a appears to be-a Integer.
        */
        static bool isInteger( Atom const & a );

        /**
           If v appears to be one of (Bool, Integer, Double) then this
           function creates a new object to wrap its value and returns
           that object. If v does not appear to be one of those types
           then an exception (deriving from std::exception) is thrown.
           Boolean atoms will evaluate to a numeric value of 0 for false
           and "an unspecified non-zero value" for true.
        */
        static Integer cast( Atom const & v );
    };


    /**
       An Atom type representing an immutable Double value.
    
       Peculiarities of this type:

       - Its API follows the Atom axiom that values are immutable.
       They can be constructed and copied over. Copying over them
       does not change the value seen by other handles which point
       to that value.

       - It uses a reference-counting allocator for its internal
       storage, so all handles pointing to the same underlying data
       all share that data. It is cleaned up when the last handle is
       destructed. It does not allocate for default-constructed
       objects or those constructed with an initial value of 0.0.

       TODOs:

        - For platforms where (void*) is large enough, don't
        dynamically allocate a double, but store it directly
        in this->m_data (void*) instead.


        Peculiarities of the implementation:

       The Atom::compare() implementation will behave as expected
       for Boolean, Integer, and Double arguments, but for other types
       it only compares the type IDs of the two atoms.
    */
    class Double : public Atom
    {
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Double;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Double;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();
        /**
           The underlying native value type.
        */
        typedef double ValueType;
    public:
        Double();
        explicit Double( ValueType v );
        Double( Double const & d );
        explicit Double( Integer const & v );
        Double & operator=( Double const & v );
        Double & operator=( Integer const & v );
        ~Double();
        double value() const;

        /**
           Returns true if a is-a Double.
        */
        static bool isDouble( Atom const & a );

        /**
           If v appears to be one of (Bool, Integer, Double) then this
           function creates a new object to wrap its value and returns
           that object. If v does not appear to be one of those types
           then a std:;exception (range_error) is thrown. Booleans
           behave the same as described for Integer::cast().
        */
        static Double cast( Atom const & v );

        /**
           Returns true if this object's value is the special
           not-a-number value.

           Note that the JSON RFC does not allow NaN numbers.
        */
        bool isNaN() const;

        /**
           Returns true if this object's value is the special
           infinity value.

           Note that the JSON RFC does not allow the JS-specified
           Inifinty value. 
       */
        bool isInfinity() const;

        /**
           Returns true if this object's value is finite.
        */
        bool isFinite() const;

        /**
           Equivalent to ::remainder( this->value(), whenDividedBy.value() ).
        */
        double remainer( Double const & whenDividedBy );

        /**
           An opaque internal type which is only in the public API so
           that certain internal templatized operations can work.
         */
        friend class Impl;

    private:
        class APIImpl;
        friend class APIImpl;
        typedef Detail::RcValue<ValueType> Impl;
        Impl * impl() const;
        void init( ValueType v, Impl * origin );
        Double( Atom const & v );
    };


    class Utf16String;

    /**
       An Atom type representing an immutable string using either
       ASCII or UTF-8 encodings. When used with UTF8, the client is
       responsible for passing correct byte ranges (e.g. the length,
       in bytes, of a UTF-8 string may be larger than the number of
       logical characters).

       Peculiarities of this type:

       - Its API follows the Atom axiom that values are immutable.
       They can be constructed and assigned to, but not modified
       in-place.

       - It uses a reference-counting allocator for its internal
       storage, so all handles pointing to the same underlying
       data all share that data. It is cleaned up when the last
       handle is destructed. It does not allocate for default- or
       empty-constructed objects.

       TODOs:

       - Indexed access? This requires special handling to get
       multi-byte UTF8 working, and it requires an O(N) algorithm
       unless i literally cache the offsets of all virtual characters
       (which would double the memory requirements).

       Peculiarities of the implementation:

       The Atom::compare() implementation will behave as expected
       for Utf8String and Utf16String arguments, but for other types
       it only compares the type IDs of the two atoms.
    */
    class Utf8String : public Atom
    {
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Utf8String;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Utf8String;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();
        /**
           The underlying native value type. The exact type is an
           implementation detail, and not be be relied upon in client
           code.

           TODO??? possibly remove std::string and use a small custom
           wrapper. We can get away with that because strings are
           immutable. OTOH, it is conventient to take advantage of
           std::string's reference counting and CoW.
        */
        typedef std::string ValueType;

    private:
        /** Internal implementaion detail. */
        class APIImpl;
        friend class APIImpl;
    protected:
        typedef Detail::RcValue<ValueType> Impl;
        Impl * impl() const;
        void init( ValueType const & val, Impl * origin );

        /**
           Allocates a new string value. If origin is not null then it is
           used as the primary copy. This function uses custom memory allocation
           and may returned any of:

           - A pointer to some shared string object, if !origin and
           val.empty(), or if origin and origin->empty().

           - A non-const pointer to origin (legally obtained without
           casting), representing a reference-counted pointer to origin.

           - A new object.

           If origin is non-null then val is ignored, otherwise it copied
           as the string value of the new object.
           
           The returned value, regardless of its value, must eventually be
           freed using Utf8String::deallocValue().

           Throws if:

           - !origin and val contains illegal UTF8 data.
        */
        static Impl * allocValue( ValueType const & val, Impl * origin );
        /**
           "Frees" val, which must have been allocated using
           Utf8String::allocValue(). It might not _actually_ free
           val, depending on several factors, but in all cases the
           semantics are as if val is freed by this call.
        */
        static void deallocValue( Impl * val );
    public:
        typedef char CharType;
        Utf8String();
        explicit Utf8String( ValueType const & v );
        /**
           Equivalent to Utf8String( v, v?strlen(v):0 ).
        */
        explicit Utf8String( char const * v );
        /**
           Equivalent to v.utf8Value().
        */
        explicit Utf8String( Utf16String const & v );
        /**
           Initializes the string with the first len bytes
           from v. If v is UTF8 and len does not point to
           a character boundary, results are undefined.

           Throws if it detects 
        */
        Utf8String( char const * v, size_t len );
        /**
           Initializes this object to point to v.  Other handles to
           this string are not invalidated - they maintain the older
           value.
        */
        Utf8String & operator=( Utf8String const & v );
        /**
           Sets this object's contents to the UTF8 form of v.
           Throws if v cannot be converted to UTF8.
         */
        Utf8String & operator=( Utf16String const & v );
        /**
           Implemented to allow using this type as std::map keys. The
           exact sorting rules are not specified by this API (it
           depends on the internal type we use to hold the string
           data), but the implementation currently uses
           ValueType::operator<().
        */
        bool operator<( Utf8String const & other ) const;

        /**
           See operator=(Utf8String).
        */
        Utf8String( Utf8String const & v );
        /**
           Cleans up any interally-allocated resources (or reduces
           their reference count).
        */
        ~Utf8String();

        /** Returns the size, in _bytes_, of the string's value.
            For UTF8 values the size may differ from the number
            of characters.
         */
        size_t lengthBytes() const;

        /**
           Returns the length, in characters (UTF8 code points), of
           this string. This is an O(N) operation and will throw if
           counting fails due to invalid UTF8.
        */
        size_t lengthChars() const;

        /** Returns the underlying string bytes, which are owned by
           this object. */
        CharType const * c_str() const;

        /**
           Returns the raw value of the string, which is lengthBytes()
           _bytes_ long. For UTF8 strings, the number of bytes might
           be larger than the number of logical characters.
        */
        ValueType value() const;

        /**
           Returns true only if a appears to be-a Utf8String.
        */
        static bool isUtf8String( Atom const & a );

        /**
           Alias for isUtf8String(). Provided for transparency in the
           StringType typedef.
        */
        static bool isString( Atom const & a ) { return isUtf8String(a); }

        /**
           If v appears to be a Utf16String or Utf8String then this
           function creates a new Utf8String object to wrap its value
           and returns that object. If v does not appear to be an one
           of those types a TypeMismatchError is thrown.

           If v is-a Utf8String then the returned copy will refer the
           same underlying string data (no allocation is required).
           
           TODO:

           - Convert other types to string: Integer, Double. Boolean
           is tricky: what string to use (true/false, 1/0, t/nil,
           "1"/"", ...)?
        */
        static Utf8String cast( Atom const & v );

        /** Returns true if this string object is empty (has a length of 0). */
        bool empty() const;

        static int strcmp( Utf8String const & lhs, Utf8String const & rhs );

    };

    /**
       An Atom type representing an immutable UTF-16 string.

       Peculiarities of this type:

       - Its API follows the Atom axiom that values are immutable.
       They can be constructed and copied over, but not modified.

       - It uses a reference-counting allocator for its internal
       storage, so all handles pointing to the same underlying
       data all share that data. It is cleaned up when the last
       handle is destructed. It does not allocate for default- or
       empty-constructed objects.

       - It uses host-dependent byte ordering, and internally makes no
       assumptions about the byte order.

       - The Atom::compare() implementation will behave as expected
       for Utf8String and Utf16String arguments, but for other types
       it only compares the type IDs of the two atoms.
    */
    class Utf16String : public Atom
    {
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Utf16String;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Utf16String;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();
        /**
           The character type for the string data.
        */
        typedef uint16_t CharType;
        /**
           The underlying native value type. The exact type is an
           implementation detail, and not be be relied upon in client
           code.

           You gotta love the STL... by using basic_string<> instead
           of vector<> we can take advantage of reference counting and
           CoW features in most implementations. i'm not certain
           whether vectors also typically refcount/CoW. We could use
           std::wstring, but that's got 4-byte chars on my platform.
        */
        typedef std::basic_string<CharType> ValueType;

        static int strcmp( Utf16String const & lhs, Utf16String const & rhs );
    private:
        /** Internal implementaion detail. */
        class APIImpl;
        friend class APIImpl;
    protected:
        typedef Detail::RcValue<ValueType> Impl;
        /** Returns this object's data, or 0 if it has none (shouldn't happen).*/
        Impl * impl() const;
        /**
           Sets up the initial state.
        
           If origin is not 0 then val is ignored and this object
           becomes a proxy for origin (incrementing its reference
           count). If !origin then a new value is allocated
           and val is assigned to it.
        */
        void init( ValueType const & val, Impl * origin );
        /**
           Functionality similar to init(char const *,size_t),
           but takes a wchar_t pointer and a length in bytes.
           Each byte is copied to the internal container, and
           NO checking is done to see if they are valid UTF16.
        */
        void init( wchar_t const * v, size_t len );

        /** Functionality similar to init(char const *,size_t),
            but takes a CharType pointer and a length in bytes.
        */
        void init( CharType const * v, size_t len );
        /**

            Initializes this string from the first len bytes of v,
            which must be ASCII or UTF8-encoded.

            Results are undefined if v is UTF8 and len does not point
            to a character boundary.

            Throws on decoding error.
        */
        void init( char const * v, size_t len );
        /**
           Allocates a new string value. If origin is not null then it is
           used as the primary copy. This function uses custom memory allocation
           and may returned any of:

           - A pointer to some shared string object, if !origin and
           val.empty(), or if origin and origin->empty().

           - A non-const pointer to origin (legally obtained without
           casting), representing a reference-counted pointer to origin.

           - A new object.

           If origin is non-null then val is ignored, otherwise it copied
           as the string value of the new object.
           
           The returned value, regardless of its value, must eventually be
           freed using deallocValue().
        */
        static Impl * allocValue( ValueType const & val, Impl * origin );
        /**
           "Frees" val, which must have been allocated using
           Utf16String::allocValue(). It might not _actually_ free
           val, depending on several factors, but in all cases the
           semantics are as if val is freed by this call.
        */
        static void deallocValue( Impl * val );
    public:
        /**
           Initializes this object as an empty string.
        */
        Utf16String();

        /**
           Initializes this object from the given
           Utf16String-compatible string value.
        */
        explicit Utf16String( ValueType const & v );

        /**
           Initializes the string with the first len bytes from v.

           Throws if v contains anything which is not legal UTF-8.

           If v is UTF-8 and len does not point to a character
           boundary, results are undefined. It tries to throw in that
           case, but i don't know the underlying conversion routines
           well enough to know whether they report that case back to
           me.

           If v or len are 0 or v is an empty string (starts with
           NULL) then string is empty-initialized (which does not
           require allocation).
        */
        Utf16String( char const * v, size_t len );

        /**
           Equivalent to Utf16String( v, v ? strlen(v) : 0 ).
        */
        explicit Utf16String( char const * v );

        /**
           Equivalent to Utf16String( v.c_str(), v.lengthChars() ).
        */
        explicit Utf16String( std::string const & v );
        /**
           Initializes the string with the first len bytes from v,
           which is assumed to be UTF16-encoded text using the host
           system's byte order. This routine does not verify the
           consistency of the data.
        */
        Utf16String( wchar_t const * v, size_t len );

        /**
           Equivalent to using Utf16String(v,LEN), where LEN is the
           count of leading non-0 chars in v. (This is _possibly_ not
           the correct way to measure the length of a wide char
           string!)
        */
        explicit Utf16String( wchar_t const * v );

        /**
           Initializes this string from the first len bytes
           of v. Results are undefined if those bytes do not
           compose a whole valid UTF16 string (e.g. ending
           in the middle of a multi-byte character). This routine
           does not validate that v contains properly-encoded
           data.
        */
        Utf16String( CharType const * v, size_t len );

        /**
           Like Utf16String( CharType, size_t ), but counts v's length
           bye looking for the first two-byte word which equals 0. i
           honestly have no clue if that's 100% legal heuristic for
           UTF16-encoded data.
        */
        explicit Utf16String( CharType const * v );

        /**
           Equivalent to Utf16String( v.c_str(), v.lengthChars() ).

           Note that wstring::char_type is 16 bits on some platforms
           and 32-bits on some others (notably gcc), but this function
           does not support UTF32 input (and will probably silently
           fail to handle it properly).
         */
        explicit Utf16String( std::wstring const & v );

        /**
           Initializes this object from v. Both objects share
           the same underlying data.
        */
        Utf16String( Utf16String const & v );

        /**
           Initializes this object from v.
        */
        explicit Utf16String( Utf8String const & v );
        
        /**
           Initializes this object from v. Both objects share
           the same underlying data.
        */
        Utf16String & operator=( Utf16String const & v );

        /**
           Initializes this object from v.
        */
        Utf16String & operator=( Utf8String const & v );

        /**
           Implemented to allow using this type as std::map keys. The
           exact sorting rules are not specified by this API (it
           depends on the internal type we use to hold the string
           data), but the implementation currently uses
           ValueType::operator<().
        */
        bool operator<( Utf16String const & other ) const;
        
        /**
           Cleans up any interally-allocated resources (or reduces
           their reference count).
        */
        ~Utf16String();

        /**
           Returns the size, in _encoded_characters_, of the string's
           value.
        */
        size_t lengthChars() const;
        /**
           Returns the length, in bytes, of this string. This is an
           O(1) operation and is always some fixed multiplier larger
           than lengthChars() (depending on the internally-used
           character type).
        */

        size_t lengthBytes() const;

        /**
           Returns an object holding the UTF-16-encoded bytes of the
           string's value. The byte order is host-dependent, and these
           values should not be serialized for long-term storage
           without accounting for the endianness.
        */
        ValueType value() const;

        /**
           Returns a pointer to the underlying bytes, which are owned
           by this object.
        */
        CharType const * c_str() const;
        /**
           Returns true only if a appears to be-a Utf16String.
        */
        static bool isUtf16String( Atom const & a );
        /**
           Alias for isUtf16String(). Provided for transparency in the
           StringType typedef.
        */
        static bool isString( Atom const & a ) { return isUtf16String(a); }

        /**
           Returns a new string containing a UTF-8-encoded copy of
           this string. Throws an exception (deriving from
           std::exception) on an encoding error.
        */
        Utf8String utf8Value() const;
        
        /**
           If v appears to be a Utf16String or Utf8String then this
           function creates a new Utf16String object to wrap its value
           and returns that object. If v does not appear to be an one
           of those types then an exception (deriving from
           std::exception) is thrown.

           If v is-a Utf16String then the returned copy will refer the
           same underlying string data (no allocation is required).
           
           TODO:

           - Convert other types to string: Integer, Double. Boolean
           is tricky: what string to use (true/false, 1/0, t/nil,
           "1"/"", ...)?
        */
        static Utf16String cast( Atom const & v );

        /**
           Returns the character code for the character at the given
           position, or 0 if the value is out of range or this object
           contains an empty value.

           This function is the native counterpart to the JavaScript
           String function of the same name.

           This is O(1) for Utf16Strings.
        */
        CharType charCodeAt( uint32_t pos ) const;

        /** Returns true if this string object is empty (has a length of 0). */
        bool empty() const;
    };

    /**
       This typedef defines the default StringType type used
       by the API.

       FIXME: this currently isn't working if set to Utf16String, but
       i'm not yet sure if it's a bug in my test case or in
       Utf16String.
    */
    typedef Utf8String StringType;

    /**
       Commonly-used StringType constants, provided here to avoid
       re-allocating them every time they are used.
    */
    struct StaticStrings_ {
        /**
           "undefined"
         */
        StringType Undefined;
        /**
           "null"
         */
        StringType Null;
        /**
           "true"
         */
        StringType True;
        /**
           "false"
         */
        StringType False;
        /**
           ""
         */
        StringType Empty;
    };
    /**
       Holds the shared strings specified in the StaticStrings_
       struct.
     */
    extern const StaticStrings_ StaticStrings;

    /**
       A template for converting Atom objects of type
       AtomType to string representations. This class
       must be specialized to be useful and must provide
       two members:

       @code
       static StringType Value( AtomType const & src );
       StringType operator()( AtomType const & src );
       @endcode

       They must behave identically (returning a stringified form of
       the source atom) and they must throw a std::exception on error.
    */
    template <typename AtomType>
    class AtomToString
    {
    protected:
        /**
           A static assertion which fails if B is false.
         */
        template <bool B>
        static void assertion()
        {
            bool const static_assertion_T_is_specialized[B ? 0 : -1] = {0};
        }
    public:
    };

    /**
       A base class for AtomToString<> specializations. Provides
       the operator()() interface. Subclasses must provide
       one static function:

       @code
       static StringType Value( AtomType const & src );
       @endcode

       Additionally, AtomToString<AtomType> must be properly
       specialized.
    */
    template <typename AtomType>
    struct AtomToString_base
    {
    public:
        /** Returns Value(v). Subclasses are expected
            to implement Value().
        */
        StringType operator()( AtomType const & v )
        {
            return Value(v);
        }
    };
   
    template <>
    class AtomToString<Boolean> : AtomToString_base<Boolean>
    {
    public:
        /** Returns the string "true" or "false", depending on v's
            value. Note that calling boolValue() on the result
            will always return true, since any non-empty
            string is "true"!
        */
        static StringType Value( Boolean const & v );
    };
    template <>
    class AtomToString<Integer> : AtomToString_base<Integer>
    {
    public:
        /** Formats v as a string. */
        static StringType Value( Integer const & v );
    };

    template <>
    class AtomToString<Double> : AtomToString_base<Double>
    {
    public:
        /** Formats v as a string, using an unspecified precision
            (6 seems to be normal on my platform).
        */
        static StringType Value( Double const & v );
    };
    template <>
    class AtomToString<Utf8String> : AtomToString_base<Utf8String>
    {
    public:
        /**
           Returns src, converted to StringType type if necessary.
        */
        static StringType Value( Utf8String const & src );
    };
    template <>
    class AtomToString<Utf16String> : AtomToString_base<Utf16String>
    {
    public:
        /**
           Returns src, converted to StringType type if necessary.
        */
        static StringType Value( Utf16String const & src );
    };

//     template <typename AtomType>
//     StringType atomToString( AtomType const & v )
//     {
//         typedef AtomToString<AtomType> CV;
//         return CV::Value( v );
//     }

    /**
       A template for mapping a TypeID::Types value to
       a JavaScript-conventional type name for that type.

       Must be specialized and specializations must have
       the same public interface as this class.
    */
    template <TypeID::Types ID>
    struct IDToJsTypeof
    {
    private:
        template <bool B>
        static void assertion()
        {
            bool const static_assertion_T_is_specialized[B ? 0 : -1] = {0};
        }

    public:
        static char const * Value()
        {
            assertion<false>();
            return NULL;
        }
    };

    /**
       A helper for implementing Atom iterators.
       KeyType must:

       - Be default-constructable.

       - Not be cvp-qualified
     */
    template <typename KeyType>
    class AtomEntry
    {
    private:
        static const KeyType UndefKey;
    public:
        KeyType const & key;
        Atom const & value;
        /**
           Initializes this entry to point at the given key/value
           pair. Those objects must outlive this object and must not
           be moved to a new memory location for the life of this
           object. (e.g. Google v8's memory manager can move objects
           around.)
        */
        AtomEntry( KeyType const & k, Atom const & v )
            : key(k), value(v)
        {}
        /**
           Constructs an "invalid" entry, intended to cover
           the case of dereferencing an 'end' iterator.
         */
        AtomEntry()
            : key(UndefKey), value(Atom::Undefined)
        {}
        /**
           Returns true if this object was not default-constructed.
        */
        bool isValid() const
        {
            return &UndefKey != &key;
        }
    };
    /**
       An "invalid" key value for AtomEntry<KeyType>'s default
       constructor.
     */
    template <typename KeyType>
    const KeyType AtomEntry<KeyType>::UndefKey = KeyType();

    /**
       A base type for iterators for the Object and Array
       classes.

       IterType must be a "real" iterator type, e.g.
       std::map<KeyType,ValueType>::iterator or const_iterator.
     */
    template <typename IterType>
    class AtomIteratorBase : public std::forward_iterator_tag
    {
    protected:
        IterType m_iter;
        IterType m_end;
    public:
        typedef IterType IteratorType;
        /**
           Initializes this object to use the given
           iterator pair, which must represent
           the current iteration position and the
           after-the-end position (they may both
           point to the after-the-end position).
        */
        AtomIteratorBase(IterType it, IterType end)
            : m_iter(it), m_end(end)
        {}
        /**
           Initializes this object as an after-the-end iterator, using
           the given after-the-end value.
        */
        AtomIteratorBase(IterType end)
            : m_iter(end), m_end(end)
        {}
        /**
           Initializes using default-constructed iterators. For some
           iterator types these are equivalent to the end iterator.
        */
        AtomIteratorBase()
            : m_iter(IterType()), m_end(m_iter)
        {}

        /** Prefix increment. */
        AtomIteratorBase & operator++()
        {
            return ++m_iter, *this;
        }

        /** Postfix increment. */
        AtomIteratorBase operator++(int)
        {
            return AtomIteratorBase(m_iter++, m_end);
        }

        /**
           Returns true if this object and other both have
           the same iterator position.
         */
        bool operator==(AtomIteratorBase const &other) const
        {
            return this->m_iter == other.m_iter;
        }

        /**
           Returns true if this object and other have different
           iterator positions.
         */
        bool operator!=(AtomIteratorBase const &other) const
        {
            return this->m_iter != other.m_iter;
        }
    };
    template <typename KeyType, typename IterType>
    class AtomEntryIterator : public AtomIteratorBase<IterType>
    {
        typedef AtomIteratorBase<IterType> ParentType;
    public:
        AtomEntryIterator(IterType it, IterType end)
            : ParentType(it,end)
        {}
        /**
           Initializes this object as an after-the-end iterator, using
           the given after-the-end value.
        */
        AtomEntryIterator(IterType end)
            : ParentType(end)
        {}
        AtomEntryIterator()
            : ParentType()
        {}
        /**
           Convenience typedef.
         */
        typedef AtomEntry<KeyType> Entry;
        /**
           Get the key/value holder for this iterator.  Its
           isValid() will return false for an invalid iterator
           (e.g. the end iterator).
        */
        Entry operator*() const
        {
            return (this->m_end == this->m_iter)
                ? Entry()
                : Entry( (*this->m_iter).first, (*this->m_iter).second )
            ;
        }
    };

    /**
       Similar to AtomEntryIterator, but works on list-style iterators.
    */
    template <typename ValueType, typename IterType>
    class AtomListIterator : public AtomIteratorBase<IterType>
    {
        typedef AtomIteratorBase<IterType> ParentType;
    public:
        AtomListIterator(IterType it, IterType end)
            : ParentType(it,end)
        {}
        /**
           Initializes this object as an after-the-end iterator, using
           the given after-the-end value.
        */
        AtomListIterator(IterType end)
            : ParentType(end)
        {}
        AtomListIterator()
            : ParentType()
        {}
        /**
           Get the key/value holder for this iterator.  Its isValid()
           will return a default-constructed ValueType value for an
           invalid iterator (e.g. the end iterator).
        */
        ValueType const & operator*() const
        {
            static ValueType dflt;
            return (this->m_end == this->m_iter)
                ? dflt
                : *this->m_iter;
            ;
        }
    };
    /**
       A base class for the Object and Array classes, providing
       iteration support for their entries.

       Template parameters:

       SubType: the type which is subclassing this type. It must
       subclass this type and provide features descrbied below.
       
       KT: a Key Type usable by AtomEntry<KT>.

       CT: a Container Type which follows STL map conventions.

       IterType: CT's non-const-iterator type.

       ConstIterType: CT's const-iterator type.

       This class provides the basic functionality for
       iterating over collections of Atoms.

       Features required by SubType:

       ContainerType * atomMap() const;

       must return a pointer to the underlying container, or
       NULL if it cannot. Ideally it is a private function
       and this class is made a friend class of the subclass.
    */
    template <typename SubType,
              typename CT,
              typename KT = typename CT::key_type,
              typename IterType = typename CT::iterator,
              typename ConstIterType = typename CT::const_iterator
              >
    class AtomMapBase
    {
    private:
        /**
           Reminder to self: we use static_cast() instead of holding a
           pointer to SubType because the latter approach costs 1
           pointer and breaks if a client (or gabage collector engine)
           copies or moves the object without using the objec't copy
           operator (potentially also the move operator, for C++0x).
        */
        SubType * self()
        {
            return static_cast<SubType*>(this);
        }
        SubType const * self() const
        {
            return static_cast<SubType const *>(this);
        }
    protected:
        /**
           The self pointer must be the 'this' object of the object
           which subclasses this type.
        */
        AtomMapBase( SubType * self )
        {}
        AtomMapBase()
        {}
    public:
        /**
           The type returned by [Const]Iterator::operator*().
        */
        typedef AtomEntry<KT> Entry;
        /**
           The key type used by the underlying container.
        */
        typedef KT KeyType;
        /**
           The underlying container type.
        */
        typedef CT ContainerType;

        /**
           The public iterator type.
        */
        typedef AtomEntryIterator<KeyType,IterType> Iterator;
        /**
           The public const-iterator type.
        */
        typedef AtomEntryIterator<KeyType,ConstIterType> ConstIterator;

        /**
           The beginning of the iterable set.
        */
        ConstIterator begin() const
        {
            ContainerType const * c = self()->atomMap();
            return c
                ? ConstIterator(c->begin(),c->end())
                : ConstIterator();
        }

        /**
           The after-the-end const-iterator.
        */
        ConstIterator end() const
        {
            ContainerType const * c = self()->atomMap();
            return c
                ? ConstIterator(c->end())
                : ConstIterator();
        }

        /**
           The beginning of the iterable set.
        */
        Iterator begin()
        {
            ContainerType * c = self()->atomMap();
            return c
                ? Iterator(c->begin(),c->end())
                : Iterator();
        }
        /**
           The after-the-end iterator.
        */
        Iterator end()
        {
            ContainerType * c = self()->atomMap();
            return c
                ? Iterator(c->end())
                : Iterator();
        }

        /**
           Returns true if this object has no entries.
        */
        bool empty() const
        {
            ContainerType const * c = self()->atomMap();
            return c ? c->empty() : true;
        }

        /**
           Returns the number of atoms in the container.
        */
        size_t size() const
        {
            ContainerType const * c = self()->atomMap();
            return c ? c->size() : 0;
        }


        /**
           Removes all entries from the underlying container
           if it is not null.
        */
        void clear()
        {
            ContainerType * c = self()->atomMap();
            if( c ) c->clear();
        }

    private:
        AtomMapBase(AtomMapBase const &rhs);
        AtomMapBase & operator=(AtomMapBase const &rhs);
    };

    /**
       A base class for Atoms which contain an int-indexed list
       of Atom values.

       This type is used identically to AtomMapBase, but works
       with a list instead of a map.

       CT must be std::vector or usage-compatible, and CT::value_type
       must have value semantics (no pointers, or at least isn't not
       intended for them).
    */
    template <typename SubType,
              typename CT,
              typename IterType = typename CT::iterator,
              typename ConstIterType = typename CT::const_iterator
              >
    class AtomListBase
    {
    private:
        /**
           Reminder to self: we use static_cast() instead of holding a
           pointer to SubType because the latter approach costs 1
           pointer and breaks if a client (or gabage collector engine)
           copies or moves the object without using the objec't copy
           operator (potentially also the move operator, for C++0x).
        */
        SubType * self()
        {
            return static_cast<SubType*>(this);
        }
        SubType const * self() const
        {
            return static_cast<SubType const *>(this);
        }
    protected:
        AtomListBase()
        {}
    public:
        /**
           The underlying container type.
        */
        typedef CT ContainerType;
        /**
           The type returned by [Const]Iterator::operator*().
        */
        typedef typename ContainerType::value_type ValueType;
        /**
           The public iterator type.
        */
        typedef AtomListIterator<ValueType,IterType> Iterator;
        /**
           The public const-iterator type.
        */
        typedef AtomListIterator<ValueType,ConstIterType> ConstIterator;

        /**
           The beginning of the iterable set.
        */
        ConstIterator begin() const
        {
            ContainerType const * c = self()->atomList();
            return c
                ? ConstIterator(c->begin(),c->end())
                : ConstIterator();
        }

        /**
           The after-the-end const-iterator.
        */
        ConstIterator end() const
        {
            ContainerType const * c = self()->atomList();
            return c
                ? ConstIterator(c->end())
                : ConstIterator();
        }

        /**
           The beginning of the iterable set.
        */
        Iterator begin()
        {
            ContainerType * c = self()->atomList();
            return c
                ? Iterator(c->begin(),c->end())
                : Iterator();
        }
        /**
           The after-the-end iterator.
        */
        Iterator end()
        {
            ContainerType * c = self()->atomList();
            return c
                ? Iterator(c->end())
                : Iterator();
        }

        /**
           Returns true if this object has no entries.
        */
        bool empty() const
        {
            ContainerType const * c = self()->atomList();
            return c ? c->empty() : true;
        }

        /**
           Returns the number of atoms in the container.
        */
        size_t size() const
        {
            ContainerType const * c = self()->atomList();
            return c ? c->size() : 0;
        }


        /**
           Removes all entries from the underlying container
           if it is not null.
        */
        void clear()
        {
            ContainerType * c = self()->atomList();
            if( c ) c->clear();
        }

    private:
        AtomListBase(AtomListBase const &rhs);
        AtomListBase & operator=(AtomListBase const &rhs);
    };

    /**
       This type exists only to avoid some code duplication. Use
       Array::IndexType in client code, please.
    */
    typedef uint32_t /*Integer::ValueType*/ ArrayIndex;

    /**
       Unlike the basic Atom types, Arrays are not immutable (but
       their keys and values are).

       This class only implements the basic array operations needed
       for creating JSON data or created objects from JSON data. It
       does not implement the full Array semantics specified by
       JavaScript. It does not implement sparse-arrays, per se. If, e.g.,
       indexes 0 and 4 are set, space is still allocated for entries 1..3,
       but they have Atom::Undefined values.

       FIXME: consider making this a subclass of Object. The problem
       with that is that i want int keys and Object uses string keys.
       Plus, the AtomAPI "subclassing" mechanism probably can't handle
       that.
       
       Reminder to self: interesting reference code (JS):

       @code
var a = [];
a['foo'] = 'xfoo';
a[1] = 'x1';
function pr() { var a = arguments[0];
    print("Array: length =",a.length);
    for( var k in a ) { print('\t',typeof k,k,'=',a[k]); }
    print(a.join(' | '));
}
pr(a);
a[0] = 'x0';
a[7.01] = 'x3';
a['bar'] = 'xbar';
pr(a);
       @endcode

       Produces:

    @code
Array: length = 2
	 string 1 = x1
	 string foo = xfoo
 | x1
Array: length = 2
	 string 0 = x0
	 string 1 = x1
	 string foo = xfoo
	 string 7.01 = x3
	 string bar = xbar
x0 | x1
    @endcode

    (Same output with both Google v8 and SpiderMonkey.)

    Note that using the key (7.0) is equivalent to using the integer
    value (7), but (7.01) is treated as a non-numeric index.

    We only allow iteger keys, since those are the only useful keys
    for JSON data.


    Peculiarities of the implementation:

    The Atom::compare() implementation will behave as expected only
    for Array  arguments, but for other types it only compares the type
    IDs of the two atoms. "As expected" means that it works as follows:

    - If the Arrays have differing sizes (counting only direct
    children), the one with the smaller size is "less than" the
    other.

    - Arrays sharing the same reference obviously compare equally.

    - Any two empty objects will compare equally.
       
    - All values in the array are (recursively) compared. If any
    values compare non-equal, then that result is returned. If all
    compare equal then the arrays are equal.
    */
    class Array : public Atom,
                  public AtomListBase< Array, std::vector<Atom> >
    {
    private:
        typedef AtomListBase< Array, std::vector<Atom> > ListParentType;
        friend class AtomListBase< Array, std::vector<Atom> >;
        
    public:
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Array;
        typedef ArrayIndex IndexType;
        static const IndexType InvalidIndex;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Array;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();
        Array();
        ~Array();
        /**
           Initializes an array containing the given number of
           entries, all of which are initialized to Atom::Null.
        */
        explicit Array( IndexType length );
        Array( Array const & other );
        Array & operator=( Array const & other );
        /**
           Throws (or grows?) on out-of-range?

           JS compatibility implies growing,
           but we could provide both by throwing
           from the equivalent set() operation
           and growing via this operation.
        */
        //Atom & operator[]( Integer const & key );
        Atom & operator[]( IndexType );
        /**
           Returns Atom::Undefined on out-of-range.
        */
        //Atom operator[]( Integer const & key ) const;
        Atom operator[]( IndexType ) const;

// Required for special-case handling of (N.0 == N):
//         Atom & operator[]( Double const & key );
//         Atom operator[]( Double const & key ) const;

        /**
           Returns the value of the given index, or defaultValue if
           key is out of bounds for this object.

           @see set()
        */
        Atom get( IndexType key, Atom const & defaultValue = Atom::Undefined ) const;

        /**
           Sets the given key to the given value.

           When array elements are inserted automatically to account
           for non-incremental insertion indexes, those elements will
           have Atom::Undefined as their value. e.g. if you set index 10 on
           an otherwise empty array, it will have a size of 11 and
           elements 0..9 will have Atom::Undefined values. We use
           Undefined because that's the way JavaScript-side JSON
           serializers do it.
        */
        void set( IndexType key, Atom const & val );

        /**
           Removes the key at the given index and shifts all
           right-side values one place to the left, changing the size
           of the array. How (in)efficient this is depends on the
           underlying container used for storage and (possibly,
           depending on the container type) what types of Atoms are
           stored in the right-side-of-key positions.
        */
        void remove( IndexType key );

        /**
           If v appears to be-a Array then this function creates a new
           Array object to wrap its value and returns that object. If
           v does not appear to be-a Array then an exception (deriving
           from std::exception) is thrown.

           If v is-a Array then the returned copy will refer the same
           underlying list (no allocation is required).
        */
        static Array cast( Atom const & v );

        /**
           Returns true if a is-a Array atom.
        */
        static bool isArray( Atom const & a );

        /**
           Adds an entry to the end of the list, using the current
           maximum index +1 (or 0 if this array is empty).

           This function is not strictly compatible with conventional
           JS usage!

           Returns the new index of the element.
        */
        IndexType push( Atom const & a );

        /**
           Current impl is broken vis-a-vis unsigned int indexes.

           Adds an entry to the beginning of the list, using the current
           minimum index -1 (or 0 if this array is empty).

           This function is not strictly compatible with conventional
           JS usage!

           Returns the new index of the element.
        */
        //IndexType unshift( Atom const & a );

        /**
           Pops the last entry from the array and returns it, or
           Atom::Undefined if the array is empty.

           This function is not strictly compatible with conventional
           JS usage!
        */
        Atom pop();

        //size_t size() const;
    protected:
        /** Internal implementaion detail. */
        struct Impl;
        friend class Impl;
        typedef Detail::RcValue<Impl> RcImpl;
    public:
        /**
           An opaque internal type which is only in the public API so
           that certain internal templatized operations can work.
           This type does not really have proper value semantics, which
           bugs me considerably.
         */
        typedef RcImpl ValueType;

    private:
        ValueType * impl() const;
        ContainerType * atomList() const;
        /** Internal implementaion detail. */
        struct APIImpl;
        friend class APIImpl;
        void allocImpl( Array::ValueType * origin ) const;
        //void rcOnCopy() const;
        void init( Array::ValueType * origin );

    };

    /**
       Models a JSON-style object, containing string keys
       and arbitrary Atom values.

       The major caveat is that it is illegal to store any object as a
       child (or descendent) of itself. Doing so will lead to endless
       recursion, stack overflows, or similar, when the object is
       converted to JSON. (This is also the case in some JS
       implementation.)


       Notes to self:

       An Object is a map of (String,Atom) pairs. This is correct for
       JSON, where all keys are Strings, but does pose some problems
       in practice. For example, it makes it difficult to convert an Object
       to a std::map via atomToNative(), because we have the limit that the
       map keys must be strings.

       If we allow arbitrary Atoms as keys it leads us to a very ugly
       corner case: the keys Integer(1) and StringType("1") would
       internally have different meanings, e.g. a lookup against key
       Integer(1) would map directly to the proper key. But when
       outputing to JSON we have to convert the keys to strings and we
       end up with two identical keys in the output. When re-parsing
       that data, we would have an object with _one_ key named "1",
       but we could not reliably predict whether it contains the value
       associated with Integer(1) or with StringType("1").
       

       Peculiarities of the implementation:

       The Atom::compare() implementation will behave as expected only
       for Object arguments, but for other types it only compares the type
       IDs of the two atoms. "As expected" means that it works as follows:

       - If the Objects have differing sizes (counting only direct
       children), the one with the smaller size is "less than" the
       other.

       - Objects sharing the same reference obviously compare equally.

       - Any two empty objects will compare equally.
       
       - If all keys and all values of both objects compare equally,
       then the the objects are equal. If they are not equal, the
       result of the comparison operation is the result of the last
       key/value comparison which did not return 0.
    */
    class Object : public Atom,
                   public AtomMapBase< Object, std::map<StringType,Atom> >
    {
    public:
        typedef StringType KeyType;
        static const nosjob::TypeID::Types TypeID = nosjob::TypeID::Object;
    private:
        typedef AtomMapBase< Object, std::map<StringType,Atom> > MapParentType;
        friend class AtomMapBase< Object, std::map<StringType,Atom> >;
        /** Internal implementaion detail. */
        class APIImpl;
        friend class APIImpl;
        /**
           Internal data for Objects.
        */
        struct Impl;
        typedef Detail::RcValue<Impl> RcImpl;
        void allocImpl( RcImpl * origin ) const;
        void init( RcImpl * origin );
    public:
        /**
           An opaque internal type which is only in the public API so
           that certain internal templatized operations can work.
         */
        typedef RcImpl ValueType;
        /**
           The TypeID used by this class.
        */
        static const TypeID::Types AtomTypeID = TypeID::Object;
        /**
           This type's underlying AtomAPI implementation.
        */
        static const AtomAPI * API();
        Object();
        ~Object();
        Object( Object const & other );
        Object & operator=( Object const & other );
        /**
           Returns Atom::Undefined on out-of-range.
        */
        //Atom operator[]( Atom const & key ) const;
        //Atom operator[]( StringType const & key ) const;

// Required for special-case handling of (N.0 == N):
//         Atom & operator[]( Double const & key );
//         Atom operator[]( Double const & key ) const;

        /**
           Returns true if this object contains a property
           of the given key.
        */
        bool has( Atom const & key ) const;

        /**
           Returns the value of the given key, or defaultValue if
           this object does not contain that key.
        */
        Atom get( Atom const & key, Atom const & defaultValue = Atom::Undefined ) const;

        /**
           Tries to fetch a value with the given key, and that value must be
           of the Atom type specified by AtomType.

           AtomType must be a concrete Atom implementation with a
           static cast(Atom const &) member returning AtomType.  That
           member must throw if the given key's value is not of (or
           cannot be converted to) the requested tyoe.
        */
        template <typename AtomType>
        AtomType get( Atom const & key )
        {
            Atom const & a = get( key, Atom::Undefined );
            if( a.isUndefined() )
            {
                throw Error("No such key.");
            }
            return AtomType::cast(a);
        }

        /**
           Tries to fetch an atom using the given key. If no item is
           found, defaultValue is returned, else AtomType::cast() is
           used to convert the value. If it throws, defaultValue is
           returned, otherwise the return value of cast() is returned.
        */
        template <typename AtomType>
        AtomType getOpt( Atom const & key, AtomType const & defaultValue )
        {
            Atom const & a = get( key, Atom::Undefined );
            if( a.isUndefined() ) {
                return defaultValue;
            }
            try {
                return AtomType::cast(a);
            }
            catch(...) {
                return defaultValue;
            }
        }
        
        /**
           Sets the given key to the given value. Key's string value
           is used as the internal key (per JSON conventions), as
           opposed to a hashcode.

           It is semantically illegal to add an object as a child
           property of itself (no matter how deep in the property
           nesting). Doing so WILL cause endless loops, stack
           overflows, or some such in certain operations
           (e.g. conversion to JSON).
        */
        void set( Atom const & key, Atom const & val );

        /**
           Removes the given property. Returns true if this object
           had the property, else false.
        */
        bool remove( Atom const & key );

        /**
           Works like remove(), but it returns the value being
           removed, or Atom::Undefined if this object has no such
           entry.
        */
        Atom take( Atom const & key );

        /**
           If v appears to be-a Object then this function creates a new
           Object object to wrap its value and returns that object. If
           v does not appear to be-a Object then an exception (deriving
           from std::exception) is thrown.

           If v is-a Object then the returned copy will refer the same
           underlying data (no allocation is required).
        */
        static Object cast( Atom const & v );

        /**
           Returns true if a is-a Object atom.
        */
        static bool isObject( Atom const & a );
    protected:
        /**
           Returns the m_data object cast to an RcImpl.
         */
        RcImpl * impl() const;
        ContainerType * atomMap() const;
    };

    /**
       Reads the next UTF8 character from the given input iterator
       (which is advanced one code-point by this function) and outputs
       a JSON-escaped version of that character to the given output
       iterator. The end parameter must be the one-past-the-end
       iterator for the input range.

       On error this function throws an exception (deriving from
       std::exception) and does not modify the out iterator.

       On success the out iterator is called enough times to insert
       the JSON-compatible value of the character, and the number
       of bytes in the encoded value is returned.

       e.g.:

       @code
       std::string str("Some\t\"value\"");
       char const * beg = str.c_str();
       char const * end = beg + str.size();
       uint8_t rc = 0;
       std::string esc;
       std::back_insert_iterator<std::string> outiter(esc);
       for( ; beg < end; ) {
           utf8CharToJSON( beg, end, outiter );
       }
       @endcode

       After that, the 'esc' string will contain the JSON-legal
       value of the input string. Note that the escaping works
       at the _character_ level, not the level of higher-order
       values. e.g. a properly-JSON'd string still requires
       enclosing quotes.
    */
    template <typename InIter, typename OutIter>
    uint8_t utf8CharToJSON( InIter & in, InIter const & end, OutIter out )
    {
        const uint32_t i = utf8::next( in, end );
        enum { BufLen = 8 };
        char buf[BufLen] = {0};
        uint8_t olen = -1;
        switch( i )
        {
          case '\n':
              buf[0]='\\'; buf[1]='n'; olen = 2;
              break;
          case '\\':
              buf[0]='\\'; buf[1]='\\'; olen = 2;
              break;
          case '\t':
              buf[0]='\\'; buf[1]='t'; olen = 2;
              break;
          case '\r':
              buf[0]='\\'; buf[1]='r'; olen = 2;
              break;
          case '\b':
              buf[0]='\\'; buf[1]='b'; olen = 2;
              break;
          case '\f':
              buf[0]='\\'; buf[1]='f'; olen = 2;
              break;
          case '"':
              buf[0]='\\'; buf[1]='"'; olen = 2;
              break;
          default: {
              if( i <= 0x001F )
              { /* control chars */
                  std::sprintf( buf, "\\u%04x", i );
                  olen = 6;
              }
              else if( i <= 0x7f )
              { /* ascii */
                  buf[0] = char(i); olen = 1;
              }
              else if( i > 0x7e )
              { /* utf8 */
                  if( i >/*i >= ????*/ 0xffff )
                  {
                      throw Error("utf8CharToJSON() does not handle multi-char_type chararacters!");
                      // WTF was this for, anyway?
                  }
                  std::sprintf( buf, "\\u%04x", i );
                  olen = 6;
              }
              else
              {
                  throw Error("utf8CharToJSON() couldn't figure out what to do!");
              }
          }
        };
        assert( (olen > 0) && (olen < BufLen) );
        buf[olen]=0;
        for( uint8_t c = 0; c < olen; ++c )
        {
            *(out++) = buf[c];
        }
        return olen;        
    }

    /**
       Assumes that beg and end point to the start and
       one-past-the-end of a valid UTF-8 sequence, JSON-escapes it,
       and copies the sequence to the given output iterator.

       Will throw if [beg,end) are not valid UTF-8.

       The output is done in a streaming fashion, not buffering the
       whole range for escaping before output.
    */
    template <typename InIter, typename OutIter>
    void utf8StringToJSON( InIter beg, InIter const & end, OutIter out )
    {
        (*out)++ = '"';
        for( ; beg < end; ) {
            utf8CharToJSON( beg, end, out );
        }
        (*out)++ = '"';
    }

    namespace Detail {
        using namespace nosjob;
        /**
           A template for matching printf/scanf specifiers to types.

           NumberT must be a basic POD numeric type and this type must
           be specialized for that type.
        */
        template <typename NumberT>
        struct PrnScnFmt
        {
            /** The printf specifier for NumberT. */
            static char const * const pfmt;
            /** The scanf specifier for NumberT. */
            static char const * const sfmt;
        };
        template <> struct PrnScnFmt< int32_t > {
            static char const * const pfmt;
            static char const * const sfmt;
        };
        template <> struct PrnScnFmt< int64_t > {
            static char const * const pfmt;
            static char const * const sfmt;
        };
        template <> struct PrnScnFmt< double > {
            static char const * const pfmt;
            static char const * const sfmt;
        };
//         template <> struct PrnScnFmt< Integer > : PrnScnFmt<Integer::ValueType> {};
//         template <> struct PrnScnFmt< Double > : PrnScnFmt<Double::ValueType> {};
    }
    
    /**
       An interface for streaming Atoms to JSON over output iterators.
       It must be specialized and AtomType must be a conformant Atom
       type.
    */
    template <typename AtomType>
    struct JsonStreamer
    {
        /**
           Must output a JSON-legal form of the given atom,
           properly escaped and quoted (if necessry), to
           the given output iterator.
        */
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest );
    };

    template <>
    struct JsonStreamer<Boolean>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest )
        {
            char const * str = atom.boolValue() ? "true" : "false";
            for( char const * p = str; *p; ++p ) *(dest++) = *p;
        }
    };

    template <>
    struct JsonStreamer<Integer>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest )
        {
            // we format the number ourselves, instead of using sprintf(), to avoid an extra copy.
            Integer const & v = Integer::cast(atom);
            enum { BufLen = 32 };
            char buf[BufLen];
            static char const * chars = "0123456789";
            Integer::ValueType iv = v.value();
            const bool isNeg = ( iv < 0 );
            if( isNeg ) iv = iv * -1;
            static const char base = 10;
            char * ptr = buf + BufLen;
            *(--ptr) = 0;
            do {
                --ptr;
                assert( ptr > buf );
                *ptr = chars[iv%base];
                iv = iv / base;
            } while( iv > 0 );
            if( isNeg ) *(--ptr) = '-';
            for( ; *ptr; ++ptr ) *(dest++) = *ptr;
        }
    };

    template <>
    struct JsonStreamer<Double>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest )
        {
            const double val = Double::cast(atom).value(); 
            enum { Len = 128 /* i saw an 84-byte double go through here once.*/ };
            char str[Len];
            int sl = sprintf( str, Detail::PrnScnFmt<Double::ValueType>::pfmt, val );
            if( sl <= 0 )
            {
                throw Error("Internal error converting Double to String.");
            }
            else if( sl >= Len )
            {
                /* stack is corrupt. We cannot do anything about it here. */
                std::abort();
            }
            if(1)
            { /* trim trailing zeros. */
                char * x = (str + sl -1);
                for( ; x && (x != str) && ('0'==*x) && ('.' != *(x-1)); --x )
                {
                    *x = 0;
                    --sl;
                }
            }
            assert( sl > 0 );
            for( char const * p = str; *p; ++p ) *(dest++) = *p;
        }
    };
    template <>
    struct JsonStreamer<Utf8String>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest )
        {
            const Utf8String val( Utf8String::cast(atom) );
            char const * v = val.c_str(); // why do val.value().begin/end() not work here?
            utf8StringToJSON( v, v + val.lengthBytes(), dest );
        }
    };

    template <>
    struct JsonStreamer<Utf16String> : JsonStreamer<Utf8String>
    {
    };

    /**
       A "catch-all" implementation which dispatches the arguments to
       one of the other specializations, depending on the value
       of atom.typeID().
    */
    template <>
    struct JsonStreamer<Atom>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest );
    };

    template <>
    struct JsonStreamer<Object>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest )
        {
            const Object val( Object::cast(atom) );
            Object::ConstIterator it = val.begin();
            Object::ConstIterator et = val.end();
            *(dest++) = '{';
            const bool gotSome = (et != it);
            if( ! gotSome ) {
                *(dest++) = '}';
                return;
            }
            //*(dest++) = ' ';
            for( ; et != it; ) {
                Object::Entry const & ent = *it;
                JsonStreamer<Object::KeyType>::streamOut( ent.key, dest );
                *(dest++) = ':';
                JsonStreamer<Atom>::streamOut( ent.value, dest );
                if( ++it != et ) {
                    *(dest++) = ',';
                    *(dest++) = ' ';
                }
            }
            //*(dest++) = ' ';
            *(dest++) = '}';
        }
    };

    template <>
    struct JsonStreamer<Array>
    {
        template <typename OutIter>
        static void streamOut( Atom const & atom, OutIter dest )
        {
            const Array val( Array::cast(atom) );
            Array::ConstIterator it = val.begin();
            Array::ConstIterator et = val.end();
            // FIXME: add a way to specify indentation!
            *(dest++) = '[';
            const bool gotSome = (et != it);
            if( ! gotSome ) {
                //*(dest++) = ' ';
                *(dest++) = ']';
                return;
            }
            //*(dest++) = ' ';
            for( ; et != it; ) {
                JsonStreamer<Atom>::streamOut( *it, dest );
                if( ++it != et ) {
                    *(dest++) = ',';
                    *(dest++) = ' ';
                }
            }
            //*(dest++) = ' ';
            *(dest++) = ']';
        }
    };

    /**
       Streams the given atom in JSON format to the given output
       iterator.

       Special case:

       The Atom::Undefined value is converted to JSON null because the
       JSON spec does not listed "undefined" as a legal token.
    */
    template <typename OutIter>
    void JsonStreamer<Atom>::streamOut( Atom const & atom, OutIter dest )
    {
        switch( atom.typeID() )
        {
          case TypeID::Undefined:
              /*
                The JSON spec does not specify 'undefined' as a legal
                token, but the impls i've used accept it.  We'll
                accept the spec here and convert undefined to null for
                this purpose.
              */
          case TypeID::Null: {
              //No: will quote "null": JsonStreamer<StringType>::streamOut( StaticStrings.Null, dest );
              StringType::CharType const * str = StaticStrings.Null.c_str();
              for( ; *str; ++str ) *(dest++) = *str;
              return;
          }
          case TypeID::Boolean:
              JsonStreamer<Boolean>::streamOut( atom, dest );
              return;
          case TypeID::Integer:
              JsonStreamer<Integer>::streamOut( atom, dest );
              return;
          case TypeID::Double:
              JsonStreamer<Double>::streamOut( atom, dest );
              return;
          case TypeID::Utf8String:
              JsonStreamer<Utf8String>::streamOut( atom, dest );
              return;
          case TypeID::Utf16String:
              JsonStreamer<Utf16String>::streamOut( atom, dest );
              return;
          case TypeID::Object:
              JsonStreamer<Object>::streamOut( atom, dest );
              return;
          case TypeID::Array:
              JsonStreamer<Array>::streamOut( atom, dest );
              return;
          default:
              throw new Error("Unhandled type ID passed to JsonStreamer<Atom>::streamOut()");
        }
    }

    /**
       JsonFormatter works like JsonStreamer, but formats its output
       to be more human-readable. The amount of whitespace it uses can
       be configured by the user.

       Potential TODOs:

       New config options:

       - No-break-arrays: do not put a newline between array elements.

       - one-entry optimization: for arrays/objects with only one
       child entry, do not put a newline between the container opener
       ('{' or '[') and the object, nor a newline before the container
       closer ('}' or ']').

       - Perhaps default a "level" of indention, and each "level" indents
       a bit more than the previous one. e.g. level 0 does no indention,
       level 1 puts a newline at the end of each object/array, but not after
       list entries. etc.

       - Perhaps define boolean options as an enum/bitmask of flags.
    */
    class JsonFormatter
    {
    private:
        unsigned short m_depth;
        unsigned short m_blanks;
        bool m_useTabs;

        void reset()
        {
            m_depth = 0;
        }

        /**
           Outputs a newline followed by the indention based on from
           the m_blanks and m_depth values.
        */
        template <typename OutIter>
        void indent( OutIter & dest )
        {
            if( 0 == m_blanks ) return;
            *(dest++) = '\n';
            const char ch = m_useTabs ? '\t' : ' ';
            for( unsigned short i = 0; i < m_depth; ++i ) {
                for( unsigned short x = 0; x < m_blanks; ++x ) {
                    *(dest++) = ch;
                }
            }
        }

        /**
           The concrete impl for Objects.
         */
        template <typename OutIter>
        void formatImpl( Object const & obj, OutIter dest )
        {
            Object::ConstIterator it = obj.begin();
            Object::ConstIterator et = obj.end();
            *(dest++) = '{';
            const bool gotSome = (et != it);
            if( ! gotSome ) {
                *(dest++) = '}';
                return;
            }
            ++m_depth;
            indent(dest);
            for( ; et != it; ) {
                Object::Entry const & ent = *it;
                JsonStreamer<Object::KeyType>::streamOut( ent.key, dest );
                *(dest++) = ':';
                //*(dest++) = ' ';
                this->formatImpl( ent.value, dest );
                if( ++it != et ) {
                    *(dest++) = ',';
                    if( m_blanks ) indent(dest);
                    else *(dest++) = ' ';
                }
            }
            --m_depth;
            indent(dest);
            *(dest++) = '}';
        }

        /**
           The concrete impl for Arrays.
         */
        template <typename OutIter>
        void formatImpl( Array const & obj, OutIter dest )
        {
            Array::ConstIterator it = obj.begin();
            Array::ConstIterator et = obj.end();
            *(dest++) = '[';
            const bool gotSome = (et != it);
            if( ! gotSome ) {
                *(dest++) = ']';
                return;
            }
            ++m_depth;
            indent(dest);
            for( ; et != it; ) {
                this->formatImpl( *it, dest );
                if( ++it != et ) {
                    *(dest++) = ',';
                    if( m_blanks ) indent(dest);
                    else *(dest++) = ' ';
                }
            }
            --m_depth;
            indent(dest);
            *(dest++) = ']';
        }

        /**
           The base format() impl. Dispatches to one of
           formatImpl<Object>(), formatImpl<Array>(), or
           JsonStreamer<Atom>, depending on obj.typeID().
        */
        template <typename OutIter>
        void formatImpl( Atom const & obj, OutIter dest )
        {
            switch( obj.typeID() )
            {
              case TypeID::Object:
                  this->formatImpl( Object::cast(obj), dest );
                  return;
              case TypeID::Array:
                  this->formatImpl( Array::cast(obj), dest );
                  return;
              default:
                  JsonStreamer<Atom>::streamOut( obj, dest );
                  return;
            }
        }
    public:
        /**
           Initializes the formatter with the specified rules. The
           parameters are:

           blankCount: the number of blanks to use for indention at
           each level. A value of zero disables extraneous spacing,
           which makes this object function essentially the same as
           JsonStreamer (there might be minor differences in spacing,
           but no sematic differences).

           useTabs: if true, each blank is one tab (ASCII 0x09), else
           each blank is one one whitespace (ASCII 0x20).
        */
        JsonFormatter(unsigned short blankCount = 1, bool useTabs = true)
            : m_blanks(blankCount),
              m_useTabs(useTabs)
        {}

        /**
           Sets the indention options. See the constructor for their
           semantics.
        */
        void indention( unsigned short blankCount, bool useTabs )
        {
            m_blanks = blankCount;
            m_useTabs = useTabs;
        }
        
        /**
           Ouputs a single "root" atom to the given iterator.  It is
           intended for Object and Array atoms, but can also be used
           on the POD types (in which case it works the same as
           JsonStreamer).

           Clients may, depending on their usage, need to add a
           newline to the output. Pass true as the addNewline value to
           do that.

           May propagate std::exceptions from the output process.

           OutIter may be any output iterator capable of accepting
           string (UTF-8) input.
        */
        template <typename OutIter>
        void format( Atom const & obj, OutIter dest, bool addNewline = false )
        {
            reset();
            try {
                formatImpl( obj, dest );
                if( addNewline ) *(dest++) = '\n';
            }
            catch(...) {
                reset();
                throw;
            }
            reset();
        }

        /**
           Streams the given Atom in JSON form to the given stream.

           If addNewline is true then a newline is added to the end of
           the output, else no newline is added.

           Note that the default value of addNewline is different than
           the one for format(Atom,OutIter).

           May propagate std::exceptions from the output process.
        */
        inline void format( Atom const & obj, std::ostream & os, bool addNewline = true )
        {
            format( obj, std::ostream_iterator<char>(os), addNewline );
        }
        
    };
    
    /**
       Returns the string form of atom v. AtomToString<> must be
       properly specialized for v.typeID() or an exception is thrown.

       If v is-a Object or is-a Array then this returns the same as
       atomToJSON().

       The primary difference between this as atomToJSON() is that
       this routine does not do escaping of JSON strings unless v is-a
       Object or is-a Array (in which cases it really has no other
       alternative but to produce JSON output).

       @see atomToJSON()
    */
    StringType atomToString( Atom const & v );

    /**
       Returns the JSON string form of the given atom.

       For the non-string types (Boolean, Integer, Double, and Null),
       the results are identical to calling atomToString(). The
       special type Undefined is treated like the special Null value
       because the JSON spec does not specify 'undefined' as being a
       legal token.

       For String objects the results are different from
       atomToString() in that the string returned by atomToJSON()
       includes surrounding quotes and JSON-escapes any data within
       the string.

       For Object and Array atoms, atomToJSON() and atomToString() are
       equivalent.

       @see atomToString()
    */
    StringType atomToJSON( Atom const & v );

    /**
       Streams the given atom in JSON format to the given output
       iterator.

       If spacing is non-0 then spacing is added to the output as
       follows. A value of 0 means not to add any extraneous newlines
       or spaces. A value of 1 means to indent using 1 TAB character
       per level. Any other number means to indent that many SPACES
       per level. See JsonFormatter for any notes about the formatting
       rules.
    */
    template <typename OutIter>
    void atomToJSON( Atom const & a, OutIter it, uint8_t spacing = 0 )
    {
        JsonFormatter(spacing, 1==spacing).format( a, it );
    }

    /**
       Streams the given atom in JSON format to the given output stream.
    */
    void atomToJSON( Atom const & a, std::ostream & os, uint8_t spacing = 0 );

    /**
       Throws an Error if a.typeID()!=t.
    */
    void assertAtomType( Atom const & a, TypeID::Types t );

    /** Outputs v to os and returns os. */
    std::ostream & operator<<( std::ostream & os, Integer const & v );

    /** Outputs v to os and returns os. */
    std::ostream & operator<<( std::ostream & os, Boolean const & v );

    /** Outputs v to os and returns os. */
    std::ostream & operator<<( std::ostream & os, Double const & v );

    /** Outputs v, NOT JSON-escaped, to os and returns os. */
    std::ostream & operator<<( std::ostream & os, Utf8String const & v );

    /** Outputs v to os in UTF8 format, NOT JSON-escaped, and returns os. */
    std::ostream & operator<<( std::ostream & os, Utf16String const & v );

    /** Outputs v to os in JSON form. */
    std::ostream & operator<<( std::ostream & os, Object const & v );

    /** Outputs v to os in JSON form. */
    std::ostream & operator<<( std::ostream & os, Array const & v );

    /**
       Dispatches to one of the other ostream operators, depending
       on the type. The special values Atom::Null and Atom::Undefined
       are output as null resp. undefined.
    */
    std::ostream & operator<<( std::ostream & os, Atom const & a );    


    /**
       A class capable of reading UTF-8-format JSON data
       and constructing either an Object tree or an Array
       from the root node of the input.
    */
    class JsonParser
    {
    public:
        /**
           Configuration options for the parser.
         */
        struct Config
        {
            /**
               Maximum depth to parse to. Attempts to parse deeper
               than this will result in a parse error.
             */
            int maxDepth;
            /**
               Whether or not to allow C-style comments in input JSON.
            */
            bool allowComments;
            /**
               Initializes this object with "reasonable" default
               values.
            */
            Config()
                : maxDepth(32),
                  allowComments(false)
            {}
        };
    private:
        class Impl;
        friend class Impl;
        mutable Impl * m_impl;
        // line/column/offset are in this class, instead of the Impl
        // class, because i need access to them in parse<>() and don't
        // want the overhead of small wrapper functions for them.
        uint32_t m_line;
        uint32_t m_column;
        uint32_t m_offset;
        /** JSON_parser push parser callback. */
        static int callback( void * cx, int type, JSON_value const * val );
        /**
           Fetch the root node.
         */
        Atom root() const;
        /**
           Processes ch as the next UTF-8 JSON character.
           This may trigger an event callback.
        */
        bool parseChar(int ch);
        /**
           Internal initialization.
        */
        void init( Config const & );
        /**
           Returns true if parsing completely succeeded. On success
           (just-finished parsing run was okay), true is returned.

           This changes the parser's state (due to implementation
           details in the underlying parser), so the return value is
           only good once, and calling it a second time will set
           the parser in an error state.
        */
        bool parseDone();
        /**
           Requires that we have just read a key and that the
           parameter is its values.  Sets that key to that value, or
           throws if we did not just read a key.
           
           The current node in the stack must be-a Object.
        */
        void setCurrentKey( Atom const & );
        /**
           Short-hand form of setCurrentKey() (if we just parsed a
           key) or doing the equivalent operation for an array
           (pushing the value onto it).
           
           Throws ParseError if there is any unexpected state.
        */
        void pushValue( Atom const & );

        /**
           Clears internal node state but not certain other internal
           info which we theoretically might possibly want to make
           use of some day.
        */
        void clearNodes();

        /**
           Clears all node state and throws an exception (derived from
           std::exception) containing info based on the current stream
           position.
        */
        void toss();


        /**
           Re-sets the parser for a further parsing run. It is
           normally not necessary to call this, as it is called
           internally by parse().
        */
        void reset();
    public:
        /**
           Initializes the parser with a copy of the given configuration.
         */
        explicit JsonParser(Config const &);
        /**
           Initializes the parser with default configuration options.
        */
        JsonParser();
        /**
           Frees all resources.
        */
        ~JsonParser();

        /**
           Exception type thrown by JsonParser::parse(), detailing
           information about the location and cause of the
           error.
        */
        class ParseError : public Error
        {
        private:
            friend class JsonParser;
            void init(JsonParser const &p, char const *);
            ParseError(JsonParser const &p);
            ParseError(JsonParser const &p, char const *);
            uint32_t m_line;
            uint32_t m_column;
            uint32_t m_offset;
            int m_err;
        public:
            /** The line number of the error. */
            uint32_t line() const;
            /** The column number of the error. */
            uint32_t column() const;
            /** The character offset of the error.
             */
            uint32_t charOffset() const;
            /**
               The error code, from the JsonParser::ErrorCodes enum.
            */
            int errorCode() const;
        };

        /**
           Returns the current line number (1-based), which is generally
           only useful when parse() throws.
        */
        uint32_t line() const;
        /**
           Returns the current column number (0-based), which is
           generally only useful when parse() throws. Note that the
           column number is the _character_ number in that column, not
           the byte offset.  See charOffset() for details about why.
        */
        uint32_t column() const;
        /**
           Returns the current byte offset (0-based), which is
           generally only useful when parse() throws.

            Note that for UTF-8 and UTF-16 input, the character offset
            might differ from the byte offset. Unfortunately we cannot
            get the actual byte offset without relying on input
            iterators having arithmetic operations with which we can
            count the distance between them after parsing each UTF
            character. This limitation is a side-effect of the
            internal use of a C-based parser which does the UTF
            conversion itself. We simply feed it bytes one at a time
            and it figures out how many bytes it takes to make the
            next character.
         */
        uint32_t charOffset() const;
        
        /**
           Error codes which can be reported by the parser.
        */
        enum ErrorCodes {
        E_NONE = 0,
        E_INVALID_CHAR,
        E_INVALID_KEYWORD,
        E_INVALID_ESCAPE_SEQUENCE,
        E_INVALID_UNICODE_SEQUENCE,
        E_INVALID_NUMBER,
        E_NESTING_DEPTH_REACHED,
        E_UNBALANCED_COLLECTION,
        E_EXPECTED_KEY,
        E_EXPECTED_COLON,
        E_OUT_OF_MEMORY,
        E_INTERNAL,
        E_UNKNOWN
        };
        /**
           Returns the last error code hit, which is generally only
           useful when parse() throws. The value will be one of those
           from the ErrorCodes enum.
        */
        int errorCode() const;

        /**
           Equivalent to errorCodeString(errorCode()).
        */
        char const * errorCodeString() const;

        /**
           Returns a human-language (probably English!) form of the
           value from errorCode(). e.g. E_INVALID_CHAR might return
           the value "Invalid character". Returns some unspecified
           string if the code is unknown

           Note that the non-error code 0 (E_NONE) also has an
           associated string, though its exact contents are
           unspecified.
           
           The returned bytes are owned by this function and are
           guaranteed to be be valid at least until the runtime
           environment is destructed (i.e. after main() exits).
        */
        static char const * errorCodeString(int code);

        /**
           Parses the given input range as JSON. The input must be
           syntactically legal JSON data in ASCII, UTF-8, or UTF-16
           format. As per the JSON specification, it must contain
           exactly one root-level Object or Array.

           On success an Atom is returned. It will be either of
           type Object (Object::isObject(val) will return true)
           or Array (Array::isArray(val) will return true).

           On error a ParseError exception is thrown and the location
           of the error and the exact error code can then be pulled
           from this object's line(), column(), charOffset(), and
           errorCode() members (or from the exception object). In such
           a case, the non-error state of the parser is cleared, but
           the error state is left intact so the caller may query
           about the location of the error and the error code. The
           error information will be re-set on the next call to
           parse().

           Examples:

           Parsing an Object:

           @code
           char const * json = "{\"a\":1}";
           Atom root = parser.parse(json, json+strlen(json));
           assert( Object::isObject(root) );
           Object obj = Object::cast(root) );
           @endcode

           Parsing an Array:
           
           @code
           char const * json = "[1,2,3]";
           Atom root = parser.parse(json, json+strlen(json));
           assert( Array::isArray(root) );
           Array ar = Array::cast(root) );
           @endcode

           Note that the cast() operations throw if the object passed
           to them is not an Object resp. Array.

           Note that this tries to consume the WHOLE input, the main
           implication being that if an input stream contains multiple
           root objects, this routine will throw an error when it
           reaches the second object because it expects no
           non-whitespace after the first object ends.
        */
        template <typename InIter>
        Atom parse( InIter it, InIter end )
        {
            this->reset();
            for( ; it != end; ++it, ++m_offset )
            {
                const int next = *it;
                if( next <= 0 ) break;
                if( '\n' == next )
                {
                    ++this->m_line;
                    this->m_column = 0;
                }
                if( ! this->parseChar(next) )
                {
                    toss()/*throws*/;
                    return Atom::Undefined/*just in case you Don't Believe.*/;
                }
                if( '\n' != next )
                {
                    ++this->m_column;
                }
            }
            if( ! parseDone() )
            {
                toss()/*throws*/;
            }
            Atom const & rc = root();
            this->clearNodes();
            return rc;
        }

        /**
           Equivalent to the two-arg form, treating all content
           of the given stream as input.
        */
        Atom parse( std::istream & in );

        /**
           Convenience overload which parses the given input as JSON.
        */
        Atom parse( Utf8String const & in );

        /**
           Convenience overload which parses the given input as JSON.
        */
        Atom parse( Utf16String const & in );

        /**
           Convenience overload which parses the given null-terminated
           input as JSON.
        */
        Atom parse( char const * in );

        /**
           Convenience overload, equivalent to parse(in.begin(),in.end()).
        */
        Atom parse( std::string const & in );
    };


} // namespace nosjob

/* LICENSE

This software's JavaScript source code, including accompanying
documentation and demonstration applications, are licensed under the
following conditions...

Certain files are imported from external projects and have their own
licensing terms. Namely, the JSON_parser.* files and the utfcpp
sub-library. See their files for their official licenses, but the
summary is "do what you want [with them] but leave the license text
and copyright in place."

The author (Stephan G. Beal [http://wanderinghorse.net/home/stephan/])
explicitly disclaims copyright in all jurisdictions which recognize
such a disclaimer. In such jurisdictions, this software is released
into the Public Domain.

In jurisdictions which do not recognize Public Domain property
(e.g. Germany as of 2010), this software is Copyright (c) 2009, 2010
by Stephan G. Beal, and is released under the terms of the MIT License
(see below).

In jurisdictions which recognize Public Domain property, the user of
this software may choose to accept it either as 1) Public Domain, 2)
under the conditions of the MIT License (see below), or 3) under the
terms of dual Public Domain/MIT License conditions described here, as
they choose.

The MIT License is about as close to Public Domain as a license can
get, and is described in clear, concise terms at:

    http://en.wikipedia.org/wiki/MIT_License

The full text of the MIT License follows:

--
Copyright (c) 2010 Stephan G. Beal (http://wanderinghorse.net/home/stephan/)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--END OF MIT LICENSE--

For purposes of the above license, the term "Software" includes
documentation and demonstration source code which accompanies
this software. ("Accompanies" = is contained in the Software's
primary public source code repository.)

*/
#endif // wanderinghorse_net_nosjob_HPP_INCLUDED
