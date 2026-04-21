#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <time.h>
#include <vector>
#include <OneButton.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <U8g2_for_Adafruit_GFX.h>

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

const String CLIENT_ID = "";
const String CLIENT_SECRET = "";
const String REFRESH_TOKEN = "";
const String LIST_ID = "";

#define BUTTON_PIN 2
#define MPU_SDA 42
#define MPU_SCL 43
#define TFT_SCK   12
#define TFT_MOSI  11
#define TFT_DC    9
#define TFT_RST   14
#define TFT_CS   -1

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

WiFiServer server(80);
OneButton btn(BUTTON_PIN, true);

String display_mode = "clock";
bool is_focus_mode_on = false;
unsigned long focus_start_time = 0;

String music_title = "Not Connected";
String music_artist = "";

int music_submenu = 0;

unsigned long last_token_update = 0;
const unsigned long TOKEN_UPDATE_INTERVAL = 3000000UL;

unsigned long last_todo_update = 0;
const unsigned long TODO_UPDATE_INTERVAL = 60000UL;

unsigned long last_mail_update = 0;
const unsigned long MAIL_UPDATE_INTERVAL = 60000UL;

unsigned long last_calendar_update = 0;
const unsigned long CALENDAR_UPDATE_INTERVAL = 60000UL;

String access_token = "";
bool is_token_valid = false;

unsigned long last_gyro_check = 0;
const int GYRO_CHECK_INTERVAL = 100;

bool needs_display_update = true;
String display_update_reason = "boot";
int last_drawn_minute = -1;

struct Task {
  String id;
  String title;
};

struct MailItem {
  String subject;
  String fromName;
};

struct CalendarItem {
  String timeText;
  String subject;
};

std::vector<Task> tasks;
int current_task_idx = 0;

MailItem latestMail = {"메일 없음", ""};
std::vector<CalendarItem> todayEvents;

unsigned long last_task_advance = 0;
const unsigned long TASK_ADVANCE_INTERVAL = 8000UL;

#define IMU_ADDR 0x68

struct ImuData {
  float ax_g = 0.0f;
  float ay_g = 0.0f;
  float az_g = 0.0f;
  float gx_dps = 0.0f;
  float gy_dps = 0.0f;
  float gz_dps = 0.0f;
};

ImuData imuData;
bool imu_ok = false;

void requestDisplayUpdate(const char* reason);
bool initIMU();
bool updateIMU();

void sync_time();
void clearScreenWhite();
void drawCenteredText(const String& text, int y, const uint8_t* font);
String ellipsizeText(const String& text, int maxWidth, const uint8_t* font);
void drawTopBar(const String& title, const String& rightText = "");
void drawCard(int x, int y, int w, int h);
void updateDisplay();
void showBootMessage(const char* line1, const char* line2);

void connect_wifi();
String get_new_access_token();
void get_todo_tasks(String token);
bool complete_todo_task(String token, String listId, String taskId);

unsigned char h2int(char c);
String urlDecode(String str);
String urlEncode(String str);

void handleLongPressComplete();
void handleShortClick();

void fetchLatestMail(String token);
void fetchTodayCalendar(String token);
String getTodayStartISO();
String getTodayEndISO();
String isoToHHMM(const String& iso);

void check_sensor_and_control();
void advanceTodoTaskIfNeeded();

void requestDisplayUpdate(const char* reason) {
  display_update_reason = String(reason);
  needs_display_update = true;
  Serial.printf("[LCD_TRIGGER] reason=%s, mode=%s, millis=%lu\n",
                reason, display_mode.c_str(), millis());
}

uint8_t imuReadReg(uint8_t reg) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0xFF;

  if (Wire.requestFrom((uint8_t)IMU_ADDR, (uint8_t)1) != 1) return 0xFF;
  return Wire.read();
}

bool imuWriteReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return (Wire.endTransmission() == 0);
}

int16_t imuRead16(uint8_t regH) {
  Wire.beginTransmission(IMU_ADDR);
  Wire.write(regH);
  if (Wire.endTransmission(false) != 0) return 0;

  if (Wire.requestFrom((uint8_t)IMU_ADDR, (uint8_t)2) != 2) return 0;

  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  return (int16_t)((hi << 8) | lo);
}

bool initIMU() {
  uint8_t who = imuReadReg(0x75);
  Serial.printf("WHO_AM_I = 0x%02X\n", who);

  if (who != 0x3D) {
    Serial.println("IMU WHO_AM_I mismatch");
    return false;
  }

  if (!imuWriteReg(0x13, 0x09)) {
    Serial.println("Failed to write DRIVE_CONFIG");
    return false;
  }
  delayMicroseconds(50);

  if (!imuWriteReg(0x4E, 0x0F)) {
    Serial.println("Failed to write PWR_MGMT0");
    return false;
  }

  delayMicroseconds(300);
  delay(50);

  uint8_t who2 = imuReadReg(0x75);
  Serial.printf("WHO_AM_I(after init) = 0x%02X\n", who2);

  return (who2 == 0x3D);
}

bool updateIMU() {
  if (!imu_ok) return false;

  int16_t ax_raw = imuRead16(0x1F);
  int16_t ay_raw = imuRead16(0x21);
  int16_t az_raw = imuRead16(0x23);
  int16_t gx_raw = imuRead16(0x25);
  int16_t gy_raw = imuRead16(0x27);
  int16_t gz_raw = imuRead16(0x29);

  imuData.ax_g = (float)ax_raw / 2048.0f;
  imuData.ay_g = (float)ay_raw / 2048.0f;
  imuData.az_g = (float)az_raw / 2048.0f;

  imuData.gx_dps = (float)gx_raw / 16.4f;
  imuData.gy_dps = (float)gy_raw / 16.4f;
  imuData.gz_dps = (float)gz_raw / 16.4f;

  return true;
}

void sync_time() {
  configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time");
  time_t now = time(nullptr);
  int retry = 0;
  while (now < 100000 && retry < 15) {
    Serial.print(".");
    delay(1000);
    now = time(nullptr);
    retry++;
  }
  if (retry < 15) Serial.println("\nTime synced");
  else Serial.println("\nNTP Timeout");
}

void clearScreenWhite() {
  tft.fillScreen(ST77XX_WHITE);
  u8g2Fonts.setForegroundColor(ST77XX_BLACK);
  u8g2Fonts.setBackgroundColor(ST77XX_WHITE);
}

void drawCenteredText(const String& text, int y, const uint8_t* font) {
  u8g2Fonts.setFont(font);
  int w = u8g2Fonts.getUTF8Width(text.c_str());
  int x = (tft.width() - w) / 2;
  if (x < 0) x = 0;
  u8g2Fonts.setCursor(x, y);
  u8g2Fonts.print(text);
}

String ellipsizeText(const String& text, int maxWidth, const uint8_t* font) {
  u8g2Fonts.setFont(font);
  if (u8g2Fonts.getUTF8Width(text.c_str()) <= maxWidth) return text;

  String out = text;
  while (out.length() > 0) {
    String candidate = out + "...";
    if (u8g2Fonts.getUTF8Width(candidate.c_str()) <= maxWidth) {
      return candidate;
    }
    out.remove(out.length() - 1);
  }
  return "...";
}

void drawTopBar(const String& title, const String& rightText) {
  tft.fillRoundRect(6, 6, tft.width() - 12, 28, 6, ST77XX_BLACK);
  u8g2Fonts.setForegroundColor(ST77XX_WHITE);
  u8g2Fonts.setBackgroundColor(ST77XX_BLACK);

  u8g2Fonts.setFont(u8g2_font_7x14_tf);
  u8g2Fonts.setCursor(14, 24);
  u8g2Fonts.print(title);

  if (rightText.length() > 0) {
    int rw = u8g2Fonts.getUTF8Width(rightText.c_str());
    u8g2Fonts.setCursor(tft.width() - rw - 14, 24);
    u8g2Fonts.print(rightText);
  }

  u8g2Fonts.setForegroundColor(ST77XX_BLACK);
  u8g2Fonts.setBackgroundColor(ST77XX_WHITE);
}

void drawCard(int x, int y, int w, int h) {
  tft.drawRoundRect(x, y, w, h, 8, ST77XX_BLACK);
}

void updateDisplay() {
  Serial.printf("[LCD_UPDATE] START reason=%s, mode=%s, millis=%lu\n",
                display_update_reason.c_str(), display_mode.c_str(), millis());

  clearScreenWhite();

  if (display_mode == "clock") {
    tft.setRotation(2);
    clearScreenWhite();

    int w = tft.width();

    if (is_focus_mode_on) {
      unsigned long elapsed = millis() - focus_start_time;
      int minutes = (elapsed / 60000);

      String minStr = String(minutes);

      drawTopBar("FOCUS MODE");
      drawCard(14, 46, w - 28, 150);
      drawCenteredText("집중 시간", 76, u8g2_font_unifont_t_korean2);
      drawCenteredText(minStr, 138, u8g2_font_logisoso50_tf);
      drawCenteredText("MIN", 172, u8g2_font_7x14_tf);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(20, 220);
      u8g2Fonts.print("Stay focused.");
    } else {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo, 10)) {
        char dateStr[24];
        char timeStr[8];

        strftime(dateStr, sizeof(dateStr), "%Y.%m.%d %a", &timeinfo);
        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);

        drawTopBar("CLOCK");
        drawCenteredText(dateStr, 58, u8g2_font_7x14_tf);
        tft.drawLine(24, 70, w - 24, 70, ST77XX_BLACK);
        drawCenteredText(timeStr, 145, u8g2_font_logisoso50_tf);

        u8g2Fonts.setFont(u8g2_font_7x14_tf);
        u8g2Fonts.setCursor(20, 220);
        u8g2Fonts.print("WiFi ");
        u8g2Fonts.print((WiFi.status() == WL_CONNECTED) ? "OK" : "OFF");
      } else {
        drawTopBar("CLOCK");
        drawCard(12, 70, w - 24, 80);
        drawCenteredText("NTP 연결 대기중...", 118, u8g2_font_unifont_t_korean2);
      }
    }
  }
  else if (display_mode == "todo") {
    tft.setRotation(1);
    clearScreenWhite();

    int w = tft.width();
    int h = tft.height();

    String rightInfo = String(current_task_idx + 1) + "/" + String(tasks.size());
    if (tasks.empty()) rightInfo = "0/0";

    drawTopBar("TODO", rightInfo);
    drawCard(8, 42, w - 16, h - 54);

    if (!tasks.empty()) {
      String title = tasks[current_task_idx].title;
      String line1 = ellipsizeText(title, w - 32, u8g2_font_unifont_t_korean2);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(18, 66);
      u8g2Fonts.print("Current Task");

      tft.drawLine(18, 74, w - 18, 74, ST77XX_BLACK);

      u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
      u8g2Fonts.setCursor(18, 108);
      u8g2Fonts.print(line1);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(18, 145);
      u8g2Fonts.print("Long press: complete");
    } else {
      drawCenteredText("할 일 없음", 105, u8g2_font_unifont_t_korean2);
      drawCenteredText("오늘은 좀 쉬어도 됨", 135, u8g2_font_7x14_tf);
    }
  }
  else if (display_mode == "music") {
    tft.setRotation(3);
    clearScreenWhite();

    int w = tft.width();
    int h = tft.height();

    if (music_submenu == 0) {
      drawTopBar("NOW PLAYING", "1/3");
      drawCard(8, 42, w - 16, h - 54);

      String title = ellipsizeText(music_title, w - 32, u8g2_font_unifont_t_korean2);
      String artist = ellipsizeText(music_artist, w - 32, u8g2_font_unifont_t_korean2);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(18, 66);
      u8g2Fonts.print("Title");

      u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
      u8g2Fonts.setCursor(18, 96);
      u8g2Fonts.print(title);

      tft.drawLine(18, 112, w - 18, 112, ST77XX_BLACK);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(18, 138);
      u8g2Fonts.print("Artist");

      u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
      u8g2Fonts.setCursor(18, 168);
      u8g2Fonts.print(artist);
    }
    else if (music_submenu == 1) {
      drawTopBar("LATEST MAIL", "2/3");
      drawCard(8, 42, w - 16, h - 54);

      String subject = ellipsizeText(latestMail.subject, w - 32, u8g2_font_unifont_t_korean2);
      String from = ellipsizeText(latestMail.fromName, w - 32, u8g2_font_unifont_t_korean2);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(18, 66);
      u8g2Fonts.print("Subject");

      u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
      u8g2Fonts.setCursor(18, 96);
      u8g2Fonts.print(subject);

      tft.drawLine(18, 112, w - 18, 112, ST77XX_BLACK);

      u8g2Fonts.setFont(u8g2_font_7x14_tf);
      u8g2Fonts.setCursor(18, 138);
      u8g2Fonts.print("From");

      u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
      u8g2Fonts.setCursor(18, 168);
      u8g2Fonts.print(from);
    }
    else if (music_submenu == 2) {
      drawTopBar("TODAY CAL", "3/3");
      drawCard(8, 42, w - 16, h - 54);

      if (todayEvents.empty()) {
        drawCenteredText("오늘 일정 없음", 115, u8g2_font_unifont_t_korean2);
      } else {
        int y = 72;
        for (size_t i = 0; i < todayEvents.size() && i < 3; i++) {
          String line = todayEvents[i].timeText + " " + todayEvents[i].subject;
          line = ellipsizeText(line, w - 28, u8g2_font_unifont_t_korean2);

          u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
          u8g2Fonts.setCursor(16, y);
          u8g2Fonts.print(line);

          y += 34;
        }
      }
    }
  }

  needs_display_update = false;

  Serial.printf("[LCD_UPDATE] END reason=%s, mode=%s, millis=%lu\n",
                display_update_reason.c_str(), display_mode.c_str(), millis());
}

void showBootMessage(const char* line1, const char* line2) {
  tft.setRotation(2);
  clearScreenWhite();

  drawTopBar("BOOT");
  drawCard(10, 60, tft.width() - 20, 90);

  drawCenteredText(String(line1), 100, u8g2_font_unifont_t_korean2);
  drawCenteredText(String(line2), 128, u8g2_font_7x14_tf);
}

void connect_wifi() {
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  showBootMessage("WiFi 연결됨!", "시간 동기화중..");

  sync_time();
  requestDisplayUpdate("wifi_connected_sync_done");
}

String get_new_access_token() {
  if (WiFi.status() != WL_CONNECTED) return "";

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String token_url = "https://login.microsoftonline.com/common/oauth2/v2.0/token";
  if (http.begin(client, token_url)) {
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String cleaned_token = REFRESH_TOKEN;
    cleaned_token.replace(" ", "");
    cleaned_token.replace("\n", "");
    cleaned_token.replace("\r", "");

    String data = "client_id=" + CLIENT_ID +
                  "&client_secret=" + CLIENT_SECRET +
                  "&grant_type=refresh_token" +
                  "&refresh_token=" + cleaned_token;

    int httpCode = http.POST(data);
    String payload = http.getString();
    http.end();

    if (httpCode == 200) {
      DynamicJsonDocument doc(4096);
      DeserializationError err = deserializeJson(doc, payload);
      if (!err) {
        return doc["access_token"].as<String>();
      }
    }
  }
  return "";
}

void get_todo_tasks(String token) {
  if (token == "") return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String cleaned_list_id = LIST_ID;
  cleaned_list_id.replace(" ", "");

  String url = "https://graph.microsoft.com/v1.0/me/todo/lists/" +
               cleaned_list_id +
               "/tasks?$filter=status%20ne%20%27completed%27";

  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + token);

    if (http.GET() == 200) {
      DynamicJsonDocument doc(8192);
      DeserializationError err = deserializeJson(doc, http.getString());

      if (!err) {
        tasks.clear();
        for (JsonObject task : doc["value"].as<JsonArray>()) {
          tasks.push_back({task["id"].as<String>(), task["title"].as<String>()});
        }

        Serial.printf("[TODO_REFRESH] count=%d, mode=%s, millis=%lu\n",
                      (int)tasks.size(), display_mode.c_str(), millis());

        if (current_task_idx >= (int)tasks.size()) current_task_idx = 0;
        last_task_advance = millis();

        if (display_mode == "todo") {
          requestDisplayUpdate("todo_tasks_refreshed");
        }
      }
    }
    http.end();
  }
}

bool complete_todo_task(String token, String listId, String taskId) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String url = "https://graph.microsoft.com/v1.0/me/todo/lists/" + listId + "/tasks/" + taskId;
  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "application/json");
    int code = http.PATCH("{\"status\": \"completed\"}");
    http.end();
    return code == 200;
  }
  return false;
}

unsigned char h2int(char c) {
  if (c >= '0' && c <= '9') return ((unsigned char)c - '0');
  if (c >= 'a' && c <= 'f') return ((unsigned char)c - 'a' + 10);
  if (c >= 'A' && c <= 'F') return ((unsigned char)c - 'A' + 10);
  return 0;
}

String urlDecode(String str) {
  String decoded = "";
  int len = str.length();
  for (int i = 0; i < len; i++) {
    char c = str.charAt(i);
    if (c == '+') decoded += ' ';
    else if (c == '%' && i + 2 < len) {
      unsigned char code0 = str.charAt(++i);
      unsigned char code1 = str.charAt(++i);
      decoded += (char)((h2int(code0) << 4) | h2int(code1));
    } else {
      decoded += c;
    }
  }
  return decoded;
}

String urlEncode(String str) {
  String encoded = "";
  char c;
  char buf[4];

  for (size_t i = 0; i < str.length(); i++) {
    c = str.charAt(i);

    if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else {
      snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
      encoded += buf;
    }
  }
  return encoded;
}

void handleLongPressComplete() {
  if (display_mode == "todo" && tasks.size() > 0) {
    Serial.printf("[TODO_COMPLETE] idx=%d, title=%s, millis=%lu\n",
                  current_task_idx, tasks[current_task_idx].title.c_str(), millis());

    tft.setRotation(1);
    clearScreenWhite();
    drawTopBar("TODO");
    drawCard(20, 70, tft.width() - 40, 80);
    drawCenteredText("완료!!", 118, u8g2_font_unifont_t_korean2);

    if (complete_todo_task(access_token, LIST_ID, tasks[current_task_idx].id)) {
      get_todo_tasks(access_token);
    }
  }
}

void handleShortClick() {
  if (display_mode == "music") {
    music_submenu = (music_submenu + 1) % 3;

    if (is_token_valid) {
      if (music_submenu == 1) {
        fetchLatestMail(access_token);
        last_mail_update = millis();
      } else if (music_submenu == 2) {
        fetchTodayCalendar(access_token);
        last_calendar_update = millis();
      }
    }

    requestDisplayUpdate("music_submenu_changed");
  }
}

void fetchLatestMail(String token) {
  if (token.isEmpty()) return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String url = "https://graph.microsoft.com/v1.0/me/messages"
               "?$top=1"
               "&$select=subject,from"
               "&$orderby=receivedDateTime%20desc";

  if (!http.begin(client, url)) {
    latestMail.subject = "메일 요청 실패";
    latestMail.fromName = "";
    return;
  }

  http.addHeader("Authorization", "Bearer " + token);
  http.addHeader("Accept", "application/json");

  int code = http.GET();
  String body = http.getString();

  Serial.printf("[MAIL] HTTP code = %d\n", code);
  Serial.println("[MAIL] body:");
  Serial.println(body);

  if (code == 200) {
    DynamicJsonDocument doc(12288);
    DeserializationError err = deserializeJson(doc, body);

    if (!err) {
      JsonArray arr = doc["value"].as<JsonArray>();

      if (!arr.isNull() && arr.size() > 0) {
        JsonObject mail = arr[0];
        latestMail.subject = mail["subject"] | "(제목 없음)";
        latestMail.fromName = mail["from"]["emailAddress"]["address"] | "주소 없음";
      } else {
        latestMail.subject = "메일 없음";
        latestMail.fromName = "";
      }
    } else {
      latestMail.subject = "파싱 실패";
      latestMail.fromName = "";
    }
  } else {
    latestMail.subject = "메일 조회 실패";
    latestMail.fromName = "";
  }

  http.end();
  requestDisplayUpdate("latest_mail_updated");
}

String getTodayStartISO() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) return "";

  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT00:00:00", &timeinfo);
  return String(buf);
}

String getTodayEndISO() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) return "";

  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT23:59:59", &timeinfo);
  return String(buf);
}

String isoToHHMM(const String& iso) {
  int tPos = iso.indexOf('T');
  if (tPos == -1 || iso.length() < tPos + 6) return "--:--";
  return iso.substring(tPos + 1, tPos + 6);
}

void fetchTodayCalendar(String token) {
  if (token == "") return;

  todayEvents.clear();

  String startISO = getTodayStartISO();
  String endISO = getTodayEndISO();
  if (startISO == "" || endISO == "") return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();

  String url = "https://graph.microsoft.com/v1.0/me/calendarView"
               "?startDateTime=" + urlEncode(startISO) +
               "&endDateTime=" + urlEncode(endISO) +
               "&$top=3"
               "&$select=subject,start"
               "&$orderby=start/dateTime";

  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Prefer", "outlook.timezone=\"Asia/Seoul\"");

    int code = http.GET();
    if (code == 200) {
      DynamicJsonDocument doc(16384);
      DeserializationError err = deserializeJson(doc, http.getString());

      if (!err) {
        JsonArray arr = doc["value"].as<JsonArray>();
        for (JsonObject ev : arr) {
          CalendarItem item;
          item.subject = ev["subject"].as<String>();
          if (item.subject.length() == 0) item.subject = "(제목 없음)";

          String startStr = ev["start"]["dateTime"].as<String>();
          item.timeText = isoToHHMM(startStr);

          todayEvents.push_back(item);
        }
      }
    }
    http.end();
  }
}

void check_sensor_and_control() {
  if (!updateIMU()) return;

  float ax = imuData.ax_g;
  float threshold_mode = 0.4f;
  String new_mode = display_mode;

  if (ax > threshold_mode) new_mode = "music";
  else if (ax < -threshold_mode) new_mode = "todo";
  else new_mode = "clock";

  if (new_mode != display_mode) {
    Serial.printf("[MODE_CHANGE] ax=%.3f, old=%s, new=%s, millis=%lu\n",
                  ax, display_mode.c_str(), new_mode.c_str(), millis());

    display_mode = new_mode;

    if (display_mode == "todo") {
      last_task_advance = millis();
    }

    if (display_mode == "music") {
      music_submenu = 0;
    }

    requestDisplayUpdate("imu_mode_changed");
  }
}

void advanceTodoTaskIfNeeded() {
  if (btn.isLongPressed()) return;
  if (display_mode != "todo") return;
  if (tasks.size() <= 1) return;

  unsigned long now = millis();
  if (now - last_task_advance >= TASK_ADVANCE_INTERVAL) {
    current_task_idx = (current_task_idx + 1) % tasks.size();
    last_task_advance = now;
    Serial.printf("[TODO_ADVANCE] new_idx=%d, millis=%lu\n",
                  current_task_idx, millis());
    requestDisplayUpdate("todo_auto_advance");
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(MPU_SDA, MPU_SCL);
  Wire.setClock(400000);
  Wire.setTimeOut(100);

  imu_ok = initIMU();
  if (!imu_ok) Serial.println("IMU init failed!");
  else Serial.println("IMU OK");

  SPI.begin(TFT_SCK, -1, TFT_MOSI, -1);

  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_WHITE);

  u8g2Fonts.begin(tft);
  u8g2Fonts.setFont(u8g2_font_unifont_t_korean2);
  u8g2Fonts.setFontMode(1);
  u8g2Fonts.setFontDirection(0);

  showBootMessage("기기 부팅중...", "WiFi 연결 대기");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  btn.attachLongPressStart(handleLongPressComplete);
  btn.attachClick(handleShortClick);

  connect_wifi();
  server.begin();
  Serial.println("Setup complete.");
}

void loop() {
  unsigned long current_time_ms = millis();
  btn.tick();

  if (current_time_ms - last_gyro_check > (unsigned long)GYRO_CHECK_INTERVAL) {
    check_sensor_and_control();
    last_gyro_check = current_time_ms;
  }

  WiFiClient client = server.available();
  if (client) {
    String request = "";
    unsigned long timeout = millis() + 20;

    while (client.connected() && millis() < timeout) {
      if (client.available()) request += (char)client.read();
      yield();
    }

    Serial.printf("[HTTP_REQUEST] %s\n", request.c_str());

    if (request.indexOf("GET /") != -1) {
      if (request.indexOf("focus?status=on") != -1) {
        is_focus_mode_on = true;
        focus_start_time = millis();
        requestDisplayUpdate("focus_on");
      }
      else if (request.indexOf("focus?status=off") != -1) {
        is_focus_mode_on = false;
        requestDisplayUpdate("focus_off");
      }

      if (request.indexOf("/music") != -1) {
        int tStart = request.indexOf("title=");
        if (tStart != -1) {
          int tEnd = request.indexOf("&", tStart);
          if (tEnd == -1) tEnd = request.indexOf(" ", tStart);

          int aStart = request.indexOf("artist=");
          int aEnd = request.indexOf(" ", aStart);

          if (aStart != -1 && aEnd != -1) {
            music_title = urlDecode(request.substring(tStart + 6, tEnd));
            music_artist = urlDecode(request.substring(aStart + 7, aEnd));

            Serial.printf("[MUSIC_UPDATE] title=%s, artist=%s, mode=%s, millis=%lu\n",
                          music_title.c_str(), music_artist.c_str(),
                          display_mode.c_str(), millis());

            if (display_mode == "music" && music_submenu == 0) {
              requestDisplayUpdate("music_metadata_changed");
            }
          }
        }
      }

      client.println("HTTP/1.1 200 OK\r\n\r\nOK");
    }
    client.stop();
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!is_token_valid || (current_time_ms - last_token_update > TOKEN_UPDATE_INTERVAL)) {
      Serial.printf("[TOKEN_REFRESH] try refresh, millis=%lu\n", millis());
      access_token = get_new_access_token();

      if (access_token != "") {
        is_token_valid = true;
        last_token_update = millis();
        Serial.printf("[TOKEN_REFRESH] success, millis=%lu\n", millis());

        get_todo_tasks(access_token);
        last_todo_update = millis();

        if (display_mode == "music") {
          if (music_submenu == 1) {
            fetchLatestMail(access_token);
            last_mail_update = millis();
          } else if (music_submenu == 2) {
            fetchTodayCalendar(access_token);
            last_calendar_update = millis();
          }
        }
      } else {
        Serial.printf("[TOKEN_REFRESH] failed, millis=%lu\n", millis());
        last_token_update = millis() - TOKEN_UPDATE_INTERVAL + 10000;
      }
    }
    else if (is_token_valid && (current_time_ms - last_todo_update > TODO_UPDATE_INTERVAL)) {
      Serial.printf("[TODO_POLL] periodic refresh, millis=%lu\n", millis());
      get_todo_tasks(access_token);
      last_todo_update = millis();
    }

    if (is_token_valid && display_mode == "music") {
      if (music_submenu == 1 && (current_time_ms - last_mail_update > MAIL_UPDATE_INTERVAL)) {
        fetchLatestMail(access_token);
        last_mail_update = current_time_ms;
        requestDisplayUpdate("mail_refresh");
      }

      if (music_submenu == 2 && (current_time_ms - last_calendar_update > CALENDAR_UPDATE_INTERVAL)) {
        fetchTodayCalendar(access_token);
        last_calendar_update = current_time_ms;
        requestDisplayUpdate("calendar_refresh");
      }
    }
  }

  current_time_ms = millis();
  advanceTodoTaskIfNeeded();

  if (display_mode == "clock") {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
      if (timeinfo.tm_min != last_drawn_minute) {
        Serial.printf("[CLOCK_TICK] minute changed %d -> %d, millis=%lu\n",
                      last_drawn_minute, timeinfo.tm_min, millis());
        last_drawn_minute = timeinfo.tm_min;
        requestDisplayUpdate("clock_minute_changed");
      }
    }
  }

  if (needs_display_update) {
    updateDisplay();
  }
}
