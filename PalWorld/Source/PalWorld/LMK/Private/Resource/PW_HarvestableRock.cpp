#include "Resource/PW_HarvestableRock.h"

#include "GameplayTags/PW_GameplayTags.h"
#include "Resource/PW_HarvestableResourceComponent.h"

APW_HarvestableRock::APW_HarvestableRock()
{
	if (ResourceComponent != nullptr)
	{
		ResourceComponent->SetResourceDefaults(PW_GameplayTags::Work_Mining, TEXT("Stone"), 1);
	}
}
