// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DSCharacter.generated.h"

UCLASS()
class PROJECTDS_API ADSCharacter : public ACharacter
{
	GENERATED_BODY()

	/* LockArmComponent 컴퍼넌트 선언 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class ULockOnArmComponent* CameraLockArm;

	/* Follow camera 선언 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/*Camera Lock On System 을 위해 TargetComponent 선언 */
	class ULockOnTargetComponent* TargetComponent;

public:
	//////////////////////////////////////////////////////////////////////////////////////////////
	// 키보드 & 마우스

	/** LockOn 시 카메라 이동속도 조절 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lock On Camera")
	float LockOnControlRotationRate;;

	/* Soft Lock Off 상태에서 대상을 전환하기 위해 입력으로 간주되는 마우스 움직임값 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchMouseDelta;

	/* Soft Lock Off 상태에서 대상 전환 delay. Lock On System의 제어성을 높이기 위해 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchMinDelaySeconds;

	/* Soft Lock Off 상태에서 대상 전환 delay 위해 마지막 대상 전환 시간 저장 */
	UPROPERTY(BlueprintReadOnly, Category = "Lock On Camera")
	float LastTargetSwitchTime;

	/* Soft Lock On 상태에서 대상을 전환하기 위해 입력으로 간주되는 마우스 움직임값 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float BreakLockMouseDelta;

	/* Soft Lock On 상태에서 대상 전환 delay Lock On System의 제어성을 높이기 위해 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float BrokeLockAimingCooldown;

	/* Soft Lock On 상태에서 대상 전환 delay 위해 마지막 대상 전환 시간 저장 */
	float BrokeLockTime;

public:
	// Sets default values for this character's properties
	ADSCharacter();
	
protected:
	/** 입력에 맞춰 기능을 바인딩하기 위해 호출 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Val);
	void LookUp(float Val);
	void ChangeLockOnLeft();
	void ChangeLockOnRight();

	bool IsChangeTarget;
	
	/** 매 프레임 마다 갱신위해 함수 호출 */
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	
};


