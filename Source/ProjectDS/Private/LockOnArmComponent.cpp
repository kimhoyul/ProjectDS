// Fill out your copyright notice in the Description page of Project Settings.


#include "LockOnArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LockOnTargetComponent.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, text)

ULockOnArmComponent::ULockOnArmComponent()
{
	MaxTargetLockDistance = 750.f;// 타겟팅 최장 거리 설정
	bDrawDebug = true; // 디버그 스피어 생성여부

	TargetArmLength = 300.0f; // 스프링암 길이	
	bUsePawnControlRotation = true; // 컨트롤러의 회전값 사용여부
	bEnableCameraLag = true; // 부드러운 카메라 효과 위하여 카메라 위치렉 사용
	bEnableCameraRotationLag = false; // 카메라의 로테이션렉 설정
	CameraLagSpeed = 3.f; // 카메라 렉 스피드 설정
	CameraRotationLagSpeed = 2.f; // 카메라 로테이션 렉 설정
	CameraLagMaxDistance = 100.f; // 카메라 렉으로 인한 최대 거리 설정
}

void ULockOnArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsCameraLockedToTarget()) // CameraTarget에 타겟이 없으면 false, 타겟있으면 true
	{
		DrawDebugSphere(GetWorld(), CameraTarget->GetComponentLocation(), 20.f, 32, FColor::Red); // 타겟이 있으면 디버그 스피어를 그려라 20의 크기로 32개의 선을 이용해서 빨간색 선으로 그려라 

		// 타겟이 있지만 MaxTargetLockDistance 이상 벌어지면 BreakTargetLock 함수 호출
		if ((CameraTarget->GetComponentLocation() - GetComponentLocation()).Size() > MaxTargetLockDistance + CameraTarget->GetScaledSphereRadius())
		{
			if (bUseSoftLock) 
			{
				// 타겟전환 위해 MaxTargetLockDistance 에 다른 타겟이 있는지 확인
				if (ULockOnTargetComponent* NewCameraTarget = GetLockTarget())
					LockToTarget(NewCameraTarget);
				else
					BreakTargetLock();
			}
			else
			{
				BreakTargetLock();
			}
		}
	}
	else
	{
		if (bUseSoftLock) // Attempt to auto target nearby enemy
		{
			if (ULockOnTargetComponent* NewCameraTarget = GetLockTarget())
			{
				if (!bSoftlockRequiresReset) // Soft-lock is reset?
					LockToTarget(NewCameraTarget);
			}
			else // If player forcibly broke soft-lock, reset it when no target is within range
			{
				bSoftlockRequiresReset = false;
			}
		}
	}
	// Draw debug
	if (bDrawDebug)
	{
		for (ULockOnTargetComponent* Target : GetTargetComponents())
		{
			DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Target->GetComponentLocation(), FColor::Green);
		}

		// Draw target inclusion sphere
		DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation(), MaxTargetLockDistance, 32, FColor::Cyan);

		UKismetSystemLibrary::DrawDebugString(this, FVector::ZeroVector, bUseSoftLock ? "Soft-lock Enabled" : "Soft-lock Disabled", GetOwner(), FLinearColor::Green);

		if (bSoftlockRequiresReset)
			UKismetSystemLibrary::DrawDebugString(this, FVector(0.f, 0.f, -10.f), "Soft-lock Requires Reset", GetOwner(), FLinearColor::Green);
	}
}

void ULockOnArmComponent::ToggleCameraLock()
{
	if (bUseSoftLock)   // Soft-lock supersedes player input
	{
		bSoftlockRequiresReset = false;
		return;
	}

	// If CameraTarget is set, unset it
	if (IsCameraLockedToTarget())
	{
		BreakTargetLock();
		return;
	}

	ULockOnTargetComponent* NewCameraTarget = GetLockTarget();

	if (NewCameraTarget != nullptr)
	{
		print(TEXT("Testing"));
		LockToTarget(NewCameraTarget);
	}
}

void ULockOnArmComponent::ToggleSoftLock()
{
	bUseSoftLock = !bUseSoftLock;

	if (bUseSoftLock)
	{
		print(TEXT("Soft-lock enabled"));
		bSoftlockRequiresReset = false;
	}
	else
	{
		BreakTargetLock();
		print(TEXT("Soft-lock disabled"));
	}
}

void ULockOnArmComponent::LockToTarget(ULockOnTargetComponent* NewTargetComponent)
{
	CameraTarget = NewTargetComponent;
	bEnableCameraRotationLag = true;
	//GetCharacterMovement()->bOrientRotationToMovement = false;
}

void ULockOnArmComponent::BreakTargetLock()
{
	if (IsCameraLockedToTarget())
	{
		CameraTarget = nullptr;
		//GetController()->SetControlRotation(FollowCamera->GetForwardVector().Rotation());
		bEnableCameraRotationLag = false;
		//GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}


ULockOnTargetComponent* ULockOnArmComponent::GetLockTarget()
{
	TArray<ULockOnTargetComponent*> AvailableTargets = GetTargetComponents(); 
	if (AvailableTargets.Num() == 0)
		return nullptr;

	// 카메라 ForwardVector 에서 각도 차이가 가장 적은 대상 가져오기
	float ClosestDotToCenter = 0.f;
	ULockOnTargetComponent* TargetComponent = nullptr;

	for (int32 i = 0; i < AvailableTargets.Num(); i++)
	{
		// Dot 에 내적값 넣어주기
		float Dot = FVector::DotProduct(GetForwardVector(), (AvailableTargets[i]->GetComponentLocation() - GetComponentLocation()).GetSafeNormal());
		
		if (Dot > ClosestDotToCenter)
		{
			ClosestDotToCenter = Dot;
			TargetComponent = AvailableTargets[i];
		}
	}
	return TargetComponent;
}

// SphereOverlapComponents 를 진행하기 위한 함수
TArray<class ULockOnTargetComponent*> ULockOnArmComponent::GetTargetComponents()
{
	TArray<UPrimitiveComponent*> TargetPrims; // 타겟이될 컴퍼넌트를 담을 배열 생성
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { EObjectTypeQuery::ObjectTypeQuery2  }; // 콜리전 프리셋 -> 오브젝트 반응 WorldDynamic) !!! 여기서 적 타입에 따라 변경해줘야 함!!!!

	// 타겟 설정 가능한 컴퍼넌트 체크
	UKismetSystemLibrary::SphereOverlapComponents(GetOwner(), GetComponentLocation(), MaxTargetLockDistance, ObjectTypes, ULockOnTargetComponent::StaticClass(), TArray<AActor*>{GetOwner()}, TargetPrims);
	// 1. 나자신, 2. 나자신의 로케이션, 3.지정해둔 최대거리, 4.월드다이나믹 으로 설정, 5.타겟으로 만들어둔 컴포넌트 클래스 지정, 6.나자신을 넣어 나는 제외, 7.위에 만들어둔 변수에 입력
	
	TArray<ULockOnTargetComponent*> TargetComps; 
	for (UPrimitiveComponent* Comp : TargetPrims)
	{
		TargetComps.Add(Cast<ULockOnTargetComponent>(Comp));
	}

	return TargetComps;
}

bool ULockOnArmComponent::IsCameraLockedToTarget()
{
	return CameraTarget != nullptr;
}

void ULockOnArmComponent::SwitchTarget(EDirection SwitchDirection)
{
	if (!IsCameraLockedToTarget()) return;

	TArray<ULockOnTargetComponent*> AvailableTargets = GetTargetComponents();	// Get targets within lock-on range	
	if (AvailableTargets.Num() < 2) return;	// Must have an existing camera target and 1 additional target

	FVector CurrentTargetDir = (CameraTarget->GetComponentLocation() - GetComponentLocation()).GetSafeNormal();

	TArray<ULockOnTargetComponent*> ViableTargets;

	for (ULockOnTargetComponent* Target : AvailableTargets)
	{
		//  Don't consider current target as a switch target
		if (Target == CameraTarget) continue;

		FVector TargetDir = (Target->GetComponentLocation() - GetComponentLocation()).GetSafeNormal();
		FVector Cross = FVector::CrossProduct(CurrentTargetDir, TargetDir);

		if ((SwitchDirection == EDirection::Left && Cross.Z < 0.f)	// Negative Z indicates left
			|| (SwitchDirection == EDirection::Right && Cross.Z > 0.f))	// Positive Z indicates right
		{
			ViableTargets.AddUnique(Target);
		}
	}

	if (ViableTargets.Num() == 0) return;

	/*
	Select the target with the smallest angle difference to the current target
	*/
	int32 BestDotIdx = 0;
	for (int32 i = 1; i < ViableTargets.Num(); i++)
	{
		float BestDot = FVector::DotProduct(CurrentTargetDir, (ViableTargets[BestDotIdx]->GetComponentLocation() - GetComponentLocation()).GetSafeNormal());
		float TestDot = FVector::DotProduct(CurrentTargetDir, (ViableTargets[i]->GetComponentLocation() - GetComponentLocation()).GetSafeNormal());

		// Higher dot product indicates this target vector has a smaller angle than the previous best
		if (TestDot > BestDot)
			BestDotIdx = i;
	}

	LockToTarget(ViableTargets[BestDotIdx]);
}
