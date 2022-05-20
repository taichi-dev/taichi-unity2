using System;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Rendering;

public static class UnityNativeGlue {

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
#else
    [DllImport("UnityNativeGlue")]
#endif
    private static extern IntPtr ungGetPluginGfxEventHandlers();


#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
#else
    [DllImport("UnityNativeGlue")]
#endif
    private static extern void ungBindGfxEventArgI32(int eventId, uint binding, int value);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
#else
    [DllImport("UnityNativeGlue")]
#endif
    private static extern void ungBindGfxEventArgF32(int eventId, uint binding, float value);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal", CharSet = CharSet.Ansi, ExactSpelling = true)]
#else
    [DllImport("UnityNativeGlue", CharSet = CharSet.Ansi, ExactSpelling = true)]
#endif
    private static extern void ungBindGfxEventArgString(int eventId, uint binding, string str);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
#else
    [DllImport("UnityNativeGlue")]
#endif
    private static extern void ungBindGfxEventArgBufferPtr(int eventId, uint binding, IntPtr native_buf);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
#else
    [DllImport("UnityNativeGlue")]
#endif
    private static extern void ungBindGfxEventArgTexturePtr(int eventId, uint binding, IntPtr native_tex);


#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
    [DllImport ("__Internal")]
#else
    [DllImport("UnityNativeGlue")]
#endif
    private static extern int ungGetLastError();



    public class EventBuilder {
        private readonly int _EventId;
        private uint _BindingCounter;

        public EventBuilder(int eventId) {
            _EventId = eventId;
            _BindingCounter = 0;
        }

        public EventBuilder ArgI32(int value) {
            ungBindGfxEventArgI32(_EventId, _BindingCounter++, value);
            return this;
        }
        public EventBuilder ArgF32(float value) {
            ungBindGfxEventArgF32(_EventId, _BindingCounter++, value);
            return this;
        }
        public EventBuilder ArgString(string str) {
            ungBindGfxEventArgString(_EventId, _BindingCounter++, str);
            return this;
        }
        public EventBuilder ArgBufferPtr(IntPtr native_buf) {
            ungBindGfxEventArgBufferPtr(_EventId, _BindingCounter++, native_buf);
            return this;
        }
        public EventBuilder ArgTexturePtr(IntPtr native_tex) {
            ungBindGfxEventArgTexturePtr(_EventId, _BindingCounter++, native_tex);
            return this;
        }

        public void Issue() {
            GL.IssuePluginEvent(ungGetPluginGfxEventHandlers(), _EventId);
        }
        public void IssueInCommandBuffer(CommandBuffer cmdbuf) {
            cmdbuf.IssuePluginEvent(ungGetPluginGfxEventHandlers(), _EventId);
        }
    }

    public static EventBuilder BuildEvent(int eventId) {
        return new EventBuilder(eventId);
    }
    public static int GetLastError() {
        return ungGetLastError();
    }
}
