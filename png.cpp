/*
 * Copyright (c) 2009, The Mozilla Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Mozilla Foundation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY The Mozilla Foundation ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL The Mozilla Foundation BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   Ted Mielczarek <ted.mielczarek@gmail.com>
 */
/*
 * screenshot.cpp : Save a screenshot of the Windows desktop in .png format.
 *  If a filename is specified as the first argument on the commandline,
 *  then the image will be saved to that filename. Otherwise, the image will
 *  be saved as "screenshot.png" in the current working directory.
 *
 *  Requires GDI+. All linker dependencies are specified explicitly in this
 *  file, so you can compile screenshot.exe by simply running:
 *    cl screenshot.cpp
 */

#include <windows.h>
#include "gdi/gdiplus.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// From http://msdn.microsoft.com/en-us/library/ms533843%28VS.85%29.aspx
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;          // number of image encoders
  UINT  size = 0;         // size of the image encoder array in bytes

  ImageCodecInfo* pImageCodecInfo = NULL;

  GetImageEncodersSize(&num, &size);
  if(size == 0)
    return -1;  // Failure

  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if(pImageCodecInfo == NULL)
    return -1;  // Failure

  GetImageEncoders(num, size, pImageCodecInfo);

  for(UINT j = 0; j < num; ++j)
    {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
        {
          *pClsid = pImageCodecInfo[j].Clsid;
          free(pImageCodecInfo);
          return j;  // Success
        }    
    }

  free(pImageCodecInfo);
  return -1;  // Failure
}

bool save_png_file(HBITMAP hBmp, HDC hDC, const char *filename)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    Bitmap* b = Bitmap::FromHBITMAP(hBmp, NULL);
    
    CLSID  encoderClsid;
    Status stat = GenericError;
    if (b && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
        int len = strlen(filename);
        WCHAR* wfilename = new WCHAR[len+1];
        int ret = MultiByteToWideChar(CP_ACP, 0,
                    filename, len+1, wfilename, len+1);
        stat = b->Save(wfilename, &encoderClsid, NULL);
        delete [] wfilename;
    }
    if (b)
        delete b;
  
    // cleanup
    GdiplusShutdown(gdiplusToken);
    return stat == Ok;
}

/*
OLD CODE for stand-alone version

int main(int argc, wchar_t** argv)
{
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  HWND desktop = GetDesktopWindow();
  HDC desktopdc = GetDC(desktop);
  HDC mydc = CreateCompatibleDC(desktopdc);
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);
  HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);
  HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);
  BitBlt(mydc,0,0,width,height,desktopdc,0,0, SRCCOPY|CAPTUREBLT);
  SelectObject(mydc, oldbmp);

  const wchar_t* filename = (argc > 1) ? argv[1] : L"screenshot.png";
  Bitmap* b = Bitmap::FromHBITMAP(mybmp, NULL);
  CLSID  encoderClsid;
  Status stat = GenericError;
  if (b && GetEncoderClsid(L"image/png", &encoderClsid) != -1) {
    stat = b->Save(filename, &encoderClsid, NULL);
  }
  if (b)
    delete b;
  
  // cleanup
  GdiplusShutdown(gdiplusToken);
  ReleaseDC(desktop, desktopdc);
  DeleteObject(mybmp);
  DeleteDC(mydc);
  return stat == Ok;
}
*/
