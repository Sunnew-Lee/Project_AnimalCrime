// Fill out your copyright notice in the Description page of Project Settings.


#include "ACMoneyComponent.h"

#include "Game/ACPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AnimalCrime.h"


UACMoneyComponent::UACMoneyComponent()
{
	SetIsReplicatedByDefault(true);
}



void UACMoneyComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UACMoneyComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UACMoneyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//DOREPLIFETIME(UACMoneyComponent, MoneyData);
	// 조건부 복제: 소유자에게만 전송 (대역폭 절약)
	DOREPLIFETIME_CONDITION(UACMoneyComponent, MoneyData, COND_OwnerOnly);
}

void UACMoneyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UACMoneyComponent::InitMoneyComponent(EMoneyType MoneyType)
{
	// 이미 초기화되었거나 Invalid 타입이면 무시
	if (MoneyType == EMoneyType::None || MoneyData.MoneyType != EMoneyType::None)
	{
		UE_LOG(LogHG, Warning, TEXT("InitMoneyComponent: Already initialized or invalid type"));
		return;
	}
	
	
	EMoneyType CurrentMoneyData = MoneyType;
	switch (CurrentMoneyData)
	{
	case EMoneyType::MoneyMafiaType:
		InitMafiaSetting();
		break;
	case EMoneyType::MoneyPoliceType:
		InitPoliceSetting();
		break;
	case EMoneyType::MoneyCitizenType:
		InitCitizenSetting();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("말이 안됌."));
	}
}

void UACMoneyComponent::EarnMoney(int32 InMoney)
{
	if (InMoney < 0)
	{
		return;
	}
	
	if (MoneyData.MoneyType == EMoneyType::None)
	{
		return;
	}
	
	// 돈 획득
	MoneyData.Money += InMoney;

	// 서버에서만 델리게이트 브로드캐스트
	  // (클라이언트는 OnRep에서 자동 호출됨)
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnMoneyChanged.Broadcast(MoneyData.Money);
	}
}

bool UACMoneyComponent::SpendMoney(int32 InMoney)
{
	if (MoneyData.MoneyType == EMoneyType::None || MoneyData.MoneyType == EMoneyType::MoneyCitizenType)
	{
		return false;
	}
	if (InMoney < 0)
	{
		return false;
	}
	if (MoneyData.Money < InMoney)
	{
		return false;
	}

	// 돈 소비
	MoneyData.Money -= InMoney;

	// 서버에서만 델리게이트 브로드캐스트
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnMoneyChanged.Broadcast(MoneyData.Money);
	}

	return true;
}

int32 UACMoneyComponent::LoseMoney(int32 InMoney)
{
	if (InMoney < 0)
	{
		return -1;
	}

	// 가진 돈보다 많이 빼앗기면 전부 빼앗김
	if (MoneyData.Money < InMoney)
	{
		InMoney = MoneyData.Money;
	}

	// 실제 값 계산
	MoneyData.Money = FMath::Clamp(MoneyData.Money - InMoney, 0, MoneyData.Money);

	// 서버에서만 델리게이트 브로드캐스트
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnMoneyChanged.Broadcast(MoneyData.Money);
	}
	return InMoney;
}

void UACMoneyComponent::OnRep_MoneyData()
{
	// 클라이언트에서 MoneyData가 복제되면 자동 호출됨
	// 델리게이트 브로드캐스트 → UI 자동 업데이트
	OnMoneyChanged.Broadcast(MoneyData.Money);
}

void UACMoneyComponent::InitMafiaSetting()
{
	MoneyData.MoneyType = EMoneyType::MoneyMafiaType;
	
	InitMoney(500);
}

void UACMoneyComponent::InitPoliceSetting()
{
	MoneyData.MoneyType = EMoneyType::MoneyPoliceType;
	
	InitMoney(800);
}

void UACMoneyComponent::InitCitizenSetting()
{
	MoneyData.MoneyType = EMoneyType::MoneyCitizenType;
	
	GenerateRandomMoney(500);
}

void UACMoneyComponent::InitMoney(int32 InMoney)
{
	MoneyData.Money = InMoney;


	// 서버에서 초기화 시 델리게이트 브로드캐스트
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnMoneyChanged.Broadcast(MoneyData.Money);
	}
}

void UACMoneyComponent::GenerateRandomMoney(int32 InMaxMoney)
{
	//MoneyData.Money = FMath::RandRange(100, InMaxMoney);
	MoneyData.Money = FMath::RandRange(1, InMaxMoney / 100);
	MoneyData.Money *= 100;

	// 서버에서 초기화 시 델리게이트 브로드캐스트
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		OnMoneyChanged.Broadcast(MoneyData.Money);
	}
}



