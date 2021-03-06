
// SmClient.h : main header file for the SmClient application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CSmClientApp:
// See SmClient.cpp for the implementation of this class
//

class CSmClientApp : public CWinAppEx
{
public:
	CSmClientApp() noexcept;


// Overrides
public:
	int RegisterOCX(CString strFileName);
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSmClientApp theApp;
