#include "Commands/UnrealMCPEditorCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "ImageUtils.h"
#include "HighResScreenshot.h"
#include "Engine/GameViewportClient.h"
#include "Misc/FileHelper.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "EditorSubsystem.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "ScopedTransaction.h"

FUnrealMCPEditorCommands::FUnrealMCPEditorCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    // Actor manipulation commands
    if (CommandType == TEXT("get_actors_in_level"))
    {
        return HandleGetActorsInLevel(Params);
    }
    else if (CommandType == TEXT("find_actors_by_name"))
    {
        return HandleFindActorsByName(Params);
    }
    else if (CommandType == TEXT("spawn_actor") || CommandType == TEXT("create_actor"))
    {
        if (CommandType == TEXT("create_actor"))
        {
            UE_LOG(LogTemp, Warning, TEXT("'create_actor' command is deprecated and will be removed in a future version. Please use 'spawn_actor' instead."));
        }
        return HandleSpawnActor(Params);
    }
    else if (CommandType == TEXT("delete_actor"))
    {
        return HandleDeleteActor(Params);
    }
    else if (CommandType == TEXT("set_actor_transform"))
    {
        return HandleSetActorTransform(Params);
    }
    else if (CommandType == TEXT("get_actor_properties"))
    {
        return HandleGetActorProperties(Params);
    }
    else if (CommandType == TEXT("set_actor_property"))
    {
        return HandleSetActorProperty(Params);
    }
    // Blueprint actor spawning
    else if (CommandType == TEXT("spawn_blueprint_actor"))
    {
        return HandleSpawnBlueprintActor(Params);
    }
    // Editor viewport commands
    else if (CommandType == TEXT("focus_viewport"))
    {
        return HandleFocusViewport(Params);
    }
    else if (CommandType == TEXT("take_screenshot"))
    {
        return HandleTakeScreenshot(Params);
    }
    // Landscape commands
    else if (CommandType == TEXT("list_landscapes"))
    {
        return HandleListLandscapes(Params);
    }
    else if (CommandType == TEXT("get_landscape_layers"))
    {
        return HandleGetLandscapeLayers(Params);
    }
    else if (CommandType == TEXT("create_landscape_paint_setup"))
    {
        return HandleCreateLandscapePaintSetup(Params);
    }
    else if (CommandType == TEXT("paint_landscape_layer_at_location"))
    {
        return HandlePaintLandscapeLayerAtLocation(Params);
    }
    else if (CommandType == TEXT("paint_landscape_open_world_biome"))
    {
        return HandlePaintLandscapeOpenWorldBiome(Params);
    }
    else if (CommandType == TEXT("execute_console_command"))
    {
        return HandleExecuteConsoleCommand(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown editor command: %s"), *CommandType));
}

static ALandscapeProxy* FindLandscapeProxyByName(UWorld* World, const FString& LandscapeName)
{
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscapeProxy::StaticClass(), LandscapeActors);

    for (AActor* Actor : LandscapeActors)
    {
        ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Actor);
        if (!Proxy)
        {
            continue;
        }

        if (LandscapeName.IsEmpty() || Proxy->GetName() == LandscapeName || Proxy->GetActorLabel() == LandscapeName)
        {
            return Proxy;
        }
    }

    return nullptr;
}

static TSharedPtr<FJsonObject> LandscapeProxyToJson(ALandscapeProxy* Proxy)
{
    TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
    Obj->SetStringField(TEXT("name"), Proxy ? Proxy->GetName() : TEXT(""));
    Obj->SetStringField(TEXT("label"), Proxy ? Proxy->GetActorLabel() : TEXT(""));
    Obj->SetStringField(TEXT("class"), Proxy ? Proxy->GetClass()->GetName() : TEXT(""));
    Obj->SetStringField(TEXT("path"), Proxy ? Proxy->GetPathName() : TEXT(""));
    Obj->SetStringField(TEXT("landscape_material"), Proxy && Proxy->GetLandscapeMaterial() ? Proxy->GetLandscapeMaterial()->GetPathName() : TEXT("None"));
    return Obj;
}

static FString SanitizeAssetNameSegment(const FString& Value)
{
    FString Result = Value;
    const TCHAR* InvalidChars = TEXT(" .,/\\:;!@#$%^&*()+={}[]|'\"<>?");
    for (const TCHAR* Char = InvalidChars; *Char; ++Char)
    {
        Result.ReplaceCharInline(*Char, TEXT('_'));
    }
    return Result.IsEmpty() ? TEXT("Layer") : Result;
}

template <typename ObjectType>
static ObjectType* LoadOrCreateAsset(const FString& PackageName, const FString& AssetName)
{
    if (ObjectType* ExistingAsset = LoadObject<ObjectType>(nullptr, *PackageName))
    {
        return ExistingAsset;
    }

    UPackage* Package = CreatePackage(*PackageName);
    ObjectType* NewAsset = NewObject<ObjectType>(Package, *AssetName, RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(NewAsset);
    Package->MarkPackageDirty();
    return NewAsset;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    TArray<TSharedPtr<FJsonValue>> ActorArray;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            ActorArray.Add(FUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("actors"), ActorArray);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params)
{
    FString Pattern;
    if (!Params->TryGetStringField(TEXT("pattern"), Pattern))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'pattern' parameter"));
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    TArray<TSharedPtr<FJsonValue>> MatchingActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(Pattern))
        {
            MatchingActors.Add(FUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("actors"), MatchingActors);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSpawnActor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActorType;
    if (!Params->TryGetStringField(TEXT("type"), ActorType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'type' parameter"));
    }

    // Get actor name (required parameter)
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Get optional transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);
    FVector Scale(1.0f, 1.0f, 1.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    }
    if (Params->HasField(TEXT("scale")))
    {
        Scale = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale"));
    }

    // Create the actor based on type
    AActor* NewActor = nullptr;
    UWorld* World = GEditor->GetEditorWorldContext().World();

    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    // Check if an actor with this name already exists
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor with name '%s' already exists"), *ActorName));
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *ActorName;

    if (ActorType == TEXT("StaticMeshActor"))
    {
        NewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("PointLight"))
    {
        NewActor = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("SpotLight"))
    {
        NewActor = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("DirectionalLight"))
    {
        NewActor = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("CameraActor"))
    {
        NewActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Location, Rotation, SpawnParams);
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown actor type: %s"), *ActorType));
    }

    if (NewActor)
    {
        // Set scale (since SpawnActor only takes location and rotation)
        FTransform Transform = NewActor->GetTransform();
        Transform.SetScale3D(Scale);
        NewActor->SetActorTransform(Transform);

        // Return the created actor's details
        return FUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create actor"));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleDeleteActor(const TSharedPtr<FJsonObject>& Params)
{
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            // Store actor info before deletion for the response
            TSharedPtr<FJsonObject> ActorInfo = FUnrealMCPCommonUtils::ActorToJsonObject(Actor);
            
            // Delete the actor
            Actor->Destroy();
            
            TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
            ResultObj->SetObjectField(TEXT("deleted_actor"), ActorInfo);
            return ResultObj;
        }
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Get transform parameters
    FTransform NewTransform = TargetActor->GetTransform();

    if (Params->HasField(TEXT("location")))
    {
        NewTransform.SetLocation(FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location")));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        NewTransform.SetRotation(FQuat(FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"))));
    }
    if (Params->HasField(TEXT("scale")))
    {
        NewTransform.SetScale3D(FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale")));
    }

    // Set the new transform
    TargetActor->SetActorTransform(NewTransform);

    // Return updated actor info
    return FUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true);
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetActorProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Always return detailed properties for this command
    return FUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true);
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetActorProperty(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Get property name
    FString PropertyName;
    if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_name' parameter"));
    }

    // Get property value
    if (!Params->HasField(TEXT("property_value")))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_value' parameter"));
    }
    
    TSharedPtr<FJsonValue> PropertyValue = Params->Values.FindRef(TEXT("property_value"));
    
    // Set the property using our utility function
    FString ErrorMessage;
    if (FUnrealMCPCommonUtils::SetObjectProperty(TargetActor, PropertyName, PropertyValue, ErrorMessage))
    {
        // Property set successfully
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("actor"), ActorName);
        ResultObj->SetStringField(TEXT("property"), PropertyName);
        ResultObj->SetBoolField(TEXT("success"), true);
        
        // Also include the full actor details
        ResultObj->SetObjectField(TEXT("actor_details"), FUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true));
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(ErrorMessage);
    }
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    // Find the blueprint
    if (BlueprintName.IsEmpty())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint name is empty"));
    }

    FString Root      = TEXT("/Game/Blueprints/");
    FString AssetPath = Root + BlueprintName;

    if (!FPackageName::DoesPackageExist(AssetPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint '%s' not found – it must reside under /Game/Blueprints"), *BlueprintName));
    }

    UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);
    FVector Scale(1.0f, 1.0f, 1.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    }
    if (Params->HasField(TEXT("scale")))
    {
        Scale = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale"));
    }

    // Spawn the actor
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(Location);
    SpawnTransform.SetRotation(FQuat(Rotation));
    SpawnTransform.SetScale3D(Scale);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *ActorName;

    AActor* NewActor = World->SpawnActor<AActor>(Blueprint->GeneratedClass, SpawnTransform, SpawnParams);
    if (NewActor)
    {
        return FUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn blueprint actor"));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleFocusViewport(const TSharedPtr<FJsonObject>& Params)
{
    // Get target actor name if provided
    FString TargetActorName;
    bool HasTargetActor = Params->TryGetStringField(TEXT("target"), TargetActorName);

    // Get location if provided
    FVector Location(0.0f, 0.0f, 0.0f);
    bool HasLocation = false;
    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
        HasLocation = true;
    }

    // Get distance
    float Distance = 1000.0f;
    if (Params->HasField(TEXT("distance")))
    {
        Distance = Params->GetNumberField(TEXT("distance"));
    }

    // Get orientation if provided
    FRotator Orientation(0.0f, 0.0f, 0.0f);
    bool HasOrientation = false;
    if (Params->HasField(TEXT("orientation")))
    {
        Orientation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("orientation"));
        HasOrientation = true;
    }

    // Get the active viewport
    FLevelEditorViewportClient* ViewportClient = (FLevelEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
    if (!ViewportClient)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get active viewport"));
    }

    // If we have a target actor, focus on it
    if (HasTargetActor)
    {
        // Find the actor
        AActor* TargetActor = nullptr;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName() == TargetActorName)
            {
                TargetActor = Actor;
                break;
            }
        }

        if (!TargetActor)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *TargetActorName));
        }

        // Focus on the actor
        ViewportClient->SetViewLocation(TargetActor->GetActorLocation() - FVector(Distance, 0.0f, 0.0f));
    }
    // Otherwise use the provided location
    else if (HasLocation)
    {
        ViewportClient->SetViewLocation(Location - FVector(Distance, 0.0f, 0.0f));
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Either 'target' or 'location' must be provided"));
    }

    // Set orientation if provided
    if (HasOrientation)
    {
        ViewportClient->SetViewRotation(Orientation);
    }

    // Force viewport to redraw
    ViewportClient->Invalidate();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleTakeScreenshot(const TSharedPtr<FJsonObject>& Params)
{
    // Get file path parameter
    FString FilePath;
    if (!Params->TryGetStringField(TEXT("filepath"), FilePath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'filepath' parameter"));
    }
    
    // Ensure the file path has a proper extension
    if (!FilePath.EndsWith(TEXT(".png")))
    {
        FilePath += TEXT(".png");
    }

    // Get the active viewport
    if (GEditor && GEditor->GetActiveViewport())
    {
        FViewport* Viewport = GEditor->GetActiveViewport();
        TArray<FColor> Bitmap;
        FIntRect ViewportRect(0, 0, Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y);
        
        if (Viewport->ReadPixels(Bitmap, FReadSurfaceDataFlags(), ViewportRect))
        {
            TArray<uint8> CompressedBitmap;
            FImageUtils::CompressImageArray(Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y, Bitmap, CompressedBitmap);
            
            if (FFileHelper::SaveArrayToFile(CompressedBitmap, *FilePath))
            {
                TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
                ResultObj->SetStringField(TEXT("filepath"), FilePath);
                return ResultObj;
            }
        }
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to take screenshot"));
} 

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleListLandscapes(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscapeProxy::StaticClass(), LandscapeActors);

    TArray<TSharedPtr<FJsonValue>> Landscapes;
    for (AActor* Actor : LandscapeActors)
    {
        if (ALandscapeProxy* Proxy = Cast<ALandscapeProxy>(Actor))
        {
            Landscapes.Add(MakeShared<FJsonValueObject>(LandscapeProxyToJson(Proxy)));
        }
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("landscapes"), Landscapes);
    ResultObj->SetNumberField(TEXT("count"), Landscapes.Num());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetLandscapeLayers(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FString LandscapeName;
    Params->TryGetStringField(TEXT("landscape_name"), LandscapeName);

    ALandscapeProxy* Proxy = FindLandscapeProxyByName(World, LandscapeName);
    if (!Proxy)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Landscape not found: %s"), LandscapeName.IsEmpty() ? TEXT("<any>") : *LandscapeName));
    }

    ULandscapeInfo* LandscapeInfo = Proxy->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Landscape has no LandscapeInfo"));
    }

    TArray<TSharedPtr<FJsonValue>> Layers;
#if WITH_EDITORONLY_DATA
    for (const FLandscapeInfoLayerSettings& LayerSettings : LandscapeInfo->Layers)
    {
        TSharedPtr<FJsonObject> LayerObj = MakeShared<FJsonObject>();
        LayerObj->SetStringField(TEXT("name"), LayerSettings.GetLayerName().ToString());
        LayerObj->SetStringField(TEXT("layer_info"), LayerSettings.LayerInfoObj ? LayerSettings.LayerInfoObj->GetPathName() : TEXT("None"));
        LayerObj->SetStringField(TEXT("owner"), LayerSettings.Owner.IsValid() ? LayerSettings.Owner->GetName() : TEXT(""));
        Layers.Add(MakeShared<FJsonValueObject>(LayerObj));
    }
#endif

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetObjectField(TEXT("landscape"), LandscapeProxyToJson(Proxy));
    ResultObj->SetArrayField(TEXT("layers"), Layers);
    ResultObj->SetNumberField(TEXT("count"), Layers.Num());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleCreateLandscapePaintSetup(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FString LandscapeName;
    Params->TryGetStringField(TEXT("landscape_name"), LandscapeName);

    FString FolderPath = TEXT("/Game/_Private/LMK/LandscapePaint");
    Params->TryGetStringField(TEXT("folder_path"), FolderPath);
    FolderPath.RemoveFromEnd(TEXT("/"));

    FString MaterialName = TEXT("M_MCP_LandscapePaint");
    Params->TryGetStringField(TEXT("material_name"), MaterialName);
    MaterialName = SanitizeAssetNameSegment(MaterialName);

    TArray<FString> LayerNames;
    const TArray<TSharedPtr<FJsonValue>>* LayerNameValues = nullptr;
    if (Params->TryGetArrayField(TEXT("layer_names"), LayerNameValues))
    {
        for (const TSharedPtr<FJsonValue>& Value : *LayerNameValues)
        {
            FString LayerName;
            if (Value.IsValid() && Value->TryGetString(LayerName) && !LayerName.IsEmpty())
            {
                LayerNames.Add(SanitizeAssetNameSegment(LayerName));
            }
        }
    }

    if (LayerNames.Num() == 0)
    {
        LayerNames = { TEXT("Grass"), TEXT("Dirt") };
    }

    ALandscapeProxy* Proxy = FindLandscapeProxyByName(World, LandscapeName);
    if (!Proxy)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Landscape not found: %s"), LandscapeName.IsEmpty() ? TEXT("<any>") : *LandscapeName));
    }

    ULandscapeInfo* LandscapeInfo = Proxy->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Landscape has no LandscapeInfo"));
    }

    const FString MaterialPackageName = FolderPath + TEXT("/") + MaterialName;
    UMaterial* Material = LoadOrCreateAsset<UMaterial>(MaterialPackageName, MaterialName);
    if (!Material)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create landscape material"));
    }

    if (Material->GetExpressionCollection().Expressions.Num() == 0)
    {
        Material->Modify();

        UMaterialExpressionLandscapeLayerBlend* LayerBlend = NewObject<UMaterialExpressionLandscapeLayerBlend>(Material);
        LayerBlend->Material = Material;
        LayerBlend->Desc = TEXT("MCP generated Landscape paint layer blend");
        LayerBlend->MaterialExpressionEditorX = -400;
        LayerBlend->MaterialExpressionEditorY = 0;

        const TArray<FVector> DefaultColors = {
            FVector(0.18f, 0.42f, 0.12f),
            FVector(0.42f, 0.28f, 0.12f),
            FVector(0.18f, 0.24f, 0.30f),
            FVector(0.55f, 0.50f, 0.42f)
        };

        for (int32 Index = 0; Index < LayerNames.Num(); ++Index)
        {
            FLayerBlendInput& Input = LayerBlend->Layers.AddDefaulted_GetRef();
            Input.LayerName = FName(*LayerNames[Index]);
            Input.BlendType = LB_WeightBlend;
            Input.PreviewWeight = Index == 0 ? 1.0f : 0.0f;
            Input.ConstLayerInput = DefaultColors[Index % DefaultColors.Num()];
        }

        LayerBlend->UpdateMaterialExpressionGuid(true, true);
        Material->GetExpressionCollection().AddExpression(LayerBlend);

        if (UMaterialEditorOnlyData* EditorOnlyData = Material->GetEditorOnlyData())
        {
            EditorOnlyData->BaseColor.Expression = LayerBlend;

            UMaterialExpressionConstant* Roughness = NewObject<UMaterialExpressionConstant>(Material);
            Roughness->Material = Material;
            Roughness->R = 1.0f;
            Roughness->Desc = TEXT("Roughness fixed high by MCP");
            Roughness->MaterialExpressionEditorX = -400;
            Roughness->MaterialExpressionEditorY = 220;
            Roughness->UpdateMaterialExpressionGuid(true, true);
            Material->GetExpressionCollection().AddExpression(Roughness);
            EditorOnlyData->Roughness.Expression = Roughness;

            UMaterialExpressionConstant* Specular = NewObject<UMaterialExpressionConstant>(Material);
            Specular->Material = Material;
            Specular->R = 0.0f;
            Specular->Desc = TEXT("Specular fixed low by MCP");
            Specular->MaterialExpressionEditorX = -400;
            Specular->MaterialExpressionEditorY = 320;
            Specular->UpdateMaterialExpressionGuid(true, true);
            Material->GetExpressionCollection().AddExpression(Specular);
            EditorOnlyData->Specular.Expression = Specular;
        }

        Material->PreEditChange(nullptr);
        Material->PostEditChange();
        Material->MarkPackageDirty();
    }

    TArray<TSharedPtr<FJsonValue>> CreatedLayerInfos;
    for (const FString& LayerName : LayerNames)
    {
        const FString LayerInfoName = TEXT("LI_") + SanitizeAssetNameSegment(LayerName);
        const FString LayerInfoPackageName = FolderPath + TEXT("/") + LayerInfoName;
        ULandscapeLayerInfoObject* LayerInfo = LoadOrCreateAsset<ULandscapeLayerInfoObject>(LayerInfoPackageName, LayerInfoName);
        if (!LayerInfo)
        {
            continue;
        }

        LayerInfo->Modify();
        LayerInfo->SetLayerName(FName(*LayerName), false);
        LayerInfo->SetLayerUsageDebugColor(LayerInfo->GenerateLayerUsageDebugColor(), false, EPropertyChangeType::ValueSet);
        LayerInfo->MarkPackageDirty();

        LandscapeInfo->CreateTargetLayerSettingsFor(LayerInfo);

        TSharedPtr<FJsonObject> LayerObj = MakeShared<FJsonObject>();
        LayerObj->SetStringField(TEXT("name"), LayerName);
        LayerObj->SetStringField(TEXT("asset"), LayerInfo->GetPathName());
        CreatedLayerInfos.Add(MakeShared<FJsonValueObject>(LayerObj));
    }

    LandscapeInfo->ForEachLandscapeProxy([Material](ALandscapeProxy* LandscapeProxy)
    {
        if (LandscapeProxy)
        {
            LandscapeProxy->Modify();
            if (FObjectPropertyBase* MaterialProperty = FindFProperty<FObjectPropertyBase>(LandscapeProxy->GetClass(), TEXT("LandscapeMaterial")))
            {
                MaterialProperty->SetObjectPropertyValue_InContainer(LandscapeProxy, Material);
                FPropertyChangedEvent PropertyChangedEvent(MaterialProperty);
                LandscapeProxy->PostEditChangeProperty(PropertyChangedEvent);
            }
        }
        return true;
    });

    LandscapeInfo->UpdateLayerInfoMap(Proxy, true);
    Proxy->MarkPackageDirty();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("material"), Material->GetPathName());
    ResultObj->SetArrayField(TEXT("layer_infos"), CreatedLayerInfos);
    ResultObj->SetObjectField(TEXT("landscape"), LandscapeProxyToJson(Proxy));
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandlePaintLandscapeLayerAtLocation(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FString LayerName;
    if (!Params->TryGetStringField(TEXT("layer_name"), LayerName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'layer_name' parameter"));
    }

    if (!Params->HasField(TEXT("location")))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'location' parameter"));
    }

    FString LandscapeName;
    Params->TryGetStringField(TEXT("landscape_name"), LandscapeName);

    const FVector WorldLocation = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    const float Radius = Params->HasField(TEXT("radius")) ? FMath::Max(0.0f, static_cast<float>(Params->GetNumberField(TEXT("radius")))) : 256.0f;
    const float Strength = Params->HasField(TEXT("strength")) ? FMath::Clamp(static_cast<float>(Params->GetNumberField(TEXT("strength"))), 0.0f, 1.0f) : 1.0f;
    const bool bUseFalloff = Params->HasField(TEXT("use_falloff")) ? Params->GetBoolField(TEXT("use_falloff")) : true;

    ALandscapeProxy* Proxy = FindLandscapeProxyByName(World, LandscapeName);
    if (!Proxy)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Landscape not found: %s"), LandscapeName.IsEmpty() ? TEXT("<any>") : *LandscapeName));
    }

    ULandscapeInfo* LandscapeInfo = Proxy->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Landscape has no LandscapeInfo"));
    }

    ULandscapeLayerInfoObject* LayerInfo = LandscapeInfo->GetLayerInfoByName(FName(*LayerName), Proxy);
    if (!LayerInfo)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Landscape layer not found or has no LayerInfo: %s"), *LayerName));
    }

    const FTransform LandscapeTransform = Proxy->GetActorTransform();
    const FVector LocalLocation = LandscapeTransform.InverseTransformPosition(WorldLocation);
    const FVector LandscapeScale = LandscapeTransform.GetScale3D().GetAbs();
    const float AverageXYScale = FMath::Max(1.0f, (LandscapeScale.X + LandscapeScale.Y) * 0.5f);
    const float LocalRadius = FMath::Max(1.0f, Radius / AverageXYScale);

    const int32 CenterX = FMath::RoundToInt(LocalLocation.X);
    const int32 CenterY = FMath::RoundToInt(LocalLocation.Y);
    const int32 X1 = FMath::FloorToInt(LocalLocation.X - LocalRadius);
    const int32 Y1 = FMath::FloorToInt(LocalLocation.Y - LocalRadius);
    const int32 X2 = FMath::CeilToInt(LocalLocation.X + LocalRadius);
    const int32 Y2 = FMath::CeilToInt(LocalLocation.Y + LocalRadius);
    const int32 Width = X2 - X1 + 1;
    const int32 Height = Y2 - Y1 + 1;

    if (Width <= 0 || Height <= 0)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Computed paint bounds are invalid"));
    }

    TArray<uint8> AlphaData;
    AlphaData.SetNumZeroed(Width * Height);

    TAlphamapAccessor<false> Accessor(LandscapeInfo, LayerInfo);
    Accessor.GetDataFast(X1, Y1, X2, Y2, AlphaData.GetData());

    const uint8 TargetAlpha = static_cast<uint8>(FMath::RoundToInt(Strength * 255.0f));
    int32 PaintedSamples = 0;

    for (int32 Y = Y1; Y <= Y2; ++Y)
    {
        for (int32 X = X1; X <= X2; ++X)
        {
            const float Distance = FVector2D(static_cast<float>(X - CenterX), static_cast<float>(Y - CenterY)).Size();
            if (Distance > LocalRadius)
            {
                continue;
            }

            const int32 Index = (Y - Y1) * Width + (X - X1);
            if (bUseFalloff)
            {
                const float Falloff = 1.0f - FMath::Clamp(Distance / LocalRadius, 0.0f, 1.0f);
                AlphaData[Index] = FMath::Max(AlphaData[Index], static_cast<uint8>(FMath::RoundToInt(TargetAlpha * Falloff)));
            }
            else
            {
                AlphaData[Index] = TargetAlpha;
            }
            ++PaintedSamples;
        }
    }

    if (PaintedSamples == 0)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Brush did not touch any landscape samples"));
    }

    const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCP", "PaintLandscapeLayerAtLocation", "Paint Landscape Layer At Location"));
    Proxy->Modify();
    Accessor.SetData(X1, Y1, X2, Y2, AlphaData.GetData(), ELandscapeLayerPaintingRestriction::None);
    Accessor.Flush();
    LandscapeInfo->UpdateLayerInfoMap(Proxy, true);

    TSharedPtr<FJsonObject> BoundsObj = MakeShared<FJsonObject>();
    BoundsObj->SetNumberField(TEXT("x1"), X1);
    BoundsObj->SetNumberField(TEXT("y1"), Y1);
    BoundsObj->SetNumberField(TEXT("x2"), X2);
    BoundsObj->SetNumberField(TEXT("y2"), Y2);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("landscape"), Proxy->GetName());
    ResultObj->SetStringField(TEXT("layer"), LayerName);
    ResultObj->SetNumberField(TEXT("painted_samples"), PaintedSamples);
    ResultObj->SetObjectField(TEXT("bounds"), BoundsObj);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandlePaintLandscapeOpenWorldBiome(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FString LandscapeName;
    Params->TryGetStringField(TEXT("landscape_name"), LandscapeName);

    FString GrassLayerName = TEXT("Grass");
    Params->TryGetStringField(TEXT("grass_layer"), GrassLayerName);

    FString DirtLayerName = TEXT("Dirt");
    Params->TryGetStringField(TEXT("dirt_layer"), DirtLayerName);

    ALandscapeProxy* Proxy = FindLandscapeProxyByName(World, LandscapeName);
    if (!Proxy)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Landscape not found: %s"), LandscapeName.IsEmpty() ? TEXT("<any>") : *LandscapeName));
    }

    ULandscapeInfo* LandscapeInfo = Proxy->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Landscape has no LandscapeInfo"));
    }

    ULandscapeLayerInfoObject* GrassLayerInfo = LandscapeInfo->GetLayerInfoByName(FName(*GrassLayerName), Proxy);
    ULandscapeLayerInfoObject* DirtLayerInfo = LandscapeInfo->GetLayerInfoByName(FName(*DirtLayerName), Proxy);
    if (!GrassLayerInfo || !DirtLayerInfo)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Grass or Dirt layer not found"));
    }

    int32 MinX = 0;
    int32 MinY = 0;
    int32 MaxX = 0;
    int32 MaxY = 0;
    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get landscape extent"));
    }

    const int32 Width = MaxX - MinX + 1;
    const int32 Height = MaxY - MinY + 1;
    if (Width <= 0 || Height <= 0)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Landscape extent is invalid"));
    }

    TArray<uint16> HeightData;
    HeightData.SetNumUninitialized(Width * Height);

    FHeightmapAccessor<false> HeightAccessor(LandscapeInfo);
    HeightAccessor.GetDataFast(MinX, MinY, MaxX, MaxY, HeightData.GetData());

    uint16 MinHeight = TNumericLimits<uint16>::Max();
    uint16 MaxHeight = TNumericLimits<uint16>::Min();
    for (uint16 Sample : HeightData)
    {
        MinHeight = FMath::Min(MinHeight, Sample);
        MaxHeight = FMath::Max(MaxHeight, Sample);
    }

    const float HeightRange = FMath::Max(1.0f, static_cast<float>(MaxHeight - MinHeight));
    const float DirtNoiseScale = Params->HasField(TEXT("dirt_noise_scale")) ? FMath::Max(1.0f, static_cast<float>(Params->GetNumberField(TEXT("dirt_noise_scale")))) : 180.0f;
    const float SlopeDirtBoost = Params->HasField(TEXT("slope_dirt_boost")) ? FMath::Max(0.0f, static_cast<float>(Params->GetNumberField(TEXT("slope_dirt_boost")))) : 5.0f;
    const float LowlandDirtBoost = Params->HasField(TEXT("lowland_dirt_boost")) ? FMath::Max(0.0f, static_cast<float>(Params->GetNumberField(TEXT("lowland_dirt_boost")))) : 0.35f;

    TArray<uint8> GrassAlpha;
    TArray<uint8> DirtAlpha;
    GrassAlpha.SetNumUninitialized(Width * Height);
    DirtAlpha.SetNumUninitialized(Width * Height);

    int32 GrassSamples = 0;
    int32 DirtSamples = 0;
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            const int32 Index = Y * Width + X;
            const float Height01 = (static_cast<float>(HeightData[Index]) - static_cast<float>(MinHeight)) / HeightRange;

            const int32 LeftIndex = Y * Width + FMath::Max(0, X - 1);
            const int32 RightIndex = Y * Width + FMath::Min(Width - 1, X + 1);
            const int32 DownIndex = FMath::Max(0, Y - 1) * Width + X;
            const int32 UpIndex = FMath::Min(Height - 1, Y + 1) * Width + X;
            const float Dx = FMath::Abs(static_cast<float>(HeightData[RightIndex]) - static_cast<float>(HeightData[LeftIndex])) / HeightRange;
            const float Dy = FMath::Abs(static_cast<float>(HeightData[UpIndex]) - static_cast<float>(HeightData[DownIndex])) / HeightRange;
            const float Slope = FMath::Clamp((Dx + Dy) * SlopeDirtBoost, 0.0f, 1.0f);

            const float WorldX = static_cast<float>(MinX + X);
            const float WorldY = static_cast<float>(MinY + Y);
            const float Noise = FMath::Frac(FMath::Sin(WorldX * 12.9898f / DirtNoiseScale + WorldY * 78.233f / DirtNoiseScale) * 43758.5453f);
            const float Lowland = 1.0f - FMath::SmoothStep(0.18f, 0.42f, Height01);
            const float DirtWeight = FMath::Clamp(0.12f + Lowland * LowlandDirtBoost + Slope * 0.55f + Noise * 0.16f, 0.0f, 0.92f);
            const float GrassWeight = 1.0f - DirtWeight;

            GrassAlpha[Index] = static_cast<uint8>(FMath::RoundToInt(GrassWeight * 255.0f));
            DirtAlpha[Index] = static_cast<uint8>(FMath::RoundToInt(DirtWeight * 255.0f));

            if (GrassAlpha[Index] > DirtAlpha[Index])
            {
                ++GrassSamples;
            }
            else
            {
                ++DirtSamples;
            }
        }
    }

    const FScopedTransaction Transaction(NSLOCTEXT("UnrealMCP", "PaintLandscapeOpenWorldBiome", "Paint Landscape Open World Biome"));
    Proxy->Modify();

    TAlphamapAccessor<false> GrassAccessor(LandscapeInfo, GrassLayerInfo);
    GrassAccessor.SetData(MinX, MinY, MaxX, MaxY, GrassAlpha.GetData(), ELandscapeLayerPaintingRestriction::None);
    GrassAccessor.Flush();

    TAlphamapAccessor<false> DirtAccessor(LandscapeInfo, DirtLayerInfo);
    DirtAccessor.SetData(MinX, MinY, MaxX, MaxY, DirtAlpha.GetData(), ELandscapeLayerPaintingRestriction::None);
    DirtAccessor.Flush();

    LandscapeInfo->UpdateLayerInfoMap(Proxy, true);
    LandscapeInfo->ForEachLandscapeProxy([](ALandscapeProxy* LandscapeProxy)
    {
        if (LandscapeProxy)
        {
            LandscapeProxy->MarkPackageDirty();
        }
        return true;
    });

    TSharedPtr<FJsonObject> BoundsObj = MakeShared<FJsonObject>();
    BoundsObj->SetNumberField(TEXT("x1"), MinX);
    BoundsObj->SetNumberField(TEXT("y1"), MinY);
    BoundsObj->SetNumberField(TEXT("x2"), MaxX);
    BoundsObj->SetNumberField(TEXT("y2"), MaxY);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("landscape"), Proxy->GetName());
    ResultObj->SetNumberField(TEXT("width"), Width);
    ResultObj->SetNumberField(TEXT("height"), Height);
    ResultObj->SetNumberField(TEXT("min_height"), MinHeight);
    ResultObj->SetNumberField(TEXT("max_height"), MaxHeight);
    ResultObj->SetNumberField(TEXT("grass_dominant_samples"), GrassSamples);
    ResultObj->SetNumberField(TEXT("dirt_dominant_samples"), DirtSamples);
    ResultObj->SetObjectField(TEXT("bounds"), BoundsObj);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleExecuteConsoleCommand(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FString Command;
    if (!Params->TryGetStringField(TEXT("command"), Command) || Command.IsEmpty())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'command' parameter"));
    }

    const bool bExecuted = GEngine && GEngine->Exec(World, *Command);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), bExecuted);
    ResultObj->SetStringField(TEXT("command"), Command);
    return ResultObj;
}
