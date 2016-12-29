#include "CATFilter.h"

CATFilter::CATFilter()
	: CTransInPlaceFilter(NAME("Audio Transform Filter"), NULL, CLSID_CATFilter, NULL)
{
}

// Checks if input media type is PCM audio.
HRESULT CATFilter::CheckInputType(const CMediaType * pMediaType)
{
	// Check AM_MEDIA_TYPE is valid PCM audio
	if (pMediaType->majortype != MEDIATYPE_Audio ||
		pMediaType->subtype != MEDIASUBTYPE_PCM ||
		pMediaType->formattype != FORMAT_WaveFormatEx ||
		pMediaType->cbFormat < sizeof(WAVEFORMATEX))
		return VFW_E_TYPE_NOT_ACCEPTED;

	WAVEFORMATEX *pWaveFormat = reinterpret_cast<WAVEFORMATEX*>(pMediaType->pbFormat);

	if (pWaveFormat->wFormatTag != WAVE_FORMAT_PCM)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

// Get input type for position 0
HRESULT CATFilter::GetMediaType(int iPosition, CMediaType * pMediaType)
{
	ASSERT(m_pInput->IsConnected());

	if (iPosition < 0)
		return E_INVALIDARG;

	if (iPosition == 0) {
		HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	return VFW_S_NO_MORE_ITEMS;
}

// Does nothing ATM
HRESULT CATFilter::Transform(IMediaSample * pSample)
{
	BYTE *pBuffer;
	HRESULT hr = pSample->GetPointer(&pBuffer);
	if (FAILED(hr)) return hr;

	AM_MEDIA_TYPE pMediaType;
	hr = m_pInput->ConnectionMediaType(&pMediaType);
	WAVEFORMATEX *pWaveFormat = reinterpret_cast<WAVEFORMATEX*>(pMediaType.pbFormat);

	TransformSample(pWaveFormat, pBuffer, pSample->GetSize());

	return S_OK;
}

// Create the instance for COM
CUnknown * CATFilter::CreateInstance(LPUNKNOWN pUnknown, HRESULT * pHr)
{
	CATFilter *pFilter = new CATFilter();
	if (pFilter == NULL)
		*pHr = E_OUTOFMEMORY;

	return pFilter;
}

// Reduce magnitude by half (50% Compression @ 0Db cutoff?)
HRESULT CATFilter::TransformSample(WAVEFORMATEX * pWaveFormat, BYTE pBuffer[], long lSize)
{
	// 8-bit mono
	if (pWaveFormat->nChannels == 1 && pWaveFormat->wBitsPerSample == 8) {
		for (long i = 0; i < lSize; ++i) {
			pBuffer[i] = 0;
		}
	} // 16-bit mono
	else if (pWaveFormat->nChannels == 1 && pWaveFormat->wBitsPerSample == 16) {
		for (long i = 0; i < lSize; i += 2) {
			short sVal = pBuffer[i] + (pBuffer[i + 1] << 8);

			sVal /= 2;

			pBuffer[i] = sVal & 0xFF;
			pBuffer[i + 1] = sVal >> 8;
		}
	} // 8-bit stereo
	else if (pWaveFormat->nChannels == 2 && pWaveFormat->wBitsPerSample == 8) {
		for (long i = 0; i < lSize; i += 2) {
			pBuffer[i] = 0;
			pBuffer[i + 1] = 0;
		}
	} // 16-bit stereo
	else if (pWaveFormat->nChannels == 2 && pWaveFormat->wBitsPerSample == 16) {
		for (long i = 0; i < lSize; i += 4) {
			short sLeft = pBuffer[i] + (pBuffer[i + 1] << 8);
			short sRight = pBuffer[i + 2] + (pBuffer[i + 3] << 8);

			// Reduce volume by half
			sLeft /= 2;
			sRight /= 2;

			pBuffer[i] = sLeft & 0xFF;
			pBuffer[i + 1] = sLeft >> 8;
			pBuffer[i + 2] = sRight & 0xFF;
			pBuffer[i + 3] = sRight >> 8;
		}
	}

	return S_OK;
}
