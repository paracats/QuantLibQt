/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file cadlibor.hpp
    \brief %CAD %LIBOR rate
*/

#ifndef quantlib_cad_libor_hpp
#define quantlib_cad_libor_hpp

#include <ql/indexes/libor.hpp>
#include <ql/calendars/unitedkingdom.hpp>
#include <ql/calendars/canada.hpp>
#include <ql/daycounters/actual360.hpp>
#include <ql/currencies/america.hpp>

namespace QuantLib {

    //! %CAD LIBOR rate
    /*! Canadian Dollar LIBOR fixed by BBA.

        See <http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1414>.

        \warning This is the rate fixed in London by BBA. Use CDOR if
                 you're interested in the Canadian fixing by IDA.
    */
    class CADLibor : public Libor {
      public:
        CADLibor(const Period& tenor,
                 const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>(),
                 BusinessDayConvention convention = ModifiedFollowing,
                 bool endOfMonth = true,
                 Natural settlementDays = 2)
        : Libor("CADLibor", tenor, settlementDays, CADCurrency(),
                UnitedKingdom(UnitedKingdom::Exchange), Canada(),
                convention, endOfMonth, Actual360(), h) {}
    };

}


#endif