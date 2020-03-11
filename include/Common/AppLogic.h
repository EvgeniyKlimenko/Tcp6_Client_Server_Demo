#if !defined(__APP_LOGIC_H__)
#define __APP_LOGIC_H__

#include "CommonDefinitions.h"
#include "Exceptioning.h"
#include "System/TerminationLogic.h"

template <typename Implementation> class AppLogic
{
public:
	template <typename... Anything>
	AppLogic(Anything&&... a)
	: m_terminationLogic(boost::bind(&AppLogic::Stop, this))
	, m_appImpl(std::forward<Anything>(a)...)
	{}

	~AppLogic() { Stop(); }

	// App is running. Possible exceptions accumulated to be shown later on.
	void Run()
	{
		try
		{
			m_appImpl.Run();
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
			m_appImpl.Stop();
		}
		catch (...)
		{
			m_exceptioning.Append(boost::current_exception());
		}
	}

protected:
	Exceptioning m_exceptioning;
	TerminationLogic m_terminationLogic;
	Implementation m_appImpl;
};

#define RUN_APP(Type, ...) 									\
	using Type##App = AppLogic< Type##Impl >; 				\
	using Type##App_ptr = boost::scoped_ptr< Type##App >;	\
	Type##App_ptr app(new Type##App( __VA_ARGS__ ));		\
    app->Run();

#endif // __APP_LOGIC_H__