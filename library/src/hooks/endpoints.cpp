#include <hooks/endpoints.hpp>

#include <hooks/hooks.hpp>
#include <hooks/veh.hpp>
#include <ui/renderer.hpp>
#include <ui/menu.hpp>
#include <options.hpp>
#include <sdk.hpp>

#pragma warning(disable: 6387)

long __stdcall hooks::endpoints::present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags) {
  auto& storage = hooks::present.get_storage();

  utils::call_once(storage.init_flag, [&] {
    swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&storage.device);
    storage.device->GetImmediateContext(&storage.context);

    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    swap_chain->GetDesc(&swap_chain_desc);
    hooks::wndproc.get_storage().window = swap_chain_desc.OutputWindow;
    hooks::wndproc.set_trampoline(SetWindowLongPtrA(hooks::wndproc.get_storage().window, GWLP_WNDPROC, (int64_t)hooks::endpoints::wndproc));

    ui::renderer::initialize();
    options::menu.opened = options::menu.open_on_start;
  });

  utils::call_once(storage.render_target_flag, [&] {
    ID3D11Texture2D* back_buffer;
    swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
    storage.device->CreateRenderTargetView(back_buffer, nullptr, &storage.render_target);
    back_buffer->Release();
  });

  ui::renderer::begin();
  storage.menu.render();
  ui::renderer::end();

  return hooks::present.get_trampoline<decltype(&hooks::endpoints::present)>()(swap_chain, sync_interval, flags);
}

#pragma warning(default: 6387)

long __stdcall hooks::endpoints::resize_buffers(IDXGISwapChain* swap_chain, unsigned int buffer_count, unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int flags) {
  auto& storage = hooks::present.get_storage();
  storage.render_target->Release();
  storage.render_target = nullptr;
  storage.render_target_flag.reset();

  return hooks::resize_buffers.get_trampoline<decltype(&hooks::endpoints::resize_buffers)>()(swap_chain, buffer_count, width, height, format, flags);
}

long long __stdcall hooks::endpoints::wndproc(HWND window, unsigned int message, unsigned long long wparam, long long lparam) {
  if (!ui::menu::handle_message(window, message, wparam, lparam) && options::menu.opened)
    return true;

  return CallWindowProcA(hooks::wndproc.get_trampoline<decltype(&hooks::endpoints::wndproc)>(), window, message, wparam, lparam);
}

void hooks::endpoints::set_field_of_view(void* _this, float value) {
  auto& storage = hooks::set_field_of_view.get_storage();
  
  utils::call_once(storage.present_flag, [] {
    hooks::present.install_swap_chain(8, &hooks::endpoints::present);
    hooks::resize_buffers.install_swap_chain(13, &hooks::endpoints::resize_buffers);
  });

  auto genshin_impact = [](float val) -> bool {
    if (val == 30.f)
      sdk::set_fog(false);

    return val >= 45.f && val <= 55.f;
  };

  auto star_rail = [](float val) -> bool {
    return val != 30.f && val != 1.f;
  };

  const auto floored = std::floor(value);
  const auto res = sdk::game_t::is(sdk::game_t::genshin_impact) ? genshin_impact(floored) : star_rail(floored);

  if (res) {
    if (!storage.is_in_battle)
      value = (float)options::tools.camera_fov;

    sdk::set_target_frame_rate(options::tools.enable_vsync ? -1 : options::tools.fps_limit);
    sdk::set_vsync_count(options::tools.enable_vsync ? 1 : 0);

    if (sdk::game_t::is(sdk::game_t::genshin_impact))
      sdk::set_fog(!options::tools.disable_fog);
  }

  hooks::set_field_of_view.get_trampoline<decltype(&hooks::endpoints::set_field_of_view)>()(_this, value);
}

void hooks::endpoints::set_field_of_view_gi(void* _this, float value) {
  hooks::veh::call_original(_this, value);

  utils::call_once(hooks::set_field_of_view.get_storage().present_flag, [] {
    hooks::present.install_swap_chain(8, &hooks::endpoints::present);
    hooks::resize_buffers.install_swap_chain(13, &hooks::endpoints::resize_buffers);
  });

  if (value != 45.f)
    return;

  hooks::set_field_of_view.install(sdk::set_field_of_view, &hooks::endpoints::set_field_of_view);
  hooks::quit.install(sdk::quit, &hooks::endpoints::quit);

  hooks::veh::destroy();
}

void hooks::endpoints::quit() {
  options::save();
  hooks::quit.get_trampoline<decltype(&hooks::endpoints::quit)>()();
}

void hooks::endpoints::enter(void* _this) {
  hooks::set_field_of_view.get_storage().is_in_battle = true;
  hooks::enter.get_trampoline<decltype(&hooks::endpoints::enter)>()(_this);
}

void hooks::endpoints::leave(void* _this, void* a1) {
  hooks::set_field_of_view.get_storage().is_in_battle = false;
  hooks::leave.get_trampoline<decltype(&hooks::endpoints::leave)>()(_this, a1);
}
