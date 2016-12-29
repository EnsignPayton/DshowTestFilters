#include "COMSupport.h"
#include "Constants.h"
#include "CExFilter.h"
#include "CATFilter.h"

// Declare media type information
FOURCCMap fccMap = FCC('MRLE');
REGPINTYPES sudInputTypes = { &MEDIATYPE_Video, &GUID_NULL };
REGPINTYPES sudOutputTypes = { &MEDIATYPE_Video, (GUID*)&fccMap };

REGPINTYPES catInputTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM };
REGPINTYPES catOutputTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM };

// Declare pin information
REGFILTERPINS sudPinReg[] = {
	// Input pin
	{ 0, FALSE,	// Rendered?
	FALSE,	// Output?
	FALSE,	// Zero?
	FALSE,	// Many?
	0, 0,
	1, &sudInputTypes
	},
	// Output pin
	{ 0, FALSE,	// Rendered?
	TRUE,	// Output?
	FALSE,	// Zero?
	FALSE,	// Many?
	0, 0,
	1, &sudOutputTypes
	}
};

REGFILTERPINS catPinReg[] = {
	{
		NULL,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		NULL,
		NULL,
		1,
		&catInputTypes
	},
	{
		NULL,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		NULL,
		NULL,
		1,
		&catOutputTypes
	}
};

REGFILTER2 rf2FilterReg = {
	1,					// Version number
	MERIT_DO_NOT_USE,	// Merit
	2,					// Number of pins
	sudPinReg			// Pointer to pin info
};

REGFILTER2 catFilterReg = {
	1,
	MERIT_DO_NOT_USE,
	2,
	catPinReg
};

static WCHAR g_wszName[] = L"Example Filter";
static WCHAR g_wszName2[] = L"Audio Transform Filter";
CFactoryTemplate g_Templates[] =
{
	{
		g_wszName,
		&CLSID_CExFilter,
		CExFilter::CreateInstance,
		NULL,
		NULL
	},
	{
		g_wszName2,
		&CLSID_CATFilter,
		CATFilter::CreateInstance,
		NULL,
		NULL
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

//
// DEF EXPORTS
//

EXTERN_C BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

STDAPI DllRegisterServer(void)
{
	HRESULT hr = AMovieDllRegisterServer2(TRUE);
	if (FAILED(hr)) return hr;

	// Register Filters with details
	IFilterMapper2 *pFM2 = NULL;
	hr = CoCreateInstance(CLSID_FilterMapper2, NULL,
		CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFM2);
	if (SUCCEEDED(hr)) {
		hr = pFM2->RegisterFilter(
			CLSID_CExFilter,				// Filter category
			g_wszName,						// Fiter name
			NULL,							// Device moniker
			&CLSID_VideoCompressorCategory,	// Video compressor category
			g_wszName,						// Instance data
			&rf2FilterReg					// Filter information
			);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		hr = pFM2->RegisterFilter(
			CLSID_CATFilter,
			g_wszName2,
			NULL,
			&CLSID_AudioCompressorCategory,
			g_wszName2,
			&catFilterReg
			);
		pFM2->Release();
	}

	return hr;
}

STDAPI DllUnregisterServer()
{
	HRESULT hr = AMovieDllRegisterServer2(FALSE);
	if (FAILED(hr)) return hr;

	IFilterMapper2 *pFM2 = NULL;
	hr = CoCreateInstance(CLSID_FilterMapper2, NULL,
		CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFM2);
	if (SUCCEEDED(hr)) {
		hr = pFM2->UnregisterFilter(&CLSID_VideoCompressorCategory,
			g_wszName, CLSID_CExFilter);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		hr = pFM2->UnregisterFilter(&CLSID_AudioCompressorCategory,
			g_wszName2, CLSID_CATFilter);
		pFM2->Release();
	}
	return hr;
}
