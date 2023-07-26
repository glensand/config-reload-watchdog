// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ConfigWatchDogEditorTarget : TargetRules
{
	public ConfigWatchDogEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V3;

		ExtraModuleNames.AddRange( new string[] { "ConfigWatchDog" } );
	}
}
