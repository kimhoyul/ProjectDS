// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DSCharacter.generated.h"

UCLASS(config=Game)
class PROJECTDS_API ADSCharacter : public ACharacter
{
	GENERATED_BODY()

	/* LockArmComponent ���۳�Ʈ ���� */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class ULockOnArmComponent* CameraLockArm;

	/* Follow camera ���� */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/*Camera Lock On System �� ���� TargetComponent ���� */
	class ULockOnTargetComponent* TargetComponent;

public:
	//////////////////////////////////////////////////////////////////////////////////////////////
	// �Ƴ��α׽�ƽ

	/** �Ƴ��α׽�ƽ X�� �Է°� ���� ���� */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
	float BaseTurnRate;

	/** �Ƴ��α׽�ƽ Y�� �Է°� ���� ���� */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
	float BaseLookUpRate;

	/* ����� ��ȯ�ϱ� ���� �Է����� ���ֵǴ� �Ƴ��α׽�ƽ �����Ӱ� ���� ���� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchAnalogValue;

	/* ��� ��ȯ ���� �Ƴ��α� ��ƽ�� �߸����� ���� ������ ���� ���� ���� */
	bool bAnalogSettledSinceLastTargetSwitch;

	//////////////////////////////////////////////////////////////////////////////////////////////
	// Ű���� & ���콺

	/** LockOn �� ī�޶� �̵��ӵ� ���� ���� ����*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
	float LockonControlRotationRate;;

	/* Soft Lock Off ���¿��� ����� ��ȯ�ϱ� ���� �Է����� ���ֵǴ� ���콺 �����Ӱ� ���� ����*/
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchMouseDelta;

	/* Soft Lock Off ���¿��� ��� ��ȯ delay ���� ����. Lock On System�� ����� ���̱� ���� ��� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float TargetSwitchMinDelaySeconds;

	/* Soft Lock Off ���¿��� ��� ��ȯ delay ���� ������ ��� ��ȯ �ð� ���� ���� ���� */
	UPROPERTY(BlueprintReadOnly, Category = "Lock On Camera")
	float LastTargetSwitchTime;

	/* Soft Lock On ���¿��� ����� ��ȯ�ϱ� ���� �Է����� ���ֵǴ� ���콺 �����Ӱ� ���� ���� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float BreakLockMouseDelta;

	/* Soft Lock On ���¿��� ��� ��ȯ delay ���� ����. Lock On System�� ����� ���̱� ���� ��� */
	UPROPERTY(EditDefaultsOnly, Category = "Lock On Camera")
	float BrokeLockAimingCooldown;

	/* Soft Lock On ���¿��� ��� ��ȯ delay ���� ������ ��� ��ȯ �ð� ���� ���� ���� */
	float BrokeLockTime;

public:
	// Sets default values for this character's properties
	ADSCharacter();



protected:
	// �Է¿� ���� ����� ���ε��ϱ� ���� ȣ��
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/** Ű���� W / S (����) �̵� �Է� �Լ� ����*/
	void MoveForward(float Value);
	
	/** Ű���� A / D (�¿�) �̵� �Է� �Լ� ����*/
	void MoveRight(float Value);

	/* ���콺 X�� �Է� �Լ� ���� */
	void Turn(float Val);
	
	/* ���콺 Y�� �Է� �Լ� ���� */
	void LookUp(float Val);

	/* �Ƴ��α׽�ƽ X�� �Է� �Լ� ���� */
	void TurnAtRate(float Rate);

	/* �Ƴ��α׽�ƽ Y�� �Է� �Լ� ���� */
	void LookUpAtRate(float Rate);

	/* �� ������ ���� �������� �Լ� ȣ�� */
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	
public:	
	/** Returns CameraBoom subobject **/
	UFUNCTION(BlueprintCallable, Category = "Lock On Camera")
		FORCEINLINE class ULockOnArmComponent* GetCameraBoom() const { return CameraLockArm; }
	/** Returns FollowCamera subobject **/
	UFUNCTION(BlueprintCallable, Category = "Lock On Camera")
		FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
