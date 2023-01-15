#include "BasicCustomCacheImplementation.h"

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