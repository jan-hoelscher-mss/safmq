#pragma once

ref class ServiceLoginUtility
{
protected:
	ServiceLoginUtility(void);

public:
	static bool GrantServiceLogon(String^ login);
};
