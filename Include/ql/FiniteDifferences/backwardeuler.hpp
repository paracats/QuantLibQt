
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

/*! \file backwardeuler.hpp
    \brief backward Euler scheme for finite differemce methods

    $Id$
*/

// $Source$
// $Log$
// Revision 1.7  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.6  2001/05/28 14:55:23  lballabio
// Missing inclusion fixed
//
// Revision 1.5  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_backward_euler_h
#define quantlib_backward_euler_h

#include "ql/FiniteDifferences/operatortraits.hpp"
#include "ql/FiniteDifferences/finitedifferencemodel.hpp"
#include "ql/FiniteDifferences/identity.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Backward Euler scheme for finite difference methods
        /*! Differential operators must be derived from either 
            TimeConstantOperator or TimeDependentOperator.
            They must also implement at least the following interface:

            \code
            // copy constructor/assignment (might be provided by the compiler)
            Operator(const Operator&);
            Operator& operator=(const Operator&);

            // modifiers
            void setTime(Time t);  // those derived from TimeConstantOperator
                                   // might skip this if the compiler allows it.

            // operator interface
            arrayType solveFor(const arrayType&);

            // operator algebra
            Operator operator*(double,const Operator&);
            Operator operator+(const Identity<arrayType>&,const Operator&);
            \endcode
        */
        template <class Operator>
        class BackwardEuler {
            friend class FiniteDifferenceModel<BackwardEuler<Operator> >;
          private:
            // typedefs
            typedef typename OperatorTraits<Operator>::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            BackwardEuler(const operatorType& D) : D(D), dt(0.0) {}
            void step(arrayType& a, Time t) const;
            void setStep(Time dt) {
                this->dt = dt;
                implicitPart = Identity<arrayType>()+dt*D;
            }
            operatorType D, implicitPart;
            Time dt;
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                // a bit of template metaprogramming to relax interface 
                // constraints on time-constant operators
                // see T. L. Veldhuizen, "Using C++ Template Metaprograms", 
                // C++ Report, Vol 7 No. 4, May 1995
                // http://extreme.indiana.edu/~tveldhui/papers/
                template <int constant>
                class BackwardEulerTimeSetter {};
                // the following specialization will be instantiated if 
                // Operator is derived from TimeConstantOperator
                template<>
                class BackwardEulerTimeSetter<0> {
                  public:
                    static inline void setTime(Operator& D, 
                    Operator& implicitPart, Time t, Time dt) {}
                };
                // the following specialization will be instantiated if 
                // Operator is derived from TimeDependentOperator:
                // only in this case Operator will be required to 
                // implement void setTime(Time t)
                template<>
                class BackwardEulerTimeSetter<1> {
                    typedef typename OperatorTraits<Operator>::arrayType 
                        arrayType;
                  public:
                    static inline void setTime(Operator& D, 
                      Operator& implicitPart, Time t, Time dt) {
                        D.setTime(t);
                        implicitPart = Identity<arrayType>()+dt*D;
                    }
                };
            #endif
        };

        // inline definitions

        template <class Operator>
        inline void BackwardEuler<Operator>::step(arrayType& a, Time t) const {
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                BackwardEulerTimeSetter<Operator::isTimeDependent>::setTime(
                    D,implicitPart,t,dt);
            #else
                if (Operator::isTimeDependent) {
                    D.setTime(t);
                    implicitPart = Identity<arrayType>()+dt*D;
                }
            #endif
            a = implicitPart.solveFor(a);
        }

    }

}


#endif
