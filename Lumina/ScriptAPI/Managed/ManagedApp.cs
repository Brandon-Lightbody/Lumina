using System;
using System.Runtime.InteropServices;

public class ManagedApp
{
    [DllImport("ScriptAPI", CallingConvention = CallingConvention.Cdecl)]
    private static extern void NativeLog(string message);

    public static void Initialize()
    {
        Console.WriteLine("[Managed] Initialized");
        NativeLog("Hello from C#!");
    }
}