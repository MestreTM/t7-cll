#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include "splash.hpp"
#include "resource.hpp"

#include <utils/nt.hpp>
#include <utils/image.hpp>

namespace splash {
namespace {
HWND window{};
HWND loading_label{};
utils::image::object image{};
std::thread window_thread{};
HFONT loading_font{};
const char *loading_text = "Loading";
int label_x = 0;
int label_y = 0;
int label_w = 110;
int label_h = 16;
WNDPROC old_label_proc{};

utils::image::object load_splash_image() {
  const auto res = utils::nt::load_resource(IMAGE_SPLASH);
  const auto img = utils::image::load_image(res);
  return utils::image::create_bitmap(img);
}

void destroy_window() {
  loading_label = nullptr;
  if (loading_font) {
    DeleteObject(loading_font);
    loading_font = nullptr;
  }
  if (window && IsWindow(window)) {
    ShowWindow(window, SW_HIDE);
    DestroyWindow(window);
    window = nullptr;

    if (window_thread.joinable()) {
      window_thread.join();
    }

    window = nullptr;
  } else if (window_thread.joinable()) {
    window_thread.detach();
  }
}

LRESULT CALLBACK label_proc(HWND hwnd, UINT msg, WPARAM wparam,
                            LPARAM lparam) {
  if (msg == WM_ERASEBKGND) {
    return 1;
  }

  if (msg == WM_PAINT) {
    PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(hwnd, &ps);

    if (image) {
      HDC mem = CreateCompatibleDC(hdc);
      auto *old_bmp = SelectObject(mem, image);
      BitBlt(hdc, 0, 0, label_w, label_h, mem, label_x, label_y, SRCCOPY);
      SelectObject(mem, old_bmp);
      DeleteDC(mem);
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    HGDIOBJ old_font = nullptr;
    if (loading_font) {
      old_font = SelectObject(hdc, loading_font);
    }
    RECT rc{0, 0, label_w, label_h};
    DrawTextA(hdc, loading_text, -1, &rc,
              DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    if (old_font) {
      SelectObject(hdc, old_font);
    }

    EndPaint(hwnd, &ps);
    return 0;
  }

  return CallWindowProcA(old_label_proc, hwnd, msg, wparam, lparam);
}

void show() {
  WNDCLASSA wnd_class{};

  const auto self = utils::nt::library::get_by_address(load_splash_image);

  wnd_class.style = CS_DROPSHADOW;
  wnd_class.cbClsExtra = 0;
  wnd_class.cbWndExtra = 0;
  wnd_class.lpszMenuName = nullptr;
  wnd_class.lpfnWndProc = DefWindowProcA;
  wnd_class.hInstance = self;
  wnd_class.hIcon = LoadIconA(self, MAKEINTRESOURCEA(ID_ICON));
  wnd_class.hCursor = LoadCursorA(nullptr, IDC_APPSTARTING);
  wnd_class.hbrBackground = reinterpret_cast<HBRUSH>(6);
  wnd_class.lpszClassName = "Black Ops III Splash Screen";

  if (RegisterClassA(&wnd_class)) {
    const auto x_pixels = GetSystemMetrics(SM_CXFULLSCREEN);
    const auto y_pixels = GetSystemMetrics(SM_CYFULLSCREEN);

    if (image) {
      window = CreateWindowExA(WS_EX_APPWINDOW, "Black Ops III Splash Screen",
                               "CLL", WS_POPUP | WS_SYSMENU,
                               (x_pixels - 320) / 2, (y_pixels - 100) / 2, 320,
                               100, nullptr, nullptr, self, nullptr);

      if (window) {
        auto *const image_window =
            CreateWindowExA(0, "Static", nullptr, WS_CHILD | WS_VISIBLE | 0xEu,
                            0, 0, 320, 100, window, nullptr, self, nullptr);
        if (image_window) {
          RECT rect{};
          SendMessageA(image_window, STM_SETIMAGE, IMAGE_BITMAP, image);
          GetWindowRect(image_window, &rect);

          const int width = rect.right - rect.left;
          rect.left = (x_pixels - width) / 2;

          const int height = rect.bottom - rect.top;
          rect.top = (y_pixels - height) / 2;

          rect.right = rect.left + width;
          rect.bottom = rect.top + height;
          AdjustWindowRect(&rect, WS_CHILD | WS_VISIBLE | 0xEu, 0);
          SetWindowPos(window, nullptr, rect.left, rect.top,
                       rect.right - rect.left, rect.bottom - rect.top,
                       SWP_NOZORDER);

          SetWindowRgn(window,
                       CreateRoundRectRgn(0, 0, rect.right - rect.left,
                                          rect.bottom - rect.top, 15, 15),
                       TRUE);

          label_w = 110;
          label_h = 16;
          label_x = width - label_w - 8;
          label_y = height - label_h - 4;

          loading_font =
              CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                          DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

          loading_label = CreateWindowExA(
              0, "Static", "", WS_CHILD | WS_VISIBLE, label_x, label_y,
              label_w, label_h, window, nullptr, self, nullptr);
          if (loading_label) {
            old_label_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
                loading_label, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(label_proc)));
          }

          ShowWindow(window, SW_SHOW);
          UpdateWindow(window);
        }
      }
    }
  }
}

bool draw_frame() {
  if (!window) {
    return false;
  }

  MSG msg{};
  bool success = true;

  while (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);

    if (msg.message == WM_DESTROY && msg.hwnd == window) {
      PostQuitMessage(0);
    }

    if (msg.message == WM_QUIT) {
      success = false;
    }
  }
  return success;
}

void draw() {
  show();

  int frame = 0;
  auto last = std::chrono::steady_clock::now();
  constexpr const char *frames[] = {"Loading", "Loading.", "Loading..",
                                    "Loading..."};

  while (draw_frame()) {
    const auto now = std::chrono::steady_clock::now();
    if (now - last >= 280ms) {
      frame = (frame + 1) % 4;
      loading_text = frames[frame];
      if (loading_label && IsWindow(loading_label)) {
        InvalidateRect(loading_label, nullptr, FALSE);
      }
      last = now;
    }
    std::this_thread::sleep_for(16ms);
  }

  window = nullptr;
  UnregisterClassA("Black Ops III Splash Screen", utils::nt::library{});
}
} // namespace

struct component final : client_component {
  component() {
    image = load_splash_image();
    window_thread = std::thread([] { draw(); });
  }

  void pre_destroy() override {
    destroy_window();
    if (window_thread.joinable()) {
      window_thread.detach();
    }
  }

  void post_unpack() override { destroy_window(); }
};

void hide() {
  if (window && IsWindow(window)) {
    ShowWindow(window, SW_HIDE);
    UpdateWindow(window);
  }

  destroy_window();
}

HWND get_window() { return window; }
} // namespace splash

REGISTER_COMPONENT(splash::component)
