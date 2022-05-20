using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class Demo : MonoBehaviour {
    [DllImport("UnityNativeGlue")]
    private static extern IntPtr ungGetPluginGfxEventHandlers();

    [DllImport("UnityNativeGlue")]
    private static extern void ungBindGfxEventArgI32(int eventId, uint binding, int value);
    [DllImport("UnityNativeGlue")]
    private static extern void ungBindGfxEventArgF32(int eventId, uint binding, float value);
    [DllImport("UnityNativeGlue", CharSet = CharSet.Ansi, ExactSpelling = true)]
    private static extern void ungBindGfxEventArgString(int eventId, uint binding, string value);
    [DllImport("UnityNativeGlue")]
    private static extern void ungBindGfxEventArgBufferPtr(int eventId, uint binding, float value);
    [DllImport("UnityNativeGlue")]
    private static extern void ungBindGfxEventArgTexturePtr(int eventId, uint binding, float value);

    [DllImport("UnityNativeGlue")]
    private static extern int ungGetLastError();


    // Start is called before the first frame update
    void Start() {
    }

    // Update is called once per frame
    void Update() {
        ungBindGfxEventArgString(1, 0, (1.5f).ToString());
        GL.IssuePluginEvent(ungGetPluginGfxEventHandlers(), 1);
        Debug.Log(ungGetLastError().ToString());
    }
}
