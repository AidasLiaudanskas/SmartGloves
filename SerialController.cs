using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

using System.IO.Ports;


public class SensorData
{
public ushort[] flexl = new ushort[6];
public ushort[] flexr = new ushort[6];
public ushort[] accl = new ushort[6];
public ushort[] accr = new ushort[6];
}

public class SerialController : MonoBehaviour {
public string portname = "\\\\.\\COM8";
public int baudrate = 19200;
public SerialPort port = new SerialPort();
public SensorData data;
public int packet_size = 26 * 2 * 2;

private IEnumerator read_coroutine;

bool Open()
{
        try
        {
                if (!port.IsOpen)
                {
                        Debug.Log("Opening port " + portname + " at " + baudrate + " baud");
                        /*port.BaudRate = baudrate;
                           port.PortName = portname;*/
                        port = new SerialPort(portname, baudrate);
                        port.StopBits = StopBits.One;
                        port.DataBits = 8;
                        port.ReadTimeout = 1; // milliseconds
                        port.DtrEnable = true;
                        port.RtsEnable = true;
                        port.Open();
                        port.DataReceived += new SerialDataReceivedEventHandler(OnDataReceived);
                }
        }
        catch (Exception e)
        {
                Debug.Log(e.Message);
                return false;
        }
        return true;
}

void Start ()
{
        Debug.Log("Avaliable ports: " + String.Join(", ", SerialPort.GetPortNames()));
        data = new SensorData();
        Debug.Log("Open returned " + this.Open());
}

void OnDataReceived(object sender, SerialDataReceivedEventArgs args)
{
        Debug.Log("Serial data received");
        byte[] buf = new byte[128];
        int rx_count = port.Read(buf, 0, buf.Length);
        Debug.Log("Received " + rx_count + " bytes.");
}

void SetDataFromSerial(byte[] recvd)
{
        /* Assume little-endian line format */
        int n = 0;
        for (int i = 0; i < 6; ++i)
        {
                data.flexr[i] = recvd[n++];
                data.flexr[i] += (ushort)(recvd[n++] << 8);
                if (i == 1)
                        Debug.Log(data.flexr[i]);
        }
        for (int i = 0; i < 6; ++i)
        {
                // Correct wiring mistake on left hand board...
                data.flexl[(i & 1) | (i & 2 << 1) | (i & 4 >> 1)] = recvd[n++];
                data.flexl[(i & 1) | (i & 2 << 1) | (i & 4 >> 1)] += (ushort)(recvd[n++] << 8);
        }
        // Gyro data is big-endian
        for (int i = 0; i < 6; ++i)
        {
                if (i == 3)
                        n += 2;  // Skip temperature data
                data.accr[i] = (ushort)(recvd[n++] << 8);
                data.accr[i] += recvd[n++];
        }
        for (int i = 0; i < 6; ++i)
        {
                if (i == 3)
                        n += 2;  // Skip temperature data
                data.accl[i] = (ushort)(recvd[n++] << 8);
                data.accl[i] += recvd[n++];
        }
}

private byte hex_char_to_nibble(byte c)
{
        if (c < 97)
                return (byte)(c - 48);
        else
                return (byte)(c - 87);
}

void Update () {
        if (!port.IsOpen)
                return;
        byte[] buf = new byte[packet_size];
        int bytecount = 0;
        try
        {
                for (int i = 0; i < packet_size; ++i)
                {
                        buf[i] = (byte)port.ReadByte();
                        bytecount++;
                }
        }
        catch //(Exception ex)
        {
                //Debug.Log(ex.Message);
                // Probably just that there are less than packet_size bytes (timeout!) followed by a gap of at least port.ReadTimeout
                // In this case the best thing to do is drop the partial data on the floor and wait for the next packet
        }
        port.DiscardInBuffer(); // If we have too much data, drop the rest (!!!)
        Debug.Log("Received " + bytecount + " bytes: " + System.Text.Encoding.ASCII.GetString(buf));
        if (bytecount == packet_size)
        {
                byte[] de_hexed = new byte[packet_size / 2];
                for (int i = 0; i < packet_size / 2; ++i)
                {
                        de_hexed[i] = (byte)((hex_char_to_nibble(buf[2 * i]) << 4) | hex_char_to_nibble(buf[2 * i + 1]));
                        Debug.Log("Byte: " + de_hexed[i].ToString("X"));
                }
                SetDataFromSerial(de_hexed);
        }


}
}
