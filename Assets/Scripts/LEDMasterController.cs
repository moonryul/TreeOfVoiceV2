using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Threading;
using UnityEngine.UI;

using System.IO.Ports;

//https://jonskeet.uk/csharp/threads/waithandles.html
//http://www.richardmeredith.net/2017/07/simple-multithreading-for-unity/2/
//Returns
//Boolean
//true if both the signal and the wait complete successfully; if the wait does not complete, the method does not return.

//nvalidOperationException
//toSignal is a semaphore, and it already has a full count.

//public static bool SignalAndWait (System.Threading.WaitHandle toSignal, System.Threading.WaitHandle toWaitOn);

// Thread in Unity: https://www.universityofgames.net/using-threads-in-unity-engine/
//There are no real problems with threads in Unity.
//Just keep in mind that pretty much everything of the Unity API can only be accessed from the main thread.
// https://arghya.xyz/articles/thread-synchronization-part-three/

//Event & Wait Handles
//Multiple threads can be synchronized by using synchronization events, 
//    which are objects of EventWaitHandle or it’s child types.
//    These objects are meant to handle thread waiting, and some events to signal them.
//    They can have named instances for system-wide synchronization.EventWaitHandle are like gates,
//        which has 2 possible states


//signaled - waiting threads are activated & executes (open)
//nonsignaled - waiting threads are suspended, continue to wait in a queue (closed)
//There are three important methods in EventWaitHandle

//Set() - signals to say it is open now, threads can proceed.State becomes signaled
//Reset() - signals to say it’s closed, other threads have to wait.State becomes nonsignaled
//WaitOne() - a thread calls this to wait for an open signal.If current state is signaled, 
//it can proceed immediately, else it has to wait until it gets a signal
//So, how the synchronization works among multiple threads is - all threads who wants
//        to use the shared resources, calls WaitOne() and waits for a signal.
//        When some thread calls Set() (or if the synchronization object was already signaled), 
//the waiting threads proceed to access the resource.Now, when Reset() is called from a thread, 
//    no more new threads can proceed and have to wait.The object remains in nonsigaled state
//    until someone calls Set() again.When Set() is called, if no threads are waiting, 
//    the synchronization object remains in signaled state, and new threads can start the work immediately.

//There are several types of synchronization events


//AutoResetEvent - allows exclusive access to a single thread at a time. It maintains 
//        an internal queue of all waiting threads, and lets one thread pass when it is signaled.
//    The moment one thread with WaitOne() gets activated, it automatically resets to nonsignaled state.
//    Hence the name.
//ManualResetEvent - allows access to any number of threads in signaled state.When it is in signaled state, 
//    it allows all of the waiting threads to get activated, and keeps allowing new threads 
//    until some thread calls Reset() manually to put it in nonsignaled state.
//    That’s why it is called manual-reset event.

// AutoResetEvent vs. ManualResetEvent:
// 두 클래스의 생성자에 인자를 주면서 생성하는데 true이면 차단기가 올라간 상태,
//false이면 차단기가 내려간 상태에서 시작한다. 두 클래스의 차이점은 WaitOne의 상태가 Set으로 풀린 이후 
//Reset을 수동으로 하느냐 자동으로 하느냐에 있다. 그러므로 WaitOne의 상태가 Set으로 풀린 이후 Reset을, 
//다른일을 하고, 천천히 수행하려면 ManualResetEvent 클래스를 사용하면 된다. 
//System.Threading.AutoResetEvent는 WaitOne의 상태가 Set으로 풀린 이후 이후 자동으로 Reset 된다.
//AutoResetEvent는 자동 Reset을 하므로 사용자의 여러 개의 스레드 실행하면 하나의 스레드가 끝날 때까지 기다렸다가 
//대기하고, 하나의 스레드가 끝나면 다음 스레드가 실행되고 
//ManualResetEvent는 자동 Reset을 수동으로 해야 하므로 한번 Set되면  차단기가 올라간 상태에서
//여러 개의 스레드가 한번에 실행된다.

//Once we call the Set() method on the ManualResetEvent object, its boolean remains true.
// To reset the value we can use Reset() method.Reset method change the boolean value to false.     
//Below is the syntax of calling Reset method.     
//manualResetEvent.Reset();
// We must immediately call Reset method after calling Set method 
// if we want to send signal to threads multiple times. => This may cause the overwriting of SignaledState
// variable. 
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

   

    // We create two waitEventHandles on the same set of threads to handle different aspects of the threads,
    // that Pause and Resume versus Shutdown.
    ManualResetEvent m_ChildThreadWaitEvent = new ManualResetEvent(false);
    ManualResetEvent m_ChildThreadPassThruEvent = new ManualResetEvent(false);
    ManualResetEvent m_ThreadShutdownEvent = new ManualResetEvent(false);   
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
    byte[] m_startByte = { (byte)'\n' };
    byte[] m_endByte = { (byte)'\r' };

   
    public int m_LEDCount; // from LEDColorGenController component

    public int m_LEDArraySemaphore = 2; // two threads should finish processing its own LED array for the 
    // main thread to prepare new LED  arrays.
    
     int  m_NumOfLEDFrames = 0;
                                                                          
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

        //m_serialPort1 = new SerialPort(m_portName1, 115200); // Serial of Master 1
        //m_serialPort2 = new SerialPort(m_portName2, 115200); // Serial of Master 2


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
                //The thread  blocks initially because  the thread sync event  m_ThreadSyncEvent was
                // created with the initial stated being unsigned (the input parameter = false)

                // Wait for the wait gate to open
                //Debug.LogFormat("Number: {0}, string {1}, number again: {0}, character: {2}", num, str, chr);
                Debug.Log("Thread 1:  waiting for the gate to open");
                
                m_ChildThreadWaitEvent.WaitOne(Timeout.Infinite);   // The current thread wait indefinitely until the main thread
                                                                    // until it gets the signal from other threads

                // m_ChildThreadPassThruEvent.Set(); // Indicate that the current child thread has passed 
                // thru m_ChildThreadWaitEvent.WaitOne(); 

                Debug.Log("Thread 1: the gate is open");
                
                if (m_ThreadShutdownEvent.WaitOne(0)) // The current thread also waits on another sync event
                                                      // m_ThreadShutdownEvent to handle thread shutdown.
                    break;

                // do work
                //Debug.Log("Thread1: Begin Running");

                try
                { //https://social.msdn.microsoft.com/Forums/vstudio/en-US/93583332-d307-4552-bd61-9a2adfcf2480/serial-port-write-method-is-blocking-execution?forum=vbgeneral


                    //The Write methods do block , until all data have been passed from the serial port driver to the UART FIFO.
                    //Usually, this is not a problem.It will not block "forever," just for as long as it takes.
                    //For example, if you were to send a 2K byte string, at 9600 bps, the write method would take about 2 seconds to return.

                    //Write(byte[] buffer, int offset, int count);

                    // m_serialPort2.Write(m_startByte, 0, 1);     // m_startByte = 255
                    // m_serialPort2.Write(m_LEDArray1, 0, m_LEDArray1.Length);   // m_LEDArray refers to the global address which is bound when the thread function is 
                    // m_serialPort2.Write(m_endByte, 0, 1); 

                    // defined; This global address is where the new  frame data is stored every frame.
                    Debug.Log("Thread 1 Begin Printing");

                    for (int i = 0; i < m_NumOfLEDsLeft * 3; i++)
                    {
                        if (i % 3 == 0)
                        {
                           
                            Debug.LogFormat(" {0}th Chain: Thread1: {1}: R: {2} ", m_NumOfLEDFrames, i,  m_LEDArray1[i]);
                        }
                        else if (i % 3 == 1)
                        {
                         
                            Debug.LogFormat("{0}th Chain: Thread1: {1}: G: {2} ", m_NumOfLEDFrames, i, m_LEDArray1[i]);
                        }
                        else
                        {
                          
                            Debug.LogFormat("{0}th Chain: Thread1: {1}: B: {2} ", m_NumOfLEDFrames, i, m_LEDArray1[i]);
                           // Debug.Log(" ");
                        }
                        //Debug.Log(" ");

                    } // for 


                    Debug.Log("Thread 1 Finished Printing");

                    // Thread1:  Wait for Thread2 to pass thru m_ChildThreadWaitEvent.WaitOne() before closing the gate
                    // Thread2 should execute m_ChildThreadPassThruEvent.Set() to pass thru
                    // in order to pass thru m_ChildThreadPassThruEvent.WaitOne();
                   // m_ChildThreadPassThruEvent.WaitOne();
                                      
                    m_ChildThreadWaitEvent.Reset();  // Indicate that passing  thru m_ChildThreadPassThruEvent.WaitOne() is finished,
                    // and   a new pass thru can occur only when the other thread executes
                    //  m_ChildThreadWaitEvent.Set() is executed by another thread.
                    
                    // close the gate, so that all the threads (Thread1 and Thread2) wait on the sync event signal
                    
                    //m_ChildThreadPassThruEvent.Reset(); // Indicate that all threads should 
                                                         // wait for  m_ChildThreadPassThruEvent.WaitOne() once again.


                    Debug.LogFormat("Thread 1  going back to the infinite loop", Thread.CurrentThread.ManagedThreadId);
                    // Reset the thread sync event (close the gate) so that it waits for a new
                    // LED frame m_LEDArray1 to arrive
                }
                catch (Exception ex)
                {

                    Debug.Log("Error:" + ex.ToString());

                }

                // The WriteBufferSize of the Serial Port is 1024, whereas that of Arduino is 64; m_LEDArray is 200 * 3 = 600 bytes less than
                // the Serial Port size.
                //https://stackoverflow.com/questions/22768668/c-sharp-cant-read-full-buffer-from-serial-port-arduino

            } // while (true)

            Debug.Log("Thread1  reaches here the end of the thread");
        };       // m_updateArduino1

        // define an action
        m_updateArduino2 = () =>
        {
            while (true)
            {
                //The thread  blocks initially because  the thread sync event  m_ThreadSyncEvent was
                // created with the initial stated being unsigned (the input parameter = false)

                // Wait for the wait gate to open
                //Debug.LogFormat("Number: {0}, string {1}, number again: {0}, character: {2}", num, str, chr);
                //Debug.LogFormat("Thread 2  waiting for signal", Thread.CurrentThread.ManagedThreadId);

                Debug.Log("Thread 2:  waiting for the gate to open");


                m_ChildThreadWaitEvent.WaitOne(Timeout.Infinite);   // The current thread wait indefinitely until the main thread
                                                                    // until it gets the signal from other threads

                //m_ChildThreadPassThruEvent.Set(); // Indicate that the current child thread has passed 
                // thru m_ChildThreadWaitEvent.WaitOne(); 
                // Increase  the number of time in which the gate is passed through.
                // This number is set to zero by the main thread, and incremented by one by each thread
                // Whis this number = the total number of threads, each thread executes the while (true) loop
                // again, waiting for the next LED frame. 

                //Debug.LogFormat("Thread {0}  got  signal", Thread.CurrentThread.ManagedThreadId);
                Debug.Log("Thread 2: the gate is open");
                if (m_ThreadShutdownEvent.WaitOne(0)) // The current thread also waits on another sync event
                                                      // m_ThreadShutdownEvent to handle thread shutdown.
                {
                    break;
                }
                // do work
               

                try
                { //https://social.msdn.microsoft.com/Forums/vstudio/en-US/93583332-d307-4552-bd61-9a2adfcf2480/serial-port-write-method-is-blocking-execution?forum=vbgeneral


                    //The Write methods do block , until all data have been passed from the serial port driver to the UART FIFO.
                    //Usually, this is not a problem.It will not block "forever," just for as long as it takes.
                    //For example, if you were to send a 2K byte string, at 9600 bps, the write method would take about 2 seconds to return.

                    //Write(byte[] buffer, int offset, int count);

                    // m_serialPort2.Write(m_startByte, 0, 1);     // m_startByte = 255
                    // m_serialPort2.Write(m_LEDArray1, 0, m_LEDArray1.Length);   // m_LEDArray refers to the global address which is bound when the thread function is 


                    // defined; This global address is where the new  frame data is stored every frame.
                    Debug.Log("Thread 2 Begin Printing");

                    for (int i = 0; i < m_NumOfLEDsRight * 3; i++)
                    {
                        if (i % 3 == 0)
                        {
                         
                            Debug.LogFormat("{0}th Chain: Thread2: {1}:  R: {2} ", m_NumOfLEDFrames, i, m_LEDArray2[i]);
                        }
                        else if (i % 3 == 1)
                        {
                           
                            Debug.LogFormat("{0}th Chain: Thread2: {1}:  G: {2} ", m_NumOfLEDFrames, i,  m_LEDArray2[i]);
                        }
                        else
                        {                                   
                            Debug.LogFormat("{0}th Chain: Thread2: {1}:  B: {2} ", m_NumOfLEDFrames, i,  m_LEDArray2[i]);
                            //Debug.Log(" ");
                        }
                        //Debug.Log(" ");

                    } // for 


                    Debug.Log("Thread 2 Finished Printing");

                    // Thread1:  Wait for Thread2 to pass thru m_ChildThreadWaitEvent.WaitOne() before closing the gate
                    // Thread2 should execute m_ChildThreadPassThruEvent.Set() to pass thru
                    // in order to pass thru m_ChildThreadPassThruEvent.WaitOne();
                    //m_ChildThreadPassThruEvent.WaitOne();
                    Debug.Log("close the gate");
                    m_ChildThreadWaitEvent.Reset();  // Indicate that passing  thru m_ChildThreadPassThruEvent.WaitOne() is finished,
                                                     // and   a new pass thru can occur only when the other thread executes
                                                     //  m_ChildThreadWaitEvent.Set() is executed by another thread.

                    // close the gate, so that all the threads (Thread1 and Thread2) wait on the sync event signal

                    //m_ChildThreadPassThruEvent.Reset(); // Indicate that all threads should 
                    // wait for  m_ChildThreadPassThruEvent.WaitOne() once again.


                    Debug.Log("Thread 2 going back to the infinite loop");
                    
                    // Reset the thread sync event (close the gate) so that it waits for a new
                    // LED frame m_LEDArray1 to arrive
                }
                catch (Exception ex)
                {

                    Debug.Log("Error:" + ex.ToString());

                }

                // The WriteBufferSize of the Serial Port is 1024, whereas that of Arduino is 64; m_LEDArray is 200 * 3 = 600 bytes less than
                // the Serial Port size.
                //https://stackoverflow.com/questions/22768668/c-sharp-cant-read-full-buffer-from-serial-port-arduino

            } // while (true)

            Debug.Log("Thread2  reaches here the end of the thread");

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
            // Both threads will wait over  m_ChildThreadWaitEvent.WaitOne(Timeout.Infinite); 


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
        m_NumOfLEDFrames++;

        Debug.Log(" ");            
        Debug.Log("NumOfLEDFrames = " + m_NumOfLEDFrames);


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
            m_LEDArray1[3 * i] = 1;
            m_LEDArray1[3 * i + 1] = 2;
            m_LEDArray1[3 * i + 2] = 3;

        }

        // the second chain

        for (int i = 0; i < m_NumOfLEDsRight; i++)
        {

            m_LEDArray2[3 * i] = 4;
            m_LEDArray2[3 * i + 1] = 5;
            m_LEDArray2[3 * i + 2] = 6;


        }
        //the  m_ChildThreadWaitEvent.Set()  releases all the threads, so that they  go thru WaitOne();

        // Open the gate
        Debug.Log("Open the gate");

        m_ChildThreadWaitEvent.Set();

       

    } //  UpdateLEDArray()

    void Update()
    {
    }


  

//    OnDestroy occurs when a Scene or game ends.
//    Stopping the Play mode when running from inside the Editor, will end the application.
//    As this end happens, an OnDestroy will be executed.
//    Also, if a Scene is closed and a new Scene is loaded, the OnDestroy call will be made.
//    When built as a standalone application OnDestroy calls are made when Scenes end.
//    A Scene ending typically means a new Scene is loaded.

//Note: OnDestroy will only be called on game objects that have previously been active.

    public void OnDestroy()
    {
        // Signal the shutdown event
        m_ThreadShutdownEvent.Set();
        Debug.Log("Thread Stopped ");

        // Make sure to resume any paused threads
        m_ChildThreadWaitEvent.Set();

        // Wait for the thread to exit: Because both threads are infinite loop, they will never finish.
        // So, the follwoing join methods will not finish the threads.
        m_Thread1.Join();
        m_Thread2.Join();

        Debug.Log("Threads Joined OnDestory");
        //m_Thread1.Abort();
        //m_Thread2.Abort();
    }

   // Sent to all GameObjects before the application quits.

//In the Editor, Unity calls this message when playmode is stopped.
    public void OnApplicationQuit()
    {

        // Signal the shutdown event
        m_ThreadShutdownEvent.Set();
        Debug.Log("Thread Stopped ");

        // Make sure to resume any paused threads
        m_ChildThreadWaitEvent.Set();

        // Wait for the thread to exit
        Debug.Log("Threads Joined OnApplicationQuit");
        m_Thread1.Join();
        m_Thread2.Join();


    }

}//public class LEDMasterController 
