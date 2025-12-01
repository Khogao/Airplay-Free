using NAudio.Wave;
using System;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Zeroconf;

namespace AirPlaySender
{
    class Program
    {
        static async Task Main(string[] args)
        {
            Console.WriteLine("=== Windows AirPlay Sender Prototype ===");
            Console.WriteLine("1. Scanning for AirPlay devices...");

            // 1. Discovery (Simple Bonjour Scan)
            var devices = await ZeroconfResolver.ResolveAsync("_raop._tcp.local.");
            
            if (!devices.Any())
            {
                Console.WriteLine("No AirPlay devices found.");
                Console.WriteLine("Press any key to start Loopback Capture anyway (local test)...");
                Console.ReadKey();
            }
            else
            {
                Console.WriteLine($"Found {devices.Count} devices:");
                foreach (var device in devices)
                {
                    Console.WriteLine($" - {device.DisplayName} ({device.IPAddress})");
                }
            }

            // 2. Audio Capture (WASAPI Loopback)
            Console.WriteLine("\n2. Starting WASAPI Loopback Capture...");
            
            // Select the default render device (speakers)
            using (var capture = new WasapiLoopbackCapture())
            {
                Console.WriteLine($"Source: {capture.WaveFormat.SampleRate}Hz, {capture.WaveFormat.BitsPerSample}bit, {capture.WaveFormat.Channels}ch");

                // This event fires every time audio buffer is available
                capture.DataAvailable += (s, e) =>
                {
                    // e.Buffer contains the raw PCM audio bytes
                    // e.BytesRecorded is the number of valid bytes
                    
                    // TODO: 
                    // 1. Encode these bytes to ALAC
                    // 2. Encrypt (AES)
                    // 3. Send via UDP to AirPlay device IP
                    
                    // For prototype, we just print volume level to prove it works
                    if (e.BytesRecorded > 0)
                    {
                        // Simple RMS calculation for visualization
                        float max = 0;
                        for (int index = 0; index < e.BytesRecorded; index += 4)
                        {
                            float sample = BitConverter.ToSingle(e.Buffer, index);
                            if (sample < 0) sample = -sample;
                            if (sample > max) max = sample;
                        }
                        // Draw a simple VU meter
                        int bars = (int)(max * 50);
                        Console.Write($"\rRecording: [{new string('|', bars).PadRight(50)}] ");
                    }
                };

                // Start recording
                capture.StartRecording();
                Console.WriteLine("Capture started! Play some music on your PC to see the VU meter.");
                Console.WriteLine("Press 'q' to quit.");

                while (Console.ReadKey(true).Key != ConsoleKey.Q)
                {
                    Thread.Sleep(100);
                }

                capture.StopRecording();
            }
        }
    }
}
