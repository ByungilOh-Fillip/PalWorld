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
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "LandscapeInfo.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
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
    else if (CommandType == TEXT("paint_landscape_layer_at_location"))
    {
        return HandlePaintLandscapeLayerAtLocation(Params);
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
