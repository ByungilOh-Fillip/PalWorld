#pragma once
#include "Logging/LogCategory.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIntegrated, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGameplay, Log, All);

/* NOTE : ... -> 가변인자 선언 (갯수를 정해두지 않고 매개변수를 받겠다.) 
 *		 ##__VA_ARGS__  가변인자 갯수를 자동으로 처리해 에러가 나지 않게 정리
 *		 __FUNCTION__ 이 메크로가 위치한 함수의 이름을 반환
 *		 __LINE__ 에러가 발생한 라인을 출력
 */ 

// 게임플레이 전용 로그
#define LOG_GAME(Format, ...)    UE_LOG(LogGameplay, Log, TEXT(Format), ##__VA_ARGS__)

// 호출한 [클래스::함수이름]과 [라인 넘버]를 자동으로 포함하는 디버그용 로그
#define LOG_TRACE_INFO(Format, ...) UE_LOG(LogIntegrated, Log, TEXT("[%s::%d] " Format), *FString(__FUNCTION__), __LINE__, ##__VA_ARGS__)
#define LOG_TRACE_WARN(Format, ...) UE_LOG(LogIntegrated, Warning, TEXT("[%s::%d] " Format), *FString(__FUNCTION__), __LINE__, ##__VA_ARGS__)
#define LOG_TRACE_ERROR(Format, ...) UE_LOG(LogIntegrated, Error, TEXT("[%s::%d] " Format), *FString(__FUNCTION__), __LINE__, ##__VA_ARGS__)

// 확장을 위해 생성 - 비워둠.
class LogManager
{
	
public:
	// 화면과 로그에 동시에 띄우고 싶을 때 사용하는 정적 헬퍼 함수
	static void LogAndScreen(const FString& Message, float DisplayTime = 5.f, FColor Color = FColor::Cyan);
	
};