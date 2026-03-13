# MS To Do & Study Assistant

**MS To Do & Study Assistant**는 ESP32 기반의 커스텀 하드웨어와 Windows PC 클라이언트 앱을 연동하여 데스크탑 환경을 스마트하게 만들어주는 통합 IoT 프로젝트입니다. 

Windows 11의 집중 모드(Focus Session), 현재 재생 중인 미디어 정보, Microsoft To Do 리스트를 OLED 화면에 표시하며, 직관적인 제스처(기기 기울임)를 통해 화면 모드를 전환할 수 있습니다.

---

## 아이디어 다이어그램

<img width="6546" height="3882" alt="드로잉 9 (1)" src="https://github.com/user-attachments/assets/8cda2208-d880-4fb7-9245-b7cddeee51d8" />


## 주요 기능 (Features)

* **Windows 11 집중 모드 동기화 (Focus Session)**
  * PC에서 집중 모드를 켜면 ESP32 화면이 자동으로 스톱워치 모드로 전환되며 진행 시간을 표시합니다.
* **실시간 PC 미디어 연동 (Now Playing)**
  * PC에서 재생 중인 음악/영상의 제목과 아티스트 정보를 실시간으로 기기에 전송됩니다.
* **Microsoft To Do 관리 (Task Management)**
  * Microsoft Graph API를 통해 클라우드에서 할 일 목록을 가져옵니다.
  * 기기의 물리 버튼을 길게 눌러(Long Press) 해당 할 일을 즉시 '완료' 처리할 수 있습니다.
* **모션 인식 화면 전환 (Tilt Control)**
  * 버튼 조작 없이 기기를 좌우로 기울이는 동작(MPU6050 센서)만으로 `시계/집중모드 ↔ To Do 목록 ↔ 음악 정보` 화면으로 전환됩니다.

---

## 시스템 아키텍처 (System Architecture)

본 프로젝트는 하드웨어와 소프트웨어가 결합된 3가지 주요 파트로 구성됩니다.

1. **ESP32 펌웨어 (C++)**: 기기 제어, OLED 출력, MPU6050 센서 인식, 로컬 웹 서버(Port 80) 구동 및 MS Graph API 통신 수행.
2. **Windows 클라이언트 앱 (C#)**: 백그라운드에서 실행되며 PC의 집중 모드 상태와 미디어 재생 상태를 감지하여 ESP32의 로컬 IP로 HTTP GET 요청을 전송.
3. **커스텀 하드웨어 (PCB & 3D Case)**: 깔끔한 데스크탑 배치를 위한 전용 회로 기판과 3D 프린팅 케이스 설계.

---

## 하드웨어 구성 (Hardware)

### 1. 회로도 및 PCB 설계 (PCB Design)

<img width="1385" height="754" alt="PCB 회로 " src="https://github.com/user-attachments/assets/50e71dac-d091-476a-b72f-3a533c3fe0a3" />
<br><br>
<img width="1385" height="754" alt="PCB 회로enl " src="https://github.com/user-attachments/assets/3664ed34-a5c5-4f20-be74-a648c8477876" />


### 2. 3D 모델링 및 케이스 (3D Modeling & Enclosure)

<img width="569" height="782" alt="Back_Side" src="https://github.com/user-attachments/assets/156bc0bb-1df7-4cc6-ae7e-b24771128747" />

---

## 소프트웨어 설정 및 실행 (Software Setup)

### Part 1: ESP32 펌웨어 (Arduino IDE)
1. Arduino IDE에서 `Firmware/` 폴더의 `.ino` 파일을 엽니다.
2. 라이브러리 매니저에서 다음 라이브러리를 설치합니다.
   * `U8g2`, `ArduinoJson`, `MPU6050_light`, `OneButton`
3. 코드 상단의 환경 설정 변수를 본인의 환경에 맞게 수정합니다.
   ```cpp
   const char* WIFI_SSID = "YOUR_WIFI_SSID";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   
   // Microsoft To Do API 인증 정보 (Azure Portal에서 앱 등록 후 발급)
   const String CLIENT_ID = "YOUR_CLIENT_ID";
   const String CLIENT_SECRET = "YOUR_CLIENT_SECRET";
   const String REFRESH_TOKEN = "YOUR_REFRESH_TOKEN";
   const String LIST_ID = "YOUR_LIST_ID";
   ```
4. 보드에 업로드 후 시리얼 모니터에서 **할당된 IP 주소**를 확인합니다. (예: `192.168.0.166`)

### Part 2: Windows 클라이언트 앱 (C# Console)
PC의 상태를 ESP32로 전달해 주는 백그라운드 프로그램입니다. (Windows 11 환경 권장 - 집중 모드 API 지원)
1. Visual Studio를 사용하여 `PC_Client/` 폴더의 프로젝트를 엽니다.
2. `Program.cs` 파일 내의 `espBaseUrl` 변수를 ESP32의 IP 주소로 변경합니다.
   ```csharp
   private readonly string espBaseUrl = "http://192.168.x.x"; // 확인한 ESP32 IP로 변경
   ```
3. 프로젝트를 빌드하고 실행합니다. (시작 프로그램에 등록해두면 PC 부팅 시 자동 실행되어 편리합니다.)

---

## 사용 방법 (How to Use)

1. **전원 켜기:** 기기에 전원을 연결하면 Wi-Fi에 연결하고 NTP 서버를 통해 시간을 동기화합니다.
2. **모드 전환:** 
   * 기기를 책상에 세로로 둠 **시계 모드 (Clock)**
   * 기기를 왼쪽으로 기울임 (가속도 < -0.7) **To Do 리스트 모드**
   * 기기를 오른쪽으로 기울임 (가속도 > 0.7) **음악 정보 모드**
3. **할 일 완료 처리:** `To Do 모드`에서 버튼을 길게 누르면 화면에 "완료!!"가 표시되며, 실제 Microsoft To Do 서버에서도 해당 항목이 완료 처리됩니다.
4. **PC 자동 연동:** C# 클라이언트가 실행 중일 때, PC의 집중 모드를 켜거나 미디어를 재생/변경하면 자동으로 기기 화면에 연동되어 표시됩니다.

---

## 실제 시제품

![KakaoTalk_20260302_140519458](https://github.com/user-attachments/assets/a1f022d8-fecb-4e0a-b0c8-cc4232b140c0)
<br><br>
![KakaoTalk_20260302_140519458_02](https://github.com/user-attachments/assets/2f185d93-00b5-404a-81ab-2275508d5e99)


## 시연 영상
*(썸네일을 클릭하면 영상이 재생됩니다.)*

**배터리 추가**
[![데모 버젼 2](https://i.ytimg.com/vi/3azfnU3NC3o/maxresdefault.jpg)](https://youtube.com/shorts/3azfnU3NC3o)

**최종본**
[![데모 버젼 3](https://i.ytimg.com/vi/Su-UZIvLEWg/maxresdefault.jpg)](https://youtube.com/shorts/Su-UZIvLEWg)

---

**MS ToDo**

<img width="1917" height="1012" alt="Image" src="https://github.com/user-attachments/assets/a8fc63bd-7dd9-4247-87e9-5f89e6044545" />
<br><br/>

**현재 재생곡**

<img width="448" height="220" alt="Image" src="https://github.com/user-attachments/assets/4b39bd39-8018-4b61-ae55-d10bf28bf7d5" />
<br><br>

**집중 섹션**

<img width="587" height="552" alt="Image" src="https://github.com/user-attachments/assets/0f9a61dc-eb94-4708-93b0-424a735f1e77" />

