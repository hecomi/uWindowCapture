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
	if (captureThread_.joinable())
	{
		captureThread_.join();
	}
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


BOOL Window::IsVisible() const
{
	return ::IsWindowVisible(window_) && !::IsIconic(window_) && GetWidth() != 0 && GetHeight() != 0;
}


RECT Window::GetRect() const
{
	RECT rect;
	if (!::GetWindowRect(window_, &rect))
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


void Window::GetTitle(WCHAR* buf, int len) const
{
	if (!GetWindowTextW(window_, buf, len))
	{
		OutputApiError("GetWindowTextW");
	}
}


void Window::CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height)
{
	if (width_ == width && height_ == height) return;
	if (width == 0 || height == 0) return;

	width_ = width;
	height_ = height;

	{
		std::lock_guard<std::mutex> lock(mutex_);
		buffer_.ExpandIfNeeded(width * height * 4);
		DeleteBitmap();
		bitmap_ = ::CreateCompatibleBitmap(hDc, width, height);
	}
}


void Window::DeleteBitmap()
{
	if (bitmap_ != nullptr) 
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (!::DeleteObject(bitmap_)) OutputApiError("DeleteObject");
		bitmap_ = nullptr;
	}
}


void Window::SetTexturePtr(ID3D11Texture2D* ptr)
{
	std::lock_guard<std::mutex> lock(mutex_);
	texture_ = ptr;
}


void Window::SetCaptureMode(CaptureMode mode)
{
	mode_ = mode;
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

	if (hasCaptureFinished_)
	{
		if (captureThread_.joinable()) {
			captureThread_.join();
		}
		captureThread_ = std::thread([&] {
			hasCaptureFinished_ = false;
			CaptureInternal();
			hasCaptureFinished_ = true;
		});
	}
}


void Window::CaptureInternal()
{
	auto hDc = ::GetDC(window_);

	const auto width = GetWidth();
	const auto height = GetHeight();
	if (width == 0 || height == 0)
	{
		if (!::ReleaseDC(window_, hDc)) OutputApiError("ReleaseDC");
	}
	CreateBitmapIfNeeded(hDc, width, height);

	auto hDcMem = ::CreateCompatibleDC(hDc);
	HGDIOBJ preObject = ::SelectObject(hDcMem, bitmap_);

	bool result = false;
	switch (mode_)
	{
		case CaptureMode::PrintWindow:
		{
			result = ::PrintWindow(window_, hDcMem, PW_RENDERFULLCONTENT);
			if (!result) OutputApiError("PrintWindow");
			break;
		}
		case CaptureMode::BitBlt:
		{
			result = ::BitBlt(hDcMem, 0, 0, width_, height_, hDc, 0, 0, SRCCOPY);
			if (!result) OutputApiError("BitBlt");
			break;
		}
	}

	if (result)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		BITMAPINFOHEADER bmi {};
		bmi.biWidth       = static_cast<LONG>(width);
		bmi.biHeight      = -static_cast<LONG>(height);
		bmi.biPlanes      = 1;
		bmi.biSize        = sizeof(BITMAPINFOHEADER);
		bmi.biBitCount    = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage  = 0;

		if (!::GetDIBits(hDcMem, bitmap_, 0, height, buffer_.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
		{
			OutputApiError("GetDIBits");
		}
	}

	::SelectObject(hDcMem, preObject);

	if (!::DeleteDC(hDcMem)) OutputApiError("DeleteDC");
	if (!::ReleaseDC(window_, hDc)) OutputApiError("ReleaseDC");
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

	{
		std::lock_guard<std::mutex> lock(mutex_);
		context->UpdateSubresource(texture_, 0, nullptr, buffer_.Get(), width_ * 4, 0);
	}
}