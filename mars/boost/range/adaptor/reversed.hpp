// Boost.Range library
//
//  Copyright Thorsten Ottosen, Neil Groves 2006 - 2008. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef BOOST_RANGE_ADAPTOR_REVERSED_HPP
#define BOOST_RANGE_ADAPTOR_REVERSED_HPP

#include <boost/range/iterator_range.hpp>
#include <boost/range/concepts.hpp>
#include <boost/iterator/reverse_iterator.hpp>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
    namespace range_detail
    {
        template< class R >
        struct reversed_range : 
            public mars_boost::iterator_range<
                      mars_boost::reverse_iterator<
                        BOOST_DEDUCED_TYPENAME range_iterator<R>::type 
                                              >
                                         >
        {
        private:
            typedef mars_boost::iterator_range<
                      mars_boost::reverse_iterator<
                        BOOST_DEDUCED_TYPENAME range_iterator<R>::type 
                                              >
                                         >
                base;
            
        public:
            typedef mars_boost::reverse_iterator<BOOST_DEDUCED_TYPENAME range_iterator<R>::type> iterator;

            explicit reversed_range( R& r ) 
                : base( iterator(mars_boost::end(r)), iterator(mars_boost::begin(r)) )
            { }
        };

        struct reverse_forwarder {};
        
        template< class BidirectionalRange >
        inline reversed_range<BidirectionalRange> 
        operator|( BidirectionalRange& r, reverse_forwarder )
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                BidirectionalRangeConcept<BidirectionalRange>));

            return reversed_range<BidirectionalRange>( r );
        }

        template< class BidirectionalRange >
        inline reversed_range<const BidirectionalRange> 
        operator|( const BidirectionalRange& r, reverse_forwarder )
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                BidirectionalRangeConcept<const BidirectionalRange>));

            return reversed_range<const BidirectionalRange>( r ); 
        }
        
    } // 'range_detail'
    
    using range_detail::reversed_range;

    namespace adaptors
    { 
        namespace
        {
            const range_detail::reverse_forwarder reversed = 
                                            range_detail::reverse_forwarder();
        }
        
        template<class BidirectionalRange>
        inline reversed_range<BidirectionalRange>
        reverse(BidirectionalRange& rng)
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                BidirectionalRangeConcept<BidirectionalRange>));

            return reversed_range<BidirectionalRange>(rng);
        }
        
        template<class BidirectionalRange>
        inline reversed_range<const BidirectionalRange>
        reverse(const BidirectionalRange& rng)
        {
            BOOST_RANGE_CONCEPT_ASSERT((
                BidirectionalRangeConcept<const BidirectionalRange>));

            return reversed_range<const BidirectionalRange>(rng);
        }
    } // 'adaptors'
    
} // 'boost'

#endif
