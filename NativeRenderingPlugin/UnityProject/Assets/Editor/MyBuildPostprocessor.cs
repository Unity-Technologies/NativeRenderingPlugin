using UnityEngine;
using UnityEditor;
using UnityEditor.Callbacks;
using System.IO;

public class MyBuildPostprocessor
{
	[PostProcessBuild]
	public static void OnPostprocessBuild(BuildTarget target, string pathToBuiltProject)
	{
		if (target == BuildTarget.iOS)
			OnPostprocessBuildIOS(pathToBuiltProject);
	}

	private static void OnPostprocessBuildIOS(string pathToBuiltProject)
	{
#if UNITY_IOS //@TODO: UnityEditor.iOS.Xcode not available if iOS editor module is not installed; disable for now
		// iOS cannot actually use dynamic libraries, so we need to link statically
		// and for that we need to include files into project

		string projPath = pathToBuiltProject + "/Unity-iPhone.xcodeproj/project.pbxproj";

		UnityEditor.iOS.Xcode.PBXProject proj = new UnityEditor.iOS.Xcode.PBXProject();
		proj.ReadFromString(File.ReadAllText(projPath));
		string target = proj.TargetGuidByName("Unity-iPhone");

		Directory.CreateDirectory(Path.Combine(pathToBuiltProject, "Libraries/Unity"));

		string[] pluginSrcFile = new string[]
		{
			"../RenderingPlugin/RenderingPlugin.h",
			"../RenderingPlugin/RenderingPlugin.cpp",
			"../RenderingPlugin/Metal/RenderingPluginMetal.mm",
		};
		string[] pluginXCodeFile = new string[]
		{
			"Libraries/RenderingPlugin.h",
			"Libraries/RenderingPlugin.cpp",
			"Libraries/RenderingPluginMetal.mm",
		};

		for(int i = 0 ; i < pluginXCodeFile.Length ; ++i)
		{
			File.Copy(pluginSrcFile[i], Path.Combine(pathToBuiltProject, pluginXCodeFile[i]), true);
			proj.AddFileToBuild(target, proj.AddFile(pluginXCodeFile[i], pluginXCodeFile[i]));
		}

		File.WriteAllText(projPath, proj.WriteToString());
#endif
	}
}
