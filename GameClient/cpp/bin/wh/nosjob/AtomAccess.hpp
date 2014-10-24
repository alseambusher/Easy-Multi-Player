// To be included by the various Atom-type CPP files, as opposed to being compiled stand-alone.
namespace nosjob {
    /**
       Provides read-write access to an Atom's internal data.

       Use with caution!
    */
    template <>
    struct Detail::AtomAccess<true>
    {
    private:
        Atom & m_a;
        AtomAccess( AtomAccess const & other ) : m_a(other.m_a)
        {}
    public:
        AtomAccess( Atom & self ) : m_a(self)
        {}
        /**
           Returns a pointer to this atoms' private data. Its meaning
           is implemention-dependent.
        */
        inline void * data() { return this->m_a.m_data; }
        /**
           Re-assigns the internal data pointer to p, WITHOUT cleaning
           up existing data if any is set. There are no side effects
           if p==this->data().
        */
        void * data( void * p )
        {
            if( p != this->m_a.m_data )
            {
                //if( d ) this->m_a.destroyData();
                this->m_a.m_data = p;
            }
            return p;
        }
    };

    /** Provides read-only access to an Atom's internal data. */
    template <>
    struct Detail::AtomAccess<false>
    {
    private:
        Atom const & m_a;
        AtomAccess( AtomAccess const & other ) : m_a(other.m_a)
        {}
    public:
        AtomAccess( Atom const & self ) : m_a(self)
        {}
        void const * data() const { return this->m_a.m_data; }
    };

}
