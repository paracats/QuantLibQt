
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

/*! \file barrieroption.hpp
    \brief barrier option

    $Id$
*/

// $Source$
// $Log$
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_barrier_option_pricer_h
#define quantlib_barrier_option_pricer_h

#include "ql/Pricers/bsmoption.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Barrier option
        /*! The analytical calculation are taken from 
            "Option pricing formulas", E.G. Haug, McGraw-Hill, 
            p.69 and following.
        */
        class BarrierOption : public BSMOption {
          public:
            // constructor
            enum BarrierType { DownIn, UpIn, DownOut, UpOut };
            BarrierOption(BarrierType barrType, Type type, double underlying,
                double strike, Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility, double barrier,
                double rebate = 0.0);
            // accessors
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            Handle<BSMOption> clone() const {
                return Handle<BSMOption>(new BarrierOption(*this));
            }
          protected:
            void calculate() const;
            mutable double greeksCalculated_, delta_, gamma_, theta_;
          private:
            BarrierType barrType_;
            double barrier_, rebate_;
            mutable double sigmaSqrtT_, mu_, muSigma_;
            mutable double dividendDiscount_, riskFreeDiscount_;
            Math::CumulativeNormalDistribution f_;
            void initialize() const;
            double A(double eta, double phi) const;
            double B(double eta, double phi) const;
            double C(double eta, double phi) const;
            double D(double eta, double phi) const;
            double E(double eta, double phi) const;
            double F(double eta, double phi) const;
        };


        // inline definitions
        
        inline double BarrierOption::A(double eta, double phi) const {
            double x1 = QL_LOG(underlying_/strike_)/sigmaSqrtT_ + muSigma_;
            double N1 = f_(phi*x1);
            double N2 = f_(phi*(x1-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * N1
                        - strike_ * riskFreeDiscount_ * N2);
        }

        inline double BarrierOption::B(double eta, double phi) const {
            double x2 = QL_LOG(underlying_/barrier_)/sigmaSqrtT_ + muSigma_;
            double N1 = f_(phi*x2);
            double N2 = f_(phi*(x2-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * N1
                      - strike_ * riskFreeDiscount_ * N2);
        }

        inline double BarrierOption::C(double eta, double phi) const {
            double HS = barrier_/underlying_;
            double powHS0 = QL_POW(HS, 2 * mu_);
            double powHS1 = powHS0 * HS * HS;
            double y1 = QL_LOG(barrier_*HS/strike_)/sigmaSqrtT_ + muSigma_;
            double N1 = f_(eta*y1);
            double N2 = f_(eta*(y1-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * powHS1 * N1
                          - strike_ * riskFreeDiscount_ * powHS0 * N2);
        }

        inline double BarrierOption::D(double eta, double phi) const {
            double HS = barrier_/underlying_;
            double powHS0 = QL_POW(HS, 2 * mu_);
            double powHS1 = powHS0 * HS * HS;
            double y2 = QL_LOG(barrier_/underlying_)/sigmaSqrtT_ + muSigma_;
            double N1 = f_(eta*y2);
            double N2 = f_(eta*(y2-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * powHS1 * N1
                          - strike_ * riskFreeDiscount_ * powHS0 * N2);
        }

        inline double BarrierOption::E(double eta, double phi) const {
            if (rebate_ > 0) {
                double powHS0 = QL_POW(barrier_/underlying_, 2 * mu_);
                double x2 = QL_LOG(underlying_/barrier_)/sigmaSqrtT_ + muSigma_;
                double y2 = QL_LOG(barrier_/underlying_)/sigmaSqrtT_ + muSigma_;
                double N1 = f_(eta*(x2 - sigmaSqrtT_));
                double N2 = f_(eta*(y2 - sigmaSqrtT_));
                return rebate_ * riskFreeDiscount_ * (N1 - powHS0 * N2);
            } else {
                return 0.0;
            }
        }

        inline double BarrierOption::F(double eta, double phi) const {
            if (rebate_ > 0) {
                double lambda = QL_SQRT(mu_*mu_ + 2.0*riskFreeRate_/
                                            (volatility_ * volatility_));
                double HS = barrier_/underlying_;
                double powHSplus = QL_POW(HS, mu_ + lambda);
                double powHSminus = QL_POW(HS, mu_ - lambda);

                double z = QL_LOG(barrier_/underlying_)/sigmaSqrtT_
                    + lambda * sigmaSqrtT_;

                double N1 = f_(eta * z);
                double N2 = f_(eta * (z - 2.0 * lambda * sigmaSqrtT_));
                return rebate_ * (powHSplus * N1 + powHSminus * N2);
            } else {
                return 0.0;
            }
        }

    }

}


#endif
