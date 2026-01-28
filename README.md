# Temperature Monitoring Embedded System

라즈베리파이를 기반으로 온도 센서를 사용해 환경 상태를 판단하고  
LCD, LED, 서보모터로 결과를 출력하는 임베디드 제어 시스템입니다.

## 프로젝트 개요
- 온도 센서 값을 주기적으로 읽어 상태를 판단
- 정상 / 주의 / 위험 단계로 구분
- 단계별로 LED, LCD, 모터 동작 제어

## 시스템 구성
- Input: 온도 센서(DHT)
- Output:
  - LCD: 온도 및 상태 표시
  - LED: 상태별 시각적 알림
  - Servo Motor: 위험 시 물리적 동작

## 동작 방식
1. 온도 센서 데이터 수신
2. 임계값 기준 상태 판단
3. LCD에 온도 및 상태 출력
4. LED 및 모터 동작 제어

## 개발 환경
- Language: C
- Platform: Raspberry Pi
- Library: wiringPi, softPwm
