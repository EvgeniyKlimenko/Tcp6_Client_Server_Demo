#if !defined(__APP_LOGIC_H__)
#define __APP_LOGIC_H__

#include "CommonDefinitions.h"
#include "Exceptioning.h"

template <class Derived> class AppLogic
{
public:
	// App is running. Possible exceptions accumulated to be shown later on.
	void Run()
	{
		try
		{
			Self().OnRun();
		}
		catch (...)
		{
			m_exceptioning.Append(std::current_exception());
		}

		if (m_exceptioning.Occurred()) m_exceptioning.Show();
	}

	// App is stopping. Exceptions accumulated.
	void Stop()
	{
		try
		{
			Self().OnStop();
		}
		catch (...)
		{
			m_exceptioning.Append(std::current_exception());
		}
	}

protected:
    Derived& Self() { return static_cast<Derived&>(*this);  }

protected:
	CExceptioning m_exceptioning;
};

#endif // __APP_LOGIC_H__