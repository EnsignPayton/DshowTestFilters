#include "CWaveSource.h"
#include <cmath>

CWaveSource::CWaveSource(IUnknown * pUnknown, HRESULT * pHr)
	: CSource(NAME("Wave Source Filter"), pUnknown, CLSID_CWaveSource, pHr)
{
	new CWaveSourceStream(pHr, this);
	ASSERT(GetPinCount() == 1);
}

CUnknown * CWaveSource::CreateInstance(IUnknown * pUnknown, HRESULT * pHr)
{
	CWaveSource * pFilter = new CWaveSource(pUnknown, pHr);
	if (pFilter == NULL)
		*pHr = E_OUTOFMEMORY;

	return pFilter;
}

CWaveSourceStream::CWaveSourceStream(HRESULT * pHr, CSource * pSource)
	: CSourceStream(NAME("Wave Source Pin"), pHr, pSource, L"Capture")
{
	// Replace with a property page
	m_nChannels = 2;		// Stereo
	m_nBitDepth = 16;		// 16-bit
	m_nSampleRate = 44100;	// 44.1 kHz
}

CWaveSourceStream::~CWaveSourceStream(void)
{
}

HRESULT CWaveSourceStream::GetMediaType(CMediaType * pMediaType)
{
	pMediaType->SetType(&MEDIATYPE_Audio);
	pMediaType->SetSubtype(&MEDIASUBTYPE_PCM);
	pMediaType->SetFormatType(&FORMAT_WaveFormatEx);

	WAVEFORMATEX * pWaveFormat = reinterpret_cast<WAVEFORMATEX*>
		(pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX)));

	pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	pWaveFormat->nChannels = m_nChannels;
	pWaveFormat->nSamplesPerSec = m_nSampleRate;
	pWaveFormat->wBitsPerSample = m_nBitDepth;
	pWaveFormat->nBlockAlign = (pWaveFormat->nChannels * pWaveFormat->wBitsPerSample) / 8;
	pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign;
	pWaveFormat->cbSize = 0;

	pMediaType->SetSampleSize(pWaveFormat->nBlockAlign);
	pMediaType->SetTemporalCompression(FALSE);

	return S_OK;
}

HRESULT CWaveSourceStream::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pProp)
{
	pProp->cbBuffer = 16 * 1024;//m_nChannels * m_nBitDepth / 8;
	pProp->cBuffers = (m_nChannels * m_nSampleRate * m_nBitDepth) / (pProp->cbBuffer * 8);//m_nSampleRate;

	if (pProp->cbAlign == 0)
		pProp->cbAlign = 1;
	if (pProp->cBuffers == 0)
		pProp->cBuffers = 1;

	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr = pAlloc->SetProperties(pProp, &Actual);
	if (FAILED(hr)) return hr;

	if (pProp->cbBuffer > Actual.cbBuffer)
		return E_FAIL;

	return S_OK;
}

HRESULT CWaveSourceStream::FillBuffer(IMediaSample * pSample)
{
	BYTE* pBuffer;
	pSample->GetPointer(&pBuffer);

	HRESULT hr = WriteWave(pBuffer, pSample->GetSize());
	if (FAILED(hr)) return hr;

	return S_OK;
}

// Quick and dirty
short __stdcall gen_square_wave(short, int, int, int);

HRESULT CWaveSourceStream::WriteWave(BYTE pBuffer[], long lSize)
{
	// 8-bit mono
	if (m_nChannels == 1 && m_nBitDepth == 8) {
		for (long i = 0; i < lSize; ++i) {
			pBuffer[i] = 0;
		}
	} // 16-bit mono
	else if (m_nChannels == 1 && m_nBitDepth == 16) {
		for (long i = 0; i < lSize; i += 2) {
			pBuffer[i] = 0;
			pBuffer[i + 1] = 0;
		}
	} // 8-bit stereo
	else if (m_nChannels == 2 && m_nBitDepth == 8) {
		for (long i = 0; i < lSize; i += 2) {
			pBuffer[i] = 0;
			pBuffer[i + 1] = 0;
		}
	} // 16-bit stereo
	else if (m_nChannels == 2 && m_nBitDepth == 16) {
		for (long i = 0; i < lSize; i += 4) {
			short test = gen_square_wave(100, 440, lSize / 4, i / 4);

			pBuffer[i] = test & 0xFF;
			pBuffer[i + 1] = test >> 8;
			pBuffer[i + 2] = test & 0xFF;
			pBuffer[i + 3] = test >> 8;
		}
	}

	return S_OK;
}

short __stdcall gen_square_wave(short amplitude, int frequency, int max, int cur)
{
	double period = (double)max / (double)frequency;
	double posInWave = fmod(cur, period) / period;
	return posInWave < 0.5 ? -1 * amplitude : amplitude;
}
