#ifndef BASIC_CUSTOM_CACHE_IMPLEMENTATION_H__
#define BASIC_CUSTOM_CACHE_IMPLEMENTATION_H__

#include <iostream>
#include <string>
#include <ctime>
#include <memory>
#include <unordered_map>
#include <map>

struct In
{
	//Dummy member variables
    	int m_dummy_value;
    	char m_dummy_char;

    	// Member functions
    	bool operator==(const In& rhs) const; //placed for code correctness
};

struct Background
{
	/* Member objects */
    	// hash is unqiue to every Background object
    	std::string m_hash; 
    
    	//member functions
};

struct State
{
	//Dummy members
    	int dummy_val1;
    	int dummy_val2;

    	// Member functions
    	bool operator==(const State& rhs) const; // placed for code-correctness
};

template<typename T>
class CacheObject 
{
public:
	CacheObject();
	CacheObject(const std::string& hash, const std::shared_ptr<T const> data);

	/* @param time_to_live: Time after which the entry of the cache is flushed to prevent
     	 * the session storage to grow indefinitely;
     	 * Stored in milliseconds (ms)
     	 */ 
	static const long time_to_live = 86400000; //milli-seconds equivalent to one day

	//method to check if the Cached data has overshot the time to live
	bool isUnderTimeToLive() const;

	//Get hash method
	std::string getHash() const;

	//Get begin time method
	std::clock_t getBeginTime() const;

	//Get data method
	std::shared_ptr<T const> getData() const;

	/* operator overloadeding to check equality  ==, !=
	 * @param: rhs_hash = the hash value of the Background object
	 */
	bool operator==(const std::string& rhs_hash);
	bool operator!=(const std::string& rhs_hash);

private:
	std::string m_hash;		//hash value to identify the background in the database
	std::clock_t m_begin_time;	//to set the timestamp when the background data was first cached
	std::shared_ptr<T> m_data;	//this is set after querying the database with the hash
};

/* - background_cache: contains all the entries of the cached data which can be used across sessions
 * - Duration for which the data exists depends on @param CacheObject<Background>::time_to_live;
 *   If a previous session is resumed, the background data is flushed (retained) if the 
 *   time_to_live value is overshot (not overshot)
 */
std::unordered_map<std::string, CacheObject<Background>> background_cache;

/* @states_inout_value_table: stores the output tate value
 * The output state value depends on:
 *    - input state value
 *    - background value
 *    - In&&: the rvalue reference 'In' object
 */
using InOutState = std::tuple<State, State>;
std::unordered_map<std::shared_ptr<Background const>, std::multimap<In, InOutState>> states_inout_value_table;

/* - This function act as an interface to the pure function 'f'
 * - It checks if the output State value corresponding to the input State value and the values of
 * the arguments Background, and In
 * - If such an entry is found in 'states_inout_states_inout_value_table', the outout State value is returned without
 * calling 'f'
 * - Assumption/s:
 *		= we assume that struct Background contains a std::string member object used to store
 *		  a unique hash value
 */
void interfaceFunction(State& state, const In& input, const std::string& hash);

#endif //BASIC_CUSTOM_CACHE_IMPLEMENTATION_H__