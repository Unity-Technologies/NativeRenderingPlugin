#import "UnityAppController.h"
#include "Unity/IUnityGraphics.h"

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();

@interface MyAppController : UnityAppController
{
}
- (void)shouldAttachRenderDelegate;
@end
@implementation MyAppController
- (void)shouldAttachRenderDelegate
{
	// unlike desktops where plugin dynamic library is automatically loaded and registered
	// we need to do that manually on iOS
	UnityRegisterRenderingPluginV5(&UnityPluginLoad, &UnityPluginUnload);
}

@end
IMPL_APP_CONTROLLER_SUBCLASS(MyAppController);
