using System;
using System.Runtime.InteropServices;

namespace Lumina.Managed
{
    public static class NativeExports
    {
        [UnmanagedCallersOnly(EntryPoint = "InitializeManaged")]
        public static void InitializeManaged()
        {
            LogToNative("CoreCLR initialized successfully!");
        }

        [UnmanagedCallersOnly(EntryPoint = "UpdateManaged")]
        public static void UpdateManaged()
        {
            LogToNative("Update called from managed code");
        }

        [DllImport("ScriptAPI", CallingConvention = CallingConvention.Cdecl)]
        private static extern void LogToNative(string message);
    }
}