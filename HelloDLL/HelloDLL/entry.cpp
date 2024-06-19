#include <Windows.h>

auto DllMain( HINSTANCE m_hInstance, DWORD m_dwReasone, LPVOID ) -> BOOL WINAPI {
	if( m_dwReasone == 1 ) {
		MessageBoxA( nullptr, "Hello", "Status", MB_OK );
	}

	return TRUE;
}

extern "C" int _fltused = 0;
extern "C" auto _DllMainCRTStartup( HINSTANCE m_hInstance, DWORD m_dwReason, LPVOID m_lpReserverd ) -> BOOL WINAPI {
	return DllMain( m_hInstance, m_dwReason, m_lpReserverd );
}