/*!
 * \file BuildInfo.h
 * \brief Version and build identification shown in the window title, About box and license requests.
 */
#pragma once

#include <string>

/*! \brief "MAJOR.MINOR.PATCH[-PRERELEASE]", e.g. "0.2.0-rc.1". */
std::string GetAppVersion();
/*! \brief Build date and time of BuildInfo.cpp, "Mon DD YYYY hh:mm:ss". */
std::string GetBuildDate();
/*! \brief "v0.2.0-rc.1" (+ "D" for Debug builds). */
std::string GetCompactVersion();
/*! \brief Window title: "Parallel Command Runner - PCR (v0.2.0-rc.1)". */
std::string GetWindowTitle();
