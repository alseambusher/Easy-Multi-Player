#ifndef wanderinghorse_net_nosjob_convert_HPP_INCLUDED
#define wanderinghorse_net_nosjob_convert_HPP_INCLUDED 1
#if !defined(wanderinghorse_net_nosjob_HPP_INCLUDED)
#  error "You must include nosjob.hpp before including this file!"
#endif
/** @file nosjob_convert.hpp

    This file contains the nosjob::NativeToAtom and nosjob::AtomToNative APIs.
*/
#include <algorithm> // std::copy()
#include <iterator> // std::back_inserter
#include <vector>
#include <utility> // make_pair()
#include <set>
namespace nosjob {

    /**
       Base interface for converting arbitrary native types to
       Atom equivalents. It must be specialized to be useful.

       Required typedefs:

       AtomType must be the concrete Atom type returned by
       operator(). It may be the base Atom type or a more specialized
       type.
    */
    template <typename NativeT>
    struct NativeToAtom
    {
        /**
           Defines the type of argument passed to operator().
         */
        //typedef NativeT const & ArgType;
        //typedef NativeT NativeType;
        /**
           Defines the concrete Atom type returned by operator().
         */
        typedef Atom AtomType;
        /**
           Must convert arg to an Atom and return it. It may
           throw (deriving from std::exception) on error.
        */
        AtomType operator()( NativeT const & arg ) const;
    };

    /**
       Returns NativeToAtom<NativeT>()(val).
    */
    template <typename NativeT>
    Atom nativeToAtom( NativeT const & val )
    {
        typedef NativeToAtom<NativeT> NTA;
        static NTA nta;
        return nta( val );
    }


    /**
       Specialized to take a pointer argument.
    */
    template <typename NativeT>
    struct NativeToAtom<NativeT const * >
    {
        typedef NativeToAtom<NativeT> ProxyType;
        typedef typename ProxyType::AtomType AtomType;
        //typedef NativeT const * ArgType;
        //typedef NativeT NativeType;
        /**
           If arg is non-NULL then it returns
           NativeToAtom<NativeT>()(*arg), else it returns
           a default-constructed AtomType.
        */
        AtomType operator()( NativeT const * arg ) const
        {
            static NativeToAtom<NativeT> proxy;
            return arg ? proxy(*arg) : AtomType();
        }
    };
    template <typename NativeT>
    struct NativeToAtom<NativeT *> : NativeToAtom<NativeT const *> {};
    template <typename NativeT>
    struct NativeToAtom<NativeT &> : NativeToAtom<NativeT> {};
    template <typename NativeT>
    struct NativeToAtom<NativeT const &> : NativeToAtom<NativeT> {};

    /**
       A helper to implement NativeToAtom for the concrete Atom types.
       It simply returns the atom passed to it.
     */
    template <typename AtomType_>
    struct NativeToAtom_atom
    {
        typedef AtomType_ AtomType;
        //typedef AtomType const & ArgType;
        inline AtomType operator()( AtomType const & arg ) const
        {
            return arg;
        }
    };

    template <>
    struct NativeToAtom<Atom> : NativeToAtom_atom<Atom> {};
    template <>
    struct NativeToAtom<Boolean> : NativeToAtom_atom<Boolean> {};
    template <>
    struct NativeToAtom<Integer> : NativeToAtom_atom<Integer> {};
    template <>
    struct NativeToAtom<Double> : NativeToAtom_atom<Double> {};
    template <>
    struct NativeToAtom<Utf8String> : NativeToAtom_atom<Utf8String> {};
    template <>
    struct NativeToAtom<Utf16String> : NativeToAtom_atom<Utf16String> {};
    template <>
    struct NativeToAtom<Object> : NativeToAtom_atom<Object> {};
    template <>
    struct NativeToAtom<Array> : NativeToAtom_atom<Array> {};


    /**
       A helper for implementing certain NativeToAtom implementations.
       AtomType must be-a Atom type and must have a constuctor which
       takes one argument type type AtomType_::ValueType.
    */
    template <typename AtomType_>
    struct NativeToAtom_copy_ctor
    {
        typedef AtomType_ AtomType;
        //typedef typename AtomType::ValueType const & ArgType;
        inline AtomType operator()( typename AtomType::ValueType const & arg ) const
        {
            return AtomType(arg);
        }
    };
    
    template <>
    struct NativeToAtom<int8_t>
        : NativeToAtom_copy_ctor<Integer>
    {};

    template <>
    struct NativeToAtom<int16_t>
        : NativeToAtom_copy_ctor<Integer>
    {};
    template <>
    struct NativeToAtom<int32_t>
        : NativeToAtom_copy_ctor<Integer>
    {};
    template <>
    struct NativeToAtom<int64_t>
        : NativeToAtom_copy_ctor<Integer>
    {};

    template <>
    struct NativeToAtom<Double::ValueType>
        : NativeToAtom_copy_ctor<Double>
    {};

    template <>
    struct NativeToAtom<Boolean::ValueType>
        : NativeToAtom_copy_ctor<Boolean>
    {};

    template <>
    struct NativeToAtom<std::string>
        : NativeToAtom_copy_ctor<Utf8String>
    {
    };

    /**
       Specialization for (char const *).
    */
    template <>
    struct NativeToAtom<char const *>
    {
        //typedef char const * ArgType;
        typedef Utf8String AtomType;
        AtomType operator()( char const * arg ) const
        {
            return AtomType(arg ? arg : "");
        }
    };

    /**
       A helper for implementing certain NativeToAtom implementations.
       ListType must be a list container type which follows STL
       conventions.

       It must have a const_iterator typedef and
       begin()/end() functions which return const_iterators.

       ListType::value_type must be convertible to an Atom via
       nativeToAtom().
    */
    template <typename ListType>
    struct NativeToAtom_list_base
    {
        //typedef ListType const & ArgType;
        typedef Array AtomType;
        inline AtomType operator()( ListType const &  arg ) const
        {
            AtomType ar;
            typedef typename ListType::const_iterator IT;
            IT it = arg.begin();
            for( ; arg.end() != it; ++it ) {
                ar.push( nativeToAtom(*it) );
            }
            return ar;
        }
    };

    template <typename T>
    struct NativeToAtom< std::list<T> > : NativeToAtom_list_base< std::list<T> > {};
    
    template <typename T>
    struct NativeToAtom< std::vector<T> > : NativeToAtom_list_base< std::vector<T> > {};

    template <typename T>
    struct NativeToAtom< std::set<T> > : NativeToAtom_list_base< std::set<T> > {};

    /**
       A helper for implementing certain NativeToAtom implementations.
       MapType must be a map container type which follows STL
       conventions.

       It must have a const_iterator typedef and begin()/end()
       functions which return const_iterators to std::pair-compatible
       structures.

       Both MapType::key_type and MapType::mapped_type must be
       convertible to nosjob via nativeToAtom().
    */
    template <typename MapType>
    struct NativeToAtom_string_map_base
    {
        //typedef MapType const & ArgType;
        typedef Object AtomType;
        inline AtomType operator()( MapType const & arg ) const
        {
            Object obj;
            typedef typename MapType::const_iterator IT;
            IT it = arg.begin();
            for( ; arg.end() != it; ++it ) {
                obj.set( nativeToAtom((*it).first),
                         nativeToAtom((*it).second) );
            }
            return obj;
        }
    };

    template <typename V>
    struct NativeToAtom< std::map<std::string,V> > : NativeToAtom_string_map_base< std::map<std::string,V> > {};

    template <typename FirstT, typename SecondT>
    struct NativeToAtom< std::pair<FirstT, SecondT > >
    {
        typedef std::pair<FirstT,SecondT> PairType;
        //typedef PairType const & ArgType;
        // TODO? use [] instead of {} for storage.
        typedef Object AtomType;
        inline AtomType operator()( PairType const & arg ) const
        {
            static const StringType str1("k");
            static const StringType str2("v");
            Object obj;
            obj.set(str1, nativeToAtom(arg.first) );
            obj.set(str2, nativeToAtom(arg.second) );
            return obj;
        }
    };

    //! Workaround for std::map pairs, which have const keys.
    template <typename FirstT, typename SecondT>
    struct NativeToAtom< std::pair<const FirstT, SecondT > >
        : NativeToAtom< std::pair<FirstT, SecondT > >
    {};


    /**
       A helper for implementing certain NativeToAtom implementations.
       MapType must be a map container type which follows STL
       conventions.

       It must have a const_iterator typedef and begin()/end()
       functions which return const_iterators to std::pair-compatible
       structures.

       Both MapType::key_type and MapType::mapped_type must be
       convertible to nosjob via nativeToAtom().

       Note that because a JSON Object uses strings as keys, this type
       serialized the native data as an Array of pair object, each with
       a "k" and "v" member which holds the actual map data.
    */
    template <typename MapType>
    struct NativeToAtom_map_base
    {
        //typedef MapType const & ArgType;
        typedef Array AtomType;
        inline AtomType operator()( MapType const & arg ) const
        {
            AtomType obj;
            typedef typename MapType::const_iterator IT;
            IT it = arg.begin();
            for( ; arg.end() != it; ++it ) {
                obj.push( nativeToAtom( *it ) );
            }
            return obj;
        }
    };

    template <typename K, typename V>
    struct NativeToAtom< std::map<K,V> > : NativeToAtom_map_base< std::map<K,V> > {};

    /**
       Base interface for converting atoms to arbitrary native types.
       It must be specialized to be useful.

       Required interface:

       ReturnType typedef must be the return type of operator()
       implementation(s).

       operator(), with this interface:
       
       ReturnType operator()( AnAtomType const & ) const;
    */
    template <typename NativeT>
    struct AtomToNative
    {
        typedef NativeT ReturnType;
//         typedef Atom AtomType;
        //ReturnType operator()( AtomType const & arg ) const;
    };
    template <typename NativeT>
    struct AtomToNative<NativeT const> : AtomToNative<NativeT> {};
    template <typename NativeT>
    struct AtomToNative<NativeT &> : AtomToNative<NativeT> {};
    template <typename NativeT>
    struct AtomToNative<NativeT const &> : AtomToNative<NativeT> {};

    /**
       Returns AtomToNative<NativeType>()( val ).
    */
    template <typename NativeType, typename AtomType>
    typename AtomToNative<NativeType>::ReturnType
    atomToNative( AtomType const & val )
    {
        typedef AtomToNative<NativeType> ATN;
        static const ATN atn = ATN();
        return atn( val );
    }

//     /**
//        A helper to implement AtomToNative for the concrete Atom types.
//        It simply returns the atom passed to it.
//      */
//     template <typename AtomType_>
//     struct AtomToNative_atom
//     {
//         typedef AtomType_ AtomType;
//         typedef AtomType ReturnType;
//         typedef AtomType const & ArgType;
//         inline ReturnType operator()( ArgType arg ) const
//         {
//             return arg;
//         }
//     };

    /**
       A helper base class for "converting" Atoms to their own native
       type (i.e. themselves). AtomType must be Atom or a concrete
       subclass of Atom.
    */
    template <typename AtomType>
    struct AtomToNative_atom
    {
        typedef AtomType ReturnType;
        /**
           Simply returns arg.
         */
        inline ReturnType operator()( AtomType const & arg ) const
        {
            return arg;
        }
#if 0
        /*
          If we do this we can take advantage of all AtomType::cast()
          operations for conversions, GREATLY simplifying the other
          AtomToNative_atom subclasses, but we lose compile-time
          type safety for some cases where i don't want to lose that.

          Long term, i'd like to re-implement the cast() operations in
          terms of AtomToNative<AtomType>, to keep the direct
          relationships between the classes to a minimum.
         */
      
        /**
           Returns AtomType::cast(arg), or throws if that
           function throws.
        */
        template <typename OtherAtomType>
        inline ReturnType operator()( OtherAtomType const & arg ) const
        {
            return AtomType::cast(arg);
        }
#endif
    };

    template <>
    struct AtomToNative<Atom> : AtomToNative_atom<Atom> {};
    template <>
    struct AtomToNative<Boolean> : AtomToNative_atom<Boolean>
    {
        /**
           Returns Boolean(arg.boolValue()).
        */
        inline ReturnType operator()( Atom const & arg ) const
        {
            return Boolean(arg.boolValue());
        }
    };
    template <>
    struct AtomToNative<Integer> : AtomToNative_atom<Integer>
    {
        inline ReturnType operator()( Double const & arg ) const
        {
            return ReturnType(arg.value() ? 1 : 0);
        }
        inline ReturnType operator()( Boolean const & arg ) const
        {
            return Integer(arg.value() ? 1 : 0);
        }
    };
    template <>
    struct AtomToNative<Double> : AtomToNative_atom<Double>
    {
        inline ReturnType operator()( Integer const & arg ) const
        {
            return ReturnType(arg.value()*1.0);
        }
        inline ReturnType operator()( Boolean const & arg ) const
        {
            return ReturnType(arg.value() ? 1.0 : 0.0);
        }
    };

    template <>
    struct AtomToNative<Utf8String>
    {
        typedef Utf8String ReturnType;
        inline ReturnType operator()( Atom const & arg ) const
        {
            return atomToString(arg);
        }
    };

    template <>
    struct AtomToNative<Utf16String> : AtomToNative_atom<Utf16String>
    {
        typedef Utf16String ReturnType;
        inline ReturnType operator()( Atom const & arg ) const
        {
            return Utf16String(atomToString(arg));
        }
    };

    template <>
    struct AtomToNative<Object> : AtomToNative_atom<Object> {};

    template <>
    struct AtomToNative<Array> : AtomToNative_atom<Array> {};

    /**
       Full specialization which returns Atom::boolValue() for any
       atom passed to it.
    */
    template <>
    struct AtomToNative<bool>
    {
        typedef bool ReturnType;
        ReturnType operator()( Atom const & arg ) const
        {
            return arg.boolValue();
        }
    };

    /**
       A helper base type for Integer and Double AtomToNative
       conversions.
    */
    template <typename AtomType>
    struct AtomToNative_number_base
    {
        typedef typename AtomType::ValueType ReturnType;
        inline ReturnType operator()( Integer const & arg ) const
        {
            return ReturnType(arg.value());
        }
        inline ReturnType operator()( Double const & arg ) const
        {
            return ReturnType(arg.value());
        }
        inline ReturnType operator()( Boolean const & arg ) const
        {
            return ReturnType(arg.value() ? 1 : 0);
        }

#if 1
        /**
           Will throw if AtomType::cast(arg) throws.

           This sacrifices compile-time type-safety so that we can do
           certain conversions which cannot be deamed type-safe at
           compile-time. e.g. AtomToNative_number_base<Double> cannot
           call Double(arg) from here because Double(Atom) does not
           exist (no default implemenation is feasible). And if it
           did, we'd have the same problem that that we have here with
           the lack of compile-time type safety. (i.e., the problem
           would just move to the AtomType(Atom) ctor
           implementations.)
        */
        ReturnType operator()( Atom const & arg ) const
        {
            return AtomType::cast(arg).value();
        }
#endif
    };

    template <>
    struct AtomToNative<int16_t> : AtomToNative_number_base<Integer> {};
    template <>
    struct AtomToNative<int32_t> : AtomToNative_number_base<Integer> {};
    template <>
    struct AtomToNative<int64_t> : AtomToNative_number_base<Integer> {};
    template <>
    struct AtomToNative<double> : AtomToNative_number_base<Double> {};

    template <>
    struct AtomToNative<std::string>
    {
        typedef std::string ReturnType;
        /**
           Equivalent to atomToString(arg).value().

           FIXME: only works when StringType == Utf8String
        */
        template <typename AtomType>
        inline ReturnType operator()( AtomType const & arg ) const
        {
            return atomToString(arg).value();
        }
    };

    template <>
    struct AtomToNative<Utf16String::ValueType>
    {
        typedef Utf16String::ValueType ReturnType;
        inline ReturnType operator()( Utf16String const & arg ) const
        {
            return arg.value();
        }

        /**
           Equivalent to Utf16String(arg).value(). If the underlying
           Utf16String::ValueType string class supports reference
           counting, this shouldn't be _too_ expensive.
        */
        inline ReturnType operator()( Utf8String const & arg ) const
        {
            return Utf16String(arg).value();
        }
    };


    /**
       A base AtomToNative implementation for STL-like list types.

       Requirements:

       ListType must be conventions-compatible with std::list and
       friends.  std::back_inserter<ListType> must be able to
       function.
    */
    template <typename ListType>
    struct AtomToNative_list_base
    {
        typedef ListType ReturnType;
        /**
           Converts ar to a list. atomToNative<ListType::value_type>()
           is used for the conversions.
        */
        inline ReturnType operator()( Array const & ar ) const
        {
            ListType rc;
            Array::ConstIterator it = ar.begin();
            Array::ConstIterator const et = ar.end();
            std::back_insert_iterator<ListType> bi( std::back_inserter<ListType>(rc) );
            for( ; it != et; ++it )
            {
                *(bi++) = atomToNative<typename ListType::value_type>(*it);
            }
            return rc;
        }
    };
    template <typename T>
    struct AtomToNative< std::list<T> > : AtomToNative_list_base< std::list<T> > {};
    template <typename T>
    struct AtomToNative< std::vector<T> > : AtomToNative_list_base< std::vector<T> > {};
    template <typename T>
    struct AtomToNative< std::set<T> > : AtomToNative_list_base< std::set<T> > {};

    /**
       Similar to AtomToNNative_map_base, but uses an Object for
       serialization instead of an Array. Such a conversion is only
       useful when MapType::key_type is directly convertible to/from a
       string.

       MapType must be conventions-compatible with std::map.
    */
    template <typename MapType>
    struct AtomToNative_string_map_base
    {
        typedef MapType ReturnType;
        /**
           Converts ar to a list. atomToNative<MapType::value_type>()
           is used for the conversions.
        */
        inline ReturnType operator()( Object const & ar ) const
        {
            MapType rc;
            Object::ConstIterator it = ar.begin();
            Object::ConstIterator const et = ar.end();
            for( ; it != et; ++it )
            {
                Object::Entry const & ent = *it;
                rc.insert( std::make_pair( atomToNative<typename MapType::key_type>(ent.key),
                                           atomToNative<typename MapType::mapped_type>(ent.value)
                                           ) );
            }
            return rc;
        }
    };

    template <typename V>
    struct AtomToNative< std::map<std::string,V> > : AtomToNative_string_map_base< std::map<std::string,V> > {};



    template <typename T1, typename T2>
    struct AtomToNative< std::pair<T1,T2> >
    {
        typedef std::pair<T1, T2> PairType;
        typedef PairType ReturnType;
        /**
           Converts ar to a std::pair. atomToNative<T1>() and
           atomToNative<T2>() are used for the conversions.
        */
        inline ReturnType operator()( Object const & ar ) const
        {
            static const StringType str1("k");
            static const StringType str2("v");
            if( 2 != ar.size() )
            {
                throw Error( "pair<>.size != 2" );
            }
            return std::make_pair( atomToNative<T1>( ar.get(str1) ),
                                   atomToNative<T1>( ar.get(str2) ) );
        }
//         inline ReturnType operator()( Array const & ar ) const
//         {
//             if( 2 != ar.size() )
//             {
//                 throw Error( "pair<>.size != 2" );
//             }
//             return std::make_pair( atomToNative<T1>( ar.get(0) ),
//                                    atomToNative<T1>( ar.get(1) ) );
//         }
    };

    
    template <typename MapType>
    struct AtomToNative_map_base
    {
        typedef MapType ReturnType;
        /**
           Converts ar to a std::pair. atomToNative<MapType::key_type>() and
           atomToNative<MapType::mapped_type>() are used for the conversions.
        */
        inline ReturnType operator()( Array const & ar ) const
        {
            MapType rc;
            Array::ConstIterator it = ar.begin();
            Array::ConstIterator const et = ar.end();
            std::back_insert_iterator<MapType> bi( std::back_inserter<MapType>(rc) );
            for( ; it != et; ++it )
            {
                Atom const & v(*it);
                if( Object::isObject(v) ) {
                    rc.insert( atomToNative< typename MapType::value_type >(Object::cast(*it)) );
                }
                else {
                    throw Error("Unexpected type in serialized Map data.");
                }
            }
            return rc;
        }
    };

    template <typename K, typename V>
    struct AtomToNative< std::map<K,V> > : AtomToNative_map_base< std::map<K,V> > {};

} // namespace

#endif // wanderinghorse_net_nosjob_convert_HPP_INCLUDED
