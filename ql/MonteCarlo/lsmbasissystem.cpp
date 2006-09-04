/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file lsmbasissystem.cpp
    \brief utility classes for longstaff schwartz early exercise Monte Carlo
*/

#include <deque>
#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <ql/Math/functional.hpp>
#include <ql/Math/gaussianorthogonalpolynomial.hpp>
#include <ql/MonteCarlo/lsmbasissystem.hpp>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>

using boost::bind;


namespace QuantLib {

    namespace {
        // do some simple template meta programming to speed-up
        // the most frequently used basis system
        const Size maxPolynomOrder = 10;

        template <Size N> 
        inline Real times(Real x) {
            return x*times<N-1>(x);
        }
        template <>
        inline Real times<0>(Real x) {
            return 1;
        }

        template <Size N>
        std::vector<boost::function1<Real, Real> > monomials() {
            std::vector<boost::function1<Real, Real> > ret(monomials<N-1>());
            ret.push_back(&times<N>);
            return ret;
        }
        template <>
        std::vector<boost::function1<Real, Real> > monomials<0>() {
            std::vector<boost::function1<Real, Real> > ret;
            ret.push_back(&times<0>);
            return ret;
        }

        // some compilers have problems with Array::operator[]
        // in conjunction with boost::bind (e.g. gcc-4.1.1). 
        // Therefore this workaround function will be defined.
        inline Real f_workaround(const Array& a, Size i) {
            return a[i];
        } 
    }

    std::vector<boost::function1<Real, Real> > 
    LsmBasisSystem::pathBasisSystem(Size order, PolynomType polynomType) {

        QL_REQUIRE(order <= maxPolynomOrder, 
                   "order of basis system exceeds max order");
        
        std::vector<boost::function1<Real, Real> > ret;
        for (Size i=0; i<=order; ++i) {
            switch (polynomType) {
              case Monomial:
                  ret.push_back(monomials<maxPolynomOrder>()[i]);
                break;
              case Laguerre:
                ret.push_back(
                    bind(&GaussianOrthogonalPolynomial::weightedValue,
                                GaussLaguerrePolynomial(), i, _1));
                break;
              case Hermite:
                ret.push_back(
                    bind(&GaussianOrthogonalPolynomial::weightedValue,
                                GaussHermitePolynomial(), i, _1));
                break;
              case Hyperbolic:
                ret.push_back(
                    bind(&GaussianOrthogonalPolynomial::weightedValue,
                                GaussHyperbolicPolynomial(), i, _1));
                break;
              case Legendre:
                ret.push_back(
                    bind(&GaussianOrthogonalPolynomial::weightedValue,
                                GaussLegendrePolynomial(), i, _1));
                break;
              case Chebyshev:
                ret.push_back(
                    bind(&GaussianOrthogonalPolynomial::weightedValue,
                                GaussChebyshevPolynomial(), i, _1));
                break;
              case Chebyshev2th:
                ret.push_back(
                    bind(&GaussianOrthogonalPolynomial::weightedValue,
                                GaussChebyshev2thPolynomial(), i, _1));
                break;
              default:
                QL_FAIL("unknown regression type");
            }
        }
        
        return ret;
    }


    std::vector<boost::function1<Real, Array> > 
    LsmBasisSystem::multiPathBasisSystem(Size dim, Size order, 
                                         PolynomType polynomType) {

        const std::vector<boost::function1<Real, Real> > b 
            = pathBasisSystem(order, polynomType);

        std::vector<boost::function1<Real, Array> > ret;
        ret.push_back(constant<Array, Real>(1.0));

        for (Size i=1; i<=order; ++i) {
            const std::vector<boost::function1<Real, Array> > a 
                = w(dim, i, polynomType, b);
            
            for (std::vector<boost::function1<Real, Array> >::const_iterator
                     iter = a.begin(); iter < a.end(); ++iter) {
                ret.push_back(*iter);
            }
        }
        
        // remove-o-zap: now remove redundant functions.
        // usually we do have a lot of them due to the construction schema.
        // We use a more "hands on" method here.
        std::deque<bool> rm(ret.size(), true);

        Array x(dim), v(ret.size());
        MersenneTwisterUniformRng rng(1234UL);

        for (Size i=0; i<10; ++i) {
            Size k;

            // calculate random x vector
            for (k=0; k<dim; ++k) {
                x[k] = rng.next().value;
            }
            
            // get return values for all basis functions
            for (k=0; k<ret.size(); ++k) {
                v[k] = ret[k](x);
            }

            // find dublicates
            for (k=0; k<ret.size(); ++k) {
                if (std::find_if(v.begin(), v.end(), 
                                 bind(equal_with<Real>(10*v[k]*QL_EPSILON), 
                                      v[k], _1) ) 
                    == v.begin() + k) {

                    // don't remove this item, it's unique!
                    rm[k] = false;
                }
            }
        }

        std::vector<boost::function1<Real, Array> >::iterator
            iter = ret.begin();

        for (Size i=0; i < rm.size(); ++i) {
            if (rm[i]) {
                iter = ret.erase(iter);
            }
            else {
                ++iter;
            }
        }

        return ret;
    }


    std::vector<boost::function1<Real, Array> > 
    LsmBasisSystem::w(Size dim, Size order, PolynomType polynomType,
                      const std::vector<boost::function1<Real, Real> > & b) {

       std::vector<boost::function1<Real, Array> > ret;

       for (Size i=order; i>=1; --i) {
           const std::vector<boost::function1<Real, Array> > left
                = w(dim, order-i, polynomType, b);

           for (Size j=0; j<dim; ++j) {
               const boost::function1<Real, Array> a 
                   = bind(b[i], bind(f_workaround, _1, j));

               if (i == order) {
                   ret.push_back(a);
               }
               else {
                   // add linear combinations
                   for (Size j=0; j<left.size(); ++j) {
                       ret.push_back( 
                            boost::lambda::bind(a,      (boost::lambda::_1))
                           *boost::lambda::bind(left[j],(boost::lambda::_1)));
                   }
               }
           }
       }

       return ret;
    }
}
