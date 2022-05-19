using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class Demo : MonoBehaviour {
    [DllImport("UnityNativeGlue")]
    private static extern IntPtr ungGetPluginGfxEventHandlers();
    [DllImport("UnityNativeGlue")]
    private static extern void ungBindGfxEventArgF32(int eventId, uint binding, float value);

    [DllImport("UnityNativeGlue")]
    private static extern int ungGetLastError();


    // Start is called before the first frame update
    void Start() {
    }

    // Update is called once per frame
    void Update() {
        ungBindGfxEventArgF32(1, 0, 1.5f);
        GL.IssuePluginEvent(ungGetPluginGfxEventHandlers(), 1);
        Debug.Log(ungGetLastError().ToString());
    }
}
