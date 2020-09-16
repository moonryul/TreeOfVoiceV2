using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Threading;
using UnityEngine.UI;

using System.IO.Ports;

//https://jonskeet.uk/csharp/threads/waithandles.html

// Thread in Unity: https://www.universityofgames.net/using-threads-in-unity-engine/
//There are no real problems with threads in Unity.
//Just keep in mind that pretty much everything of the Unity API can only be accessed from the main thread.
//http://www.richardmeredith.net/2017/07/simple-multithreading-for-unity/2/
// http://dotnetpattern.com/threading-autoresetevent
//https://generacodice.it/en/articolo/41855/Concatenate-RTF-files-in-PHP-%28REGEX%29
// Use this example in this code: https://stackoverflow.com/questions/382173/what-are-alternative-ways-to-suspend-and-resume-a-thread
// combined with https://generacodice.it/en/articolo/41855/Concatenate-RTF-files-in-PHP-%28REGEX%29
//https://docs.microsoft.com/en-us/dotnet/api/system.threading.manualresetevent?view=netcore-3.1

// Difference between ManualResetEvent and AutoResetEvent: https://generacodice.it/en/articolo/41855/Concatenate-RTF-files-in-PHP-%28REGEX%29

//  The most important difference is that an AutoResetEvent will only allow 
//    one single waiting thread to continue. A ManualResetEvent on the other hand will keep allowing threads, 
//    several at the same time even, to continue until you tell it to stop(Reset it).

//A ManualResetEvent is a variation on AutoResetEvent.   
//    It differs in that it doesn't automatically reset after a thread is let through on a WaitOne call,
//    and so functions like a gate: calling Set opens the gate, allowing any number of threads 
//    that WaitOne at the gate through; calling Reset closes the gate, causing, potentially,
//    a queue of waiters to accumulate until its next opened.

//One could simulate this functionality with a boolean "gateOpen" field (declared with the volatile keyword) 
//in combination with "spin-sleeping"
//    – repeatedly checking the flag, and then sleeping for a short period of time.

//ManualResetEvents are sometimes used to signal that a particular operation is complete, 
//    or that a thread's completed initialization and is ready to perform work.


public class LEDMasterController : MonoBehaviour
{

    //////////////////////////////////
    //
    // Private Variable
    //
    //////////////////////////////////
    /// <summary>

    public string m_portName0 = "COM26"; // should be specified in the inspector, serial1
    public string m_portName1 = "COM22"; // should be specified in the inspector, serial

 
    SerialPort m_serialPort0, m_serialPort1;
    //public int m_threadCounter = 0;
    //public int m_arduinoSendPeriod = 500;    // Is it used somewhere else?

    /// </summary>
    //SerialToArduinoMgr m_SerialToArduinoMgr; 
    Thread m_Thread1;      // thread for the first serial line, serial1
    Thread m_Thread2;      // thread for the second serial line, serial

    // AutoResetEvent vs. ManualResetEvent:
    // 두 클래스의 생성자에 인자를 주면서 생성하는데 true이면 차단기가 올라간 상태,
    //false이면 차단기가 내려간 상태에서 시작한다. 두 클래스의 차이점은 WaitOne의 상태가 Set으로 풀린 이후 
    //Reset을 수동으로 하느냐 자동으로 하느냐에 있다. 그러므로 WaitOne의 상태가 Set으로 풀린 이후 Reset을, 
    //다른일을 하고, 천천히 수행하려면 ManualResetEvent 클래스를 사용하면 된다. 
    //System.Threading.AutoResetEvent는 WaitOne의 상태가 Set으로 풀린 이후 이후 자동으로 Reset 된다.
    //AutoResetEvent는 자동 Reset을 하므로 사용자의 여러 개의 스레드 실행하면 하나의 스레드가 끝날 때까지 기다렸다가 
    //대기하고, 하나의 스레드가 끝나면 다음 스레드가 실행되고 
    //ManualResetEvent는 자동 Reset을 수동으로 해야 하므로 한번 Set되면  차단기가 올라간 상태에서 여러 개의 스레드가 한번에 실행된다.


    // We create two waitEventHandles on the same set of threads to handle different aspects of the threads,
    // that Pause and Resume versus Shutdown.
    ManualResetEvent m_ShutdownEvent = new ManualResetEvent(false);
    ManualResetEvent m_PauseEvent = new ManualResetEvent(false);   
    // do not block but go thru it. 

    bool  m_Thread1Stopped = false;
    bool m_Thread2Stopped = false;

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
    bool m_NewLEDFrameHasArrived  = false;
                                                                          
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

            //m_serialPort1.Open();  //Serial1
           // m_serialPort0.Open();  //Serial

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


        // define an action
        m_updateArduino1 = () =>
        {
            while (true)
            {
                //The thread  blocks initially on the m_PauseEvent.WaitOne() because m_PauseEvent
                // was created with the unsignaled state (signal being false)
                m_PauseEvent.WaitOne(Timeout.Infinite);   // wait indefinitely until the main thread
                                                          // until it gets the signal
                if (m_ShutdownEvent.WaitOne(0))
                    break;

                // do work
                Debug.Log("Thread2: sent the LED data");

                try
                { //https://social.msdn.microsoft.com/Forums/vstudio/en-US/93583332-d307-4552-bd61-9a2adfcf2480/serial-port-write-method-is-blocking-execution?forum=vbgeneral


                    //The Write methods do block , until all data have been passed from the serial port driver to the UART FIFO.
                    //Usually, this is not a problem.It will not block "forever," just for as long as it takes.
                    //For example, if you were to send a 2K byte string, at 9600 bps, the write method would take about 2 seconds to return.

                    //Write(byte[] buffer, int offset, int count);

                    // m_serialPort2.Write(m_startByte, 0, 1);     // m_startByte = 255
                    // m_serialPort2.Write(m_LEDArray1, 0, m_LEDArray1.Length);   // m_LEDArray refers to the global address which is bound when the thread function is 


                    // defined; This global address is where the new  frame data is stored every frame.
                    Debug.Log("Thread 1 Printing");

                    for (int i = 0; i < m_NumOfLEDsLeft * 3; i++)
                    {
                        if (i % 3 == 0)
                        {
                            Debug.Log("R: " + m_LEDArray1[i]);
                        }
                        else if (i % 3 == 1)
                        {
                            Debug.Log("G: " + m_LEDArray1[i]);
                        }
                        else
                        {
                            Debug.Log("R: " + m_LEDArray1[i]);

                        }
                    } // for 


                }
                catch (Exception ex)
                {

                    Debug.Log("Error:" + ex.ToString());

                }

                // The WriteBufferSize of the Serial Port is 1024, whereas that of Arduino is 64; m_LEDArray is 200 * 3 = 600 bytes less than
                // the Serial Port size.
                //https://stackoverflow.com/questions/22768668/c-sharp-cant-read-full-buffer-from-serial-port-arduino

            } // while (true)
        };       // m_updateArduino1


        // define an action
        m_updateArduino2 = () =>
        {
            while (true)
            {
                //The thread  blocks initially on the m_PauseEvent.WaitOne() because m_PauseEvent
                // was created with the unsignaled state (signal being false)
                m_PauseEvent.WaitOne(Timeout.Infinite);   // wait indefinitely until the main thread
                                                          // executes m_PauseEvent.Set()
                if ( m_ShutdownEvent.WaitOne(0))       //m_ShutdownEvent.WaitOne(0) becomes true when
                                                       // the main thread executes m_ShutdownEvent.Set()
                    break;

                // do work
                Debug.Log("Thread2: sent the LED data");

                try
                { //https://social.msdn.microsoft.com/Forums/vstudio/en-US/93583332-d307-4552-bd61-9a2adfcf2480/serial-port-write-method-is-blocking-execution?forum=vbgeneral


                    //The Write methods do block , until all data have been passed from the serial port driver to the UART FIFO.
                    //Usually, this is not a problem.It will not block "forever," just for as long as it takes.
                    //For example, if you were to send a 2K byte string, at 9600 bps, the write method would take about 2 seconds to return.

                    //Write(byte[] buffer, int offset, int count);

                    // m_serialPort2.Write(m_startByte, 0, 1);     // m_startByte = 255
                    // m_serialPort2.Write(m_LEDArray2, 0, m_LEDArray2.Length);   // m_LEDArray refers to the global address which is bound when the thread function is 


                    // defined; This global address is where the new  frame data is stored every frame.

                    Debug.Log("Thread 2 Printing");

                    for (int i = 0; i < m_NumOfLEDsRight * 3; i++)
                    {
                        if (i % 3 == 0)
                        {
                            Debug.Log("R: " + m_LEDArray2[i]);
                        }
                        else if (i % 3 == 1)
                        {
                            Debug.Log("G: " + m_LEDArray2[i]);
                        }
                        else
                        {
                            Debug.Log("R: " + m_LEDArray2[i]);

                        }
                    } // for 


                }
                catch (Exception ex)
                {

                    Debug.Log("Error:" + ex.ToString());

                }

                // The WriteBufferSize of the Serial Port is 1024, whereas that of Arduino is 64; m_LEDArray is 200 * 3 = 600 bytes less than
                // the Serial Port size.
                //https://stackoverflow.com/questions/22768668/c-sharp-cant-read-full-buffer-from-serial-port-arduino

            } // while (true)
        };       // m_updateArduino2

        //Thread: https://knightk.tistory.com/8
        // m_Thread1 = new Thread(new ThreadStart(m_updateArduino)); // ThreadStart() is a delegate (pointer type)
        // Thread state = unstarted  when created

        //m_Thread1 = new Thread(new ThreadStart(m_updateArduino1)); // ThreadStart() is a delegate (pointer type)
        try
        {
            ThreadStart th1 = new ThreadStart(m_updateArduino1);
            // ThreadStart() is a delegate. a type-safe function pointer
            //All threads require an entry point to start execution. By definition when a primary thread is created
            //it always runs Main() as it's entry point. 
            //   Any additional threads you create will need an explicitly defined entry point -
            // a pointer to the function where they should begin execution. So threads always require a delegate,
            //  "ThreadStart" means the entry point of the thread.
            // cf: https://stackoverflow.com/questions/3793105/threads-and-delegates-i-dont-fully-understand-their-relations

            m_Thread1 = new Thread(th1);
        


            ThreadStart th2 = new ThreadStart(m_updateArduino2);
            // ThreadStart() is a delegate. a type-safe function pointer
            //All threads require an entry point to start execution. By definition when a primary thread is created
            //it always runs Main() as it's entry point. 
            //   Any additional threads you create will need an explicitly defined entry point -
            // a pointer to the function where they should begin execution. So threads always require a delegate,
            //  "ThreadStart" means the entry point of the thread.
            // cf: https://stackoverflow.com/questions/3793105/threads-and-delegates-i-dont-fully-understand-their-relations

            m_Thread2 = new Thread(th2);

            m_Thread1.Start();
            m_Thread2.Start();



        }

        catch (Exception ex)
        {
            Debug.Log(" Threads Creation and Start Exception =" + ex.ToString());
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

            m_LEDArray2[3 * i] = 0;
            m_LEDArray2[3 * i + 1] = 250;
            m_LEDArray2[3 * i + 2] = 0;


        }
        //the Set method releases all the threads; They go thru WaitOne() immediately, without blocking
        // because m_PauseEvent waitHandle was created with the signaled state (the boolean label true);
         
        m_PauseEvent.Set();

        //Once we call the Set() method on the ManualResetEvent object, its boolean remains true.
        // To reset the value we can use Reset() method.Reset method change the boolean value to false.     
        //Below is the syntax of calling Reset method.     
        //manualResetEvent.Reset();
        // We must immediately call Reset method after calling Set method 
        // if we want to send signal to threads multiple times. 

    } //  UpdateLEDArray()

    void Update()
    {
    }

   

    public void OnDestroy()
    {
        // Signal the shutdown event
        m_ShutdownEvent.Set();
        Debug.Log("Thread Stopped ");

        // Make sure to resume any paused threads
        m_PauseEvent.Set();

        // Wait for the thread to exit
        m_Thread1.Join();
        m_Thread2.Join();
    }


    public void OnApplicationQuit()
    {

        // Signal the shutdown event
        m_ShutdownEvent.Set();
        Debug.Log("Thread Stopped ");

        // Make sure to resume any paused threads
        m_PauseEvent.Set();

        // Wait for the thread to exit
        m_Thread1.Join();
        m_Thread2.Join();


    }

}//public class LEDMasterController 
