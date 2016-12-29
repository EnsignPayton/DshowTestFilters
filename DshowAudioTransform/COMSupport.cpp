#include "COMSupport.h"
#include "Constants.h"
#include "CExFilter.h"
#include "CATFilter.h"
#include "CWaveSource.h"

FOURCCMap fccMap = FCC('MRLE');
REGPINTYPES cexInputTypes = { &MEDIATYPE_Video, &GUID_NULL };
REGPINTYPES cexOutputTypes = { &MEDIATYPE_Video, (GUID*)&fccMap };

REGPINTYPES catInputTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM };
REGPINTYPES catOutputTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM };

REGPINTYPES cwsCaptureTypes = { &MEDIATYPE_Audio, &MEDIASUBTYPE_PCM };

REGFILTERPINS cexPinReg[] = {
	{ NULL, FALSE, FALSE, FALSE, FALSE, NULL, NULL, 1, &cexInputTypes },
	{ NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, 1, &cexOutputTypes }
};

REGFILTERPINS catPinReg[] = {
	{ NULL, FALSE, FALSE, FALSE, FALSE, NULL, NULL, 1, &catInputTypes },
	{ NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, 1, &catOutputTypes }
};

REGFILTERPINS cwsPinReg[] = {
	{ NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, 1, &cwsCaptureTypes }
};

REGFILTER2 cexFilterReg = { 1, MERIT_DO_NOT_USE, 2, cexPinReg };

REGFILTER2 catFilterReg = { 1, MERIT_DO_NOT_USE, 2, catPinReg };

REGFILTER2 cwsFilterReg = { 1, MERIT_DO_NOT_USE, 1, cwsPinReg };

CFactoryTemplate g_Templates[] = {
	{ NAME_CExFilter, &CLSID_CExFilter, CExFilter::CreateInstance, NULL, NULL },
	{ NAME_CATFilter, &CLSID_CATFilter, CATFilter::CreateInstance, NULL, NULL },
	{ NAME_CWaveSource, &CLSID_CWaveSource, CWaveSource::CreateInstance, NULL, NULL }
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

	// Register filters with details
	IFilterMapper2 *pFM2 = NULL;
	hr = CoCreateInstance(CLSID_FilterMapper2, NULL,
		CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void **)&pFM2);

	if (SUCCEEDED(hr)) {
		hr = pFM2->RegisterFilter(
			CLSID_CExFilter,
			NAME_CExFilter,
			NULL,
			&CLSID_VideoCompressorCategory,
			NAME_CExFilter,
			&cexFilterReg
			);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		hr = pFM2->RegisterFilter(
			CLSID_CATFilter,
			NAME_CATFilter,
			NULL,
			&CLSID_AudioCompressorCategory,
			NAME_CATFilter,
			&catFilterReg
			);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		hr = pFM2->RegisterFilter(
			CLSID_CWaveSource,
			NAME_CWaveSource,
			NULL,
			&CLSID_AudioInputDeviceCategory,
			NAME_CWaveSource,
			&cwsFilterReg
			);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

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
			NAME_CExFilter, CLSID_CExFilter);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		hr = pFM2->UnregisterFilter(&CLSID_AudioCompressorCategory,
			NAME_CATFilter, CLSID_CATFilter);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		hr = pFM2->UnregisterFilter(&CLSID_AudioInputDeviceCategory,
			NAME_CWaveSource, CLSID_CWaveSource);
		if (FAILED(hr)) {
			pFM2->Release();
			return hr;
		}

		pFM2->Release();
	}

	return hr;
}
