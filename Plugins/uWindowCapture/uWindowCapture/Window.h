#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include "Common.h"

class Window
{
public:
	explicit Window(HWND hwnd);
	~Window();

	BOOL IsWindow() const;
	BOOL IsVisible() const;
	HWND GetHandle() const;
	RECT GetRect() const;
	UINT GetWidth() const;
	UINT GetHeight() const;
	void GetTitle(wchar_t* buf, int len) const;
	void SetTexturePtr(ID3D11Texture2D* ptr);

	void Capture();
	void Draw();

private:
	void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
	void DeleteBitmap();

	HWND window_ = nullptr;
	Buffer<BYTE> buffer_;
	HBITMAP bitmap_ = nullptr;
	UINT width_ = 0;
	UINT height_ = 0;
	ID3D11Texture2D* texture_ = nullptr;
};

