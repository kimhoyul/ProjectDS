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
		if (bUseSoftLock) // 타겟범위 내의 적 자동 목표 설정
		{
			if (ULockOnTargetComponent* NewCameraTarget = GetLockTarget())
			{
				if (!bSoftlockRequiresReset) 
					LockToTarget(NewCameraTarget);
			}
			else // 타겟 범위 내 적이 없으면 재설정 해줌
			{
				bSoftlockRequiresReset = false;
			}
		}
	}
	// Draw debug On 일떄
	if (bDrawDebug)
	{
		for (ULockOnTargetComponent* Target : GetTargetComponents())
		{
			DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), Target->GetComponentLocation(), FColor::Green); //대상이 되는 적과 나를 연결하는선 그리기
		}

		// 타겟 가능 최대 범위 그리기
		DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation(), MaxTargetLockDistance, 32, FColor::Cyan);
		//Soft-Lock On,Off 여부 
		UKismetSystemLibrary::DrawDebugString(this, FVector::ZeroVector, bUseSoftLock ? "Soft-lock Enabled" : "Soft-lock Disabled", GetOwner(), FLinearColor::Green);
		//Soft-Lock 재설정 필요 문구
		if (bSoftlockRequiresReset)
			UKismetSystemLibrary::DrawDebugString(this, FVector(0.f, 0.f, -10.f), "Soft-lock Requires Reset", GetOwner(), FLinearColor::Green);
	}
}

void ULockOnArmComponent::ToggleCameraLock()
{
	if (bUseSoftLock) 
	{
		bSoftlockRequiresReset = false;
		return;
	}

	// 이미 록온 상태이면 해제
	if (IsCameraLockedToTarget())
	{
		BreakTargetLock();
		return;
	}

	ULockOnTargetComponent* NewCameraTarget = GetLockTarget();

	if (NewCameraTarget != nullptr)
	{
		LockToTarget(NewCameraTarget);
	}
}

void ULockOnArmComponent::ToggleSoftLock()
{
	//거짓이면 참으로 참이면 거짓으로 변경하여 넣어줌
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
}

void ULockOnArmComponent::BreakTargetLock()
{
	if (IsCameraLockedToTarget())
	{
		CameraTarget = nullptr;
		bEnableCameraRotationLag = false;
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

// 타겟팅 가능 범위내에서 타겟 확보
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

//LockOn 타겟 변경
void ULockOnArmComponent::SwitchTarget(EDirection SwitchDirection)
{
	if (!IsCameraLockedToTarget()) return;

	TArray<ULockOnTargetComponent*> AvailableTargets = GetTargetComponents();	// 타겟팅 가능 범위내에서 타겟 확보 
	if (AvailableTargets.Num() < 2) return;	// 둘 이상의 타겟이 있는지 확인

	FVector CurrentTargetDir = (CameraTarget->GetComponentLocation() - GetComponentLocation()).GetSafeNormal(); // 타겟의 방향구하기

	TArray<ULockOnTargetComponent*> ViableTargets;

	for (ULockOnTargetComponent* Target : AvailableTargets) // 범위기반루프 (배열 INDEX 기반 X , 배열 요소 값 O)
	{
		//  현재 타겟은 건너뛰어라
		if (Target == CameraTarget) continue;

		FVector TargetDir = (Target->GetComponentLocation() - GetComponentLocation()).GetSafeNormal(); // 배열내의 타겟들을 루프로 돌려서 방향 구하기
		FVector Cross = FVector::CrossProduct(CurrentTargetDir, TargetDir); //외적 구하기

		//언리얼은 왼손좌표계이다.
		if ((SwitchDirection == EDirection::Left && Cross.Z < 0.f)	// Left로 들어왔고 외적이 플러스값이면 현재 대상보다 우측에 있기 때문에 추가 하지 않음 
			|| (SwitchDirection == EDirection::Right && Cross.Z > 0.f))	// Right로 들어왔고 외적이 외적이 마이너스값이면 현재 대상보다 우측에 있기 때문에 추가 하지 않음
		{
			ViableTargets.AddUnique(Target); // 겹치는 않는 아이템만 저장
		}
	}

	if (ViableTargets.Num() == 0) return; 

	/*
	추가된 값중에서 현재 대상과의 각도 차이가 가장 작은 대상을 선택. 
	*/
	int32 BestDotIdx = 0;
	for (int32 i = 1; i < ViableTargets.Num(); i++)
	{
		float BestDot = FVector::DotProduct(CurrentTargetDir, (ViableTargets[BestDotIdx]->GetComponentLocation() - GetComponentLocation()).GetSafeNormal());
		float TestDot = FVector::DotProduct(CurrentTargetDir, (ViableTargets[i]->GetComponentLocation() - GetComponentLocation()).GetSafeNormal());

		// 내적의 각이 적을수록 내적의 값이 크기때문에 내적값이 더 큰쪽을 BestDot으로 치환 
		if (TestDot > BestDot)
			BestDotIdx = i;
	}

	LockToTarget(ViableTargets[BestDotIdx]);
}
