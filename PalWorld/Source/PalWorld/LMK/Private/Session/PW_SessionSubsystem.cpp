#include "Session/PW_SessionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

const FName UPW_SessionSubsystem::RoomNameSettingKey(TEXT("ROOM_NAME"));
const FName UPW_SessionSubsystem::HostNicknameSettingKey(TEXT("HOST_NICKNAME"));
const FName UPW_SessionSubsystem::ProjectSettingKey(TEXT("PW_GAME_ID"));
const FString UPW_SessionSubsystem::ProjectSettingValue(TEXT("PALWORLD_PROJECT"));

void UPW_SessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RefreshSessionInterface();
	BindSessionDelegates();
}

void UPW_SessionSubsystem::Deinitialize()
{
	ClearSessionDelegates();
	SessionSearch.Reset();
	SessionInterface.Reset();
	Super::Deinitialize();
}

void UPW_SessionSubsystem::CreateListenSession(const FString& RoomName, const FString& HostNickname, int32 MaxPlayers)
{
	if (!RefreshSessionInterface() || !SessionInterface.IsValid())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Create, false, NSLOCTEXT("PWSession", "NoSessionInterface", "Online session interface is not available."));
		return;
	}

	PendingRoomName = RoomName;
	PendingHostNickname = HostNickname;
	PendingMaxPlayers = FMath::Clamp(MaxPlayers, 2, 64);

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		bCreateSessionAfterDestroy = true;
		if (!SessionInterface->DestroySession(NAME_GameSession))
		{
			bCreateSessionAfterDestroy = false;
			BroadcastOperationFinished(
				EPW_SessionOperation::Create,
				false,
				NSLOCTEXT("PWSession", "CreateDestroyStartFailed", "Failed to destroy the existing session before creating a new one."));
		}
		return;
	}

	CreateListenSessionInternal(PendingRoomName, PendingHostNickname, PendingMaxPlayers);
}

void UPW_SessionSubsystem::FindSessions()
{
	if (!RefreshSessionInterface() || !SessionInterface.IsValid())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Find, false, NSLOCTEXT("PWSession", "FindNoSessionInterface", "Online session interface is not available."));
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = IsLanSession();
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(ProjectSettingKey, ProjectSettingValue, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(FName(TEXT("LobbyDistanceFilter")), (int32)3, EOnlineComparisonOp::Equals); // 3 = Worldwide

	OnSessionSearchStateChanged.Broadcast(true);
	if (!SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		OnSessionSearchStateChanged.Broadcast(false);
		BroadcastOperationFinished(EPW_SessionOperation::Find, false, NSLOCTEXT("PWSession", "FindStartFailed", "Failed to start session search."));
	}
}

void UPW_SessionSubsystem::JoinSession(int32 SearchIndex, const FString& PlayerNickname)
{
	if (!RefreshSessionInterface() || !SessionInterface.IsValid() || !SessionSearch.IsValid())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinNoSearch", "No session search result is available."));
		return;
	}

	if (!SessionSearch->SearchResults.IsValidIndex(SearchIndex))
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinInvalidIndex", "Selected session is no longer available."));
		return;
	}

	const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[SearchIndex];
	if (!SearchResult.IsValid())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinInvalidResult", "Selected session is invalid."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWSession] Join session requested. Player=%s Index=%d"), *PlayerNickname, SearchIndex);
	if (!SessionInterface->JoinSession(0, NAME_GameSession, SearchResult))
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinStartFailed", "Failed to start joining session."));
	}
}

void UPW_SessionSubsystem::DestroySession()
{
	if (!RefreshSessionInterface() || !SessionInterface.IsValid())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Destroy, false, NSLOCTEXT("PWSession", "DestroyNoSessionInterface", "Online session interface is not available."));
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
	{
		BroadcastOperationFinished(EPW_SessionOperation::Destroy, true, NSLOCTEXT("PWSession", "DestroyNoSession", "There is no active session."));
		return;
	}

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		BroadcastOperationFinished(EPW_SessionOperation::Destroy, false, NSLOCTEXT("PWSession", "DestroyStartFailed", "Failed to start destroying session."));
	}
}

void UPW_SessionSubsystem::SetGameMapPath(const FString& InGameMapPath)
{
	if (!InGameMapPath.IsEmpty())
	{
		GameMapPath = InGameMapPath;
	}
}

void UPW_SessionSubsystem::BindSessionDelegates()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (!CreateSessionDelegateHandle.IsValid())
	{
		CreateSessionDelegateHandle = SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPW_SessionSubsystem::HandleCreateSessionComplete);
	}

	if (!FindSessionsDelegateHandle.IsValid())
	{
		FindSessionsDelegateHandle = SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPW_SessionSubsystem::HandleFindSessionsComplete);
	}

	if (!JoinSessionDelegateHandle.IsValid())
	{
		JoinSessionDelegateHandle = SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPW_SessionSubsystem::HandleJoinSessionComplete);
	}

	if (!DestroySessionDelegateHandle.IsValid())
	{
		DestroySessionDelegateHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPW_SessionSubsystem::HandleDestroySessionComplete);
	}

	if (!SessionUserInviteAcceptedDelegateHandle.IsValid())
	{
		SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UPW_SessionSubsystem::HandleSessionUserInviteAccepted));
	}
}

void UPW_SessionSubsystem::ClearSessionDelegates()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (CreateSessionDelegateHandle.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.Remove(CreateSessionDelegateHandle);
		CreateSessionDelegateHandle.Reset();
	}

	if (FindSessionsDelegateHandle.IsValid())
	{
		SessionInterface->OnFindSessionsCompleteDelegates.Remove(FindSessionsDelegateHandle);
		FindSessionsDelegateHandle.Reset();
	}

	if (JoinSessionDelegateHandle.IsValid())
	{
		SessionInterface->OnJoinSessionCompleteDelegates.Remove(JoinSessionDelegateHandle);
		JoinSessionDelegateHandle.Reset();
	}

	if (DestroySessionDelegateHandle.IsValid())
	{
		SessionInterface->OnDestroySessionCompleteDelegates.Remove(DestroySessionDelegateHandle);
		DestroySessionDelegateHandle.Reset();
	}

	if (SessionUserInviteAcceptedDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
		SessionUserInviteAcceptedDelegateHandle.Reset();
	}
}

bool UPW_SessionSubsystem::RefreshSessionInterface()
{
	if (SessionInterface.IsValid())
	{
		return true;
	}

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr)
	{
		return false;
	}

	SessionInterface = OnlineSubsystem->GetSessionInterface();
	BindSessionDelegates();
	return SessionInterface.IsValid();
}

bool UPW_SessionSubsystem::IsLanSession() const
{
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	return OnlineSubsystem == nullptr || OnlineSubsystem->GetSubsystemName() == NAME_None || OnlineSubsystem->GetSubsystemName() == FName(TEXT("NULL"));
}

void UPW_SessionSubsystem::BroadcastOperationFinished(EPW_SessionOperation Operation, bool bWasSuccessful, const FText& Message)
{
	OnSessionOperationFinished.Broadcast(Operation, bWasSuccessful, Message);
	UE_LOG(LogTemp, Log, TEXT("[PWSession] Operation=%d Success=%d Message=%s"), static_cast<int32>(Operation), bWasSuccessful ? 1 : 0, *Message.ToString());
}

void UPW_SessionSubsystem::CreateListenSessionInternal(const FString& RoomName, const FString& HostNickname, int32 MaxPlayers)
{
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = IsLanSession();
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.NumPublicConnections = FMath::Max(2, MaxPlayers);
	SessionSettings.Set(RoomNameSettingKey, RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(HostNicknameSettingKey, HostNickname, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(ProjectSettingKey, ProjectSettingValue, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (!SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings))
	{
		BroadcastOperationFinished(EPW_SessionOperation::Create, false, NSLOCTEXT("PWSession", "CreateStartFailed", "Failed to start creating session."));
	}
}

FString UPW_SessionSubsystem::MakeListenTravelUrl() const
{
	return GameMapPath.EndsWith(TEXT("?listen")) ? GameMapPath : FString::Printf(TEXT("%s?listen"), *GameMapPath);
}

void UPW_SessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		BroadcastOperationFinished(EPW_SessionOperation::Create, false, NSLOCTEXT("PWSession", "CreateFailed", "Failed to create session."));
		return;
	}

	BroadcastOperationFinished(EPW_SessionOperation::Create, true, NSLOCTEXT("PWSession", "CreateSucceeded", "Session created."));

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->ServerTravel(MakeListenTravelUrl());
	}
}

void UPW_SessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	OnSessionSearchStateChanged.Broadcast(false);

	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Find, false, NSLOCTEXT("PWSession", "FindFailed", "Failed to find sessions."));
		return;
	}

	for (int32 Index = 0; Index < SessionSearch->SearchResults.Num(); ++Index)
	{
		const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[Index];
		if (!SearchResult.IsValid())
		{
			continue;
		}

		FPW_SessionSearchResult Result;
		Result.SearchIndex = Index;
		Result.PingMs = SearchResult.PingInMs;
		Result.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		Result.CurrentPlayers = Result.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
		SearchResult.Session.SessionSettings.Get(RoomNameSettingKey, Result.RoomName);
		SearchResult.Session.SessionSettings.Get(HostNicknameSettingKey, Result.HostNickname);

		OnSessionSearchResult.Broadcast(Result);
	}

	BroadcastOperationFinished(
		EPW_SessionOperation::Find,
		true,
		FText::Format(
			NSLOCTEXT("PWSession", "FindSucceeded", "Session search completed. {0} room(s) found."),
			FText::AsNumber(SessionSearch->SearchResults.Num())));
}

void UPW_SessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[PWSession] Join completed. Session=%s Result=%d"),
		*SessionName.ToString(),
		static_cast<int32>(Result));

	if (!SessionInterface.IsValid() || Result != EOnJoinSessionCompleteResult::Success)
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinFailed", "Failed to join session."));
		return;
	}

	FString TravelUrl;
	if (!SessionInterface->GetResolvedConnectString(SessionName, TravelUrl) || TravelUrl.IsEmpty())
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinNoUrl", "Failed to resolve session travel URL."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PWSession] Resolved travel URL: %s"), *TravelUrl);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		BroadcastOperationFinished(EPW_SessionOperation::Join, false, NSLOCTEXT("PWSession", "JoinNoController", "Local player controller is not available."));
		return;
	}

	BroadcastOperationFinished(EPW_SessionOperation::Join, true, NSLOCTEXT("PWSession", "JoinSucceeded", "Joined session."));
	PlayerController->ClientTravel(TravelUrl, TRAVEL_Absolute);
}

void UPW_SessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bCreateSessionAfterDestroy)
	{
		bCreateSessionAfterDestroy = false;
		if (bWasSuccessful)
		{
			CreateListenSessionInternal(PendingRoomName, PendingHostNickname, PendingMaxPlayers);
			return;
		}

		BroadcastOperationFinished(
			EPW_SessionOperation::Create,
			false,
			NSLOCTEXT("PWSession", "CreateDestroyFailed", "Failed to destroy the existing session before creating a new one."));
		return;
	}

	BroadcastOperationFinished(
		EPW_SessionOperation::Destroy,
		bWasSuccessful,
		bWasSuccessful ? NSLOCTEXT("PWSession", "DestroySucceeded", "Session destroyed.") : NSLOCTEXT("PWSession", "DestroyFailed", "Failed to destroy session."));
}

void UPW_SessionSubsystem::HandleSessionUserInviteAccepted(
	bool bWasSuccessful,
	int32 ControllerId,
	FUniqueNetIdPtr UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful || !SessionInterface.IsValid() || !InviteResult.IsValid())
	{
		BroadcastOperationFinished(
			EPW_SessionOperation::Join,
			false,
			NSLOCTEXT("PWSession", "InviteInvalid", "The Steam session invite is invalid."));
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		BroadcastOperationFinished(
			EPW_SessionOperation::Join,
			false,
			NSLOCTEXT("PWSession", "InviteSessionExists", "Leave the current session before accepting an invite."));
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[PWSession] Accepted Steam invite. Controller=%d User=%s"),
		ControllerId,
		UserId.IsValid() ? *UserId->ToDebugString() : TEXT("Invalid"));

	if (!SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult))
	{
		BroadcastOperationFinished(
			EPW_SessionOperation::Join,
			false,
			NSLOCTEXT("PWSession", "InviteJoinStartFailed", "Failed to join the invited Steam session."));
	}
}
