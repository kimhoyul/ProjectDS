// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DSCharacter.generated.h"

UCLASS(config=Game)
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
	// 아날로그스틱

	/** 아날로그스틱 X축 입력값 변수 선언 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
	float BaseTurnRate;

	/** 아날로그스틱 Y축 입력값 변수 선언 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
	float BaseLookUpRate;

	/* 대상을 전환하기 위해 입력으로 간주되는 아날로그스틱 움직임값 변수 선언 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchAnalogValue;

	/* 대상 전환 이후 아날로그 스틱이 중립으로 돌아 갔는지 여부 변수 선언 */
	bool bAnalogSettledSinceLastTargetSwitch;

	//////////////////////////////////////////////////////////////////////////////////////////////
	// 키보드 & 마우스

	/** LockOn 시 카메라 이동속도 조절 변수 선언*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
	float LockonControlRotationRate;;

	/* Soft Lock Off 상태에서 대상을 전환하기 위해 입력으로 간주되는 마우스 움직임값 변수 선언*/
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchMouseDelta;

	/* Soft Lock Off 상태에서 대상 전환 delay 변수 선언. Lock On System의 제어성을 높이기 위해 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchMinDelaySeconds;

	/* Soft Lock Off 상태에서 대상 전환 delay 위해 마지막 대상 전환 시간 저장 변수 선언 */
	UPROPERTY(BlueprintReadOnly, Category = "Lock On Camera")
	float LastTargetSwitchTime;

	/* Soft Lock On 상태에서 대상을 전환하기 위해 입력으로 간주되는 마우스 움직임값 변수 선언 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float BreakLockMouseDelta;

	/* Soft Lock On 상태에서 대상 전환 delay 변수 선언. Lock On System의 제어성을 높이기 위해 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float BrokeLockAimingCooldown;

	/* Soft Lock On 상태에서 대상 전환 delay 위해 마지막 대상 전환 시간 저장 변수 선언 */
	float BrokeLockTime;

public:
	// Sets default values for this character's properties
	ADSCharacter();



protected:
	// 입력에 맞춰 기능을 바인딩하기 위해 호출
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/** 키보드 W / S (전후) 이동 입력 함수 선언*/
	void MoveForward(float Value);
	
	/** 키보드 A / D (좌우) 이동 입력 함수 선언*/
	void MoveRight(float Value);

	/* 마우스 X축 입력 함수 선언 */
	void Turn(float Val);
	
	/* 마우스 Y축 입력 함수 선언 */
	void LookUp(float Val);

	/* 아날로그스틱 X축 입력 함수 선언 */
	void TurnAtRate(float Rate);

	/* 아날로그스틱 Y축 입력 함수 선언 */
	void LookUpAtRate(float Rate);

	/* 매 프레임 마다 갱신위해 함수 호출 */
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	
public:	
	/** Returns CameraBoom subobject **/
	UFUNCTION(BlueprintCallable, Category = "Lock On Camera")
		FORCEINLINE class ULockOnArmComponent* GetCameraBoom() const { return CameraLockArm; }
	/** Returns FollowCamera subobject **/
	UFUNCTION(BlueprintCallable, Category = "Lock On Camera")
		FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
