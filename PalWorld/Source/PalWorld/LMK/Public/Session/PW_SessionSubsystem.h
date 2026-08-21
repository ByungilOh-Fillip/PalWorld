#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PW_SessionSubsystem.generated.h"

UENUM(BlueprintType)
enum class EPW_SessionOperation : uint8
{
	Create,
	Find,
	Join,
	Destroy
};

USTRUCT(BlueprintType)
struct FPW_SessionSearchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session")
	FString RoomName;

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session")
	FString HostNickname;

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session")
	int32 PingMs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PW|Session")
	int32 SearchIndex = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPW_OnSessionOperationFinished, EPW_SessionOperation, Operation, bool, bWasSuccessful, FText, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPW_OnSessionSearchResult, const FPW_SessionSearchResult&, SearchResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPW_OnSessionSearchStateChanged, bool, bSearching);

UCLASS()
class PALWORLD_API UPW_SessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "PW|Session")
	void CreateListenSession(const FString& RoomName, const FString& HostNickname, int32 MaxPlayers);

	UFUNCTION(BlueprintCallable, Category = "PW|Session")
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "PW|Session")
	void JoinSession(int32 SearchIndex, const FString& PlayerNickname);

	UFUNCTION(BlueprintCallable, Category = "PW|Session")
	void DestroySession();

	UFUNCTION(BlueprintCallable, Category = "PW|Session")
	void SetGameMapPath(const FString& InGameMapPath);

	UFUNCTION(BlueprintPure, Category = "PW|Session")
	FString GetGameMapPath() const { return GameMapPath; }

	UPROPERTY(BlueprintAssignable, Category = "PW|Session")
	FPW_OnSessionOperationFinished OnSessionOperationFinished;

	UPROPERTY(BlueprintAssignable, Category = "PW|Session")
	FPW_OnSessionSearchResult OnSessionSearchResult;

	UPROPERTY(BlueprintAssignable, Category = "PW|Session")
	FPW_OnSessionSearchStateChanged OnSessionSearchStateChanged;

private:
	static const FName RoomNameSettingKey;
	static const FName HostNicknameSettingKey;
	static const FName ProjectSettingKey;
	static const FString ProjectSettingValue;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

	FString GameMapPath = TEXT("/Game/_Private/LMK/Levels/Palworld");
	FString PendingRoomName;
	FString PendingHostNickname;
	int32 PendingMaxPlayers = 4;
	bool bCreateSessionAfterDestroy = false;

	void BindSessionDelegates();
	void ClearSessionDelegates();
	bool RefreshSessionInterface();
	bool IsLanSession() const;
	void BroadcastOperationFinished(EPW_SessionOperation Operation, bool bWasSuccessful, const FText& Message);
	void CreateListenSessionInternal(const FString& RoomName, const FString& HostNickname, int32 MaxPlayers);
	FString MakeListenTravelUrl() const;

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
};
