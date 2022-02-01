// Fill out your copyright notice in the Description page of Project Settings.


#include "DSCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LockOnTargetComponent.h"
#include "LockOnArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ADSCharacter
ADSCharacter::ADSCharacter()
{
	/* Lock On ���� */
	LockonControlRotationRate = 10.f;	// Lock On �� ��󿡰� �̵� �ϴ� ī�޶��� �ӵ��� ����												(Default 10.f)
	//Soft Lock Off
	TargetSwitchMouseDelta = 3.f;		// Lock On ����� ��ȯ�ϱ� ���� �Է����� ���ֵǴ� ���콺 �����ӿ� ���� ���ġ�� ����				(Default 3.f)
	TargetSwitchMinDelaySeconds = .5f;	// Lock On ��� ��ȯ delay. Lock On System�� ����� ���̱� ���� ���								(Default .5f)
	//Soft Lock On
	BreakLockMouseDelta = 10.f;			// Lock On �� �����ϱ� ���� �Է����� ���ֵǴ� ���콺�� �����ӿ� ���� ����							(Default 10.f)
	BrokeLockAimingCooldown = .5f;		// ���콺 ���������� Lock On ��� ��ȯ�� �÷��̾��� ī�޶� �Է� delay.								(Default .5f)

	// Gameped �Ƴ��α� ��ƽ
	TargetSwitchAnalogValue = .8f;		// Lock On ����� ��ȯ�ϱ� ���� �Է����� ���ֵǴ� �Ƴ��α� ��ƽ �����ӿ� ���� ���ġ�� ����			(Default .8f)
	BaseTurnRate = 45.f;				// Gameped �Ƴ��α� ��ƽ X�� ������ Base turn rate													(Default 45.f)
	BaseLookUpRate = 45.f;				// Gameped �Ƴ��α� ��ƽ Y�� ������ LookUp Rate														(Default 45.f)

	// ĳ���� ĸ�� ������Ʈ ������ (Default 34.f, 88.f)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// ��Ʈ�ѷ��� Pitch, Yaw, Roll �� ������� �ʱ�. ī�޶󿡸� ������ ��ġ�� ����.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// ĳ���� �̵� ����
	GetCharacterMovement()->bOrientRotationToMovement = true; // ĳ������ �̵������� �����Ѵ�...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...���⼭ ������ ȸ�� �ӵ���									(Default 0.f,540.f,0.f)
	GetCharacterMovement()->JumpZVelocity = 600.f;  // ĳ������ ���� ����																	(Default 600.f)
	GetCharacterMovement()->AirControl = 0.0f; // ĳ���Ͱ� ���߿� ������ �����ϼ� �ִ� �� ����												(Default 0.2f)


	// Spring Arm ����(ī�޶� Block �ϴ� �ݸ����� �浹 ���� ��� �÷��̾� ������ �������).
	CameraLockArm = CreateDefaultSubobject<ULockOnArmComponent>(TEXT("CameraLockArm")); // ���۳�Ʈ �߰� CameraLockArm ���� �̸� ����
	CameraLockArm->SetupAttachment(RootComponent); // ���۳�Ʈ�� ��ġ�� "RootComponent"�� ������
	CameraLockArm->SetRelativeLocation(FVector(0.f, 0.f, 50.f)); // �� ĳ������ ���۳�Ʈ Ʈ������ ���� �����̼� ���� ����

	// Follo Camera ����
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")); // ���۳�Ʈ �߰� FollowCamera �� �̸� ����
	FollowCamera->SetupAttachment(CameraLockArm, USpringArmComponent::SocketName); // ī�޶� Spring Arm ���� �޾� ��Ʈ�ѷ��� ���⿡ ���߾� Spring Arm�� ����
	FollowCamera->bUsePawnControlRotation = false; // ī�޶� Spring Arm �� �������� ȸ������ �ʵ��� ����

	// target component ���� (Lock On �� ���� �ٿ��)
	TargetComponent = CreateDefaultSubobject<ULockOnTargetComponent>(TEXT("TargetComponent"));
	TargetComponent->SetupAttachment(GetRootComponent());

}

// Called to bind functionality to input
void ADSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// �����÷��� Ű ���ε�

	/* PlayerInputComponent �� �ִ��� Assert ���� */
	check(PlayerInputComponent);

	// Ű���� �Է�
	PlayerInputComponent->BindAxis("MoveForward", this, &ADSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADSCharacter::MoveRight);

	// ���콺 �Է�
	PlayerInputComponent->BindAxis("Turn", this, &ADSCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ADSCharacter::LookUp);

	// Gameped �Ƴ��α� ��ƽ �Է�
	PlayerInputComponent->BindAxis("TurnRate", this, &ADSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADSCharacter::LookUpAtRate);

	// Lock On �Է�
	PlayerInputComponent->BindAction("ToggleCameraLock", IE_Pressed, CameraLockArm, &ULockOnArmComponent::ToggleCameraLock);
	PlayerInputComponent->BindAction("ToggleSoftLock", IE_Pressed, CameraLockArm, &ULockOnArmComponent::ToggleSoftLock);
}

// ���� �Է�
void ADSCharacter::MoveForward(float Val)
{
	if ((Controller != NULL) && (Val != .0f))
	{
		// �յ� ����� ���� �̵��ϴ��� �˾Ƴ���(Lock On �� Target�� ������ Controller�� �����̼ǰ� ��ȯ�ϰ�, Ÿ���� ������ (Ÿ�� �����̼� - ���� �����̼�)A�� �������� ���� ������ A�� ���������� ���� 1�� ������ ���°��� �ٽ� A�� ������ �����̼ǰ����� ��ȯ 
		// �������Ʈ Get Unit Direction (vector) �� �ش�
		const FRotator Rotation = CameraLockArm->CameraTarget == nullptr ? Controller->GetControlRotation() : (CameraLockArm->CameraTarget->GetOwner()->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// ���� �Ǵ� Ÿ�����κ����� forward vector �� ��ǲ
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Val);
	}
}

// �¿� �Է�
void ADSCharacter::MoveRight(float Val)
{
	if ((Controller != NULL) && (Val != 0.0f))
	{
		// �¿� ����� ���� �̵��ϴ��� �˾Ƴ���
		const FRotator Rotation = CameraLockArm->CameraTarget == nullptr ? Controller->GetControlRotation() : (CameraLockArm->CameraTarget->GetOwner()->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// ���� �Ǵ� Ÿ�����κ����� right vector �� ��ǲ
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Val);
	}
}

// ���콺 X�� �Է�
void ADSCharacter::Turn(float Val)
{
	float TimeSinceLastTargetSwitch = GetWorld()->GetRealTimeSeconds() - LastTargetSwitchTime;
	//Lock On ��
	if (CameraLockArm->IsCameraLockedToTarget())
	{
		// Soft Lock On �����϶� Ÿ�� ����
		if (CameraLockArm->bUseSoftLock && FMath::Abs(Val) > BreakLockMouseDelta) //Soft Lock On �����̰� ���콺 �������� BreakLockMouseDelta �̻����� �����̸�...
		{
			CameraLockArm->BreakTargetLock(); // ...BreakTargetLock �Լ��� �����ض�
			BrokeLockTime = GetWorld()->GetRealTimeSeconds(); // ��ٿ��� ���� �ð� ����
			CameraLockArm->bSoftlockRequiresReset = true; // ����Ʈ�� �缳�� ���� true 
		}
		// Soft Lock Off �����϶�
		else if (FMath::Abs(Val) > TargetSwitchMouseDelta // Soft Lock Off �����̰� ���콺 �������� TargetSwitchMouseDelta �̻����� �����̰�...
			&& TimeSinceLastTargetSwitch > TargetSwitchMinDelaySeconds)	// ...����Ī ��Ÿ���� �������� �����ض�
		{
			if (Val < 0)
				CameraLockArm->SwitchTarget(EDirection::Left);
			else
				CameraLockArm->SwitchTarget(EDirection::Right);

			LastTargetSwitchTime = GetWorld()->GetRealTimeSeconds(); // ����Ī ��Ÿ�� ���� �ð� ����
		}
	}
	else
	{
		// If camera lock was recently broken by a large mouse delta, allow a cooldown time to prevent erratic camera movement
		bool bRecentlyBrokeLock = (GetWorld()->GetRealTimeSeconds() - BrokeLockTime) < BrokeLockAimingCooldown;
		if (!bRecentlyBrokeLock)
			AddControllerYawInput(Val);
	}
}

//���콺 Y�� �Է�
void ADSCharacter::LookUp(float Val)
{
	if (!CameraLockArm->IsCameraLockedToTarget())
		AddControllerPitchInput(Val);
}

//Gameped X�� �Է�
void ADSCharacter::TurnAtRate(float Val)
{
	// ������ Ÿ�� ����ġ �õ� ���� �Ƴ��α� ��ƽ�� �߸����� ���� ������ Ȯ���ʿ�
	if (FMath::Abs(Val) < .1f)
		bAnalogSettledSinceLastTargetSwitch = true;

	if (CameraLockArm->IsCameraLockedToTarget() && (FMath::Abs(Val) > TargetSwitchAnalogValue) && bAnalogSettledSinceLastTargetSwitch)
	{
		if (Val < 0)
			CameraLockArm->SwitchTarget(EDirection::Left);
		else
			CameraLockArm->SwitchTarget(EDirection::Right);

		bAnalogSettledSinceLastTargetSwitch = false;
	}
	else
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Val * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

// Gameped Y�� �Է�
void ADSCharacter::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	if (!CameraLockArm->IsCameraLockedToTarget())
		AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// Lock On �� ī�޶� ����
void ADSCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (CameraLockArm->IsCameraLockedToTarget())
	{
		// �÷��̾�� Ÿ������ ī�޶� ���� ��ȯ�� InterpTo ���Ͽ� easein���� �̵�... 
		FVector TargetVect = CameraLockArm->CameraTarget->GetComponentLocation() - CameraLockArm->GetComponentLocation();
		FRotator TargetRot = TargetVect.GetSafeNormal().Rotation();
		FRotator CurrentRot = GetControlRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, LockonControlRotationRate);

		//... �ϰ� ī�޶� ���͸� ƽ���� ����
		GetController()->SetControlRotation(NewRot);
	}
}


