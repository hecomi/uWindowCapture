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
	return ::IsWindow(window_);
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


std::string Window::GetTitle() const
{
	const size_t size = 256;
	char buf[size];
	GetWindowText(window_, buf, size);
	return buf;
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


void Window::OutputApiError(const char* apiName) const
{
	const auto error = GetLastError();
	Debug::Error(apiName, "() failed width error code: ", error);
}


void Window::DeleteBitmap()
{
	if (bitmap_) 
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

	auto hDc = GetDC(window_);
	auto hDcMem = CreateCompatibleDC(hDc);

	const int width = GetWidth();
	const int height = GetHeight();
	CreateBitmapIfNeeded(hDc, width, height);

	HGDIOBJ preObject = SelectObject(hDcMem, bitmap_);
	if (BitBlt(hDcMem, 0, 0, width_, height_, hDc, 0, 0, SRCCOPY))
	{
		BITMAPINFOHEADER bmi{};
		bmi.biWidth       = width;
		bmi.biHeight      = -height;
		bmi.biPlanes      = 1;
		bmi.biSize        = sizeof(BITMAPINFOHEADER);
		bmi.biBitCount    = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage  = 0;

		GetDIBits(hDcMem, bitmap_, 0, height, buffer_.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS);
	}
	else
	{
		OutputApiError("BitBlt");
	}

	SelectObject(hDcMem, preObject);
	DeleteDC(hDcMem);
	ReleaseDC(window_, hDc);
}


void Window::Draw()
{
	if (texture_ == nullptr) return;

	D3D11_TEXTURE2D_DESC desc;
	texture_->GetDesc(&desc);
	if (desc.Width != width_ || desc.Height != height_)
	{
		Debug::Error("texture sizes of '", GetTitle(), "' does not match: (", width_, ", ", height_, ") != (", desc.Width, ", ", desc.Height, ").");
		return;
	}

	auto device = GetDevice();
	ComPtr<ID3D11DeviceContext> context;
	device->GetImmediateContext(&context);

	context->UpdateSubresource(texture_, 0, nullptr, buffer_.Get(), width_ * 4, 0);
}