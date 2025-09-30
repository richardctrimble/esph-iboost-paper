#include "esphWirelessPaper.h"
#include "esphome/core/log.h"
#include "heltec-eink-modules.h"
#include <string>

namespace esphome
{
  namespace esphWirelessPaper
  {
    static const char *const TAG = "esphWirelessPaper";

    // Constants for better maintainability
    static const int TEXT_X_OFFSET = 10;
    static const int TEXT_CLEAR_WIDTH = 250;
    static const int TEXT_LINE_HEIGHT = 10;
    static const int TITLE_Y_POS = 10;
    static const int STATUS_Y_POS = 20;
    static const int DATA_START_Y_POS = 30;
    static const int COUNTER_Y_POS = 110;
    static const int FAST_MODE_DELAY_MS = 20;
    static const int SETUP_DELAY_MS = 200;
    static const long MAX_COUNTER_VALUE = 99999999;

    void PaperDisplay::setup()
    {
      ESP_LOGV(TAG, "Display setup started");
      this->display.landscape();
      // this->display.clear();
      this->screen_writeTitleLine(config_TopTitle);
      this->screen_writeStatusLine("Initializing...");
      delay(SETUP_DELAY_MS);
      ESP_LOGV(TAG, "Display setup complete");
    }

    void PaperDisplay::update() // called on poll
    {
      ESP_LOGV(TAG, "Update Call Started");
 

      ESP_LOGV(TAG, "Update Call Complete");
    }

    void PaperDisplay::screen_Clear()
    {
      ESP_LOGV(TAG, "screen_Clear called");
      this->display.clear();
    }

    void screen_fastWriteMessage(EInkDisplay_WirelessPaperV1_1 &disp, const std::string &message, int yPos)
    {
      ESP_LOGV(TAG, "fast write called with message: %s at yPos: %d", message.c_str(), yPos);
      disp.fastmodeOn();
      disp.fillRect(TEXT_X_OFFSET, yPos, TEXT_CLEAR_WIDTH, TEXT_LINE_HEIGHT, WHITE);
      disp.setCursor(TEXT_X_OFFSET, yPos);
      disp.print(message.c_str());
      disp.update();
      disp.fastmodeOff();
      delay(FAST_MODE_DELAY_MS);
      ESP_LOGV(TAG, "fast write completed for message: %s", message.c_str());
    }

    void PaperDisplay::screen_writeDataLine(int Line, const std::string &data)
    {
      ESP_LOGV(TAG, "Called write Data Line: %d with data: %s", Line, data.c_str());
      if (Line < 1 || Line > 8)
      {
        ESP_LOGW(TAG, "Invalid line number: %d", Line);
        return;
      }
      int yPos = DATA_START_Y_POS + (Line - 1) * TEXT_LINE_HEIGHT;
      screen_fastWriteMessage(this->display, data, yPos);
    }

    void screen_writeMessage(EInkDisplay_WirelessPaperV1_1 &disp, const std::string &message, int yPos)
    {
      ESP_LOGV(TAG, "Write Message called with message: %s at yPos: %d", message.c_str(), yPos);
      disp.setCursor(10, yPos);
      disp.print(message.c_str());
      disp.update();
    }

    void PaperDisplay::screen_writeStatusLine(const std::string &status)
    {
      ESP_LOGV(TAG, "Write Status Line called with status: %s", status.c_str());
      screen_fastWriteMessage(this->display, status, STATUS_Y_POS);
    }

    void PaperDisplay::screen_writeTitleLine(const std::string &title)
    {
      ESP_LOGV(TAG, "write Title Line called with title: %s", title.c_str());
      screen_fastWriteMessage(this->display, title, TITLE_Y_POS);
    }

  } // namespace esphWirelessPaper
} // namespace esphome