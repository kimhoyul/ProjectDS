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
	/** 타겟팅 최장 거리 설정  */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float MaxTargetLockDistance;
	
	/** Soft Lock 사용 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	bool bUseSoftLock;

	/** 디버그선 보일지 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	bool bDrawDebug;

	/** 소프트락 해제후 딜레이 */
	bool bSoftlockRequiresReset;

	/** 락온한 대상을 넣어주는 컴포넌트 클래스 생성 */
	UPROPERTY(BlueprintReadOnly)
	class ULockOnTargetComponent* CameraTarget;

	//생성자
	ULockOnArmComponent();
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ToggleCameraLock();
	void ToggleSoftLock();
	void LockToTarget(ULockOnTargetComponent* NewTargetComponent);
	void BreakTargetLock();
	class ULockOnTargetComponent* GetLockTarget();
	void SwitchTarget(EDirection SwitchDirection);
	TArray<class ULockOnTargetComponent*> GetTargetComponents();

	/* 카메라가 현재 Lock On 상태이면 true*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lock On Camera")
	bool IsCameraLockedToTarget();
};
