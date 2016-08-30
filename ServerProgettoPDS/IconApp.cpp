#include "stdafx.h"
#include <afx.h>
#include <afxwin.h>
#include <atlbase.h>
struct ICONDIRENTRY
{
    UCHAR nWidth;
    UCHAR nHeight;
    UCHAR nNumColorsInPalette; // 0 if no palette
    UCHAR nReserved; // should be 0
    WORD nNumColorPlanes; // 0 or 1
    WORD nBitsPerPixel;
    ULONG nDataLength; // length in bytes
    ULONG nOffset; // offset of BMP or PNG data from beginning of file
};

// Helper class to release GDI object handle when scope ends:
class CGdiHandle
{
public:
    CGdiHandle(HGDIOBJ handle) : m_handle(handle) {};
    ~CGdiHandle() { DeleteObject(m_handle); };
private:
    HGDIOBJ m_handle;
};
// Save icon referenced by handle 'hIcon' as file with name 'szPath'.
// The generated ICO file has the color depth specified in 'nColorBits'.
//
bool SaveIcon(HICON hIcon, DWORD& szSize ,int nColorBits, const TCHAR* szPath)
{
    ASSERT(nColorBits == 4 || nColorBits == 8 || nColorBits == 24 || nColorBits == 32);

    if (offsetof(ICONDIRENTRY, nOffset) != 12)
    {
       return false;
    }

    CDC dc;
    dc.Attach(::GetDC(NULL)); // ensure that DC is released when function ends
    // Open file for writing:
    CFile file;
    if (!file.Open(szPath, CFile::modeWrite | CFile::modeCreate))
    {
        return false;
    }
    // Write header:
    UCHAR icoHeader[6] = { 0, 0, 1, 0, 1, 0 }; // ICO file with 1 image
    file.Write(icoHeader, sizeof(icoHeader));
    // Get information about icon:
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);
    CGdiHandle handle1(iconInfo.hbmColor), handle2(iconInfo.hbmMask); // free bitmaps when function ends
    BITMAPINFO bmInfo = { 0 };
    bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo.bmiHeader.biBitCount = 0;    // don't get the color table     
    if (!GetDIBits(dc, iconInfo.hbmColor, 0, 0, NULL, &bmInfo, DIB_RGB_COLORS))
    {
        return false;
    }
    // Allocate size of bitmap info header plus space for color table:
    int nBmInfoSize = sizeof(BITMAPINFOHEADER);
    if (nColorBits < 24)
    {
        nBmInfoSize += sizeof(RGBQUAD) * (int)(1 << nColorBits);
    }
    CAutoVectorPtr<UCHAR> bitmapInfo;
    bitmapInfo.Allocate(nBmInfoSize);
    BITMAPINFO* pBmInfo = (BITMAPINFO*)(UCHAR*)bitmapInfo;
    memcpy(pBmInfo, &bmInfo, sizeof(BITMAPINFOHEADER));

    // Get bitmap data:
    ASSERT(bmInfo.bmiHeader.biSizeImage != 0);
    CAutoVectorPtr<UCHAR> bits;
    bits.Allocate(bmInfo.bmiHeader.biSizeImage);
    pBmInfo->bmiHeader.biBitCount = nColorBits;
    pBmInfo->bmiHeader.biCompression = BI_RGB;
    if (!GetDIBits(dc, iconInfo.hbmColor, 0, bmInfo.bmiHeader.biHeight, (UCHAR*)bits, pBmInfo, DIB_RGB_COLORS))
    {
         return false;
    }
    // Get mask data:
    BITMAPINFO maskInfo = { 0 };
    maskInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    maskInfo.bmiHeader.biBitCount = 0;  // don't get the color table     
    if (!GetDIBits(dc, iconInfo.hbmMask, 0, 0, NULL, &maskInfo, DIB_RGB_COLORS))
    {
        return false;
    }
    ASSERT(maskInfo.bmiHeader.biBitCount == 1);
    CAutoVectorPtr<UCHAR> maskBits;
    maskBits.Allocate(maskInfo.bmiHeader.biSizeImage);
    CAutoVectorPtr<UCHAR> maskInfoBytes;
    maskInfoBytes.Allocate(sizeof(BITMAPINFO) + 2 * sizeof(RGBQUAD));
    BITMAPINFO* pMaskInfo = (BITMAPINFO*)(UCHAR*)maskInfoBytes;
    memcpy(pMaskInfo, &maskInfo, sizeof(maskInfo));
    if (!GetDIBits(dc, iconInfo.hbmMask, 0, maskInfo.bmiHeader.biHeight, (UCHAR*)maskBits, pMaskInfo, DIB_RGB_COLORS))
    {
        return false;
    }
    // Write directory entry:
    ICONDIRENTRY dir;
    dir.nWidth = (UCHAR)pBmInfo->bmiHeader.biWidth;
    dir.nHeight = (UCHAR)pBmInfo->bmiHeader.biHeight;
    dir.nNumColorsInPalette = (nColorBits == 4 ? 16 : 0);
    dir.nReserved = 0;
    dir.nNumColorPlanes = 0;
    dir.nBitsPerPixel = pBmInfo->bmiHeader.biBitCount;
    dir.nDataLength = pBmInfo->bmiHeader.biSizeImage + pMaskInfo->bmiHeader.biSizeImage + nBmInfoSize;
    dir.nOffset = sizeof(dir) + sizeof(icoHeader);
    file.Write(&dir, sizeof(dir));
    // Write DIB header (including color table):
    int nBitsSize = pBmInfo->bmiHeader.biSizeImage;
    pBmInfo->bmiHeader.biHeight *= 2; // because the header is for both image and mask
    pBmInfo->bmiHeader.biCompression = 0;
    pBmInfo->bmiHeader.biSizeImage += pMaskInfo->bmiHeader.biSizeImage; // because the header is for both image and mask
    file.Write(&pBmInfo->bmiHeader, nBmInfoSize);
    // Write image data:
    file.Write((UCHAR*)bits, nBitsSize);

    // Write mask data:
    file.Write((UCHAR*)maskBits, pMaskInfo->bmiHeader.biSizeImage);
	szSize = file.GetLength();
    file.Close();
    return true;
}
/*
// Test program for SaveIcon() function.
//
// Usage: first argument is input ICO file (must be 32x32 pixels); second argument is output ICO file
//
int _tmain(int argc, _TCHAR* argv[])
144 {
	145     ASSERT(argc == 3);
	146
		147     // Load a 32x32 icon:
		148     HICON hIcon = (HICON)LoadImage(0, argv[1], IMAGE_ICON, 32, 32, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	149     ASSERT(hIcon != NULL);
	150
		151     // Save with 24-bits colors:
		152     if (!SaveIcon(hIcon, 24, argv[2]))
		153     {
		154         _ftprintf(stderr, _T("Error: saving icon to %s failed"), argv[2]);
		155         return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
 }
 */