#pragma once

#include <streams.h>
#include <initguid.h>

// {8C9010C3-57EE-41E9-A9B9-2640313EC2B4}
DEFINE_GUID(CLSID_CWaveSource,
	0x8c9010c3, 0x57ee, 0x41e9, 0xa9, 0xb9, 0x26, 0x40, 0x31, 0x3e, 0xc2, 0xb4);

class CWaveSource : public CSource
{
public:
	// Constructor
	CWaveSource(IUnknown * pUnknown, HRESULT * pHr);


	static CUnknown * WINAPI CreateInstance(IUnknown * pUnknown, HRESULT * pHr);
};

class CWaveSourceStream : public CSourceStream
{
public:
	// Constructor
	CWaveSourceStream(HRESULT * pHr, CSource * pSource);
	~CWaveSourceStream(void);

	HRESULT GetMediaType(CMediaType * pMediaType) override;
	HRESULT DecideBufferSize(IMemAllocator * pAlloc,
		ALLOCATOR_PROPERTIES * ppropInputRequest) override;
	HRESULT FillBuffer(IMediaSample * pSample) override;

private:
	WORD m_nChannels;
	WORD m_nBitDepth;
	DWORD m_nSampleRate;

	HRESULT __stdcall WriteWave(BYTE pBuffer[], long lSize);
};