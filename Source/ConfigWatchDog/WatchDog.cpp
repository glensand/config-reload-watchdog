/* Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
* You may use, distribute and modify this code under the
 * terms of the MIT license.
 *
 * You should have received a copy of the MIT license with
 * this file. If not, please write to: bezborodoff.gleb@gmail.com, or visit : https://github.com/glensand/TinyTls
 */

#include "WatchDog.h"

void UWatchDogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if PLATFORM_WINDOWS && !UE_BUILD_SHIPPING
    const FString Dir = FPaths::ProjectConfigDir();
    IFileManager::Get().IterateDirectoryRecursively(*Dir, [this](const TCHAR* File, bool bDir)
    {
        if (!bDir)
        {
            FString PathStr = File;
            FPaths::NormalizeFilename(PathStr);
            PathStr = FPaths::ConvertRelativePathToFull(PathStr);
            FileToHash.Emplace(MoveTemp(PathStr),  FMD5Hash::HashFile(*PathStr));
        }
        return true;
    });
    
    WakeupEvent = FPlatformProcess::GetSynchEventFromPool(false);
    bRunning.store(true, std::memory_order_release);
    
    UpdateFileStatusThread = std::thread([this]
    {
        while (bRunning.load(std::memory_order_acquire))
        {
            if (UpdateFileStatus())
            {
                AsyncTask(ENamedThreads::GameThread, [this]{ ReloadConfig(); });
            }
            constexpr static int32 TimeOut = 1000; // ms
            WakeupEvent->Wait(TimeOut);
        }  
    });
#endif
}

void UWatchDogSubsystem::Deinitialize()
{
#if PLATFORM_WINDOWS && !UE_BUILD_SHIPPING
    bRunning.store(false, std::memory_order_release);
    WakeupEvent->Trigger();
    UpdateFileStatusThread.join();
#endif
    
    Super::Deinitialize();
}

void UWatchDogSubsystem::ReloadConfig()
{
    FString* FileNamePtr;
    while (PendingRebuildFiles.Dequeue(FileNamePtr))
    {
        GConfig->UnloadFile(*FileNamePtr);
        GConfig->LoadFile(*FileNamePtr);
    }

    OnConfigChanged.Broadcast();
}

bool UWatchDogSubsystem::UpdateFileStatus()
{
    bool bHasChanges = false;
    for (auto&& [File, Hash] : FileToHash)
    {
        if (const auto NewHash = FMD5Hash::HashFile(*File); NewHash != Hash)
        {
            Hash = NewHash;
            bHasChanges = true;
            PendingRebuildFiles.Enqueue(&File);
        }
    }
    return bHasChanges;
}
