/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file simplex.hpp
    \brief Simplex optimization method

    \fullpath
    ql/Optimization/%simplex.hpp
*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 10
 */

#ifndef quantlib_optimization_simplex_h
#define quantlib_optimization_simplex_h

#include <ql/Optimization/problem.hpp>

#include <vector>

namespace QuantLib {

    namespace Optimization {

        //! Multi-dimensionnal Simplex class
        class Simplex : public Method {
          public:
            /*! Constructor taking as input lambda as the
                characteristic length and tol as the precision
            */
            Simplex(double lambda, double tol) 
            : Method(), lambda_(lambda), tol_(tol) {}
            virtual ~Simplex() {}

            virtual void minimize(const Problem& P) const;
          private:
            double extrapolate(const Problem& P, Size iHighest, 
                               double& factor) const;
            double lambda_;
            double tol_;
            mutable std::vector<Array> vertices_;
            mutable Array values_, sum_;
        };

    }

}


#endif
