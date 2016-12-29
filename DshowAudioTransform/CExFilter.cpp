#include "CExFilter.h"
#include "Constants.h"

// Constructor for CExFilter
CExFilter::CExFilter()
	: CTransformFilter(NAME("Example Filter"), NULL, CLSID_CExFilter)
{
}

// Checks if an input media type can be accepted. The filter accepts 8-bit uncompressed RGB video.
// Returns S_OK if media type can be accepted, or VFW_E_TYPE_NOT_ACCETED otherwise.
HRESULT CExFilter::CheckInputType(const CMediaType * mtIn)
{
	// Check video type
	if (mtIn->majortype  != MEDIATYPE_Video   ||		// Check if Major type is Video
		mtIn->subtype    != MEDIASUBTYPE_RGB8 ||		// Check if Subtype is 8-bit RGB
		mtIn->formattype != FORMAT_VideoInfo  ||		// Check if format block is VIDEOINFOHEADER
		mtIn->cbFormat < sizeof(VIDEOINFOHEADER))
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Cast format block
	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mtIn->pbFormat);

	// Verify 8-bit RGB
	if (pVih->bmiHeader.biBitCount    != 8 ||			// Check if format block is 8-bit
		pVih->bmiHeader.biCompression != BI_RGB)		// Check if format block is RGB
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Check if palette entries are compatible with bit depth
	if (pVih->bmiHeader.biClrUsed > PALETTE_ENTRIES(pVih))
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Check if format block is large enough to hold palette entries
	DWORD cbPalette = pVih->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	if (mtIn->cbFormat < sizeof(VIDEOINFOHEADER) + cbPalette)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

HRESULT CExFilter::GetMediaType(int iPosition, CMediaType * pMediaType)
{
	ASSERT(m_pInput->IsConnected());

	if (iPosition < 0)
		return E_INVALIDARG;

	if (iPosition == 0) {
		HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
		if (FAILED(hr)) return hr;

		FOURCCMap fccMap = FCC('MRLE');
		pMediaType->subtype = static_cast<GUID>(fccMap);		// Assign a new subtype GUID
		pMediaType->SetVariableSize();							// Indicate variable-sized samples
		pMediaType->SetTemporalCompression(FALSE);				// Indicate each frame is a key frame

		ASSERT(pMediaType->formattype == FORMAT_VideoInfo);

		VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pMediaType->pbFormat);
		pVih->bmiHeader.biCompression = BI_RLE8;				// Set compression to RLE
		pVih->bmiHeader.biSizeImage = DIBSIZE(pVih->bmiHeader);	// Set the image size
		return S_OK;
	}

	return VFW_S_NO_MORE_ITEMS;
}

HRESULT CExFilter::CheckTransform(const CMediaType * mtIn, const CMediaType * mtOut)
{
	if (mtOut->majortype != MEDIATYPE_Video)
		return VFW_E_TYPE_NOT_ACCEPTED;

	FOURCCMap fccMap = FCC('MRLE');
	if (mtOut->subtype != static_cast<GUID>(fccMap))
		return VFW_E_TYPE_NOT_ACCEPTED;

	if (mtOut->formattype != FORMAT_VideoInfo ||
		mtOut->cbFormat < sizeof(VIDEOINFOHEADER))
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Compare the bitmap info against the input type
	ASSERT(mtIn->formattype == FORMAT_VideoInfo);
	BITMAPINFOHEADER *pBmiOut = HEADER(mtOut->pbFormat);
	BITMAPINFOHEADER *pBmiIn = HEADER(mtIn->pbFormat);

	if (pBmiOut->biPlanes      != 1               ||
		pBmiOut->biBitCount    != 8               ||
		pBmiOut->biCompression != BI_RLE8         ||
		pBmiOut->biWidth       != pBmiIn->biWidth ||
		pBmiOut->biHeight      != pBmiIn->biHeight)
		return VFW_E_TYPE_NOT_ACCEPTED;

	// Compare source and target rectangles
	RECT rcImage;
	SetRect(&rcImage, 0, 0, pBmiIn->biWidth, pBmiIn->biHeight);
	RECT *prcSource = &((VIDEOINFOHEADER*)(mtIn->pbFormat))->rcSource;
	RECT *prcTarget = &((VIDEOINFOHEADER*)(mtOut->pbFormat))->rcTarget;

	if (!IsRectEmpty(prcSource) && !EqualRect(prcSource, &rcImage))
		return VFW_E_INVALIDMEDIATYPE;
	if (!IsRectEmpty(prcTarget) && !EqualRect(prcTarget, &rcImage))
		return VFW_E_INVALIDMEDIATYPE;

	// Check the palette table
	if (pBmiOut->biClrUsed != pBmiIn->biClrUsed)
		return VFW_E_TYPE_NOT_ACCEPTED;

	DWORD cbPalette = pBmiOut->biClrUsed * sizeof(RGBQUAD);
	if (mtOut->cbFormat < sizeof(VIDEOINFOHEADER) + cbPalette)
		return VFW_E_TYPE_NOT_ACCEPTED;

	if (memcmp(pBmiOut + 1, pBmiIn + 1, cbPalette) != 0)
		return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

HRESULT CExFilter::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pProp)
{
	AM_MEDIA_TYPE mediaType;
	HRESULT hr = m_pOutput->ConnectionMediaType(&mediaType);
	if (FAILED(hr)) return hr;

	ASSERT(mediaType.formattype == FORMAT_VideoInfo);
	BITMAPINFOHEADER *pBmi = HEADER(mediaType.pbFormat);
	pProp->cbBuffer = DIBSIZE(*pBmi) * 2;

	if (pProp->cbAlign == 0)
		pProp->cbAlign = 1;
	if (pProp->cBuffers == 0)
		pProp->cBuffers = 1;

	FreeMediaType(mediaType);

	// Set allocator properties
	ALLOCATOR_PROPERTIES actualProp;
	hr = pAlloc->SetProperties(pProp, &actualProp);
	if (FAILED(hr)) return hr;

	if (pProp->cbBuffer > actualProp.cbBuffer)
		return E_FAIL;

	return S_OK;
}

// Example transform filter implementation
HRESULT CExFilter::Transform(IMediaSample * pSource, IMediaSample * pDest)
{
	// Get buffers
	BYTE *pBufferIn, *pBufferOut;
	HRESULT hr = pSource->GetPointer(&pBufferIn);
	if (FAILED(hr)) return hr;
	hr = pDest->GetPointer(&pBufferOut);
	if (FAILED(hr)) return hr;

	// Process the data
	DWORD cbDest = EncodeFrame(pBufferIn, pBufferOut);
	KASSERT((long)cbDest <= pDest->GetSize());

	pDest->SetActualDataLength(cbDest);
	pDest->SetSyncPoint(TRUE);
	return S_OK;
}

CUnknown * CExFilter::CreateInstance(LPUNKNOWN pUnknown, HRESULT * pHr)
{
	CExFilter * pFilter = new CExFilter();

	if (pFilter == NULL)
		*pHr = E_OUTOFMEMORY;
	
	return pFilter;
}

DWORD CExFilter::EncodeFrame(BYTE * pBufferIn, BYTE  * pBufferOut)
{
	//*pBufferOut = *pBufferIn;
	return 0;
}
