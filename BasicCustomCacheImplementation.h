#ifndef BASIC_CUSTOM_CACHE_IMPLEMENTATION_H__
#define BASIC_CUSTOM_CACHE_IMPLEMENTATION_H__

#include <iostream>
#include <string>
#include <ctime>
#include <memory>
#include <unordered_map>

struct In
{
	// member objects and member functions
};

struct Background
{
	// member objects and member functions
};

struct State
{
	// member objects and member functions
};

template<typename T>
class CacheObject 
{
public:
	CacheObject();
	CacheObject(const std::string& hash, const std::shared_ptr<T const> data);

	/*	Time after which the entry of the cache is flushed to prevent
	 *	the session storage to grow indefinitely;
	 *	Stored in milliseconds (ms)
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

	/*	Other methods related to this cache such as overloaded operator ==, !=
	 * to check if the hash of a stored Background matches that of the search hash
	 * 
	 * @param: rhs_hash = the hash-value of the Background object
	 */
	bool operator==(const std::string& rhs_hash);
	bool operator!=(const std::string& rhs_hash);

private:
	std::string m_hash;			//hash value to identify the background in the database
	std::clock_t m_begin_time;	//to set first time the Background data was cached
	std::shared_ptr<T> m_data;	//this is set after querying the hash from the database
};

// Dummy function for fetching Background data from a database
const std::shared_ptr<Background const> queryBackgroundFromDB(const std::string& hash);

/* - background_cache: contains all the entries of the cached data which can be used across sessions
 * - Size of the cache: BACKGROUND_CACHE_SIZE
 * - Duration for which the data exists: depends on the static const parameter 
 *   CacheObject<Background>::time_to_live;
 *   If a previous session is resumed, the background data is flushed (retained) if the 
 *   time_to_live value is overshot (not overshot)
 */
std::unordered_map<std::string, CacheObject<Background>> background_cache;

// For keeping the 
std::unordered_map<std::shared_ptr<Background const>, std::unordered_map<In, State>> output_map;

template<typename T>
void checkCacheDataValidity(const std::unordered_map<std::string, CacheObject<T>>& cache);

// Pure function which has high running time
template<typename In>
State f(const State& state, In&&, const std::shared_ptr<Background const> background);

/* - This function act as an interface to the pure function 'f'
 * - It checks if the State value corresponding to the given values of Background and In is already
 * present in the output_map
 * - If it finds the State value in output_map, it returns this value without calling 'f'; else it
 * calls 'f'
 * - Assumptions:
 *			= we assume that struct Background contains a std::string member object used to store 
 *			  a unique hash value
 */
void interfaceFunction(
	State& state, const In& input, const std::string& hash) 
{
	std::shared_ptr<Background const> background;

	//Look if the background data corresponding to the hash value is present in the background_cache
	if (background_cache.find(hash) != background_cache.end())
	{
		background = std::make_shared<Background const>(background_cache.at(hash).getData());
	}
	else
	{
		/* If a required Background value is not present in the cache, then the following is done :
		 * 1. query and fetch the Background data from the database
		 * 2. Create a temporary background cache object
		 * 3. cache the data
		 */

		//1. Fetch the data from the DB using the hash value
		background = queryBackgroundFromDB(hash);

		// 2 and 3. creating a CacheObject with 'background' and caching it
		background_cache.insert({ hash, CacheObject<Background>(hash, background)});
	}

	if (output_map.find(background) != output_map.end())
	{
		if (output_map.at(background).find(input) != output_map.at(background).end())
		{
			state = output_map.at(background).at(input);
		}
	}
	else
	{
		/* Assumptions: all properties of pure function is satisfied, i.e.,
		 *		- the function returns identical arguments (no variation with local static variables,
		 *		  non-local variables, mutable reference arguments or input streams).
		 *		- function has no side-effects, i.e., no mutation of local static variables, non-local
		 *		  variables, mutable refernce arguments or input/output streams
		 *		=> the above points imply that this function does not change any attribute within State
		 *		  that is a mutable reference or calls any method in State that can change the input/output
		 *		  stream
		 */
		state = f(state, input, background);

		/* Following is done to enter the result into the unordered_map 'output_map' to prevent 'f' to
		 * recompute the State value for same value of In and Background
		 * std::unordered_map<std::shared_ptr<Background const>, std::unordered_map<In, State>> output_map;
		 */
		//if Background data corresponding to the unique hash is already present in the output_map
		if (output_map.find(background) != output_map.end())
		{
			output_map.at(background).insert({ input, state });
		}
		//if Background data corresponding to the unique hash is not present in the ouput map
		else
		{
			std::unordered_map<In, State> temp_input_state_map = {{input, state}};
			output_map.insert({background, temp_input_state_map});
		}
	}
}

#endif //BASIC_CUSTOM_CACHE_IMPLEMENTATION_H__