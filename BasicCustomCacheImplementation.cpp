#include "BasicCustomCacheImplementation.h"

bool In::operator==(const In& rhs) const
{
    return (m_dummy_value == rhs.m_dummy_value) && (m_dummy_char == rhs.m_dummy_char);
}

bool State::operator==(const State& rhs) const
{
    return (dummy_val1 == rhs.dummy_val1) && (dummy_val2 == rhs.dummy_val2);
}


const long CacheObject<Background>::time_to_live;

CacheObject<Background>::CacheObject()
{

}

CacheObject<Background>::CacheObject(const std::string& hash,
	const std::shared_ptr<Background const> data)
{
	m_hash = hash;
	m_begin_time = std::clock();
	m_data = std::make_shared<Background>(data);
}

bool CacheObject<Background>::isUnderTimeToLive() const
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

template<typename T> 
void checkCacheDataValidity(const std::unordered_map<std::string, CacheObject<T>>& cache)
{
	/* If a required Background value is not present in the cache, then the following is done :
	 * 1. query and fetch the Background data from the database
	 * 2. Create a temporary background cache object
	 * 3. cache the data
	 */
	for (auto it = cache.begin(); it != cache.end();)
	{
		if (!it->second.isUnderTimeToLive())
		{
			it = cache.erase(it);
		}
		else
		{
			it++;
		}
	}
}

template<typename In>
State f(const State& state, In&&, const std::shared_ptr<Background const> background)
{
    //Make changes to the output_state instead of directly working on the the input argument 'state'
    State return_state = state;

    // The function runs from here on and returns ......

    return return_state;
}

void callfAndStoreOutputToTable(
	State& state,
	const In& input,
	const std::shared_ptr<Background const> background,
	std::unordered_map<std::shared_ptr<	Background const>, std::multimap<In, InOutState>>& value_table)
{
	// Calling 'f'
	State input_state = state;
	state = f(input_state, input, background);

	// if 'background' is already present in the 'states_inout_states_inout_value_table'
	if (states_inout_value_table.find(background) != states_inout_value_table.end())
	{
		states_inout_value_table.at(background).insert({ input, std::make_tuple(input_state, state) });
	}
	// if 'background' is not present in the 'states_inout_states_inout_value_table'
	else
	{
		std::multimap<In, InOutState> temp_inout_multimap = { { input, std::make_tuple(input_state, state) } };
		states_inout_value_table.insert({ background, temp_inout_multimap });
	}
}

void interfaceFunction(State& state, const In& input, const std::string& hash) 
{
	// @param background: stores the value fetched from either the cache or the database
	std::shared_ptr<Background const> background;

	// Looks if the background data corresponding to the hash value is present in 'background_cache'
	if (background_cache.find(hash) != background_cache.end())
	{
		background = std::make_shared<Background const>(background_cache.at(hash).getData());
	}
	else // Fetch from the database and cache 
	{
		/* If a required Background value is not present in the cache, then the following is done :
		 * 1. query and fetch the Background data from the database
		 * 2. Create a temporary background cache object
		 * 3. cache the data
		 */

		 //1. Fetch the data from the DB using the hash value
		background = queryBackgroundFromDB(hash);

		// 2 and 3. creating a CacheObject with 'background' and caching it
		background_cache.insert({ hash, CacheObject<Background>(hash, background) });
	}

	//Checks for the value corresponding to the background in 'states_inout_states_inout_value_table'
	if (states_inout_value_table.find(background) != states_inout_value_table.end())
	{
		//checks if the input value for the background is present in 'states_inout_value_table'
		if (states_inout_value_table.at(background).find(input) != states_inout_value_table.at(background).end())
		{
			auto states_multimap = states_inout_value_table.at(background);
			auto itr = states_multimap.begin();
			for (; itr != states_multimap.end(); itr++)
			{
				//checks if the In 'input' value and the input State values match; then, returns and breaks
				if (itr->first == input && std::get<0>(itr->second) == state)
				{
					state = std::get<1>(itr->second);
					break;
				}
			}

			if (itr == states_multimap.end())
			{
				callfAndStoreOutputToTable(state, input, background, states_inout_value_table);
			}
		}
		else //calls 'f'
		{
			callfAndStoreOutputToTable(state, input, background, states_inout_value_table);
		}
	}
	else // calls 'f'
	{
		callfAndStoreOutputToTable(state, input, background, states_inout_value_table);
	}
}

