// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "LockOnArmComponent.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right"),
};

/**
 * 
 */


UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTDS_API ULockOnArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	/* Ÿ���� ���� �Ÿ� ���� ���� ���� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float MaxTargetLockDistance;

	/* Soft Lock ��� ���� ���� ���� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	bool bUseSoftLock;


	/* ����׼� ������ ���� ���� ���� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	bool bDrawDebug;

	/* ����Ʈ�� ������ �����̸� ���� ���� ���� */
	bool bSoftlockRequiresReset;

	/* ������ ����� �־��ִ� ������Ʈ Ŭ���� ���� */
	UPROPERTY(BlueprintReadOnly)
	class ULockOnTargetComponent* CameraTarget;

	//������
	ULockOnArmComponent();
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ToggleCameraLock();
	void ToggleSoftLock();
	void LockToTarget(ULockOnTargetComponent* NewTargetComponent);
	void BreakTargetLock();
	class ULockOnTargetComponent* GetLockTarget();
	void SwitchTarget(EDirection SwitchDirection);
	TArray<class ULockOnTargetComponent*> GetTargetComponents();

	/* True if the camera is currently locked to a target */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lock On Camera")
	bool IsCameraLockedToTarget();
};
