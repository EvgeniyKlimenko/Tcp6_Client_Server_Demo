#include "Exceptioning.h"

bool CExceptioning::Occurred() const
{
	return !m_exceptions.empty();
}

void CExceptioning::Append(const std::exception_ptr& entry)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_exceptions.push_back(entry);
}

void CExceptioning::Show()
{
	for (auto& entry : m_exceptions)
	{
		try
		{
			if (entry != nullptr)
			{
				std::rethrow_exception(entry);
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}