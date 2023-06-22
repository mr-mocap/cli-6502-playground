#ifndef PAGEVIEW_H
#define PAGEVIEW_H

#include "ftxui/dom/node.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/util/ref.hpp"
#include "rambusdeviceview.hpp"
#include <memory>


ftxui::Element pageview(std::shared_ptr<RamBusDeviceView> model,
                        ftxui::Ref<int>                   current_byte,
                        ftxui::Ref<int>                   program_counter,
                        bool                              in_edit_mode);

int pageview_byte(int x_coord, int y_coord);

ftxui::Box pageview_ui_box_for_byte(int byte);

#endif // PAGEVIEW_H
