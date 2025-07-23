using System;
using System.Runtime.InteropServices;

namespace ScriptAPI
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

        // Combined shutdown and resource release
        [UnmanagedCallersOnly(EntryPoint = "ShutdownManaged")]
        public static void ShutdownManaged()
        {
            LogToNative("Cleaning up managed resources");

            // Release managed resources directly
            LogToNativeDelegate = null;

            // Perform aggressive garbage collection
            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced, true, true);
            GC.WaitForPendingFinalizers();
            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced, true, true);

            LogToNative("Managed cleanup complete");
        }

        [DllImport("Runtime", CallingConvention = CallingConvention.Cdecl)]
        private static extern void LogToNative(string message);

        // Delegate for logging to prevent GC issues
        private static Action<string> LogToNativeDelegate = LogToNative;
    }
}