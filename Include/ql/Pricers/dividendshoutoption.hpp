
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file dividendshoutoption.hpp
    \brief base class for shout option with dividends

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/06/05 12:45:27  nando
// R019-branch-merge4 merged into trunk
//

#ifndef quantlib_dividend_shout_option_pricer_h
#define quantlib_dividend_shout_option_pricer_h

#include "ql/Pricers/dividendoption.hpp"
#include "ql/Pricers/shoutcondition.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Shout option with dividends
        class DividendShoutOption : public DividendOption {
          public:
            // constructor
            DividendShoutOption(Type type, double underlying,
                double strike, Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                const std::vector<double>& dividends = std::vector<double>(),
                const std::vector<Time>& exdivdates = std::vector<Time>(),
                int timeSteps = 100, int gridPoints = 100);

            Handle<BSMOption> clone() const;
          protected:
            void initializeStepCondition() const;
        };

    }

}


#endif
