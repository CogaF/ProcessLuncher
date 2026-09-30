/*!
 * \file BuildInfo.cpp
 * \brief Implementation of BuildInfo.h.
 */

#include "BuildInfo.h"

#include <format>

#include "AppInfo.h"
#include "Version.h"

std::string GetAppVersion()
{
	std::string v = std::format("{}.{}.{}", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
	const std::string pre = APP_VERSION_PRERELEASE;
	if (!pre.empty()) v += "-" + pre;
	return v;
}

std::string GetBuildDate()
{
	return std::string(__DATE__) + " " + __TIME__;
}

std::string GetCompactVersion()
{
	std::string s = "v" + GetAppVersion();
#ifdef _DEBUG
	s += "D";
#endif
	return s;
}

std::string GetWindowTitle()
{
	return std::format("Parallel Command Runner - {} ({})", AppInfo::kShortName, GetCompactVersion());
}
