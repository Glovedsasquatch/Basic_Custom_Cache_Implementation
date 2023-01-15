#ifndef TEMPLATE_CLASSES_AND_DERIVATION_H__
#define TEMPLATE_CLASSES_AND_DERIVATION_H__

#include <iostream>
#include <string>
#include <ctime>
#include <memory>
#include <unordered_map>

//maximum size of the background cache
constexpr int BACKGROUND_CACHE_SIZE = 1000;

struct In;
struct Background;
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
	bool isUnderTimeToLive();

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

const long CacheObject<Background>::time_to_live;

CacheObject<Background>::CacheObject()
{

}

CacheObject<Background>::CacheObject(	const std::string& hash,
										const std::shared_ptr<Background const> data)
{
	m_hash = hash;
	m_begin_time = std::clock();
	m_data = std::make_shared<Background>(data);
}

bool CacheObject<Background>::isUnderTimeToLive()
{
	return ((std::clock() - m_begin_time) < time_to_live) ? true : false;
}

bool CacheObject<Background>::operator==(const std::string& rhs_hash)
{
	return m_hash == rhs_hash;
}

bool CacheObject<Background>::operator!=(const std::string& rhs_hash)
{
	return m_hash != rhs_hash;
}

std::string CacheObject<Background>::getHash() const
{
	return m_hash;
}

std::clock_t CacheObject<Background>::getBeginTime() const
{
	return m_begin_time;
}

std::shared_ptr<Background const> CacheObject<Background>::getData() const
{
	return m_data;
}

//Dummy function for fetching Background data from a database
const std::shared_ptr<Background const> queryBackgroundFromDB(const std::string& hash);

/* - background_cache: contains all the entries of the cached data which can be used across sessions
 * - Size of the cache: BACKGROUND_CACHE_SIZE
 * - Duration for which the data exists: depends on the static const parameter 
 *   CacheObject<Background>::time_to_live;
 *   If a previous session is resumed, the background data is flushed (retained) if the 
 *   time_to_live value is overshot (not overshot)
 */
std::unordered_map<std::string, CacheObject<Background>> background_cache;

//For keeping the 
std::unordered_map<std::shared_ptr<Background const>, std::unordered_map<In, State>> output_map;

void checkBackgroundCache(const std::string& hash)
{
	/* If a required Background value is not present in the cache, then the following is done :
	 * 1. query and fetch the Background data from the database
	 * 2. Create a temporary background cache object
	 * 3. cache the data
	 */ 
	if (background_cache.find(hash) == background_cache.end())
	{
		CacheObject<Background> temp_background_cache_obj(hash, queryBackgroundFromDB(hash)); // 1. and 2.: Done
		background_cache.insert({ hash, temp_background_cache_obj });	//3.: Done
	}
}




template<typename In>
State f(const State& state, In&&, const std::shared_ptr<Background const> background)
{
	State return_state = state;
	// The function runs from here on and returns ......

	return return_state;
}

/* - This function act as an interface to the pure function 'f'
 * - It checks if the State value corresponding to the given values of Background and In is already
 * present in the output_map
 * - If it finds the State value in output_map, it returns this value without calling 'f'; else it
 * calls 'f'
 * - Assumptions:
 *			= we assume that struct Background contains a std::string member object used to store 
 *			  a unique hash value
 */
State interfaceFunction(const State& state, In&& input, const std::shared_ptr<Background const> background) 
{
	if (output_map.find(background) != output_map.end())
	{
		if (output_map.at(background).find(input) != output_map.at(background).end())
		{
			return output_map.at(background).at(input);
		}
		else
		{

		}
	}

	State output_state = state;

	/* Make changes to the output state; 
	 * Assumptions: all properties of pure function is satisfied, i.e.,
	 *		- the function returns identical arguments (no variation with local static variables,
	 *		  non-local variables, mutable reference arguments or input streams). 
	 *		- function has no side-effects, i.e., no mutation of local static variables, non-local
	 *		  variables, mutable refernce arguments or input/output streams
	 *		=> the above points imply that this function does not change any attribute within State
	 *		  that is a mutable reference
	 */

	return output_state;

}
#endif //TEMPLATE_CLASSES_AND_DERIVATION_H__