using CPlusSharpQueue.Telemetry;
using System.Reflection;

public class Program()
{
    static void Main(string[] args)
    {
        Console.WriteLine(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly()!.Location));

        var latestBatchPath = args.Length > 0 ? args[0] : Path.Combine("data", "latest.bin");
        if (!File.Exists(latestBatchPath))
        {
            Console.Error.WriteLine($"File not found {Path.GetFullPath(latestBatchPath)}");
        }

        Batch batch = Batch.Parser.ParseFrom(File.ReadAllBytes(latestBatchPath));

        Console.WriteLine(batch);
    }
}
