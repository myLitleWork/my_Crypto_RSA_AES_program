//---------------------------------------------------------------------------

#ifndef myBoostSerPairH
#define myBoostSerPairH
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_free.hpp>

// Специфическая реализация для std::pair
namespace boost {
    namespace serialization {

        template<class Archive, class T1, class T2>
        void save(Archive & ar, const std::pair<T1, T2> & p, const unsigned int version) {
            ar & p.first;
            ar & p.second;
        }

        template<class Archive, class T1, class T2>
        void load(Archive & ar, std::pair<T1, T2> & p, const unsigned int version) {
            ar & p.first;
            ar & p.second;
        }

        template<class Archive, class T1, class T2>
        void serialize(Archive & ar, std::pair<T1, T2> & p, const unsigned int version) {
            boost::serialization::split_free(ar, p, version);
        }

    } // namespace serialization
} // namespace boost

//---------------------------------------------------------------------------
#endif
