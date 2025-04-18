// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2017-2024.
// Modifications copyright (c) 2017-2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_HAS_SELF_INTERSECTIONS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_HAS_SELF_INTERSECTIONS_HPP

#include <deque>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/throw_exception.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>

#include <boost/geometry/policies/disjoint_interrupt_policy.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_HAS_SELF_INTERSECTIONS
#  include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#  include <boost/geometry/io/dsv/write.hpp>
#endif


namespace boost { namespace geometry
{


#if ! defined(BOOST_GEOMETRY_OVERLAY_NO_THROW)

/*!
\brief Overlay Invalid Input Exception
\ingroup overlay
\details The overlay_invalid_input_exception is thrown at invalid input
 */
class overlay_invalid_input_exception : public geometry::exception
{
public:

    inline overlay_invalid_input_exception() {}

    char const* what() const noexcept override
    {
        return "Boost.Geometry Overlay invalid input exception";
    }
};

#endif


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename Geometry, typename Strategy>
inline bool has_self_intersections(Geometry const& geometry,
        Strategy const& strategy,
        bool throw_on_self_intersection = true)
{
    using point_type = point_type_t<Geometry>;
    using turn_info = turn_info
    <
        point_type,
        typename segment_ratio_type<point_type>::type
    >;
    std::deque<turn_info> turns;
    detail::disjoint::disjoint_interrupt_policy policy;

    detail::self_get_turn_points::self_turns
        <
            false,
            detail::overlay::assign_null_policy
        >(geometry, strategy, turns, policy, 0, false);

#ifdef BOOST_GEOMETRY_DEBUG_HAS_SELF_INTERSECTIONS
    bool first = true;
#endif
    for (auto const& info : turns)
    {
        bool const both_union_turn =
            info.operations[0].operation == detail::overlay::operation_union
            && info.operations[1].operation == detail::overlay::operation_union;
        bool const both_intersection_turn =
            info.operations[0].operation == detail::overlay::operation_intersection
            && info.operations[1].operation == detail::overlay::operation_intersection;

        bool const valid = (both_union_turn || both_intersection_turn)
            && (info.method == detail::overlay::method_touch
                || info.method == detail::overlay::method_touch_interior);

        if (! valid)
        {
#ifdef BOOST_GEOMETRY_DEBUG_HAS_SELF_INTERSECTIONS
            if (first)
            {
                std::cout << "turn points: " << std::endl;
                first = false;
            }
            std::cout << method_char(info.method);
            for (int i = 0; i < 2; i++)
            {
                std::cout << " " << operation_char(info.operations[i].operation);
                std::cout << " " << info.operations[i].seg_id;
            }
            std::cout << " " << geometry::dsv(info.point) << std::endl;
#endif

#if ! defined(BOOST_GEOMETRY_OVERLAY_NO_THROW)
            if (throw_on_self_intersection)
            {
                BOOST_THROW_EXCEPTION(overlay_invalid_input_exception());
            }
#endif
            return true;
        }

    }
    return false;
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_HAS_SELF_INTERSECTIONS_HPP

