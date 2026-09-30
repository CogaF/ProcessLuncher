/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * This file is part of Process Launcher.
 */

#pragma once

/*!
 * \file LicenseHost.h
 * \brief What the shared license code (LicenseManager.cpp, identical in every product that uses
 * this license system) needs from the application it runs in:
 *  - AppInfo::kName                          - names the registry key of the trial state;
 *  - DataDir::file(name), DataDir::exeDirectory() - where license.lic and the trial state live;
 *  - Log::info(text), Log::warning(text)     - the application log;
 *  - GetAppVersion(), kAppVersionHistory     - written in license requests / checked against
 *                                              "updates until".
 * In Process Launcher they all exist already.
 */
#include "AppInfo.h"
#include "BuildInfo.h"
#include "DataDir.h"
#include "Log.h"
#include "Version.h"
