// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/GameFramework/DBDGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "JMS/ScratchMark/PoolEntry_ScratchMark.h"
#include "Shared/AssetManager/DBDAssetManager.h"
#include "Shared/DataAsset/DA_PreloadAssets.h"
#include "Shared/DataAsset/DBDDataBase.h"
#include "Shared/ObjectPool/GenericObjectPool.h"

void UDBDGameInstance::Init()
{
	Super::Init();
}

void UDBDGameInstance::OnStart()
{
	Super::OnStart();
	
	StartDatabaseLoading();
}

void UDBDGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);
	
}

void UDBDGameInstance::StartDatabaseLoading()
{
	// JMS: 미리 로딩�셋 추�
	// JMS: 미리 로딩�셋 추�
	FPrimaryAssetId DatabaseAssetId("DBDDataBase", FName(TEXT("DBDDB")));
	TArray<FPrimaryAssetId> AssetsToLoad;
	AssetsToLoad.Add(DatabaseAssetId);

	UDBDAssetManager& DBDAssetManager = UDBDAssetManager::Get();
	
	AssetLoadHandle = DBDAssetManager.LoadPrimaryAssets(
		AssetsToLoad,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(
			this,
			&UDBDGameInstance::OnDatabaseLoaded,
			DatabaseAssetId)
		);

}

void UDBDGameInstance::OnDatabaseLoaded(FPrimaryAssetId LoadedId)
{
	UDBDAssetManager& DBDAssetManager = UDBDAssetManager::Get();
	UDBDDataBase* DB = Cast<UDBDDataBase>(DBDAssetManager.GetPrimaryAssetObject(LoadedId));
	if (DB)
	{
		LoadedDB = DB;
	}
	SecondaryDatabaseLoading(DB);
	AssetLoadHandle.Reset();
}

void UDBDGameInstance::SecondaryDatabaseLoading(UDBDDataBase* DB)
{
	if (!DB) return;


	TArray<TSoftObjectPtr<UDataTable>> AllDataTables = {
		DB->SurvivorPerkDB,
		DB->KillerPerkDB,
		DB->SurvivorItemDB,
		DB->ItemAddonDB,
		DB->SurvivorCharacterDB
	};
	TArray<FSoftObjectPath> SecondaryDataBaseToLoad;
	
	for (TSoftObjectPtr<UDataTable> DataTable : AllDataTables)
	{
		SecondaryDataBaseToLoad.Add(DataTable.ToSoftObjectPath());
	}

	if (SecondaryDataBaseToLoad.Num() > 0)
	{
		UDBDAssetManager& DBDAssetManager = UDBDAssetManager::Get();
		
		SecondaryDatabaseLoadHandle = DBDAssetManager.GetStreamableManager()
			.RequestAsyncLoad(
			SecondaryDataBaseToLoad,
			FStreamableDelegate::CreateUObject(this,
				&UDBDGameInstance::OnSecondaryDatabaseLoaded,
				DB)
			);
	}
}

void UDBDGameInstance::OnSecondaryDatabaseLoaded(UDBDDataBase* DB)
{
	LoadedTables.Empty();
	LoadedTables.Add(DB->SurvivorCharacterDB.Get());
	LoadedTables.Add(DB->SurvivorPerkDB.Get());
	LoadedTables.Add(DB->KillerPerkDB.Get());
	LoadedTables.Add(DB->SurvivorItemDB.Get());
	LoadedTables.Add(DB->ItemAddonDB.Get());
	SecondaryAssetLoading(LoadedTables);
	SecondaryDatabaseLoadHandle.Reset();
}

void UDBDGameInstance::SecondaryAssetLoading(TArray<UDataTable*> DataTables)
{
	TArray<FSoftObjectPath> SecondaryAssetsToLoad;


	for (const UDataTable* DataTable : DataTables)
	{
		if (!DataTable) continue;

		TArray<FName> RowNames = DataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FSurvivorCharacterData* CharacterData = DataTable->FindRow<FSurvivorCharacterData>(RowName, TEXT("Load FSurvivorCharacterData Secondary Assets"));
			if (CharacterData)
			{
				SecondaryAssetsToLoad.Add(CharacterData->CharacterPortrait.ToSoftObjectPath());
			}

			FItemData* ItemData = DataTable->FindRow<FItemData>(RowName, TEXT("Load FItemData Secondary Assets"));
			if (ItemData)
			{
				SecondaryAssetsToLoad.Add(ItemData->Icon.ToSoftObjectPath());
			}

			FPerkData* PerkData = DataTable->FindRow<FPerkData>(RowName, TEXT("Load FPerkData Secondary Assets"));
			if (PerkData)
			{
				SecondaryAssetsToLoad.Add(PerkData->Icon.ToSoftObjectPath());
			}

			FItemAddonData* ItemAddonData = DataTable->FindRow<FItemAddonData>(RowName, TEXT("Load FItemAddonData Secondary Assets"));
			if (ItemAddonData)
			{
				SecondaryAssetsToLoad.Add(ItemAddonData->Icon.ToSoftObjectPath());
			}
		}
	}


	// JMS: 미리 로딩�셋
	// FPrimaryAssetId PreloadAssetId("DA_PreloadAssets", FName(TEXT("DA_PreloadAssets")));
	// UDA_PreloadAssets* PreloadAssets = Cast<UDA_PreloadAssets>(UAssetManager::Get().GetPrimaryAssetObject(PreloadAssetId));
	//
	// for (TSoftObjectPtr<UObject> ObjectPtr : PreloadAssets->AssetsToPreload)
	// {
	// 	SecondaryAssetsToLoad.Add(ObjectPtr.ToSoftObjectPath());
	// }
	
	if (SecondaryAssetsToLoad.Num() > 0)
	{
		UDBDAssetManager& DBDAssetManager = UDBDAssetManager::Get();
		
		AssetLoadHandle = DBDAssetManager.GetStreamableManager()
			.RequestAsyncLoad(
				SecondaryAssetsToLoad,
				FStreamableDelegate::CreateUObject(
					this,
					&UDBDGameInstance::OnAllSecondaryAssetLoaded,
					DataTables)
			);
	}
}

void UDBDGameInstance::OnAllSecondaryAssetLoaded(TArray<UDataTable*> DataTables)
{
	AssetTextures.Empty();
	for (const UDataTable* DataTable : DataTables)
	{
		if (!DataTable) continue;

		TArray<FName> RowNames = DataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FSurvivorCharacterData* CharacterData = DataTable->FindRow<FSurvivorCharacterData>(RowName, TEXT("Load FSurvivorCharacterData Secondary Assets"));
			if (CharacterData)
			{
				AssetTextures.Add(CharacterData->CharacterPortrait.Get());
			}

			FItemData* ItemData = DataTable->FindRow<FItemData>(RowName, TEXT("Load FItemData Secondary Assets"));
			if (ItemData)
			{
				AssetTextures.Add(ItemData->Icon.Get());
			}

			FPerkData* PerkData = DataTable->FindRow<FPerkData>(RowName, TEXT("Load FPerkData Secondary Assets"));
			if (PerkData)
			{
				AssetTextures.Add(PerkData->Icon.Get());
			}

			FItemAddonData* ItemAddonData = DataTable->FindRow<FItemAddonData>(RowName, TEXT("Load FItemAddonData Secondary Assets"));
			if (ItemAddonData)
			{
				AssetTextures.Add(ItemAddonData->Icon.Get());
			}
		}
	}
	AssetLoadHandle.Reset();

}

void UDBDGameInstance::StartLoadingScreen()
{
	if (LoadingScreenClass)
	{
		LoadingScreenInstance = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (LoadingScreenInstance)
		{
			LoadingScreenInstance->AddToViewport(100);
		}
	}
}

void UDBDGameInstance::EndLoadingScreen()
{
	if (LoadingScreenInstance)
	{
		LoadingScreenInstance->RemoveFromParent();
		LoadingScreenClass = nullptr;
	}
}


void UDBDGameInstance::StartInGameAssetLoading()
{
	
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	AssetRegistryModule.Get().SearchAllAssets(false);
	
	if (AssetRegistryModule.Get().IsLoadingAssets())
	{
		AssetRegistryModule.Get().OnFilesLoaded().AddUObject(this, &UDBDGameInstance::OnAssetRegistryScanCompleted);
	}
	else
	{
		StartLoadingScreen();
		StartAssetLoadingFromRegistry();
	}
}


void UDBDGameInstance::OnInGameAssetLoaded()
{
	InGameAssetLoadHandle->GetLoadedAssets(LoadedObjects);

	InGameAssetLoadHandle.Reset();
}

void UDBDGameInstance::StartAsyncLoading(const TArray<FSoftObjectPath>& AssetsToLoad)
{
	UDBDAssetManager& AssetManager = UDBDAssetManager::Get();

	// InGameAssetLoadHandle = AssetManager.GetStreamableManager().RequestAsyncLoad(
	// 	AssetsToLoad,
	// 	FStreamableDelegate::CreateUObject(this, &UDBDGameInstance::OnInGameAssetLoaded) // 완료 콜백 연결
	// );

	
	// FStreamableHandle을 저장하여 로딩이 완료될 때까지 참조를 유지해야 합니다.
	TSharedPtr<FStreamableHandle> LoadHandle = AssetManager.GetStreamableManager().RequestAsyncLoad(
		AssetsToLoad,
		FStreamableDelegate::CreateLambda([AssetsToLoad = AssetsToLoad, this]()
		{
			// 로딩이 완료되면 이 람다 함수가 호출됩니다.
			for (const FSoftObjectPath& AssetPath : AssetsToLoad)
			{
				// 로드된 에셋을 메모리에서 가져옵니다.
				UStaticMesh* LoadedMesh = Cast<UStaticMesh>(AssetPath.ResolveObject());
				if (LoadedMesh)
				{
					// 로드된 메쉬로 원하는 작업을 수행합니다.
					LoadedStaticMeshes.Add(LoadedMesh);
				}

				USkeletalMesh* LoadedSkeletalMesh = Cast<USkeletalMesh>(AssetPath.ResolveObject());
				if (LoadedSkeletalMesh)
				{
					LoadedSkeletalMeshes.Add(LoadedSkeletalMesh);
				}
			}

			EndLoadingScreen();
		})
	);
}

void UDBDGameInstance::OnAssetRegistryScanCompleted()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnFilesLoaded().RemoveAll(this);

	StartLoadingScreen();
	StartAssetLoadingFromRegistry();
}

void UDBDGameInstance::StartAssetLoadingFromRegistry()
{
	FName TargetPath = FName(TEXT("/Game/Assets/YHG"));
	
	FARFilter AssetFilter;
	AssetFilter.PackagePaths.Add(TargetPath);
	AssetFilter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
	AssetFilter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
	AssetFilter.bRecursivePaths = true;

	TArray<FAssetData> AssetDatas;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// 2. 이미 Asset Registry가 로딩 완료된 경우 (바로 실행)
	AssetRegistryModule.Get().GetAssets(AssetFilter, AssetDatas);


	TArray<FSoftObjectPath> AssetsToLoad;
	for (const FAssetData& AssetData : AssetDatas)
	{
		// AssetData를 FSoftObjectPath로 변환하여 목록에 추가
		AssetsToLoad.Add(AssetData.GetSoftObjectPath()); 
	}
    
	// 비동기 로드 시작
	if (AssetsToLoad.Num() > 0)
	{
		StartAsyncLoading(AssetsToLoad);
	}
}
