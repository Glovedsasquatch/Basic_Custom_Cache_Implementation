/* @author: Prabha Shankar
 *
 * The file contains utility functions that are used, namely, to:
 * 	- Fetch Background data from a database
 * 	- @name checkCacheDataValidity : Check and remove invalid entries in the cache based on 
 * 	  threshold time to live value
 * 	- @name f : the computationally time intensive function (with suitable changes to the interface
 * 	  in order to ensure that it is behaves as a pure function)
 * 	- @name callfAndStoreOutputToTable : a function that calls 'f' and enters the output State value 
 * 	  to the output table along with its dependent parameters
 */

#ifndef XUTILITY_H__
#define XUTILITY_H__

#include "BasicCustomCacheImplementation.h"

namespace utility
{
	// Dummy function for fetching Background data from a database
	const std::shared_ptr<Background const> queryBackgroundFromDB(const std::string& hash);
	
	/* Iterate over the cache to check the CacheObject objects that have overshot the time_to_live
	 * and remove them
	 */
	template<typename T> 
	void checkCacheDataValidity(std::unordered_map<std::string, CacheObject<T>>& cache)
	{
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
	
	/* f: Pure function that has a high running time
	 * Assumptions: all properties of pure function is satisfied, i.e.,
	 *	- Rule 1: the function returns identical arguments (no variation with local static variables,
	 *	  non-local variables, mutable reference arguments or input streams).
	 *	- Rule 2: function has no side-effects, i.e., no mutation of local static variables, non-local
	 *	  variables, mutable refernce arguments or input/output streams
	 *	=> the above points imply that this function does not change any attribute within State
	 *	   that is a mutable reference or calls any method in State that can change the input/output
	 *	   stream
	 */
	template<typename In>
	State f(const State& state, const In& in_input, const std::shared_ptr<Background const> background)
	{
	    //Make changes to the output_state instead of directly working on the the input argument 'state'
	    //to ensure there are no changes to state (violation of rule 2)
	    State output_state = state;

	    // The function runs from here on and returns ......

	    return output_state;
	}
	
	/* This method does two tasks:
	 *		1. call 'f'
	 *		2. insert the output State value computed by 'f' along with the argument values into 
	 *		   states_inout_value_table
	 * The output State result is placed in 'states_inout_states_inout_value_table' to prevent 'f' from recomputing
	 * this for the same input arguments
	 */
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
}

// Explicit instantiation of template function
template void utility::checkCacheDataValidity<Background>(std::unordered_map<std::string, CacheObject<Background>>& cache);

#endif //XUTILITY_H__