# MS To Do & Study Assistant

### ESP32와 Windows 클라이언트, Microsoft To Do API를 연동한 스마트 데스크 학습 보조 시스템

<p align="center">
  <img src="https://github.com/user-attachments/assets/056c028f-27b4-4ee8-9e2f-f4abb99aca2c" width="700"/>
</p>

---

## 1. 핵심 정보 요약

| 항목 | 내용 |
|------|------|
| **개발 기간** | 2025.11 ~ 2026.01 |
| **프로젝트 유형** | IoT / 임베디드 / API 연동 |
| **주요 역할** | ESP32 펌웨어 개발, Windows 클라이언트 구현, PCB 설계 |
| **기술 스택** | C++, C#, ESP32, Microsoft Graph API, PCB 설계 |
| **핵심 성과** | 하드웨어와 소프트웨어를 통합한 학습 보조 시스템 시제품 구현 |

---

## 2. 주요 기능

- **Windows 11 집중 모드 연동**  
  PC의 집중 모드 상태를 감지하여 ESP32 OLED 화면에 진행 상태를 표시했습니다.

- **실시간 미디어 정보 표시**  
  PC에서 재생 중인 음악 및 영상의 제목과 아티스트 정보를 기기에 실시간으로 출력했습니다.

- **Microsoft To Do 동기화**  
  Microsoft Graph API를 활용하여 클라우드의 할 일 목록을 불러오고, 버튼 입력으로 완료 처리할 수 있도록 구현했습니다.

- **기울기 기반 화면 전환**  
  MPU6050 센서를 활용해 기기를 좌우로 기울이는 동작만으로 화면 모드를 전환할 수 있도록 구성했습니다.

---

## 3. 시스템 아키텍처

본 프로젝트는 다음 3개 파트로 구성된 통합 시스템입니다.

- **ESP32 펌웨어 (C++)**  
  OLED 출력, 센서 입력 처리, 버튼 인터랙션, 로컬 서버 구동, API 통신 담당

- **Windows 클라이언트 앱 (C#)**  
  PC의 집중 모드 및 미디어 재생 상태를 감지하고 ESP32로 데이터 전송

- **커스텀 하드웨어 (PCB & 3D Case)**  
  데스크탑 환경에 최적화된 전용 회로 및 외형 설계

<p align="center">
  <img src="https://github.com/user-attachments/assets/8cda2208-d880-4fb7-9245-b7cddeee51d8" width="700"/>
</p>

---

## 4. 구현 내용

### ESP32 펌웨어 개발
- OLED 기반 UI 구성
- 버튼 입력 및 기울기 센서 기반 인터랙션 구현
- Wi-Fi 연결 및 시간 동기화
- Microsoft To Do 데이터 표시 및 상태 반영

### Windows 클라이언트 개발
- Windows 11 집중 모드 상태 감지
- 미디어 재생 정보 수집
- ESP32 디바이스로 HTTP 요청 전송

### 하드웨어 설계
- ESP32, OLED, MPU6050 기반 회로 구성
- PCB 설계 및 제작
- 3D 프린팅 케이스 설계로 실제 데스크 환경에 맞는 시제품 제작

---

## 5. 실제 결과물 / 시연 영상

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
[![유튜브 링크](https://i.ytimg.com/vi/moYDjGIwqeQ/maxresdefault.jpg)](https://youtube.com/shorts/moYDjGIwqeQ)

---

## 6. 기술 스택

- **Firmware / Embedded**: C++, ESP32
- **Desktop Client**: C#
- **API / Cloud**: Microsoft Graph API
- **Hardware**: PCB 설계, OLED, MPU6050
- **Etc.**: 3D Modeling

---

## 7. 프로젝트 요약

MS To Do & Study Assistant는  
**임베디드 시스템, Windows 클라이언트, 외부 API 연동, PCB 설계**를 하나의 프로젝트로 통합한 IoT 기반 학습 보조 시스템입니다.

단순한 기능 구현을 넘어,  
소프트웨어와 하드웨어를 함께 설계하고 실제 시제품으로 구현했다는 점에서 의미가 있는 프로젝트입니다.
