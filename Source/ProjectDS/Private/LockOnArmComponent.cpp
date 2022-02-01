// Fill out your copyright notice in the Description page of Project Settings.


#include "LockOnArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LockOnTargetComponent.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, text)

ULockOnArmComponent::ULockOnArmComponent()
{
	MaxTargetLockDistance = 750.f;// Ÿ���� ���� �Ÿ� ����
	bDrawDebug = true; // ����� ���Ǿ� ��������

	TargetArmLength = 300.0f; // �������� ����	
	bUsePawnControlRotation = true; // ��Ʈ�ѷ��� ȸ���� ��뿩��
	bEnableCameraLag = true; // �ε巯�� ī�޶� ȿ�� ���Ͽ� ī�޶� ��ġ�� ���
	bEnableCameraRotationLag = false; // ī�޶��� �����̼Ƿ� ����
	CameraLagSpeed = 3.f; // ī�޶� �� ���ǵ� ����
	CameraRotationLagSpeed = 2.f; // ī�޶� �����̼� �� ����
	CameraLagMaxDistance = 100.f; // ī�޶� ������ ���� �ִ� �Ÿ� ����
}

void ULockOnArmComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsCameraLockedToTarget()) // CameraTarget�� Ÿ���� ������ false, Ÿ�������� true
	{
		DrawDebugSphere(GetWorld(), CameraTarget->GetComponentLocation(), 20.f, 32, FColor::Red); // Ÿ���� ������ ����� ���Ǿ �׷��� 20�� ũ��� 32���� ���� �̿��ؼ� ������ ������ �׷��� 

		// Ÿ���� ������ MaxTargetLockDistance �̻� �������� BreakTargetLock �Լ� ȣ��
		if ((CameraTarget->GetComponentLocation() - GetComponentLocation()).Size() > MaxTargetLockDistance + CameraTarget->GetScaledSphereRadius())
		{
			if (bUseSoftLock) 
			{
				// Ÿ����ȯ ���� MaxTargetLockDistance �� �ٸ� Ÿ���� �ִ��� Ȯ��
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

	// Get the target with the smallest angle difference from the camera forward vector
	float ClosestDotToCenter = 0.f;
	ULockOnTargetComponent* TargetComponent = nullptr;

	for (int32 i = 0; i < AvailableTargets.Num(); i++)
	{
		float Dot = FVector::DotProduct(GetForwardVector(), (AvailableTargets[i]->GetComponentLocation() - GetComponentLocation()).GetSafeNormal());
		if (Dot > ClosestDotToCenter)
		{
			ClosestDotToCenter = Dot;
			TargetComponent = AvailableTargets[i];
		}
	}
	return TargetComponent;
}

// SphereOverlapComponents �� �����ϱ� ���� �Լ�
TArray<class ULockOnTargetComponent*> ULockOnArmComponent::GetTargetComponents()
{
	TArray<UPrimitiveComponent*> TargetPrims; // Ÿ���̵� ���۳�Ʈ�� ���� �迭 ����
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { EObjectTypeQuery::ObjectTypeQuery2  }; // �ݸ��� ������ -> ������Ʈ ���� WorldDynamic) !!! ���⼭ �� Ÿ�Կ� ���� ��������� ��!!!!

	// Ÿ�� ���� ������ ���۳�Ʈ üũ
	UKismetSystemLibrary::SphereOverlapComponents(GetOwner(), GetComponentLocation(), MaxTargetLockDistance, ObjectTypes, ULockOnTargetComponent::StaticClass(), TArray<AActor*>{GetOwner()}, TargetPrims);
	// 1. ���ڽ�, 2. ���ڽ��� �����̼�, 3.�����ص� �ִ�Ÿ�, 4.������̳��� ���� ����, 5.Ÿ������ ������ ������Ʈ Ŭ���� ����, 6.���ڽ��� �־� ���� ����, 7.���� ������ ������ �Է�
	
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
