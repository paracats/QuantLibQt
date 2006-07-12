/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib {

    class CheckedCumulativeNormalDistribution
        : public CumulativeNormalDistribution {
      public:
        Real operator()(Real z) const {
            #if defined(QL_PATCH_MSVC)
            QL_REQUIRE(!_isnan(z), "Math error - not a number");
            #endif
            return CumulativeNormalDistribution::operator()(z);
        }
    };

    CMSCoupon::CMSCoupon(Real nominal,
                         const Date& paymentDate,
                         const boost::shared_ptr<SwapRate>& index,
                         const Date& startDate, const Date& endDate,
                         Integer fixingDays,
                         const DayCounter& dayCounter,
                         Rate baseRate,
                         Real multiplier,
                         Rate cap, Rate floor,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd)
    : FloatingRateCoupon(nominal, paymentDate, startDate, endDate,
                         fixingDays, 0.0, refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter), baseRate_(baseRate),
      cap_(cap), floor_(floor), multiplier_(multiplier) {
        registerWith(index_);
    }

    namespace {

        DiscountFactor discount(Rate R, const Date& tp,
                                DiscountFactor D_s0,
                                const Schedule& s, const DayCounter& dc) {
            Time alpha = dc.yearFraction(s[0],s[1]);
            Time beta = dc.yearFraction(s[0],tp);
            return D_s0/std::pow(1+alpha*R, beta/alpha);
        }

        Real level(Rate R, DiscountFactor D_s0,
                   const Schedule& s, const DayCounter& dc) {
            Real sum = 0.0;
            DiscountFactor discount = D_s0;
            for (Size j=1; j<s.size(); j++) {
                Time alpha = dc.yearFraction(s[j-1],s[j]);
                discount *= 1.0/(1.0+alpha*R);
                sum += alpha*discount;
            }
            return sum;
        }

        Real G(Rate R, const Date& tp,
               DiscountFactor D_s0,
               const Schedule& s, const DayCounter& dc) {
            return discount(R,tp,D_s0,s,dc)/level(R,D_s0,s,dc);
        }

        Real Gprime(Rate R, const Date& tp,
                    DiscountFactor D_s0,
                    const Schedule& s, const DayCounter& dc) {
            static const Spread dR = 1.0e-5;
            return (G(R+dR,tp,D_s0,s,dc)-G(R-dR,tp,D_s0,s,dc))/(2.0*dR);
        }

        Real d_lambda(Real lambda, Rate R, Rate K,
                      Volatility sigma, Time tau) {
            QL_REQUIRE(R > 0.0, "invalid forward rate:" << R);
            QL_REQUIRE(K > 0.0, "invalid strike:" << K);
            QL_REQUIRE(sigma > 0.0, "invalid volatility:" << sigma);
            QL_REQUIRE(tau > 0.0, "invalid residual time:" << tau);
            return (std::log(R/K)+lambda*sigma*sigma*tau)
                /(sigma*std::sqrt(tau));
        }

    }

    Rate CMSCoupon::rate() const {
        Date d = fixingDate();
        const Rate Rs = index_->fixing(d);
        Date today = Settings::instance().evaluationDate();
        if (d <= today || multiplier_ == 0.0) {
            // the fixing is determined
            Rate r = multiplier_*Rs + baseRate_;
            if (cap_ != Null<Rate>())
                r = std::min(r, cap_);
            if (floor_ != Null<Rate>())
                r = std::max(r, floor_);
            return r;
        } else {
            // a convexity adjustment is required
            QL_REQUIRE(!swaptionVol_.empty(), "missing swaption volatility");

            Rate rate = Rs;
            const DayCounter& dc = dayCounter_;
            Volatility sigma =
                swaptionVol_->volatility(d, index_->tenor(), Rs);
            QL_REQUIRE(sigma > 0.0, "internal error: corrupted volatility");
            Time tau = dc.yearFraction(today,d);
            Schedule s(index_->calendar(),
                       d, d+index_->tenor(),
                       index_->fixedLegFrequency(),
                       index_->fixedLegConvention());
            Date tp = date();
            DiscountFactor D_s0 = index_->termStructure()->discount(d);
            Real g = G(Rs,tp,D_s0,s,dc), g1 = Gprime(Rs,tp,D_s0,s,dc);
            Spread adjustment = (g1/g)*Rs*Rs*(std::exp(sigma*sigma*tau)-1.0);
            rate += adjustment;

            // translate cap and floor for the coupon to those for the rate.
            // also ensure that both strikes are positive as we use BS.
            Rate capStrike = Null<Rate>(), floorStrike = Null<Rate>();
            if (cap_ != Null<Rate>()) {
                if (multiplier_ > 0.0)
                    capStrike = std::max((cap_-baseRate_)/multiplier_,
                                         QL_EPSILON);
                else
                    floorStrike = std::max((cap_-baseRate_)/multiplier_,
                                           QL_EPSILON);
            }
            if (floor_ != Null<Rate>()) {
                if (multiplier_ > 0.0)
                    floorStrike = std::max((floor_-baseRate_)/multiplier_,
                                           QL_EPSILON);
                else
                    capStrike = std::max((floor_-baseRate_)/multiplier_,
                                         QL_EPSILON);
            }

            if (capStrike != Null<Rate>()) {
                Rate caplet = BlackModel::formula(Rs, capStrike,
                                                  sigma*std::sqrt(tau), 1.0);
                CheckedCumulativeNormalDistribution N;
                Real N32 = N(d_lambda(1.5,Rs,capStrike,sigma,tau));
                Real N12 = N(d_lambda(0.5,Rs,capStrike,sigma,tau));
                Real Nm12 = N(d_lambda(-0.5,Rs,capStrike,sigma,tau));

                Spread adjustment =
                    (g1/g)*(Rs*Rs*std::exp(sigma*sigma*tau)*N32
                            - Rs*(Rs+capStrike)*N12
                            + Rs*capStrike*Nm12);

                caplet += adjustment;
                rate -= caplet;
            }

            if (floorStrike != Null<Rate>()) {
                Rate floorlet = BlackModel::formula(Rs, floorStrike,
                                                    sigma*std::sqrt(tau),
                                                    -1.0);
                CheckedCumulativeNormalDistribution N;
                Real N32 = N(-d_lambda(1.5,Rs,floorStrike,sigma,tau));
                Real N12 = N(-d_lambda(0.5,Rs,floorStrike,sigma,tau));
                Real Nm12 = N(-d_lambda(-0.5,Rs,floorStrike,sigma,tau));

                Spread adjustment =
                    -(g1/g)*(Rs*Rs*std::exp(sigma*sigma*tau)*N32
                             - Rs*(Rs+floorStrike)*N12
                             + Rs*floorStrike*Nm12);

                floorlet += adjustment;
                rate += floorlet;
            }
            return multiplier_*rate + baseRate_;
        }
    }

    Rate CMSCoupon::fixing() const {
        return rate();
    }

    Date CMSCoupon::fixingDate() const {
        return index_->calendar().advance(accrualStartDate_,
                                          -fixingDays_, Days);
    }

    Rate CMSCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

    Real CMSCoupon::amount() const {
        return fixing() * accrualPeriod() * nominal();
    }

    void CMSCoupon::setSwaptionVolatility(
                   const Handle<SwaptionVolatilityStructure>& vol) {
        if (!swaptionVol_.empty())
            unregisterWith(swaptionVol_);
        swaptionVol_ = vol;
        if (!swaptionVol_.empty())
            registerWith(swaptionVol_);
        notifyObservers();
    }

    void CMSCoupon::accept(AcyclicVisitor& v) {
        Visitor<CMSCoupon>* v1 = dynamic_cast<Visitor<CMSCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }


    namespace {

        Real get(const std::vector<Real>& v, Size i,
                 Real defaultValue = Null<Real>()) {
            if (v.empty()) {
                return defaultValue;
            } else if (i < v.size()) {
                return v[i];
            } else {
                return v.back();
            }
        }

    }

    std::vector<boost::shared_ptr<CashFlow> >
    CMSCouponVector(const Schedule& schedule,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapRate>& index,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const std::vector<Rate>& baseRates,
                    const std::vector<Real>& fractions,
                    const std::vector<Rate>& caps,
                    const std::vector<Rate>& floors) {

        std::vector<boost::shared_ptr<CashFlow> > leg;
        Calendar calendar = schedule.calendar();
        Size N = schedule.size();

        QL_REQUIRE(!nominals.empty(), "no nominal given");

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        if (schedule.isRegular(1)) {
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CMSCoupon(get(nominals,0), paymentDate, index,
                              start, end, fixingDays, dayCounter,
                              get(baseRates,0,0.0), get(fractions,0,1.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              start, end)));
        } else {
            Date reference = end + Period(-12/schedule.frequency(),Months);
            reference =
                calendar.adjust(reference,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CMSCoupon(get(nominals,0), paymentDate, index,
                              start, end, fixingDays, dayCounter,
                              get(baseRates,0,0.0), get(fractions,0,1.0),
                              get(caps,0,Null<Rate>()),
                              get(floors,0,Null<Rate>()),
                              reference, end)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; i++) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            leg.push_back(boost::shared_ptr<CashFlow>(
                new CMSCoupon(get(nominals,i-1), paymentDate, index,
                              start, end, fixingDays, dayCounter,
                              get(baseRates,i-1,0.0), get(fractions,i-1,1.0),
                              get(caps,i-1,Null<Rate>()),
                              get(floors,i-1,Null<Rate>()),
                              start, end)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CMSCoupon(get(nominals,N-2), paymentDate, index,
                                  start, end, fixingDays, dayCounter,
                                  get(baseRates,N-2,0.0),
                                  get(fractions,N-2,1.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  start, end)));
            } else {
                Date reference =
                    start + Period(12/schedule.frequency(),Months);
                reference =
                    calendar.adjust(reference,paymentAdjustment);
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new CMSCoupon(get(nominals,N-2), paymentDate, index,
                                  start, end, fixingDays, dayCounter,
                                  get(baseRates,N-2,0.0),
                                  get(fractions,N-2,1.0),
                                  get(caps,N-2,Null<Rate>()),
                                  get(floors,N-2,Null<Rate>()),
                                  start, reference)));
            }
        }
        return leg;
    }

}