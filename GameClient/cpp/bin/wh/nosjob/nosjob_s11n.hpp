#ifndef wanderinghorse_net_nosjob_s11n_HPP_INCLUDED
#define wanderinghorse_net_nosjob_s11n_HPP_INCLUDED 1
/** @file nosjob_s11n.hpp

    This file contains the nosjob::s11n API, a miniature serialization
    framework modelled after libs11n.
*/
#if !defined(wanderinghorse_net_nosjob_HPP_INCLUDED)
#  error "You must include nosjob.hpp before including this file!"
#endif
#include <iostream>
namespace nosjob {
/**
   The s11n namespace contains code for de/serializing native objects
   via JSON objects. This is basically a stripped-down version of
   libs11n (http://s11n.net), limited to the world of JSON. libs11n
   has a gross amount of documentation on the subject, and covers it
   in a great deal of detail (which we will not repeat here!).

   To add support for client types, the easiest way is to specialize
   NativeToAtom for serialization support and AtomToNative for
   deserialization support. If those specializations exist (and
   compile) then the s11n API will use those to de/serialize
   client-provided types.

*/
namespace s11n {

    /**
       The counterpart to DeserializeProxy.

       Base serialization interface. SerT must be a type
       which can be serialized as JSON data. Specializations
       of this class may define their own serialization
       structure. This implementation uses nativeToAtom()
       to perform the serialization.
    */
    template <typename SerT>
    struct SerializeProxy
    {
        /**
           The native type to be serialized.
         */
        typedef SerT SerializableType;
        //! Internal detail - not specified by the interface.
        typedef nosjob::NativeToAtom<SerT> ProxyType;
        /**
           Must specify the Atom type required for deserialization.
           May be of type Atom, but the implementation must then
           take care to check the concrete type itself.
        */
        typedef typename ProxyType::AtomType AtomType;

        /**
           Must serialize src's state into dest. Returns true on
           success, false on error. It may throw on error.
        */
        bool operator()( AtomType & dest, SerT const & src ) const
        {
            dest = AtomType::cast( nosjob::nativeToAtom( src ) );
            return true;
        }
    };

    template <typename SerT>
    struct SerializeProxy<SerT *> : SerializeProxy<SerT>
    {
        bool operator()( typename SerializeProxy<SerT>::AtomType & dest, SerT const * src ) const
        {
            return src
                ? this->operator()( dest, *src )
                : false;
        }

    };
    template <typename SerT>
    struct SerializeProxy<SerT const *> : SerializeProxy<SerT *>
    {};

    
    /**
       The counterpart to SerializeProxy.
       
       Base deserialization interface. SerT must be a type which can
       be deserialized from JSON data. Specializations of this class
       must deserialize the state of on object from an Atom and store
       it in a native object. This implementation uses atomToNative()
       to perform the conversion.
    */
    template <typename SerT>
    struct DeserializeProxy
    {
        typedef SerT SerializableType;
        typedef nosjob::AtomToNative<SerT> ProxyType;
        template <typename AtomType>
        bool operator()( AtomType const & src, SerT & dest ) const
        {
            dest = nosjob::atomToNative<SerT>(src);
            return true;
        }
    };

    template <typename SerT>
    struct DeserializeProxy<SerT *> : DeserializeProxy<SerT>
    {
        bool operator()( typename DeserializeProxy<SerT>::AtomType const & src, SerT * dest ) const
        {
            return dest
                ? this->operator()( src, *dest )
                : false;
        }
    };
    template <typename SerT>
    struct DeserializeProxy<SerT const *> : DeserializeProxy<SerT *>
    {};

    /**
       Returns SerializeProxy<SerT>()(dest, src).
    */
    template <typename AtomType, typename SerT>
    bool serialize( AtomType & dest, SerT const & ser )
    {
        typedef SerializeProxy<SerT> PT;
        static const PT proxy = PT();
        return proxy( dest, ser );
    }

    /**
       Returns DeserializeProxy<SerT>()(src, dest).
    */
    template <typename AtomType, typename SerT>
    bool deserialize( AtomType const & src, SerT & dest )
    {
        typedef DeserializeProxy<SerT> PT;
        static const PT proxy = PT();
        return proxy( src, dest );
    }

    /**
       Serializes ser to JSON and ouptputs it to the given
       iterator. The spacing argument is documented in atomToJSON().

       Returns true on success, false on error. It may also throw on
       error.

       This is only intended to be used for root-level objects. For
       non-root, use serialize() instead.
    */
    template <typename SerT,typename OutIter>
    bool save( SerT const & ser, OutIter dest, uint8_t spacing = 0 )
    {
        typedef SerializeProxy<SerT> PT;
        typedef typename PT::AtomType CT;
        CT obj;
        if( ! TypeID::isCompound(obj.typeID()) )
        {
            throw new Error("save() requires that SerializeProxy<SerT>::AtomType be an Object or Array.");
        }
        if( ! serialize<CT,SerT>( obj, ser ) ) return false;
        else
        {
            atomToJSON( obj, dest, spacing );
        }
        return false;
    }

    /**
       Convenience overload which takes a stream instead of output
       iterators.
    */
    template <typename SerT>
    bool save( SerT const & ser, std::ostream & dest, uint8_t spacing = 0 )
    {
        bool const b = save<SerT>( ser, std::ostream_iterator<char>(dest), spacing );
        dest << '\n';
        return b;
    }

    /**
       Populates (deserializes) dest from the JSON input source specified
       by the given range. SerT must be serializable and the input is presumed
       to be in a format written by (or compatible with) that produced
       by save() with the same SerT.

       Returns true on success, false on error. It may also throw on
       error.

       This is only intended to be used for root-level objects. For
       non-root, use deserialize() instead.
    */    
    template <typename SerT, typename InIter>
    inline bool load( SerT & dest, InIter begin, InIter end )
    {
        return deserialize( nosjob::JsonParser().parse(begin,end), dest );
    }

    
    /**
       Convenience overload which takes a stream instead of input
       iterators.
    */
    template <typename SerT>
    inline bool load( SerT & dest, std::istream src )
    {
        typedef std::istream_iterator<char> IT;
        return load( dest, IT(src), IT() );
    }
}} // namespaces

#endif // wanderinghorse_net_nosjob_s11n_HPP_INCLUDED
