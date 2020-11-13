/*
 * lexical_cast.h
 *
 *  Created on: 2008-9-25
 *      Author: peterfan
 */

#ifndef _MPL_LEXICAL_CAST_H_
#define _MPL_LEXICAL_CAST_H_

#include <string>
#include <sstream>
#include <iomanip>

namespace owl {

template <typename TargetType, typename SourceType>
TargetType lexical_cast(const SourceType& value, const TargetType& defValue = TargetType()) {
	TargetType target;
	std::stringstream strm;
	strm << std::boolalpha << value;
	strm >> target >> std::noboolalpha;

	if (!strm) {
		return defValue;
	}
	return target;
}

} //namespace owl

#endif /* _MPL_LEXICAL_CAST_H_ */
