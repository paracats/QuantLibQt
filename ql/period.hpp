/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file period.hpp
    \brief period- and frequency-related classes and enumerations
*/

#ifndef quantlib_period_hpp
#define quantlib_period_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <ostream>

namespace QuantLib {

    //! Frequency of events
    /*! \ingroup datetime */
    enum Frequency { NoFrequency = -1,     //!< null frequency
                     Once = 0,             //!< only once, e.g., a zero-coupon
                     Annual = 1,           //!< once a year
                     Semiannual = 2,       //!< twice a year
                     EveryFourthMonth = 3, //!< every fourth month
                     Quarterly = 4,        //!< every third month
                     Bimonthly = 6,        //!< every second month
                     Monthly = 12          //!< once a month
    };

    /*! \relates Frequency */
    std::ostream& operator<<(std::ostream&, Frequency);

    //! Units used to describe time periods
    /*! \ingroup datetime */
    enum TimeUnit { Days,
                    Weeks,
                    Months,
                    Years
    };

    //! Time period described by a number of a given time unit
    /*! \ingroup datetime */
    class Period {
      public:
        Period()
        : length_(0), units_(Days) {}
        Period(Integer n, TimeUnit units)
        : length_(n), units_(units) {}
        Period(Frequency f);
        Integer length() const { return length_; }
        TimeUnit units() const { return units_; }
      private:
        Integer length_;
        TimeUnit units_;
    };

    /*! \relates Period */
    Period operator*(Integer n, TimeUnit units);
    /*! \relates Period */
    Period operator*(TimeUnit units, Integer n);

    /*! \relates Period */
    bool operator<(const Period&, const Period&);
    /*! \relates Period */
    bool operator==(const Period&, const Period&);
    /*! \relates Period */
    bool operator!=(const Period&, const Period&);
    /*! \relates Period */
    bool operator>(const Period&, const Period&);
    /*! \relates Period */
    bool operator<=(const Period&, const Period&);
    /*! \relates Period */
    bool operator>=(const Period&, const Period&);

    /*! \relates Period */
    std::ostream& operator<<(std::ostream&, const Period&);

    namespace detail {

        struct long_period_holder {
            long_period_holder(const Period& p) : p(p) {}
            Period p;
        };
        std::ostream& operator<<(std::ostream&, const long_period_holder&);

        struct short_period_holder {
            short_period_holder(Period p) : p(p) {}
            Period p;
        };
        std::ostream& operator<<(std::ostream&, const short_period_holder&);

    }

    namespace io {

        //! output periods in long format (e.g. "2 weeks")
        /*! \ingroup manips */
        detail::long_period_holder long_period(const Period&);

        //! output periods in short format (e.g. "2w")
        /*! \ingroup manips */
        detail::short_period_holder short_period(const Period&);

    }



    // inline definitions

    inline Period operator*(Integer n, TimeUnit units) {
        return Period(n,units);
    }

    inline Period operator*(TimeUnit units, Integer n) {
        return Period(n,units);
    }

    inline bool operator==(const Period& p1, const Period& p2) {
        return !(p1 < p2 || p2 < p1);
    }

    inline bool operator!=(const Period& p1, const Period& p2) {
        return !(p1 == p2);
    }

    inline bool operator>(const Period& p1, const Period& p2) {
        return p2 < p1;
    }

    inline bool operator<=(const Period& p1, const Period& p2) {
        return !(p1 > p2);
    }

    inline bool operator>=(const Period& p1, const Period& p2) {
        return !(p1 < p2);
    }

}

#endif