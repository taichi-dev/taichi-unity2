using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class Demo : MonoBehaviour {
    // Start is called before the first frame update
    void Start() {
    }

    // Update is called once per frame
    void Update() {
        UnityNativeGlue.BuildEvent(1)
            .ArgString((1.5f).ToString())
            .Issue();
        Debug.Log(UnityNativeGlue.GetLastError().ToString());
    }
}
