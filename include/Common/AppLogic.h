#if !defined(__APP_LOGIC_H__)
#define __APP_LOGIC_H__

#include "CommonDefinitions.h"
#include "Exceptioning.h"
#include "System/TerminationLogic.h"

template <typename Derived> class AppLogic
{
public:
	template <typename... Anything>
	AppLogic(Anything&&... a)
	: m_terminationLogic(boost::bind(&AppLogic::Stop, this))
	{}

	// App is running. Possible exceptions accumulated to be shown later on.
	void Run()
	{
		try
		{
			Self().OnRun();
		}
		catch (...)
		{
			m_exceptioning.Append(boost::current_exception());
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
			m_exceptioning.Append(boost::current_exception());
		}
	}

protected:
	CRTP_SELF(Derived)

	Exceptioning m_exceptioning;
	TerminationLogic m_terminationLogic;
};

#define RUN_APP(AppType, ...) 							\
	using AppType_ptr = boost::scoped_ptr< AppType >;	\
	AppType_ptr app(new AppType( __VA_ARGS__ ));		\
    app->Run();

#endif // __APP_LOGIC_H__