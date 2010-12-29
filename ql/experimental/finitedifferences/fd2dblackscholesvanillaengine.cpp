/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen
 
 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/exercise.hpp>
#include <ql/experimental/finitedifferences/fdm2dblackscholessolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesmesher.hpp>
#include <ql/experimental/finitedifferences/fd2dblackscholesvanillaengine.hpp>

namespace QuantLib {

    Fd2dBlackScholesVanillaEngine::Fd2dBlackScholesVanillaEngine(
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& p1,
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& p2,
        Real correlation,
        Size xGrid, Size yGrid,
        Size tGrid, Size dampingSteps,
        const FdmSchemeDesc& schemeDesc)
    : p1_(p1),
      p2_(p2),
      correlation_(correlation),
      xGrid_(xGrid), yGrid_(yGrid), tGrid_(tGrid),
      dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc) {
    }

    void Fd2dBlackScholesVanillaEngine::calculate() const {
        // 1. Layout
        std::vector<Size> dim;
        dim.push_back(xGrid_);
        dim.push_back(yGrid_);
        boost::shared_ptr<FdmLinearOpLayout> layout(new FdmLinearOpLayout(dim));

        const boost::shared_ptr<BasketPayoff> payoff =
            boost::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);

        // 2. Mesher
        const Time maturity = p1_->time(arguments_.exercise->lastDate());
        const boost::shared_ptr<Fdm1dMesher> em1(
            new FdmBlackScholesMesher(
                    xGrid_, p1_, maturity, p1_->x0(), 
                    Null<Real>(), Null<Real>(), 0.0001, 1.5, 
                    std::pair<Real, Real>(p1_->x0(), 0.1)));

        const boost::shared_ptr<Fdm1dMesher> em2(
            new FdmBlackScholesMesher(
                    xGrid_, p2_, maturity, p2_->x0(), 
                    Null<Real>(), Null<Real>(), 0.0001, 1.5, 
                    std::pair<Real, Real>(p2_->x0(), 0.1)));

        std::vector<boost::shared_ptr<Fdm1dMesher> > meshers;
        meshers.push_back(em1);
        meshers.push_back(em2);
        const boost::shared_ptr<FdmMesher> mesher (
                                     new FdmMesherComposite(layout, meshers));

        // 3. Calculator
        const boost::shared_ptr<FdmInnerValueCalculator> calculator(
                                new FdmLogBasketInnerValue(payoff, mesher));

        // 4. Step conditions
        const boost::shared_ptr<FdmStepConditionComposite> conditions =
            FdmStepConditionComposite::vanillaComposite(
                                    DividendSchedule(), arguments_.exercise, 
                                    mesher, calculator, 
                                    p1_->riskFreeRate()->referenceDate(),
                                    p1_->riskFreeRate()->dayCounter());

        // 5. Boundary conditions
        std::vector<boost::shared_ptr<FdmDirichletBoundary> > boundaries;

        // 6. Solver
        boost::shared_ptr<Fdm2dBlackScholesSolver> solver(
                new Fdm2dBlackScholesSolver(
                             Handle<GeneralizedBlackScholesProcess>(p1_),
                             Handle<GeneralizedBlackScholesProcess>(p2_),
                             correlation_,
                             mesher, boundaries, conditions, calculator,
                             maturity, tGrid_, dampingSteps_,
                             schemeDesc_));

        const Real x = p1_->x0();
        const Real y = p2_->x0();

        results_.value = solver->valueAt(x, y);
        results_.theta = solver->thetaAt(x, y);
    }
}