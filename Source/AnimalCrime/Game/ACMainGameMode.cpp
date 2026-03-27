

#include "ACMainGameMode.h"

#include "ACAdvancedFriendsGameInstance.h"
#include "ACGameRuleManager.h"
#include "ACMainGameState.h"
#include "ACPlayerState.h"
#include "ACMainPlayerController.h"
#include "NavigationSystem.h"
#include "Character/ACCharacter.h"
#include "Character/ACCitizen.h"
#include "Character/ACMafiaCharacter.h"
#include "Character/ACPoliceCharacter.h"
#include "Character/ACBlackMarketDealer.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "ACGameEnums.h"
#include "AnimalCrime.h"

#include "ACPrisonManager.h"
#include "Components/CapsuleComponent.h"
#include "Prison/ACPrisonBase.h"
#include "StartPosition/ACPlayerStart.h"
#include "Item/ACEscapeMissionContraband.h"
#include "EscapeQuest/ACContrabandSpawnPoint.h"
#include "Algo/RandomShuffle.h"

AACMainGameMode::AACMainGameMode()
{
	PlayerControllerClass = AACMainPlayerController::StaticClass();
	//DefaultPawnClass = AACMafiaCharacter::StaticClass();
	DefaultPawnClass = nullptr;

	GameStateClass = AACMainGameState::StaticClass();
	PlayerStateClass = AACPlayerState::StaticClass();

	//MafiaPawnClass = AACMafiaCharacter::StaticClass();
	//static ConstructorHelpers::FClassFinder<APawn> MafiaPawnBP(
	//	TEXT("/Game/Project/Character/BP_ACMafia")
	//);
	static ConstructorHelpers::FClassFinder<APawn> MafiaPawnBP(
		TEXT("/Game/Project/Character/BP_TestMafia")
	);

	static ConstructorHelpers::FClassFinder<APawn> PolicePawnBP(
		TEXT("/Game/Project/Character/BP_TestPolice")
	);
	if (MafiaPawnBP.Succeeded())
	{
		MafiaPawnClass = MafiaPawnBP.Class;
	}
	if (PolicePawnBP.Succeeded())
	{
		PolicePawnClass = PolicePawnBP.Class;
	}
	//PolicePawnClass = AACPoliceCharacter::StaticClass();

	static ConstructorHelpers::FClassFinder<AACCitizen> CitizenBP(TEXT("/Script/Engine.Blueprint'/Game/Project/AI/BP_Citizen.BP_Citizen_C'"));
	if (CitizenBP.Succeeded())
	{
		CitizenBPClass = CitizenBP.Class;
	}

	bUseSeamlessTravel = true;
}

void AACMainGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	AC_LOG(LogHY, Warning, TEXT("End"));
}

APlayerController* AACMainGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	APlayerController* PlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	AC_LOG(LogHY, Warning, TEXT("End"));
	return PlayerController;
}

void AACMainGameMode::PostLogin(APlayerController* NewPlayer)
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PostLogin(NewPlayer);
	AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACMainGameMode::PostInitializeComponents()
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::PostInitializeComponents();

	TArray<AActor*> FoundStarts;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AACPlayerStart::StaticClass(),
		FoundStarts
	);

	for (AActor* Actor : FoundStarts)
	{
		AACPlayerStart* Start = Cast<AACPlayerStart>(Actor);
		if (Start == nullptr)
		{
			continue;
		}

		switch (Start->GetSpawnType())
		{
		case ESpawnTypeState::POLICE:
		{
			AC_LOG(LogHY, Error, TEXT("Police 언제"));
			PoliceStartArray.Add(Start);
			break;
		}

		case ESpawnTypeState::MAFIA:
		{
			AC_LOG(LogHY, Error, TEXT("Mafia 언제"));
			MafiaStartArray.Add(Start);
			break;
		}
		}
	}
	AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACMainGameMode::StartPlay()
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::StartPlay();
	AC_LOG(LogHY, Warning, TEXT("End"));
}

void AACMainGameMode::BeginPlay()
{
	AC_LOG(LogHY, Warning, TEXT("Begin"));
	Super::BeginPlay();

	// Game Rule Manager 생성 및 초기화
	GameRuleManager = NewObject<UACGameRuleManager>(this);
	if (GameRuleManager == nullptr)
	{
		AC_LOG(LogSW, Log, TEXT("Create Fail GameRuleManager null manager"))
	}
	GameRuleManager->Init(this);

	PrisonManager = NewObject<UACPrisonManager>(this);
	if (PrisonManager == nullptr)
	{
		AC_LOG(LogSW, Log, TEXT("Create Fail PrisonManager null manager"))
	}
	else
	{
		// 월드에 있는 모든 감옥을 찾아서 등록 (초기화 순서 문제 해결)
		TArray<AActor*> FoundPrisons;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AACPrisonBase::StaticClass(), FoundPrisons);

		for (AActor* Actor : FoundPrisons)
		{
			AACPrisonBase* Prison = Cast<AACPrisonBase>(Actor);
			if (Prison == nullptr)
			{
				continue;
			}
			PrisonManager->RegisterPrison(Prison);
			AC_LOG(LogSW, Log, TEXT("Prison registered: %s"), *Prison->GetName());
		}

		AC_LOG(LogSW, Log, TEXT("Total prisons registered: %d"), FoundPrisons.Num());
	}

	GenerateOutfitPool();
	SpawnAllAI();
	SpawnRandomContrabands();

	AC_LOG(LogHY, Warning, TEXT("End"));
}

AActor* AACMainGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	Super::ChoosePlayerStart_Implementation(Player);




	AACPlayerState* ACPlayerState = Cast<AACPlayerState>(Player->PlayerState);
	if (ACPlayerState == nullptr)
	{
		AC_LOG(LogHY, Error, TEXT("ACPlayerState is nullptr"));
		return nullptr;
	}

	AC_LOG(LogHY, Error, TEXT("ACPlayerState Choose 뭐시기"));
	switch (ACPlayerState->PlayerRole)
	{
	case EPlayerRole::Police:
	{
		++PoliceCount;
		int32 PlayerIndex = PoliceCount - 1;

		// FVector Location = PoliceStartArray[PlayerIndex % PoliceStartArray.Num()]->GetActorLocation();	
		// AC_LOG(LogHY, Error, TEXT("Police 들어옴 %s"), *Location.ToString());
		return PoliceStartArray.Num() > 0 ? PoliceStartArray[PlayerIndex % PoliceStartArray.Num()] : nullptr;
		break;
	}

	case EPlayerRole::Mafia:
	{
		++MafiaCount;
		int32 PlayerIndex = MafiaCount - 1;
		// FVector Location = MafiaStartArray[PlayerIndex % MafiaStartArray.Num()]->GetActorLocation();
		// AC_LOG(LogHY, Error, TEXT("Mafia 들어옴 %s"), *Location.ToString());
		return MafiaStartArray.Num() > 0 ? MafiaStartArray[PlayerIndex % MafiaStartArray.Num()] : nullptr;
		break;
	}
	}

	return nullptr;

}

void AACMainGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	//게임 시작 시 폰을 관전 가능 대상 폰으로 등록함.
	APawn* Pawn = NewPlayer->GetPawn();
	if (Pawn == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("Pawn이 nullptr 이라 폰을 관전 대상으로 등록시키지 못함."));
		return;
	}

	AACMainGameState* GS = GetGameState<AACMainGameState>();
	if (GS == nullptr)
	{
		AC_LOG(LogSY, Error, TEXT("GameState가 nullptr 이라 폰을 관전 대상으로 등록시키지 못함."));
		return;
	}

	GS->AddSpectatablePawn(Pawn);
}

UClass* AACMainGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (InController == nullptr)
	{
		return DefaultPawnClass;
	}

	AACPlayerState* PS = InController->GetPlayerState<AACPlayerState>();

	if (PS == nullptr)
	{
		return DefaultPawnClass;
	}
	UACAdvancedFriendsGameInstance* GI = GetGameInstance<UACAdvancedFriendsGameInstance>();

	if (GI == nullptr)
	{
		return DefaultPawnClass;
	}

	// GameInstance에서 역할 복원
	const FUniqueNetIdRepl& NetId = PS->GetUniqueId();

	AC_LOG(LogSY, Log, TEXT("%s : %s"), *PS->GetPlayerName(), *PS->GetUniqueId()->ToString());

	if (NetId.IsValid() == false)
	{
		return DefaultPawnClass;
	}

	if (GI->SavedPlayerRoles.Contains(NetId))
	{
		const EPlayerRole PlayerRole = GI->SavedPlayerRoles[NetId];

		PS->PlayerRole = PlayerRole;

		return (PlayerRole == EPlayerRole::Police) ? PolicePawnClass : MafiaPawnClass;
	}

	return DefaultPawnClass;
}

void AACMainGameMode::AddTeamScore(int32 Score)
{
	UE_LOG(LogTemp, Log, TEXT("[AddTeamScore]"));
	AACMainGameState* MainGameState = GetGameState<AACMainGameState>();
	if (MainGameState)
	{
		MainGameState->UpdateTeamScore(MainGameState->GetTeamScore() + Score);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("AddTeamScore Nullptr"));
	}
}

int32 AACMainGameMode::GetTeamScore() const
{
	UE_LOG(LogTemp, Log, TEXT("[GetTeamScore]"));
	AACMainGameState* MainGameState = GetGameState<AACMainGameState>();
	if (MainGameState)
	{
		return MainGameState->GetTeamScore();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GetTeamScore Nullptr"));
	}
	return -1;
}

void AACMainGameMode::UpdateGameScoreFromPolice(EPoliceAction PoliceActionType, float Score)
{
	GameRuleManager->RewardPoliceForAction(PoliceActionType, Score);
}

void AACMainGameMode::UpdateGameScoreFromMafia(EMafiaAction MafiaActionType, float Score)
{
	GameRuleManager->RewardMafiaForAction(MafiaActionType, Score);
}

void AACMainGameMode::UpdateGameScoreFromObject(float Score)
{
	// @Todo: 나중에 구현
}

float AACMainGameMode::GetGameScore() const
{
	UE_LOG(LogTemp, Log, TEXT("[GetGameScore]에서 GameRuleManager->GetScoreGauge을 호출합니다."));

	return GameRuleManager->GetScoreGauge();
}

void AACMainGameMode::SpawnAllAI()
{
	for (int32 i = 0; i < AINums; ++i)
	{
		FVector Pos = GetRandomSpawnLocation();
		FRotator Rot = FRotator::ZeroRotator;

		FTransform Transform(Pos);
		Transform.SetRotation(Rot.Quaternion());

		AACCitizen* NewAI = nullptr;

		// 첫 번째는 BlackMarketDealer로 스폰
		if (i == 0 && BlackMarketDealerBPClass != nullptr)
		{
			NewAI = GetWorld()->SpawnActorDeferred<AACCitizen>(BlackMarketDealerBPClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		}
		else
		{
			NewAI = GetWorld()->SpawnActorDeferred<AACCitizen>(CitizenBPClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		}

		//AACCitizen* NewAI = GetWorld()->SpawnActorDeferred<AACCitizen>(CitizenBPClass, Transform,nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (NewAI)
		{
			// AI 관리하는 애
			AIObjectArray.Add(NewAI);

			FOutfitCombo OutfitCombo = GiveOutfitFromPool();

			USkeletalMesh* LoadedHair = OutfitCombo.HairAsset.LoadSynchronous();
			if (LoadedHair)
			{
				NewAI->SetHeadMesh(LoadedHair);
				NewAI->OnRep_HeadMesh();
				AC_LOG(LogHY, Error, TEXT("%d:Hair %s"), i, *LoadedHair->GetName())
			}

			USkeletalMesh* LoadedFace = OutfitCombo.FaceAsset.LoadSynchronous();
			if (LoadedFace)
			{
				NewAI->SetFaceMesh(LoadedFace);
				NewAI->OnRep_FaceMesh();
				AC_LOG(LogHY, Error, TEXT("%d:Face %s"), i, *LoadedHair->GetName())
			}

			USkeletalMesh* LoadedTop = OutfitCombo.TopAsset.LoadSynchronous();
			if (LoadedTop)
			{
				NewAI->SetTopMesh(LoadedTop);
				NewAI->OnRep_TopMesh();
				AC_LOG(LogHY, Error, TEXT("%d:Top %s"), i, *LoadedTop->GetName())
			}

			USkeletalMesh* LoadedBottom = OutfitCombo.BottomAsset.LoadSynchronous();
			if (LoadedBottom)
			{
				NewAI->SetBottomMesh(LoadedBottom);
				NewAI->OnRep_BottomMesh();
				AC_LOG(LogHY, Error, TEXT("%d:Bottom %s"), i, *LoadedBottom->GetName())
			}

			USkeletalMesh* LoadedShoes = OutfitCombo.ShoesAsset.LoadSynchronous();
			if (LoadedShoes)
			{
				NewAI->SetShoesMesh(LoadedShoes);
				NewAI->OnRep_ShoesMesh();
				AC_LOG(LogHY, Error, TEXT("%d:Shoes %s"), i, *LoadedShoes->GetName())
			}

			USkeletalMesh* LoadedFaceAcc = OutfitCombo.FaceAccAsset.LoadSynchronous();
			if (LoadedFaceAcc)
			{
				NewAI->SetFaceAccMesh(LoadedFaceAcc);
				NewAI->OnRep_FaceAccMesh();
				AC_LOG(LogHY, Error, TEXT("%d:FaceAcc %s"), i, *LoadedFaceAcc->GetName())
			}
			UE_LOG(LogTemp, Log, TEXT("Spawn Success %d"), i);
		}
		else
		{
			AC_LOG(LogHY, Warning, TEXT("스폰 실패"));
		}

		// Capsule 크기 가져오기
		ACharacter* DefaultChar = CitizenBPClass->GetDefaultObject<ACharacter>();
		UCapsuleComponent* Capsule = DefaultChar->GetCapsuleComponent();

		FVector Location = Transform.GetLocation();
		Location.Z += Capsule->GetScaledCapsuleHalfHeight();
		Transform.SetLocation(Location);
		NewAI->FinishSpawning(Transform);
#if WITH_EDITOR
		if (IsValid(NewAI) == false)
		{
			DrawDebugCapsule(GetWorld(), Transform.GetLocation(), Capsule->GetScaledCapsuleHalfHeight(), Capsule->GetScaledCapsuleRadius(), Transform.GetRotation(), FColor::Red, false, 200.0f);
		}
#endif
	}
}

FVector AACMainGameMode::GetRandomSpawnLocation() const
{
	FNavLocation RandomLocation;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	float Radius = 13000;
	if (NavSys && NavSys->GetRandomReachablePointInRadius(FVector(17200.f, 2200.0f, 90.0f), Radius, RandomLocation))
	{
		return RandomLocation.Location;
	}

	return FVector::ZeroVector;
}

FVector AACMainGameMode::GetRandomSpawnLocation(const FVector& Location, float Radius) const
{
	FNavLocation RandomLocation;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys && NavSys->GetRandomReachablePointInRadius(Location, Radius, RandomLocation))
	{
		return RandomLocation.Location;
	}

	return FVector::ZeroVector;
}

void AACMainGameMode::GenerateOutfitPool()
{
	OutfitPool.Empty();

	for (TSoftObjectPtr<USkeletalMesh>& Hair : HairList)
	{
		for (TSoftObjectPtr<USkeletalMesh>& Face : FaceList)
		{
			for (TSoftObjectPtr<USkeletalMesh>& Top : TopList)
			{
				for (TSoftObjectPtr<USkeletalMesh>& Bottom : BottomList)
				{
					for (TSoftObjectPtr<USkeletalMesh>& Shoe : ShoesList)
					{
						for (TSoftObjectPtr<USkeletalMesh>& FaceAcc : FaceAccList)
						{
							FOutfitCombo Combo;
							Combo.HairAsset = Hair;
							Combo.FaceAsset = Face;
							Combo.TopAsset = Top;
							Combo.BottomAsset = Bottom;
							Combo.ShoesAsset = Shoe;
							Combo.FaceAccAsset = FaceAcc;

							OutfitPool.Add(Combo);
						}
					}
				}
			}
		}
	}

	// 랜덤 셔플
	OutfitPool.Sort([](auto&, auto&) { return FMath::RandBool(); });

	NextOutfitIndex = 0;
}

FOutfitCombo AACMainGameMode::GiveOutfitFromPool()
{
	// int32 CurrentOutfitIndex = NextOutfitIndex++;
	// return OutfitPool[CurrentOutfitIndex];

	if (OutfitPool.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("OutfitPool is EMPTY!"));
		return FOutfitCombo();
	}

	if (NextOutfitIndex >= OutfitPool.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("OutfitPool INDEX OUT OF RANGE: %d / %d"),
			NextOutfitIndex, OutfitPool.Num());
		return FOutfitCombo();
	}

	int32 CurrentOutfitIndex = NextOutfitIndex++;

	UE_LOG(LogTemp, Warning, TEXT("GiveOutfitFromPool: %d / %d"),
		CurrentOutfitIndex, OutfitPool.Num());

	return OutfitPool[CurrentOutfitIndex];
}

FOutfitCombo AACMainGameMode::GetClothesFromPool()
{
	if (OutfitPool.Num() == 0)
	{
		AC_LOG(LogHY, Error, TEXT("OutfitPool is EMPTY!"));
		return FOutfitCombo();
	}

	int32 RandIndex = FMath::RandRange(0, OutfitPool.Num() - 1);
	return OutfitPool[RandIndex];
}

void AACMainGameMode::ImprisonCharacter(AACCharacter* Character)
{
	if (PrisonManager == nullptr)
	{
		return;
	}
	if (Character == nullptr)
	{
		return;
	}

	PrisonManager->ImprisonCharacter(Character);
}


void AACMainGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	UE_LOG(LogTemp, Warning, TEXT("Seamless Travel 완료, 서버 PostSeamlessTravel 호출"));

	// // 모든 PlayerController 초기화
	// for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	// {
	// 	APlayerController* PC = It->Get();
	// 	if (PC && PC->GetPawn() == nullptr)
	// 	{
	// 		UE_LOG(LogTemp, Error, TEXT("들어오니"));
	// 		// 새로운 Pawn Spawn 후 Possess
	// 		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, PC->GetSpawnLocation(), FRotator::ZeroRotator);
	// 		PC->Possess(NewPawn);
	//
	// 		// 클라이언트에서도 새 Pawn Possess 필요하면 RPC 호출 가능
	// 		PC->ClientRestart(NewPawn);
	// 	}
	// }
}

void AACMainGameMode::GameRemainTimeUp()
{
	GameRuleManager->RemainTimeUp(60);
}

void AACMainGameMode::GameRemainTimeDown()
{
	GameRuleManager->RemainTimeDown(60);
}

void AACMainGameMode::SpawnRandomContrabands()
{
	// 월드에 배치된 모든 밀수품 찾기
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AACContrabandSpawnPoint::StaticClass(), SpawnPoints);

	// 랜덤 셔플
	Algo::RandomShuffle(SpawnPoints);

	// 스폰 개수 결정 (배치된 개수보다 많으면 전부 활성화)
	int32 NumToSpawn = FMath::Min(ActiveContrabandCount, SpawnPoints.Num());

	// 랜덤하게 밀수품 생성
	for (int32 i = 0; i < NumToSpawn; ++i)
	{
		GetWorld()->SpawnActor<AACEscapeMissionContraband>(
			AACEscapeMissionContraband::StaticClass(),
			SpawnPoints[i]->GetActorLocation(),
			SpawnPoints[i]->GetActorRotation()
		);
	}
}
