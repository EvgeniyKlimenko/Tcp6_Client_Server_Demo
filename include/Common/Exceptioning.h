#if !defined(__EXCEPTIONING_H__)
#define __EXCEPTIOING_H__

#include "CommonDefintions.h"

class CExceptioning final
{
public:
	bool Occurred() const;
	void Append(const std::exception_ptr& entry);
	void Show();

private:
	std::mutex m_mutex;
	std::vector<std::exception_ptr> m_exceptions;
};

#endif // __EXCEPTIONING_H__