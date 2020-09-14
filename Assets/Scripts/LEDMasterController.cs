using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Threading;
using UnityEngine.UI;

using System.IO.Ports;


public class LEDMasterController : MonoBehaviour
{

    //////////////////////////////////
    //
    // Private Variable
    //
    //////////////////////////////////
    /// <summary>
<<<<<<< Updated upstream
    public string m_portName1 = "COM22"; // should be specified in the inspector, serial1
    public string m_portName2 = "COM11"; // should be specified in the inspector, serial

    int m_NumOfThread1Start = 0;
    int m_NumOfThread2Start = 0;
=======
    public string m_portName1 = "COM22"; // should be specified in the inspector    COM22=Serial1
    public string m_portName2 = "COM11"; // should be specified in the inspector    COM11=Serial
>>>>>>> Stashed changes

    SerialPort m_serialPort1, m_serialPort2;
    //public int m_threadCounter = 0;
    //public int m_arduinoSendPeriod = 500;    // Is it used somewhere else?

    /// </summary>
    //SerialToArduinoMgr m_SerialToArduinoMgr; 
    Thread m_Thread1;      // thread for the first serial line, serial1
    Thread m_Thread2;      // thread for the second serial line, serial
    Action m_updateArduino1;
    Action m_updateArduino2;

    int m_NumOfLEDsLeft;
    int m_NumOfLEDsRight;

    LEDColorGenController m_LEDColorGenController;

    //public byte[] m_LEDArray; // 200  LEDs
    public byte[] m_LEDArray1; // 200  LEDs
    public byte[] m_LEDArray2; // 200  LEDs
    byte[] m_startByte = { 255 };


    float m_Delay;
    public int m_LEDCount; // from LEDColorGenController component

    int m_index;
    //////////////////////////////////
    //
    // Function
    //
    //////////////////////////////////

    //byte[] m_LEDArray1;
    bool m_ThreadCreatedAndStartedFirstTime = false;

    private void Awake()
    { // init me


        // Serial Communication between C# and Arduino
        //http://www.hobbytronics.co.uk/arduino-serial-buffer-size = how to change the buffer size of arduino
        //https://www.codeproject.com/Articles/473828/Arduino-Csharp-and-Serial-Interface
        //Don't forget that the Arduino reboots every time you open the serial port from the computer - 
        //and the bootloader runs for a second or two gobbling up any serial data you send its way..
        //This takes a second or so, and during that time any data that you send is lost.
        //https://forum.arduino.cc/index.php?topic=459847.0

        //https://docs.microsoft.com/en-us/dotnet/api/system.io.ports.serialport.writetimeout?view=netframework-4.8

        // Set up the serial Port

        //https://forum.unity.com/threads/serial-port-communication-in-unity-c.600511/

        //m_serialPort1 = new SerialPort(m_portName1, 57600); // bit rate= 567000 bps = , serial1
        //m_serialPort2 = new SerialPort(m_portName2, 57600); // bit rate= 567000 bps = ,serial


        //m_SerialPort.ReadTimeout = 50;
        // m_serialPort.ReadTimeout = 1000;  // sets the timeout value: 1000 ms  = sufficient for our purpose?
        // InfiniteTimeout is the default.
        //  m_SerialPort1.WriteTimeout = 5000??

        try
        {
<<<<<<< Updated upstream
            //m_serialPort1.Open();      // serial1
            // m_serialPort2.Open();        // serial
=======
            //m_serialPort1.Open();  //Serial1
            m_serialPort2.Open();  //Serial
>>>>>>> Stashed changes
        }
        catch (Exception ex)
        {
            Debug.Log("Error:" + ex.ToString());
            //#if UNITY_EDITOR
            //            // Application.Quit() does not work in the editor so
            //            // UnityEditor.EditorApplication.isPlaying = false;
            //            UnityEditor.EditorApplication.Exit(0);
            //#else
            //                   Application.Quit();
            //#endif
        }


        //m_SerialToArduinoMgr = new SerialToArduinoMgr();

        //m_SerialToArduinoMgr.Setup();

        //m_port = m_SerialToArduinoMgr.port;
    }

    //https://www.codeproject.com/Questions/1178661/How-do-I-receive-a-byte-array-over-the-serial-port
    //That is most likely because you only read the first byte. 
    //Serial ports are extremely slow and you need to keep checking whether there is any data ready to read,
    //you cannot assume that you will get the complete message on your first attempt.

    //int MyInt = 1;

    //byte[] b = BitConverter.GetBytes(MyInt);
    //serialPort1.Write(b,0,4);
    void Start()
    {

        m_LEDColorGenController = this.gameObject.GetComponent<LEDColorGenController>();

        //m_LEDCount = m_LEDColorGenController.m_totalNumOfLEDs + 2;
        m_LEDCount = m_LEDColorGenController.m_totalNumOfLEDs;
        m_NumOfLEDsLeft = m_LEDColorGenController.m_NumOfLEDsLeft;
        m_NumOfLEDsRight = m_LEDColorGenController.m_NumOfLEDsRight;


        //m_LEDArray = new byte[m_LEDCount * 3]; // 186*3 < 1024

        m_LEDArray1 = new byte[m_NumOfLEDsLeft * 3]; // 186*3 < 1024
        m_LEDArray2 = new byte[m_NumOfLEDsRight * 3]; // 186*3 < 1024

        //// define an action
        //m_updateArduino1 = () =>
        //{

        //    try
        //    { //https://social.msdn.microsoft.com/Forums/vstudio/en-US/93583332-d307-4552-bd61-9a2adfcf2480/serial-port-write-method-is-blocking-execution?forum=vbgeneral

        //        //Yes, the Write methods do block , until all data have been passed from the serial port driver to the UART FIFO.
        //        //Usually, this is not a problem.It will not block "forever," just for as long as it takes.
        //        //For example, if you were to send a 2K byte string, at 9600 bps, the write method would take about 2 seconds to return.

        //        //Write(byte[] buffer, int offset, int count);
        //        m_serialPort1.Write(m_startByte, 0, 1);     // m_startByte = 255
        //        m_serialPort1.Write(m_LEDArray1, 0, m_LEDArray1.Length);   // m_LEDArray refers to the global address which is bound when the thread function is 


        //    }
        //    catch (Exception ex)
        //    {

        //        Debug.Log("Error:" + ex.ToString());

        //    }

        //    // The WriteBufferSize of the Serial Port is 1024, whereas that of Arduino is 64; m_LEDArray is 200 * 3 = 600 bytes less than
        //    // the Serial Port size.
        //    //https://stackoverflow.com/questions/22768668/c-sharp-cant-read-full-buffer-from-serial-port-arduino

        //};      // u_updateArduino1


        // define an action
        m_updateArduino2 = () =>
        {

            try
            { //https://social.msdn.microsoft.com/Forums/vstudio/en-US/93583332-d307-4552-bd61-9a2adfcf2480/serial-port-write-method-is-blocking-execution?forum=vbgeneral


                //Yes, the Write methods do block , until all data have been passed from the serial port driver to the UART FIFO.
                //Usually, this is not a problem.It will not block "forever," just for as long as it takes.
                //For example, if you were to send a 2K byte string, at 9600 bps, the write method would take about 2 seconds to return.

                //Write(byte[] buffer, int offset, int count);

                // m_serialPort2.Write(m_startByte, 0, 1);     // m_startByte = 255
                // m_serialPort2.Write(m_LEDArray2, 0, m_LEDArray2.Length);   // m_LEDArray refers to the global address which is bound when the thread function is 


                // defined; This global address is where the new  frame data is stored every frame.

                Debug.Log("Thread2: defiined the target function");

            }
            catch (Exception ex)
            {

                Debug.Log("Error:" + ex.ToString());

            }

            // The WriteBufferSize of the Serial Port is 1024, whereas that of Arduino is 64; m_LEDArray is 200 * 3 = 600 bytes less than
            // the Serial Port size.
            //https://stackoverflow.com/questions/22768668/c-sharp-cant-read-full-buffer-from-serial-port-arduino

        };       // m_updateArduino2

        //Thread: https://knightk.tistory.com/8
        // m_Thread1 = new Thread(new ThreadStart(m_updateArduino)); // ThreadStart() is a delegate (pointer type)
        // Thread state = unstarted  when created

        //m_Thread1 = new Thread(new ThreadStart(m_updateArduino1)); // ThreadStart() is a delegate (pointer type)
        try
        {
            ThreadStart th = new ThreadStart(m_updateArduino2);   // ThreadStart() is a delegate (pointer type)
            m_Thread2 = new Thread(th);
            m_ThreadCreatedAndStartedFirstTime = true;
        }

        catch (Exception ex)
        {
            Debug.Log(" Thread Creation Exception =" + ex.ToString());
#if UNITY_EDITOR
            // Application.Quit() does not work in the editor so
            // UnityEditor.EditorApplication.isPlaying = false;
            //UnityEditor.EditorApplication.Exit(0);
#else
                   //Application.Quit();
#endif

        }


    }// void Start()



    //m_ledColorGenController.m_ledSenderHandler += UpdateLEDArray; // THis is moved to CommHub.cs

    // public delegate LEDSenderHandler (byte[] LEDArray); defined in LEDColorGenController
    // public event LEDSenderHandler m_ledSenderHandler;

    // UpdateLEDArray() is the event handler for updating the LED array; it is pointed by the delegate
    //  m_ledColorGenController.m_ledSenderHandler, which is defined in Commhub.cs
    public void UpdateLEDArray(byte[] ledArray) // ledArray is a reference type
    {
        // for debugging:    to see if the thread causes the termination of the code
        //return;
        for (int i = 0; i < m_NumOfLEDsLeft * 3; i++)
        {
            m_LEDArray1[i] = ledArray[i];
        }

        for (int i = 0; i < m_NumOfLEDsRight * 3; i++)
        {
            m_LEDArray2[i] = ledArray[m_NumOfLEDsLeft * 3 + i];
        }

        // create test data


        for (int i = 0; i < m_NumOfLEDsLeft; i++)
        {
            m_LEDArray1[3 * i] = 250;
            m_LEDArray1[3 * i + 1] = 0;
            m_LEDArray1[3 * i + 2] = 0;

        }

        // the second chain

        for (int i = 0; i < m_NumOfLEDsRight; i++)
        {
<<<<<<< Updated upstream
            m_LEDArray2[3 * i] = 0;
            m_LEDArray2[3 * i + 1] = 250;
            m_LEDArray2[3 * i + 2] = 0;
=======
            m_LEDArray2 [3 * i] = 0;
            m_LEDArray2[ 3 * i + 1] = 250;
            m_LEDArray2[ 3 * i + 2] = 0;
>>>>>>> Stashed changes

        }



        //Debug.Log("1) Thread State == " + m_Thread.ThreadState);

        //Debug.Log("2) Thread.IsAlive " + m_Thread.IsAlive);

        //       
        if (m_ThreadCreatedAndStartedFirstTime)
        {    // The thread was initially created and started at the same time;
            // Because it is alive, let it go.
            m_ThreadCreatedAndStartedFirstTime = false;

            return;
        }
        else
        {    // The thread was terminated
            if (m_Thread2.IsAlive)   // The thread is alive means that it is started and not terminated.
                                     // not alive means the thread is either not started or terminated. 
                                     // IsAlive is false if the thread method returned or the thread was aborted.

            { // the thread2 is alive: started and not terminated.   The sending thread is still busy sending  the previous LED array
                Debug.Log("Thread2 is alive; Wait until it finishes; it means that the arrived array of led bytes is discarded");

            }
            else  //the thread2 is not alive: it means terminated at this point. 
            {
                Debug.Log(" the thread2 is  terminated; so start it");
                try
                {
                    m_Thread2.Start();
                }

                catch (Exception ex)
                {
                    Debug.Log(" Thread2 Start Exception =" + ex.ToString());
#if UNITY_EDITOR
                    // Application.Quit() does not work in the editor so
                    // UnityEditor.EditorApplication.isPlaying = false;
                    //UnityEditor.EditorApplication.Exit(0);
#else
                   //Application.Quit();
#endif

                }
                m_NumOfThread1Start++;

                //Thread.Sleep(1000);
                Debug.Log(" thread2 (serial) started  to send LED array to arduino: " + m_NumOfThread1Start);


            }     //the thread2 is not alive: it means terminated at this point. 


        } // if(!m_ThreadJustCreatedAndStarted)


    } //  UpdateLEDArray()

    void Update()
    {
    }

}//public class LEDMasterController 
