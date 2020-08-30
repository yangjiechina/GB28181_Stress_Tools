
// GB28181_Stress_Tools.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CGB28181StressToolsApp:
// See GB28181_Stress_Tools.cpp for the implementation of this class
//

class CGB28181StressToolsApp : public CWinApp
{
public:
	CGB28181StressToolsApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGB28181StressToolsApp theApp;
