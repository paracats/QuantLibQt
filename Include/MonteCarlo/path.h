
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file path.h
	\brief Monte Carlo path
	
	$Source$
	$Name$
	$Log$
	Revision 1.4  2001/01/08 11:44:17  lballabio
	Array back into QuantLib namespace - Math namespace broke expression templates, go figure

	Revision 1.3  2001/01/08 10:28:16  lballabio
	Moved Array to Math namespace
	
	Revision 1.2  2001/01/05 15:58:27  lballabio
	Fixed Doxygen documentation
	
	Revision 1.1  2001/01/04 17:31:22  marmar
	Alpha version of the Monte Carlo tools.
	
*/

#ifndef quantlib_montecarlo_path_h
#define quantlib_montecarlo_path_h

#include "qldefines.h"
#include "array.h"

namespace QuantLib {

	namespace MonteCarlo {

    	/*!
    	For the time being Path is equivalent to Array.	In the future this 
    	could change and Path might contain more information.
    	As of today, Path contains the list of continuously-compunded
    	variations,
    	\f[
    		\log \frac{Y_{i+1}}{Y_i} \mathrm{for} i = 0, \ldots, n-1
    	\f]
    	where \f$ Y_i \f$ is the value of the underlying at discretized time 
    	\f$ t_i \f$.
    	*/
		typedef Array Path;
	}

}


#endif
