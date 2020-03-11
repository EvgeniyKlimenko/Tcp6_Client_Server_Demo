#include "Exceptioning.h"

bool Exceptioning::Occurred() const
{
	return !m_exceptions.empty();
}

void Exceptioning::Append(const boost::exception_ptr& entry)
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	m_exceptions.push_back(entry);
}

void Exceptioning::Show()
{
	for (auto& entry : m_exceptions)
	{
		try
		{
			if (entry != nullptr)
			{
				boost::rethrow_exception(entry);
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}