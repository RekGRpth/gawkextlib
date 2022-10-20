/*
 * awkjsonhandler.cpp - Class to accept JSON events and decode
 * them into a gawk array.
 */

/*
 * Copyright (C) 2017 the Free Software Foundation, Inc.
 * Copyright (C) 2017 Arnold David Robbins.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

// This code adapted from the rwarray.c extension in the gawk distribution.
// Primarily for the logic to walk an array and encode / decode it.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"
#include "awkjsonhandler.h"

using namespace rapidjson;
extern gawk_api_t *const api;
extern awk_ext_id_t ext_id;

#define setValueType(type) \
	memset(& m_currentValue, 0, sizeof(m_currentValue)); \
	m_currentValue.val_type = type;

#define setNumericValue(val) \
	setValueType(AWK_NUMBER); \
	m_currentValue.num_value = val;

#if gawk_api_major_version > 3 || (gawk_api_major_version == 3 && gawk_api_minor_version >= 2)
#define setBooleanValue(val) \
	setValueType(AWK_BOOL); \
	m_currentValue.bool_value = val ? awk_true : awk_false;
#endif

// Null --- create a null value
bool AwkJsonHandler::Null()
{
	setValueType(AWK_UNDEFINED);
	return setElement();
}

// Bool --- create a boolean value - use 0 or 1
bool AwkJsonHandler::Bool(bool b)
{
	double val = b ? 1.0 : 0.0;

#if gawk_api_major_version > 3 || (gawk_api_major_version == 3 && gawk_api_minor_version >= 2)
	setBooleanValue(val);
#else
	setNumericValue(val);
#endif
	return setElement();
}

// Int --- create an integer value
bool AwkJsonHandler::Int(int i)
{
	double val = i;

	setNumericValue(val);
	return setElement();
}

// Uint --- create an unsigned integer value
bool AwkJsonHandler::Uint(unsigned u)
{
	double val = u;

	setNumericValue(val);
	return setElement();
}

// Int64 --- create a 64-bit integer value
bool AwkJsonHandler::Int64(int64_t i)
{
	double val = i;

	setNumericValue(val);
	return setElement();
}


// Uint64 --- create an unsigned 64-bit integer value
bool AwkJsonHandler::Uint64(uint64_t u)
{
	double val = u;

	setNumericValue(val);
	return setElement();
}

// Double --- create a double value
bool AwkJsonHandler::Double(double d)
{
	double val = d;

	setNumericValue(val);
	return setElement();
}

// String --- create a string value
bool AwkJsonHandler::String(const char* str, SizeType length, bool copy)
{
	bool strIsRegexp = (str[0] == '@' && str[1] == '/' && str[length-1] == '/');

	if (strIsRegexp) {
		size_t newLen = length - 3;

		setValueType(AWK_REGEX);

		m_currentValue.str_value.len = newLen;
		m_currentValue.str_value.str = (char *) gawk_malloc(newLen + 1);
		memcpy(m_currentValue.str_value.str, str + 2, newLen);
		m_currentValue.str_value.str[newLen] = '\0';
	} else {
		setValueType(AWK_STRNUM);

		m_currentValue.str_value.len = length;
		m_currentValue.str_value.str = (char *) gawk_malloc(length+1);
		memcpy(m_currentValue.str_value.str, str, length);
		m_currentValue.str_value.str[length] = '\0';
	}

	return setElement();
}

// Key --- Set the current key
bool AwkJsonHandler::Key(const char* str, SizeType length, bool copy)
{
	m_currentKey.str = (char *) gawk_malloc(length + 1);
	memcpy(m_currentKey.str, str, length);
	m_currentKey.str[length] = '\0';
	m_currentKey.len = length;

	return true;
}

// setElement --- install an element into the array being built
bool AwkJsonHandler::setElement()
{
	awk_element_t elem;

	memset(& elem, 0, sizeof(elem));
	elem.value = m_currentValue;

	if (m_processingArray) {
		char buf[1000];

		sprintf(buf, "%lu", m_currentIndex++);

		awk_value_t val;
		make_const_string(buf, strlen(buf), & val);
		elem.index = val;
	} else {
		elem.index.val_type = AWK_STRING;
		elem.index.str_value = m_currentKey;
	}

	// set a variable in case we need to use a debugger
	bool result = set_array_element_by_elem(m_outArray, & elem);

	return result;
}

// pushMembers --- push working members onto stacks
void AwkJsonHandler::pushMembers()
{
	inProgress currentState(m_outArray, m_currentKey, m_currentValue, m_currentIndex, m_processingArray);
	m_memberStack.push_back(currentState);
}

// popMembers --- pop them off again
void AwkJsonHandler::popMembers()
{
	inProgress currentState = m_memberStack.back();
	m_memberStack.pop_back();

	m_outArray = currentState.outArray;
	m_currentKey = currentState.key;
	m_currentValue = currentState.value;
	m_currentIndex = currentState.index;
	m_processingArray = currentState.processingArray;
}

// zeroMembers --- init everything to zero
void AwkJsonHandler::initMembers()
{
	memset(& m_outArray, 0, sizeof(m_outArray));

	memset(& m_currentKey, 0, sizeof(m_currentKey));
	memset(& m_currentValue, 0, sizeof(m_currentValue));

	m_currentIndex = 0;
	m_processingArray = false;
}

// StartObject --- start a new subarray
bool AwkJsonHandler::StartObject()
{
	if (m_level++ == 0)
		return true;

	pushMembers();
	initMembers();

	m_outArray = create_array();
	return true;
}

// EndObject --- end the current subarray
bool AwkJsonHandler::EndObject(SizeType memberCount)
{
	if (--m_level == 0)
		return true;

	// save the array we just built as a value
	awk_value_t array_val;
	array_val.val_type = AWK_ARRAY;
	array_val.array_cookie = m_outArray;

	popMembers();

	// make the array the value to be set
	m_currentValue = array_val;

	// and set it
	return setElement();
}

// StartArray --- start an integer indexed array
bool AwkJsonHandler::StartArray()
{
	StartObject();

	m_processingArray = true;
	m_currentIndex = 1;	// AWK arrays start at 1

	return true;
}


// EndArray --- end an integer indexed array
bool AwkJsonHandler::EndArray(SizeType elementCount)
{
	if (m_currentIndex != elementCount) {
		// log an error
	}

	bool ret = EndObject(elementCount);

	m_processingArray = false;
	m_currentIndex = 1;	// AWK arrays start at 1

	return ret;
}
