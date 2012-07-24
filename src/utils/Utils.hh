/*----------------------------------------------------------------------------*
 * General utils
 *----------------------------------------------------------------------------*/

#ifndef Utils_hh
#define Utils_hh

#include <cstdint>
#include <map>
#include <vector>
#include <sstream>
#include <typeinfo>

namespace ska {

#if defined(DEBUG)
#include <cassert>
#define DEBUG_ASSERT(statement) \
	assert((statement))
#else
#define DEBUG_ASSERT(statement)
#endif // DEBUG

/*----------------------------------------------------------------------------*
 * Miscellaneous
 *----------------------------------------------------------------------------*/

uint32_t nanodelay(uint32_t i);
double waste_time(std::size_t n);

int util_memalign(void ** handle, std::size_t alignment, std::size_t bytes);
void util_free(void * handle);

template<size_t v1, size_t v2>
struct CompileMax {
	enum { val = v1 > v2 ? v1 : v2 };
}; // struct CompileMax

template<size_t v1, size_t v2>
struct CompileMin {
	enum { val = v1 < v2 ? v1 : v2 };
}; // struct CompileMax

template<int32_t n, int32_t p>
struct StaticPower {
	enum { val = n * StaticPower<n, p-1>::val };
}; // struct StaticPower

template<int32_t n>
struct StaticPower<n, 0> {
	enum { val = 1 };
}; // struct StaticPower

template<int32_t n, int32_t p>
size_t CompilePow() {
	return StaticPower<n,p>::val;
} // CompilePow

static inline bool address_aligned(void * x, uintptr_t n) {
	return((uintptr_t(x) & (n-1)) == 0);
} // address_aligned

static inline std::size_t address_alignment(void * x) {
	std::size_t n(1);
	while(address_aligned(x, n)) { n=n<<1; }
	return n>>1;
} // alignment

std::string format_nano(double value, unsigned places = 0) {
	value *= 1000.0;
	std::stringstream stream;
	return (unsigned(value) > 0 || places > 0) &&
		!(stream << value << (places+1 == 1 ? " ms" : " us")).fail() ?
		stream.str() : format_nano(value, places+1);
} // format_nano

/*----------------------------------------------------------------------------*
 * std::string and type conversions
 *----------------------------------------------------------------------------*/

int32_t string_to_bool(bool & value, const std::string & str) {
	std::stringstream stream(str);
	std::string tmp;
	int32_t retval(0);

	retval = !(stream >> tmp).fail();	
	value = tmp == "true" ? true : false;
	return (str == "true" || str == "false") ? retval : 1;
} // string_to_bool

int32_t bool_to_string(const bool value, std::string & str) {
	str = value ? "true" : "false";
	return 0;
} // bool_to_string

template<typename T>
int32_t string_to_type(T & value, const std::string & str) {
	std::stringstream stream(str);
	return !(stream >> value).fail();
} // string_to_t

template<typename T>
int32_t type_to_string(const T value, std::string & str) {
	std::stringstream stream;
	int32_t retval = !(stream << value).fail();
	str = stream.str();
	return retval;
} // type_to_string

int32_t string_to_bool_vector(std::vector<bool> & values,
	const std::string & str) {
	std::stringstream stream(str);
	std::string tmp;
	int32_t retval(0);

	for(size_t i(0); i<values.size(); ++i) {
		retval |= !(stream >> tmp).fail();
		values[i] = tmp == "true" ? true : false;
	} // for

	return retval;
} // string_to_bool

int32_t bool_vector_to_string(const std::vector<bool> & values,
	std::string & str) {
	std::stringstream stream;
	int32_t retval(0);
	
	for(size_t i(0); i<values.size(); ++i) {
		std::string tmp;
		retval |= bool_to_string(values[i], tmp);
		retval |= !(stream << tmp).fail();
		if(i<values.size()-1) { stream << " "; }
	} // for

	str = stream.str();

	return retval;
} // bool_vector_to_string

template<typename T>
int32_t string_to_vector(std::vector<T> & values, const std::string & str) {
	std::stringstream stream(str);
	int32_t retval(0);

	for(size_t i(0); i<values.size(); ++i) {
		retval = !(stream >> values[i]).fail();
	}

	return retval;
} // string_to_vector

template<typename T>
int32_t vector_to_string(const std::vector<T> & values, std::string & str) {
	std::stringstream stream;
	int32_t retval(0);
	
	for(size_t i(0); i<values.size(); ++i) {
		std::string tmp;
		retval |= type_to_string(values[i], tmp);
		retval |= !(stream << tmp).fail();
		if(i<values.size()-1) { stream << " "; }
	} // for

	str = stream.str();

	return retval;
} // vector_to_string

/*----------------------------------------------------------------------------*
 * Check for existence/non-existence of std::map keys
 *----------------------------------------------------------------------------*/

template<typename Key, class Value>
bool stdMapKeyExists(std::map<Key, Value> & map, Key key)
	{
		typename std::map<Key, Value>::const_iterator i = map.find(key);
		return i == map.end() ? false : true;
	} // stdMapKeyExists

// const version
template<typename Key, class Value>
bool stdMapKeyExists(const std::map<Key, Value> & map, Key key)
	{
		typename std::map<Key, Value>::const_iterator i = map.find(key);
		return i == map.end() ? false : true;
	} // stdMapKeyExists

template<typename Key, class Value>
bool stdMapKeyDoesNotExist(std::map<Key, Value> & map, Key key)
	{
		typename std::map<Key, Value>::const_iterator i = map.find(key);
		return i == map.end() ? true : false;
	} // stdMapKeyDoesNotExist

// const version
template<typename Key, class Value>
bool stdMapKeyDoesNotExist(const std::map<Key, Value> & map, Key key)
	{
		auto i = map.find(key);
		return i == map.end() ? true : false;
	} // stdMapKeyDoesNotExist

} // namespace ska

#endif // Utils_hh

/*----------------------------------------------------------------------------*
 * Local Variables: 
 * mode:c++
 * c-basic-offset:3
 * indent-tabs-mode:t
 * tab-width:3
 * End:
 *
 * vim: set ts=3 :
 *----------------------------------------------------------------------------*/
