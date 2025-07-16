using System;
using System.Runtime.InteropServices;

public class ManagedApp
{
    // Callback delegate for string passing
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void StringCallback(IntPtr message);

    // Import from ScriptAPI instead of __Internal
    [DllImport("ScriptAPI", CallingConvention = CallingConvention.Cdecl)]
    public static extern void CreateString(StringCallback callback, IntPtr message);

    // Managed methods called from native
    [UnmanagedCallersOnly(EntryPoint = "InitializeManaged")]
    public static void Initialize()
    {
        Console.WriteLine("[Managed] Initialized!");
        string message = "Hello from embedded C#!";
        IntPtr messagePtr = Marshal.StringToHGlobalAnsi(message);
        CreateString(LogMessage, messagePtr);
        Marshal.FreeHGlobal(messagePtr);
    }

    [UnmanagedCallersOnly(EntryPoint = "UpdateManaged")]
    public static void Update()
    {
        string message = "Updating from managed code...";
        IntPtr messagePtr = Marshal.StringToHGlobalAnsi(message);
        CreateString(LogMessage, messagePtr);
        Marshal.FreeHGlobal(messagePtr);
    }

    // Callback implementation
    public static void LogMessage(IntPtr messagePtr)
    {
        string message = Marshal.PtrToStringAnsi(messagePtr);
        Console.WriteLine($"[Managed] Received: {message}");
    }
}