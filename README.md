# Workspace Assistant

### 이메일, 일정, Focus Mode, 할 일 정보를 통합 연동하는 워크스페이스용 IoT 어시스턴트

<p align="center">
  <img src="https://github.com/user-attachments/assets/056c028f-27b4-4ee8-9e2f-f4abb99aca2c" width="700"/>
</p>

> **Windows PC와 ESP32 디바이스를 연동하여 집중 상태, 미디어 정보, 할 일, 메일, 일정을 실시간으로 확인할 수 있도록 구현한 워크스페이스용 IoT 시스템**

---

## 1. 프로젝트 개요

Workspace Assistant는  
**ESP32 기반 임베디드 디바이스**, **Windows 연동 프로그램**, **Microsoft Graph API**, **커스텀 하드웨어 설계**를 결합한 통합형 워크스페이스 어시스턴트입니다.

단순히 정보를 표시하는 수준을 넘어,  
PC의 상태와 클라우드 데이터를 하나의 디바이스에서 확인할 수 있도록 구성했으며,  
실제 데스크 환경에서 사용할 수 있는 형태의 **시제품 제작**까지 진행했습니다.

---

## 2. 핵심 정보 요약

| 항목 | 내용 |
|------|------|
| **개발 기간** | 2025.11 ~ 2026.01 |
| **프로젝트 유형** | IoT / 임베디드 / API 연동 |
| **주요 역할** | ESP32 펌웨어 개발, Windows 연동 프로그램 구현, PCB 설계 |
| **기술 스택** | C++, C#, ESP32, Microsoft Graph API, Wi-Fi, HTTP |
| **핵심 성과** | 일정, 메일, 할 일, 집중 상태를 하나의 디바이스로 통합 확인 가능한 워크스페이스 시스템 구현 |

---

## 3. 주요 기능

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

## 4. 시스템 구성

본 프로젝트는 크게 다음 3개 파트로 구성됩니다.

### ESP32 펌웨어
- Wi-Fi 연결 및 시간 동기화
- Microsoft Graph API 인증 및 데이터 요청
- TFT 디스플레이 UI 출력
- 버튼 입력 처리
- IMU 센서 기반 화면 전환
- 로컬 HTTP 서버 구동

### Windows 연동 프로그램
- Windows Focus Mode 상태 감지
- 현재 재생 중인 미디어 정보 수집
- ESP32 디바이스로 HTTP 요청 전송

### 커스텀 하드웨어
- ESP32 기반 회로 설계
- TFT 디스플레이 및 센서 연결
- PCB 설계 및 제작
- 3D 프린팅 케이스 설계

<p align="center">
  <img src="https://github.com/user-attachments/assets/a50b6474-f033-4a5d-bff8-4ce1c31cde4c" width="700"/>
</p>

---

## 5. 구현 내용

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

## 6. 실제 결과물

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

## 7. 기술 스택

- **Firmware / Embedded**: C++, ESP32
- **Desktop Client**: C#
- **API / Cloud**: Microsoft Graph API
- **Communication**: Wi-Fi, HTTP
- **Hardware**: TFT Display, IMU Sensor, PCB 설계
- **Etc.**: 3D Modeling

---

## 8. 프로젝트 의의

이 프로젝트는 단순한 임베디드 장치 제작이 아니라,  
**클라우드 API 연동**, **PC 상태 감지**, **하드웨어 설계**, **실제 시제품 구현**까지 포함한 통합형 시스템 개발 프로젝트입니다.

특히 소프트웨어와 하드웨어를 분리하지 않고 하나의 사용자 경험으로 연결했다는 점에서 의미가 있으며,  
실제 데스크 환경에서 활용 가능한 형태로 구현했다는 점이 강점입니다.
