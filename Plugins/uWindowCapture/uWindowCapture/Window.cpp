#include <wrl/client.h>
#include "Window.h"
#include "Debug.h"

using namespace Microsoft::WRL;



Window::Window(HWND hwnd)
	: window_(hwnd)
{
	if (!IsWindow())
	{
		Debug::Error("Given window handle is not a window.");
		window_ = nullptr;
	}
}


Window::~Window()
{
	DeleteBitmap();
}


HWND Window::GetHandle() const
{
	return window_;
}


BOOL Window::IsWindow() const
{
	return ::IsWindow(window_) && ::IsWindowVisible(window_) && !::IsIconic(window_);
}


BOOL Window::IsVisible() const
{
	return ::IsWindow(window_) && ::IsWindowVisible(window_) && !::IsIconic(window_);
}


RECT Window::GetRect() const
{
	RECT rect;
	if (!GetWindowRect(window_, &rect))
	{
		OutputApiError("GetWindowRect");
	}
	return std::move(rect);
}


UINT Window::GetWidth() const
{
	const auto rect = GetRect();
	return rect.right - rect.left;
}


UINT Window::GetHeight() const
{
	const auto rect = GetRect();
	return rect.bottom - rect.top;
}


void Window::GetTitle(wchar_t* buf, int len) const
{
	if (!GetWindowTextW(window_, buf, len))
	{
		OutputApiError("GetWindowTextW");
	}
}


void Window::CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height)
{
	if (width_ == width && height_ == height) return;

	width_ = width;
	height_ = height;
	buffer_.ExpandIfNeeded(width * height * 4);

	DeleteBitmap();
	bitmap_ = CreateCompatibleBitmap(hDc, width, height);
}


void Window::DeleteBitmap()
{
	if (bitmap_ != nullptr) 
	{
		if (!DeleteObject(bitmap_)) OutputApiError("DeleteObject");
		bitmap_ = nullptr;
	}
}


void Window::SetTexturePtr(ID3D11Texture2D* ptr)
{
	texture_ = ptr;
}


void Window::Capture()
{
	if (!IsWindow())
	{
		Debug::Error("Window doesn't exist anymore.");
		return;
	}

	if (!IsVisible())
	{
		return;
	}

	auto hDc = GetDC(window_);

	const auto width = GetWidth();
	const auto height = GetHeight();
	CreateBitmapIfNeeded(hDc, width, height);

	auto hDcMem = CreateCompatibleDC(hDc);
	HGDIOBJ preObject = SelectObject(hDcMem, bitmap_);

	if (BitBlt(hDcMem, 0, 0, width_, height_, hDc, 0, 0, SRCCOPY))
	{
		BITMAPINFOHEADER bmi {};
		bmi.biWidth       = static_cast<LONG>(width);
		bmi.biHeight      = -static_cast<LONG>(height);
		bmi.biPlanes      = 1;
		bmi.biSize        = sizeof(BITMAPINFOHEADER);
		bmi.biBitCount    = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage  = 0;

		if (!GetDIBits(hDcMem, bitmap_, 0, height, buffer_.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
		{
			OutputApiError("GetDIBits");
		}
	}
	else
	{
		OutputApiError("BitBlt");
	}

	SelectObject(hDcMem, preObject);

	if (!DeleteDC(hDcMem)) OutputApiError("DeleteDC");
	if (!ReleaseDC(window_, hDc)) OutputApiError("ReleaseDC");
}


void Window::Draw()
{
	if (texture_ == nullptr) return;

	D3D11_TEXTURE2D_DESC desc;
	texture_->GetDesc(&desc);
	if (desc.Width != width_ || desc.Height != height_) return;

	auto device = GetDevice();
	ComPtr<ID3D11DeviceContext> context;
	device->GetImmediateContext(&context);
	context->UpdateSubresource(texture_, 0, nullptr, buffer_.Get(), width_ * 4, 0);
}