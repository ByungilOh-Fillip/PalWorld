#include "Resource/PW_HarvestableTree.h"

#include "GameplayTags/PW_GameplayTags.h"
#include "Resource/PW_HarvestableResourceComponent.h"

APW_HarvestableTree::APW_HarvestableTree()
{
	if (ResourceComponent != nullptr)
	{
		ResourceComponent->SetResourceDefaults(PW_GameplayTags::Work_Lumbering, TEXT("Wood"), 1);
	}
}
