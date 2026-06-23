# 프로젝트 추적 관리

> PalSystem 프로젝트 진행 상황 추적 문서

---

## 목차

1. [GitHub Projects 사용 방법 (무료 권장)](#1-github-projects-사용-방법)
2. [스프린트 계획 템플릿](#2-스프린트-계획-템플릿)
3. [Jira 세팅 방법](#3-jira-세팅-방법)
4. [주간 진행 상황 템플릿](#4-주간-진행-상황-템플릿)
5. [Pal 시스템 전체 태스크 목록](#5-pal-시스템-전체-태스크-목록)

---

## 1. GitHub Projects 사용 방법

> 팀이 소규모(5인 이하)이고 기간이 3주라면 Jira보다 GitHub Projects가 훨씬 빠릅니다.

### 세팅 방법

```
1. GitHub 레포지토리 → Projects 탭 → New Project
2. Board 템플릿 선택 (칸반 방식)
3. 컬럼 구성:

   📋 Backlog | 🔜 Todo | 🔄 In Progress | 👀 In Review | ✅ Done
```

### 컬럼 의미

| 컬럼 | 의미 |
|------|------|
| `📋 Backlog` | 아직 스프린트에 포함되지 않은 전체 태스크 |
| `🔜 Todo` | 이번 스프린트에서 할 태스크 |
| `🔄 In Progress` | 현재 작업 중 |
| `👀 In Review` | PR 리뷰 대기 중 |
| `✅ Done` | 완료 및 머지 완료 |

### Issue와 연동하는 방법

```
Issue 생성 → 오른쪽 사이드바 → Projects → 프로젝트 선택
→ Issue가 자동으로 Board에 카드로 추가됨

PR 머지 시 Issue에 "Closes #번호" 작성하면
→ 머지와 동시에 Issue가 자동으로 Done으로 이동
```

---

## 2. 스프린트 계획 템플릿

> 3주 프로젝트 기준 스프린트 계획

### Week 1 — 기반 시스템

| 태스크 | 담당자 | 우선순위 | 상태 |
|--------|--------|----------|------|
| UE5 프로젝트 생성 및 Git 세팅 | | 🔴 High | |
| APalCharacter 기본 구현 + Health Replication | | 🔴 High | |
| PalAIController + BehaviorTree 기본 설정 | | 🔴 High | |
| Pal Follow System (서버 이동 + 복제) | | 🔴 High | |
| Pal Detection System | | 🟡 Medium | |

### Week 2 — 핵심 시스템

| 태스크 | 담당자 | 우선순위 | 상태 |
|--------|--------|----------|------|
| Pal Combat System (데미지 + RPC) | | 🔴 High | |
| Pal Skill System 기본 구조 | | 🟡 Medium | |
| Pal Capture System (Server RPC + 확률 계산) | | 🔴 High | |
| Pal Spawn System (서버 스폰 + 복제) | | 🔴 High | |
| Animation Replication (State 기반) | | 🟡 Medium | |

### Week 3 — 완성 및 통합

| 태스크 | 담당자 | 우선순위 | 상태 |
|--------|--------|----------|------|
| Pal Status Effect System | | 🟡 Medium | |
| Ownership 시스템 정리 | | 🔴 High | |
| 버그 수정 및 안정화 | | 🔴 High | |
| 팀 시스템 통합 테스트 | | 🔴 High | |
| 최종 멀티플레이어 테스트 (PIE 3인 이상) | | 🔴 High | |

---

## 3. Jira 세팅 방법

> 팀이 Jira를 사용하기로 결정한 경우

### 3-1. 프로젝트 생성

```
1. jira.atlassian.com 접속 → 새 프로젝트
2. Scrum 템플릿 선택
3. 프로젝트 이름: PalSystem
4. 팀원 초대
```

### 3-2. Issue Type 설정

```
Jira의 기본 Issue Type을 프로젝트에 맞게 조정:

Epic    → 대형 시스템 단위 (Pal Follow System, Pal Combat System 등)
Story   → 기능 단위 (플레이어가 Pal에게 Follow 명령을 내릴 수 있다)
Task    → 구현 단위 (APalCharacter 이동 Replication 구현)
Bug     → 버그 리포트
```

### 3-3. Epic 구성 (Pal 시스템 기준)

```
🟣 Epic 1: Pal Character Base
   - APalCharacter C++ 구현
   - Health, State Replication
   - 기본 컴포넌트 구성

🟣 Epic 2: Pal AI System
   - AIController 구현
   - BehaviorTree 구성
   - Blackboard 설계

🟣 Epic 3: Pal Follow System
   - Follow 명령 처리 (Server RPC)
   - 이동 Replication
   - Stay / Return 상태

🟣 Epic 4: Pal Combat System
   - Detection
   - Target Selection
   - 데미지 처리 (Server Only)
   - 피격 애니메이션 (Multicast RPC)

🟣 Epic 5: Pal Capture System
   - 포획볼 Server RPC
   - 포획 확률 계산
   - 포획 결과 Replication

🟣 Epic 6: Pal Spawn System
   - 서버 스폰 로직
   - Ownership 설정
   - Respawn 처리
```

### 3-4. Jira ↔ GitHub 연동 방법

```
Jira → 앱 → GitHub 연동 설치

이후 커밋 메시지에 Jira Issue Key 포함:
feat(pal-character): Add Health Replication [PAL-12]

→ Jira Issue PAL-12에 자동으로 커밋이 연결됨
```

### 3-5. 스프린트 운영 방법

```
스프린트 기간: 1주일
스프린트 이벤트:

월요일: 스프린트 플래닝 (30분)
  - 이번 주 할 태스크 선정
  - 담당자 배정

매일: 데일리 스탠드업 (10분)
  - 어제 한 것
  - 오늘 할 것
  - 블로킹 이슈

금요일: 스프린트 리뷰 (30분)
  - 완료된 기능 시연
  - 다음 스프린트 준비
```

---

## 4. 주간 진행 상황 템플릿

> 매주 금요일 작성

---

### Week N 진행 상황 (YYYY.MM.DD)

**완료된 태스크**

- [ ] 태스크명 (#Issue번호)
- [ ] 태스크명 (#Issue번호)

**진행 중인 태스크**

- [ ] 태스크명 (#Issue번호) — 진행률 NN%

**다음 주 계획**

- [ ] 태스크명
- [ ] 태스크명

**블로킹 이슈**

- 이슈 내용 및 해결 방법

**멀티플레이어 테스트 결과**

| 테스트 항목 | 결과 | 비고 |
|-------------|------|------|
| Health Replication | ✅ / ❌ | |
| Follow Replication | ✅ / ❌ | |
| Combat RPC | ✅ / ❌ | |

---

## 5. Pal 시스템 전체 태스크 목록

> 전체 구현 범위 체크리스트

### Character

- [ ] APalCharacter 기본 클래스 구현
- [ ] Health 변수 Replication
- [ ] PalState Enum Replication
- [ ] OnRep 콜백 구현

### AI

- [ ] APalAIController 구현
- [ ] UBehaviorTree 에셋 연결
- [ ] UBlackboardData 설계
- [ ] 기본 Task / Decorator / Service 구현

### Follow System

- [ ] Follow 명령 Server RPC
- [ ] AI 이동 로직 (서버)
- [ ] 이동 Replication 확인
- [ ] Stay / Return 상태 전환

### Detection

- [ ] 감지 범위 설정
- [ ] 타겟 감지 로직 (서버)
- [ ] Blackboard 타겟 업데이트

### Combat

- [ ] 공격 Server RPC
- [ ] 데미지 계산 (서버)
- [ ] 피격 Multicast RPC
- [ ] 사망 처리 (서버)

### Skill

- [ ] 스킬 기본 구조 설계
- [ ] 스킬 발동 Server RPC
- [ ] 스킬 이펙트 Multicast RPC
- [ ] 쿨타임 Replication

### Capture

- [ ] 포획볼 투척 Server RPC
- [ ] 포획 확률 계산 (서버)
- [ ] 포획 결과 Client RPC
- [ ] 포획된 Pal Ownership 이전

### Spawn

- [ ] 서버 스폰 로직
- [ ] Pal Ownership 설정
- [ ] Respawn 처리
- [ ] 스폰 위치 유효성 검사

### Animation

- [ ] AnimInstance State 연동
- [ ] 이동 애니메이션 Replication
- [ ] 공격 애니메이션 Multicast RPC
- [ ] 사망 애니메이션 처리

### Status

- [ ] 상태이상 구조 설계
- [ ] 상태이상 Replication
- [ ] 상태이상 효과 처리 (서버)
