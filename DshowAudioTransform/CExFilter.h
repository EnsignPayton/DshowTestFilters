#pragma once

#include <streams.h>
#include <initguid.h>

// {A535D91E-6D8F-4267-9F11-A38E9146614E}
DEFINE_GUID(CLSID_CExFilter,
	0xa535d91e, 0x6d8f, 0x4267, 0x9f, 0x11, 0xa3, 0x8e, 0x91, 0x46, 0x61, 0x4e);

class CExFilter : public CTransformFilter
{
public:
	// Constructor
	CExFilter();

	// Tranform Filter Methods
	HRESULT CheckInputType(const CMediaType * mtIn) override;
	HRESULT GetMediaType(int iPosition, CMediaType * pMediaType) override;
	HRESULT CheckTransform(const CMediaType * mtIn, const CMediaType * mtOut) override;
	HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pProp) override;
	HRESULT Transform(IMediaSample * pSource, IMediaSample * pDest) override;

	// COM Support
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnknown, HRESULT * pHr);
private:
	DWORD EncodeFrame(BYTE * pBufferIn, BYTE * pBufferOut);
};
