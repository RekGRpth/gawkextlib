/*
 * awkjasonhandler.h - Class to accept JSON events and decode
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

// NOTE: You must include "gawkapi.h" before this file

#include <list>
#include "rapidjson/reader.h"

struct AwkJsonHandler :
public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, AwkJsonHandler> {
	// rapidJson methods:
	bool Null();
	bool Bool(bool b);
	bool Int(int i);
	bool Uint(unsigned u);
	bool Int64(int64_t i);
	bool Uint64(uint64_t u);
	bool Double(double d);
	bool String(const char* str, rapidjson::SizeType length, bool copy);
	bool StartObject();
	bool Key(const char* str, rapidjson::SizeType length, bool copy);
	bool EndObject(rapidjson::SizeType memberCount);
	bool StartArray();
	bool EndArray(rapidjson::SizeType elementCount);

	AwkJsonHandler(awk_array_t outArray)
	: m_outArray(outArray),
	  m_currentIndex(0),
	  m_processingArray(false),
	  m_level(0)
	{
		memset(& m_currentKey, 0, sizeof(m_currentKey));
	}

private:
	bool setElement();	// set the current element into the current array
	void pushMembers();	// push working members onto stack
	void popMembers();	// pop them off again
	void initMembers();	// zero out the members

	awk_array_t  m_outArray;

	awk_string_t m_currentKey;		// key of element being processed
	awk_value_t  m_currentValue;		// value being read

	size_t       m_currentIndex;		// for linear arrays
	bool         m_processingArray;

	size_t       m_level;			// how deep we are in the object

	struct inProgress {
		awk_array_t  outArray;

		awk_string_t key;
		awk_value_t  value;

		size_t       index;
		bool         processingArray;

		// constructor
		inProgress(awk_array_t Out, awk_string_t Key,
		           awk_value_t Value, size_t Index, bool ProcessingArray)
		: outArray(Out),
		  key(Key),
		  value(Value),
		  index(Index),
		  processingArray(ProcessingArray)
		{
		}
	};

	std::list<inProgress>  m_memberStack;
};
