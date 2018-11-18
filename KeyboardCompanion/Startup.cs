using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace KeyboardCompanion
{
    public class Startup
    {
        public Startup()
        {
            var drives = GetDrives();
            FindComPort();
        }

        public DriveInfo[] GetDrives()
        {
            try
            {
                DriveInfo[] myDrives = DriveInfo.GetDrives();

                foreach (DriveInfo drive in myDrives)
                {
                    Console.WriteLine("Drive:" + drive.Name);
                    Console.WriteLine("Drive Type:" + drive.DriveType);
                    if (drive.IsReady)
                    {
                        Console.WriteLine("Drive Letter: " + drive.RootDirectory);
                        Console.WriteLine("Vol Label:" + drive.VolumeLabel);
                        Console.WriteLine("File System: " + drive.DriveFormat);
                    }
                }

                return myDrives;
            }
            catch (Exception)
            {
                throw;
            }
        }
        public bool FindComPort()
        {
            if (Variables.KeyboardDetected) return true;
            string[] portNames = SerialPort.GetPortNames();
            string sInstanceName = string.Empty;
            string sPortName = string.Empty;
            bool bFound = false;

            for (int y = 0; y < portNames.Length; y++)
            {
                ManagementObjectSearcher searcher = new ManagementObjectSearcher("root\\WMI", "SELECT * FROM MSSerial_PortName");
                foreach (ManagementObject queryObj in searcher.Get())
                {
                    sInstanceName = queryObj["InstanceName"].ToString();

                    if (sInstanceName.IndexOf($"Vid_{Variables.KeyboardVid}&Pid_{Variables.KeyboardPid}") > -1)
                    {
                        sPortName = queryObj["PortName"].ToString();
                        Variables.KeyBoardPort = new SerialPort(sPortName, 115200, Parity.None, 8, StopBits.One);
                        bFound = true;
                        break;
                    }
                }

                if (bFound)
                    break;
            }

            return bFound;
        }
    }
}
