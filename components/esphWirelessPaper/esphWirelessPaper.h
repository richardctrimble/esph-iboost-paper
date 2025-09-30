#pragma once
#include "esphome.h"
#include "heltec-eink-modules.h"

namespace esphome
{
  namespace esphWirelessPaper
  {

    class PaperDisplay : public PollingComponent
    {
    public:
      PaperDisplay() : PollingComponent(15000) {} // fire every 15 seconds
      EInkDisplay_WirelessPaperV1_1 display;

      std::string config_TopTitle = "";

      void set_TopTitle(const std::string &top_title)
      {
        config_TopTitle = top_title;
      }
      void setup() override;
      void update() override;
      void screen_Clear();
      void screen_writeDataLine(int Line, const std::string &data);
      void screen_writeStatusLine(const std::string &status);
      void screen_writeTitleLine(const std::string &title);
    };

  } // namespace esphWirelessPaper
} // namespace esphome
