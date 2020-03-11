#if !defined(__EXCEPTIONING_H__)
#define __EXCEPTIOING_H__

#include "CommonDefinitions.h"

class Exceptioning final
{
public:
	bool Occurred() const;
	void Append(const boost::exception_ptr& entry);
	void Show();

private:
	boost::mutex m_mutex;
	std::vector<boost::exception_ptr> m_exceptions;
};

#endif // __EXCEPTIONING_H__