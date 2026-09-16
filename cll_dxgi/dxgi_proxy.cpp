#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <atomic>

static HMODULE g_sys = nullptr;
static void *g_orig_present = nullptr;
static void *g_orig_create_swap = nullptr;
static std::atomic<bool> g_swap_hooked{false};

using PFN_OnPresent = void (*)(void *);
static PFN_OnPresent g_cll_present = nullptr;

static HMODULE system_dxgi() {
  if (g_sys) {
    return g_sys;
  }
  g_sys = LoadLibraryExW(L"C:\\Windows\\System32\\dxgi.dll", nullptr,
                         LOAD_LIBRARY_SEARCH_SYSTEM32);
  if (!g_sys) {
    g_sys = LoadLibraryW(L"C:\\Windows\\System32\\dxgi.dll");
  }
  return g_sys;
}

static void *sys_proc(const char *name) {
  HMODULE m = system_dxgi();
  return m ? GetProcAddress(m, name) : nullptr;
}

static void hook_vtable(void *obj, int index, void *hook, void **orig) {
  if (!obj) {
    return;
  }
  void **vtable = *reinterpret_cast<void ***>(obj);
  DWORD old = 0;
  if (!VirtualProtect(&vtable[index], sizeof(void *), PAGE_EXECUTE_READWRITE,
                      &old)) {
    return;
  }
  if (orig && !*orig) {
    *orig = vtable[index];
  }
  vtable[index] = hook;
  VirtualProtect(&vtable[index], sizeof(void *), old, &old);
}

static void log_msg(const char *msg) {
  wchar_t path[MAX_PATH]{};
  HMODULE self = nullptr;
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                         GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     reinterpret_cast<LPCWSTR>(&log_msg), &self);
  if (!self || !GetModuleFileNameW(self, path, MAX_PATH)) {
    return;
  }
  wchar_t *slash = wcsrchr(path, L'\\');
  if (!slash) {
    slash = wcsrchr(path, L'/');
  }
  if (slash) {
    wcscpy(slash + 1, L"cll-dxgi.log");
  }
  HANDLE f = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr,
                         OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (f == INVALID_HANDLE_VALUE) {
    return;
  }
  DWORD wr = 0;
  WriteFile(f, msg, static_cast<DWORD>(strlen(msg)), &wr, nullptr);
  WriteFile(f, "\n", 1, &wr, nullptr);
  CloseHandle(f);
}

static void resolve_cll() {
  if (g_cll_present) {
    return;
  }
  const wchar_t *names[] = {nullptr, L"boiii.exe", L"BlackOps3.exe"};
  for (const wchar_t *name : names) {
    HMODULE host = name ? GetModuleHandleW(name) : GetModuleHandleW(nullptr);
    if (!host) {
      continue;
    }
    g_cll_present = reinterpret_cast<PFN_OnPresent>(
        GetProcAddress(host, "CLL_OnPresent"));
    if (g_cll_present) {
      log_msg("found CLL_OnPresent");
      return;
    }
  }
  log_msg("CLL_OnPresent not found in host modules");
}

HRESULT STDMETHODCALLTYPE Present_hook(IDXGISwapChain *self, UINT sync,
                                       UINT flags) {
  DXGI_SWAP_CHAIN_DESC desc{};
  if (self && SUCCEEDED(self->GetDesc(&desc)) && desc.BufferDesc.Width >= 640) {
    resolve_cll();
    if (g_cll_present) {
      g_cll_present(self);
    }
  }
  using PFN = HRESULT(STDMETHODCALLTYPE *)(IDXGISwapChain *, UINT, UINT);
  return reinterpret_cast<PFN>(g_orig_present)(self, sync, flags);
}

HRESULT STDMETHODCALLTYPE CreateSwapChain_hook(IDXGIFactory *self,
                                               IUnknown *device,
                                               DXGI_SWAP_CHAIN_DESC *desc,
                                               IDXGISwapChain **out) {
  using PFN = HRESULT(STDMETHODCALLTYPE *)(IDXGIFactory *, IUnknown *,
                                           DXGI_SWAP_CHAIN_DESC *,
                                           IDXGISwapChain **);
  const HRESULT hr =
      reinterpret_cast<PFN>(g_orig_create_swap)(self, device, desc, out);
  const UINT w = (desc && SUCCEEDED(hr)) ? desc->BufferDesc.Width : 0;
  if (SUCCEEDED(hr) && out && *out && w >= 640 && !g_swap_hooked.exchange(true)) {
    hook_vtable(*out, 8, &Present_hook, &g_orig_present);
    log_msg("hooked Present on CreateSwapChain");
  }
  return hr;
}

static void hook_factory(void *factory) {
  if (!factory || g_orig_create_swap) {
    return;
  }
  hook_vtable(factory, 10, &CreateSwapChain_hook, &g_orig_create_swap);
  log_msg("hooked IDXGIFactory::CreateSwapChain");
}

extern "C" {

int CLL_DXGI_PROXY() { return 1; }

HRESULT WINAPI CreateDXGIFactory(REFIID riid, void **factory) {
  using PFN = HRESULT(WINAPI *)(REFIID, void **);
  auto *fn = reinterpret_cast<PFN>(sys_proc("CreateDXGIFactory"));
  if (!fn) {
    return E_FAIL;
  }
  const HRESULT hr = fn(riid, factory);
  if (SUCCEEDED(hr) && factory) {
    hook_factory(*factory);
  }
  return hr;
}

HRESULT WINAPI CreateDXGIFactory1(REFIID riid, void **factory) {
  using PFN = HRESULT(WINAPI *)(REFIID, void **);
  auto *fn = reinterpret_cast<PFN>(sys_proc("CreateDXGIFactory1"));
  if (!fn) {
    return E_FAIL;
  }
  const HRESULT hr = fn(riid, factory);
  log_msg("CreateDXGIFactory1");
  if (SUCCEEDED(hr) && factory) {
    hook_factory(*factory);
  }
  return hr;
}

HRESULT WINAPI CreateDXGIFactory2(UINT flags, REFIID riid, void **factory) {
  using PFN = HRESULT(WINAPI *)(UINT, REFIID, void **);
  auto *fn = reinterpret_cast<PFN>(sys_proc("CreateDXGIFactory2"));
  if (!fn) {
    return CreateDXGIFactory1(riid, factory);
  }
  const HRESULT hr = fn(flags, riid, factory);
  if (SUCCEEDED(hr) && factory) {
    hook_factory(*factory);
  }
  return hr;
}

HRESULT WINAPI DXGIGetDebugInterface1(UINT flags, REFIID riid, void **out) {
  using PFN = HRESULT(WINAPI *)(UINT, REFIID, void **);
  auto *fn = reinterpret_cast<PFN>(sys_proc("DXGIGetDebugInterface1"));
  return fn ? fn(flags, riid, out) : E_NOINTERFACE;
}

HRESULT WINAPI DXGIDeclareAdapterRemovalSupport() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("DXGIDeclareAdapterRemovalSupport"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI ApplyCompatResolutionQuirking() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("ApplyCompatResolutionQuirking"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI CompatString() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("CompatString"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI CompatValue() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("CompatValue"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI DXGIDumpJournal() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("DXGIDumpJournal"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI PIXBeginCapture() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("PIXBeginCapture"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI PIXEndCapture() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("PIXEndCapture"));
  return fn ? fn() : S_OK;
}

HRESULT WINAPI PIXGetCaptureState() {
  using PFN = HRESULT(WINAPI *)();
  auto *fn = reinterpret_cast<PFN>(sys_proc("PIXGetCaptureState"));
  return fn ? fn() : S_OK;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, void *) {
  if (reason == DLL_PROCESS_ATTACH) {
    system_dxgi();
    log_msg("CLL dxgi proxy loaded");
  }
  return TRUE;
}

} // extern "C"
