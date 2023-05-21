#include <common.hpp>

void ui::menu::initialize() {
  ui::gui::initialize_imgui();

  QueryPerformanceFrequency(&ui::menu::performance_frequency);
  QueryPerformanceCounter(&ui::menu::performance_counter);
}

long long ui::menu::handle_message(HWND window, unsigned int message, unsigned long long wparam, long long lparam) {
  static bool insert = false;

  if (wparam == VK_INSERT) {
    if (message == WM_KEYDOWN && !insert) {
      variables::menu::opened = !variables::menu::opened;
      insert = true;
    }
    else if (message == WM_KEYUP) 
      insert = false;
  }

  return ui::gui::handle_message(window, message, wparam, lparam);
}

void ui::menu::handle_frame() {
  ui::menu::frames++;

  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  auto delta = (double)(now.QuadPart - ui::menu::performance_counter.QuadPart) / ui::menu::performance_frequency.QuadPart;

  if (delta >= 1.0) {
    ui::menu::frame_rate = ui::menu::frames;
    ui::menu::frames = 0;
    QueryPerformanceCounter(&ui::menu::performance_counter);
  }
}

void ui::menu::render_menu() {
  if (!variables::menu::opened)
    return;

  ui::gui::add_window("Genshin Utility by lanylow");
  ui::gui::add_groupbox("Settings", 19, 20 + 18, 286, 162);
  ui::gui::add_checkbox("Open menu on start", &variables::menu::open_on_start);
  ui::gui::add_checkbox("Frame rate counter", &variables::tools::fps_counter);
  ui::gui::add_checkbox("Enable V-Sync", &variables::tools::enable_vsync);
  ui::gui::add_checkbox("Disable fog", &variables::tools::disable_fog);
  ui::gui::add_slider("Frame rate limit", 10, 360, &variables::tools::fps_limit, 1);
  ui::gui::add_slider("Camera field of view", 5, 175, &variables::tools::camera_fov, 1);
}

void ui::menu::render_counter() {
  if (!variables::tools::fps_counter)
    return;

  auto text = std::to_string(ui::menu::frame_rate) + " fps";
  auto text_size = ImGui::CalcTextSize(text.c_str());

  auto screen_width = ImGui::GetMainViewport()->Size.x;
  auto text_begin = screen_width - 2 - 5 - text_size.x;
  auto rect_begin = text_begin - 5;
  auto rect_end = screen_width - (rect_begin) - 2;

  ui::gui::add_rectangle(rect_begin, 2, rect_end, 20, 50, 50, 50, 255);
  ui::gui::add_rectangle(rect_begin, 21, rect_end, 1, 195, 141, 145, 255);
  ui::gui::add_outlined_rectangle(rect_begin, 2, rect_end, 20, 0, 0, 0, 84);
  ui::gui::add_text(text.c_str(), text_begin, 4, 180, 180, 180, 255);
}