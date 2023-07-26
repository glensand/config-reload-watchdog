/* Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
* You may use, distribute and modify this code under the
 * terms of the MIT license.
 *
 * You should have received a copy of the MIT license with
 * this file. If not, please write to: bezborodoff.gleb@gmail.com, or visit : https://github.com/glensand/TinyTls
 */

#pragma once

#include "CoreMinimal.h"
#include <thread>

#include "WatchDog.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfigChanged);

UCLASS()
class CONFIGWATCHDOG_API UWatchDogSubsystem final : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY()
    FOnConfigChanged OnConfigChanged;
    
private:
    void ReloadConfig();
    bool UpdateFileStatus();
    
    TMap<FString, FMD5Hash> FileToHash;
    TQueue<FString*> PendingRebuildFiles;
    std::thread UpdateFileStatusThread;
    std::atomic_bool bRunning;
    FEvent* WakeupEvent;
};
