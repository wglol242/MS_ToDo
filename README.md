# 🕐 Workspace Assistant

### 이메일, 일정, Focus Mode, 할 일 정보를 통합 연동하는 워크스페이스용 IoT 어시스턴트

<p align="center">
  <img src="https://github.com/user-attachments/assets/056c028f-27b4-4ee8-9e2f-f4abb99aca2c" width="700"/>
</p>

---

## 1. 핵심 정보 요약

| 항목 | 내용 |
|------|------|
| **기간** | 2025.11 ~ 2026.01 |
| **작업 인원** | 1명 |
| **기술 스택** | C++, C#, PCB 설계, Microsoft Graph API |
| **성과** | 소프트웨어 구현과 하드웨어 설계를 연계한 통합 워크스페이스 시스템 제작 |

---

## 2. 주요 기능

### 1) Focus Mode 상태 표시
Windows 측 프로그램에서 집중 모드 상태를 감지하고,  
ESP32 디바이스 화면에 현재 집중 상태와 경과 시간을 표시하도록 구현했습니다.

### 2) 실시간 미디어 정보 표시
PC에서 재생 중인 음악 및 영상의 제목, 아티스트 정보를 수집하여  
ESP32 디바이스 화면에 실시간으로 출력했습니다.

### 3) Microsoft To Do 연동
ESP32가 Wi-Fi를 통해 Microsoft Graph API에 직접 접근하여  
클라우드의 할 일 목록을 불러오고, 버튼 입력으로 완료 처리할 수 있도록 구현했습니다.

### 4) 최신 메일 / 오늘 일정 조회
Microsoft Graph API를 통해 최신 메일 정보와 오늘 일정을 조회하고,  
디바이스 화면에서 간단하게 확인할 수 있도록 구성했습니다.

### 5) 기울기 기반 화면 전환
IMU 센서 값을 활용하여 디바이스를 좌우로 기울이는 동작만으로  
Clock / Todo / Music 화면을 전환할 수 있도록 설계했습니다.

---

## 3. 시스템 아키텍쳐

### ESP32
- Wi-Fi 연결 및 시간 동기화
- Microsoft Graph API 인증 및 데이터 요청
- TFT 디스플레이 UI 출력
- 버튼 입력 처리
- IMU 센서 기반 화면 전환

### Windows 연동 프로그램
- Windows Focus Mode 상태 감지
- 현재 재생 중인 미디어 정보 수집
- ESP32 디바이스로 HTTP 요청 전송

### 커스텀 하드웨어
- ESP32 기반 회로 설계
- TFT 디스플레이 및 센서 연결
- PCB 설계 및 제작
- 3D 프린팅 케이스 설계


---

## 4. 구현 내용

<p align="center">
  <img src="https://github.com/user-attachments/assets/a50b6474-f033-4a5d-bff8-4ce1c31cde4c" width="700"/>
</p>

### ESP32 펌웨어 개발
- TFT 디스플레이 기반 UI 구성
- Wi-Fi 연결 및 NTP 시간 동기화
- Microsoft OAuth 토큰 갱신 처리
- Microsoft To Do 목록 조회 및 완료 처리
- 최신 메일 및 오늘 일정 조회
- 버튼 입력 및 센서 입력 기반 인터랙션 구현
- 로컬 HTTP 서버를 통한 PC 연동

### Windows 연동 프로그램 개발
- Focus Mode 상태 감지
- 미디어 재생 정보 추출
- ESP32에 상태 데이터 전송

### 하드웨어 설계
- ESP32, 디스플레이, 버튼, IMU 센서를 포함한 회로 설계
- PCB 제작 및 조립
- 3D 프린팅 케이스 설계 및 시제품 제작

---

## 5. 실제 결과물

### 시제품 이미지

<p align="center">
  <img src="https://github.com/user-attachments/assets/056c028f-27b4-4ee8-9e2f-f4abb99aca2c" width="600"/>
</p>

### 하드웨어 설계

<p align="center">
  <img src="https://github.com/user-attachments/assets/f09de4df-6953-42f4-8e80-4ee0e3f659db" width="600"/>
</p>

<p align="center">
  <img src="https://github.com/user-attachments/assets/74677c15-c578-4518-a368-5bfefe50a08f" width="600"/>
</p>

### 3D 모델링

<p align="center">
  <img src="https://github.com/user-attachments/assets/156bc0bb-1df7-4cc6-ae7e-b24771128747" width="350"/>
</p>

### 시연 영상

[![유튜브 링크](https://img.youtube.com/vi/xw0jj7G-Dvg/maxresdefault.jpg)](https://youtube.com/shorts/xw0jj7G-Dvg?feature=share)

---

## 5. 기술 스택

- **Firmware / Embedded**: C++, ESP32
- **Desktop Client**: C#
- **API / Cloud**: Microsoft Graph API
- **Communication**: Wi-Fi, HTTP
- **Hardware**: TFT Display, IMU Sensor, PCB 설계
- **Etc.**: 3D Modeling

---

## 6. 프로젝트 요약

Workspace Assistant는  
**ESP32 기반 임베디드 시스템**, **Windows 연동 프로그램**, **Microsoft Graph API**, **커스텀 하드웨어 설계**를 하나의 프로젝트로 통합한 워크스페이스용 IoT 어시스턴트입니다.

단순히 정보를 표시하는 디바이스를 만드는 데 그치지 않고,  
PC의 상태 정보와 클라우드 데이터를 실시간으로 연동하여  
사용자가 **집중 상태, 미디어 정보, 할 일, 메일, 일정**을 하나의 기기에서 직관적으로 확인할 수 있도록 구현했습니다.

또한 소프트웨어 개발뿐 아니라 **회로 설계, PCB 제작, 3D 케이스 설계**까지 직접 설계하여 완성도를 높였습니다. 


