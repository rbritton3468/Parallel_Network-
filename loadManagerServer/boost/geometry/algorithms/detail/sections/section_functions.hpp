// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015-2024.
// Modifications copyright (c) 2015-2024, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_FUNCTIONS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_FUNCTIONS_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tag_cast.hpp>

// For spherical/geographic longitudes covered_by point/box
#include <boost/geometry/strategies/cartesian/point_in_box.hpp>

#include <boost/geometry/util/select_coordinate_type.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace section
{

// TODO: This code is CS-specific, should be moved to strategies

template
<
    std::size_t Dimension,
    typename Geometry,
    typename CastedCSTag = tag_cast_t<cs_tag_t<Geometry>, spherical_tag>
>
struct preceding_check
{
    template <typename Point, typename Box>
    static inline bool apply(int dir, Point const& point, Box const& /*point_box*/, Box const& other_box)
    {
        return (dir == 1  && get<Dimension>(point) < get<min_corner, Dimension>(other_box))
            || (dir == -1 && get<Dimension>(point) > get<max_corner, Dimension>(other_box));
    }
};

template <typename Geometry>
struct preceding_check<0, Geometry, spherical_tag>
{
    template <typename Point, typename Box>
    static inline bool apply(int dir, Point const& point, Box const& point_box, Box const& other_box)
    {
        using calc_t = typename select_coordinate_type<Point, Box>::type;
        using units_t = detail::coordinate_system_units_t<Point>;

        calc_t const c0 = 0;

        calc_t const value = get<0>(point);
        calc_t const other_min = get<min_corner, 0>(other_box);
        calc_t const other_max = get<max_corner, 0>(other_box);

        bool const pt_covered = strategy::within::detail::covered_by_range
                                    <
                                        Point, 0, spherical_tag
                                    >::apply(value,
                                             other_min,
                                             other_max);

        if (pt_covered)
        {
            return false;
        }

        if (dir == 1)
        {
            calc_t const diff_min = math::longitude_distance_signed
                                        <
                                            units_t, calc_t
                                        >(other_min, value);

            calc_t const diff_min_min = math::longitude_distance_signed
                                        <
                                            units_t, calc_t
                                        >(other_min, get<min_corner, 0>(point_box));

            return diff_min < c0 && diff_min_min <= c0 && diff_min_min <= diff_min;
        }
        else if (dir == -1)
        {
            calc_t const diff_max = math::longitude_distance_signed
                                        <
                                            units_t, calc_t
                                        >(other_max, value);

            calc_t const diff_max_max = math::longitude_distance_signed
                                        <
                                            units_t, calc_t
                                        >(other_max, get<max_corner, 0>(point_box));

            return diff_max > c0 && diff_max_max >= c0 && diff_max <= diff_max_max;
        }

        return false;
    }
};


template
<
    std::size_t Dimension,
    typename Point,
    typename Box
>
inline bool preceding(int dir,
                      Point const& point,
                      Box const& point_box,
                      Box const& other_box)
{
    return preceding_check<Dimension, Box>::apply(dir, point, point_box, other_box);
}

template
<
    std::size_t Dimension,
    typename Point,
    typename Box
>
inline bool exceeding(int dir,
                      Point const& point,
                      Box const& point_box,
                      Box const& other_box)
{
    return preceding<Dimension>(-dir, point, point_box, other_box);
}


}} // namespace detail::section
#endif


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_SECTIONS_FUNCTIONS_HPP
