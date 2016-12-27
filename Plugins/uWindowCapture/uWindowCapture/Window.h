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
	HWND GetHandle() const;
	RECT GetRect() const;
	UINT GetWidth() const;
	UINT GetHeight() const;
	std::string GetTitle() const;
	void SetTexturePtr(ID3D11Texture2D* ptr);

	void Capture();
	void Draw();

private:
	void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
	void DeleteBitmap();
	void OutputApiError(const char* apiName) const;

	HWND window_;
	Buffer<BYTE> buffer_;
	HBITMAP bitmap_;
	UINT width_ = 0;
	UINT height_ = 0;
	ID3D11Texture2D* texture_ = nullptr;
};

