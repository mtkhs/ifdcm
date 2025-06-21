#include <windows.h>
#include <string>
#include <memory>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>

// #include <ifdcm.h>
#include <susie.h>

extern "C" int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	if ( infono && (buflen < 64) ) infono = -1;
	switch (infono){
		case 0:
			strcpy(buf, "00IN");
			break;
		case 1:
			strcpy(buf, "DICOM Plug-in Version 0.1 by mtkhs");
			break;
		case 2:
			strcpy(buf, "*.dcm");
			break;
		case 3:
			strcpy(buf, "DICOM image format (*.dcm)");
			break;
		default:
			buf[0] = '\0';
			break;
	}
	return static_cast<int>(strlen(buf));
}

extern "C" int __stdcall GetPluginInfoW(int infono, LPWSTR buf, int buflen)
{
	char bufA[0x400];

	if ( infono && (buflen < 64) ){
		buf[0] = '\0';
		return 0;
	}
	GetPluginInfo(infono, bufA, 0x400);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, bufA, -1, buf, buflen);
	buf[buflen - 1] = '\0';
	return static_cast<int>(wcslen(buf));
}

// 共通処理関数 for IsSupported
static int SusieIsSupportedFromFile(const char* filename)
{
    if (!filename) return 0;
    std::string fname(filename);
    std::string ext = fname.substr(fname.find_last_of(".") + 1);
    if (_stricmp(ext.c_str(), "dcm") == 0) {
        return 1;
    }
    return 0;
}

extern "C" int __stdcall IsSupported(LPCSTR filename, const void *dw)
{
    return SusieIsSupportedFromFile(filename);
}

extern "C" int __stdcall IsSupportedW(LPCWSTR filename, const void *dw)
{
    if (!filename) return 0;
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
    std::string utf8_filename(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, filename, -1, &utf8_filename[0], utf8_len, NULL, NULL);
    return SusieIsSupportedFromFile(utf8_filename.c_str());
}

// 共通処理関数 for GetPictureInfo
static int SusieGetPictureInfoFromFile(const char* filename, struct PictureInfo *lpInfo)
{
    if (!filename || !lpInfo) {
        return SUSIEERROR_UNKNOWNFORMAT;
    }
    try {
        // OFLog::configure(OFLogger::WARN_LOG_LEVEL);
        DicomImage* dcmImage = new DicomImage(filename);
        if (dcmImage == NULL || dcmImage->getStatus() != EIS_Normal) {
            delete dcmImage;
            return SUSIEERROR_NOTSUPPORT;
        }
        unsigned long width = dcmImage->getWidth();
        unsigned long height = dcmImage->getHeight();
        unsigned long depth = dcmImage->getDepth();
        lpInfo->left = 0;
        lpInfo->top = 0;
        lpInfo->width = width;
        lpInfo->height = height;
        lpInfo->x_density = 0;
        lpInfo->y_density = 0;
        lpInfo->colorDepth = 24;
        lpInfo->hInfo = NULL;
        delete dcmImage;
        return 0;
    } catch (...) {
        return SUSIEERROR_UNKNOWNFORMAT;
    }
}

extern "C" int __stdcall GetPictureInfo(LPCSTR buf, LONG_PTR len, unsigned int flag, struct PictureInfo *lpInfo)
{
    return SusieGetPictureInfoFromFile(buf, lpInfo);
}

extern "C" int __stdcall GetPictureInfoW(LPCWSTR buf, LONG_PTR len, unsigned int flag, struct PictureInfo *lpInfo)
{
    if (!buf) return SUSIEERROR_UNKNOWNFORMAT;
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
    std::string utf8_filename(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8_filename[0], utf8_len, NULL, NULL);
    return SusieGetPictureInfoFromFile(utf8_filename.c_str(), lpInfo);
}

// 共通処理関数
static int SusieGetPictureFromFile(const char* filename, HLOCAL *pHBInfo, HLOCAL *pHBm)
{
    if (!filename || !pHBInfo || !pHBm) {
        return SUSIEERROR_UNKNOWNFORMAT;
    }
    try {
        // Initialize DCMTK
        // OFLog::configure(OFLogger::WARN_LOG_LEVEL);

        // Load DICOM image
        DicomImage* dcmImage = new DicomImage(filename);
        if (dcmImage == NULL || dcmImage->getStatus() != EIS_Normal) {
            delete dcmImage;
            return SUSIEERROR_NOTSUPPORT;
        }

        // Get image dimensions
        unsigned long width = dcmImage->getWidth();
        unsigned long height = dcmImage->getHeight();
        unsigned long depth = dcmImage->getDepth();

        // Convert to 8-bit grayscale if needed
        if (depth > 8) {
            dcmImage->setMinMaxWindow();
            dcmImage->setPolarity(EPP_Normal);
        }

        // Allocate memory for bitmap info header
        HLOCAL hBmpInfo = LocalAlloc(LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER));
        if (!hBmpInfo) {
            delete dcmImage;
            return SUSIEERROR_FAULTMEMORY;
        }
        BITMAPINFOHEADER* pBmpInfo = (BITMAPINFOHEADER*)LocalLock(hBmpInfo);
        ZeroMemory(pBmpInfo, sizeof(BITMAPINFOHEADER));

        // Setup bitmap info header
        pBmpInfo->biSize = sizeof(BITMAPINFOHEADER);
        pBmpInfo->biWidth = width;
        pBmpInfo->biHeight = (LONG)height; // Bottom-up bitmap
        pBmpInfo->biPlanes = 1;
        pBmpInfo->biBitCount = 24; // RGB
        pBmpInfo->biCompression = BI_RGB;
        int lineBytes = ((width * 3 + 3) & ~3); // 4バイト境界に揃える
        pBmpInfo->biSizeImage = lineBytes * height;
        pBmpInfo->biXPelsPerMeter = 0;
        pBmpInfo->biYPelsPerMeter = 0;
        pBmpInfo->biClrUsed = 0;
        pBmpInfo->biClrImportant = 0;

        LocalUnlock(hBmpInfo);

        // Allocate memory for bitmap data
        HLOCAL hBmpData = LocalAlloc(LMEM_MOVEABLE, pBmpInfo->biSizeImage);
        if (!hBmpData) {
            LocalFree(hBmpInfo);
            delete dcmImage;
            return SUSIEERROR_FAULTMEMORY;
        }
        unsigned char* pBmpData = (unsigned char*)LocalLock(hBmpData);

        // Get image data
        const void* imageData = dcmImage->getOutputData(8, 0, 0);
        if (!imageData) {
            LocalFree(hBmpInfo);
            LocalFree(hBmpData);
            delete dcmImage;
            return SUSIEERROR_UNKNOWNFORMAT;
        }

        // Convert to RGB
        for (unsigned long y = 0; y < height; y++) {
            for (unsigned long x = 0; x < width; x++) {
                unsigned char pixel = ((const unsigned char*)imageData)[y * width + x];
                int offset = (height - 1 - y) * lineBytes + x * 3;
                pBmpData[offset + 0] = pixel; // B
                pBmpData[offset + 1] = pixel; // G
                pBmpData[offset + 2] = pixel; // R
            }
        }

        LocalUnlock(hBmpData);

        // ハンドルを返す
        *pHBInfo = hBmpInfo;
        *pHBm = hBmpData;

        delete dcmImage;
        return 0; // Success
    }
    catch (...) {
        if (*pHBInfo) LocalFree(*pHBInfo);
        if (*pHBm) LocalFree(*pHBm);
        *pHBInfo = NULL;
        *pHBm = NULL;
        return SUSIEERROR_UNKNOWNFORMAT;
    }
}

extern "C" int __stdcall GetPicture(LPCSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, FARPROC lpPrgressCallback, LONG_PTR lData)
{
    return SusieGetPictureFromFile(buf, pHBInfo, pHBm);
}

extern "C" int __stdcall GetPictureW(LPCWSTR filename, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, FARPROC lpPrgressCallback, LONG_PTR lData)
{
    if (!filename) return SUSIEERROR_UNKNOWNFORMAT;
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
    std::string utf8_filename(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, filename, -1, &utf8_filename[0], utf8_len, NULL, NULL);
    return SusieGetPictureFromFile(utf8_filename.c_str(), pHBInfo, pHBm);
}

// 共通処理関数 for GetPreview
static int SusieGetPreviewFromFile(const char* filename, HLOCAL *pHBInfo, HLOCAL *pHBm)
{
    if (!filename || !pHBInfo || !pHBm) {
        return SUSIEERROR_UNKNOWNFORMAT;
    }
    try {
        DicomImage* dcmImage = new DicomImage(filename);
        if (dcmImage == NULL || dcmImage->getStatus() != EIS_Normal) {
            delete dcmImage;
            return SUSIEERROR_NOTSUPPORT;
        }

        unsigned long orig_width = dcmImage->getWidth();
        unsigned long orig_height = dcmImage->getHeight();
        unsigned long new_width, new_height;
        const unsigned long max_dim = 256;
        if (orig_width > orig_height) {
            new_width = max_dim;
            new_height = (unsigned long)((double)orig_height * ((double)max_dim / orig_width));
        } else {
            new_height = max_dim;
            new_width = (unsigned long)((double)orig_width * ((double)max_dim / orig_height));
        }

        if (new_width == 0) new_width = 1;
        if (new_height == 0) new_height = 1;

        DicomImage* scaledImage = dcmImage->createScaledImage(new_width, new_height);
        delete dcmImage;

        if (scaledImage == NULL || scaledImage->getStatus() != EIS_Normal) {
            delete scaledImage;
            return SUSIEERROR_NOTSUPPORT;
        }

        unsigned long width = scaledImage->getWidth();
        unsigned long height = scaledImage->getHeight();
        unsigned long depth = scaledImage->getDepth();
        if (depth > 8) {
            scaledImage->setMinMaxWindow();
            scaledImage->setPolarity(EPP_Normal);
        }

        HLOCAL hBmpInfo = LocalAlloc(LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER));
        if (!hBmpInfo) {
            delete scaledImage;
            return SUSIEERROR_FAULTMEMORY;
        }

        BITMAPINFOHEADER* pBmpInfo = (BITMAPINFOHEADER*)LocalLock(hBmpInfo);
        ZeroMemory(pBmpInfo, sizeof(BITMAPINFOHEADER));
        pBmpInfo->biSize = sizeof(BITMAPINFOHEADER);
        pBmpInfo->biWidth = width;
        pBmpInfo->biHeight = (LONG)height;
        pBmpInfo->biPlanes = 1;
        pBmpInfo->biBitCount = 24;
        pBmpInfo->biCompression = BI_RGB;
        int lineBytes = ((width * 3 + 3) & ~3);
        pBmpInfo->biSizeImage = lineBytes * height;
        pBmpInfo->biXPelsPerMeter = 0;
        pBmpInfo->biYPelsPerMeter = 0;
        pBmpInfo->biClrUsed = 0;
        pBmpInfo->biClrImportant = 0;

        LocalUnlock(hBmpInfo);

        HLOCAL hBmpData = LocalAlloc(LMEM_MOVEABLE, pBmpInfo->biSizeImage);
        if (!hBmpData) {
            LocalFree(hBmpInfo);
            delete scaledImage;
            return SUSIEERROR_FAULTMEMORY;
        }
        unsigned char* pBmpData = (unsigned char*)LocalLock(hBmpData);

        const void* imageData = scaledImage->getOutputData(8, 0, 0);
        if (!imageData) {
            LocalFree(hBmpInfo);
            LocalFree(hBmpData);
            delete scaledImage;
            return SUSIEERROR_UNKNOWNFORMAT;
        }

        for (unsigned long y = 0; y < height; y++) {
            for (unsigned long x = 0; x < width; x++) {
                unsigned char pixel = ((const unsigned char*)imageData)[y * width + x];
                int offset = (height - 1 - y) * lineBytes + x * 3;
                pBmpData[offset + 0] = pixel;
                pBmpData[offset + 1] = pixel;
                pBmpData[offset + 2] = pixel;
            }
        }

        LocalUnlock(hBmpData);

        *pHBInfo = hBmpInfo;
        *pHBm = hBmpData;

        delete scaledImage;
        return 0;
    } catch (...) {
        if (*pHBInfo) LocalFree(*pHBInfo);
        if (*pHBm) LocalFree(*pHBm);
        *pHBInfo = NULL;
        *pHBm = NULL;
        return SUSIEERROR_UNKNOWNFORMAT;
    }
}

extern "C" int __stdcall GetPreview(LPCSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, FARPROC lpPrgressCallback, LONG_PTR lData)
{
    return SusieGetPreviewFromFile(buf, pHBInfo, pHBm);
}

extern "C" int __stdcall GetPreviewW(LPCWSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, FARPROC lpPrgressCallback, LONG_PTR lData)
{
    if (!buf) return SUSIEERROR_UNKNOWNFORMAT;
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
    std::string utf8_filename(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8_filename[0], utf8_len, NULL, NULL);
    return SusieGetPreviewFromFile(utf8_filename.c_str(), pHBInfo, pHBm);
}

