#pragma once

#include <streams.h>
#include <initguid.h>

// {A0365714-2DA4-47CF-A50F-406CEB4BE640}
DEFINE_GUID(CLSID_CATFilter,
	0xa0365714, 0x2da4, 0x47cf, 0xa5, 0xf, 0x40, 0x6c, 0xeb, 0x4b, 0xe6, 0x40);


class CATFilter : public CTransInPlaceFilter
{
public:
	// Constructor
	CATFilter();

	// Transform Filter Methods
	HRESULT CheckInputType(const CMediaType * pMediaType) override;
	HRESULT GetMediaType(int iPosition, CMediaType * pMediaType) override;
	HRESULT Transform(IMediaSample * pSample) override;

	// COM Support
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnknown, HRESULT * pHr);

private:
	HRESULT __stdcall TransformSample(WAVEFORMATEX *pWaveFormat, BYTE pBuffer[], long lSize);
};

