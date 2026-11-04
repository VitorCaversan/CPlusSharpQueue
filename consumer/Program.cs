using CPlusSharpQueue.Telemetry;
using System.Reflection;

public class Program()
{
    static void Main(string[] args)
    {
        var latestBatchPath = args.Length > 0 ? args[0] : Path.Combine("data", "latest.bin");
        if (!File.Exists(latestBatchPath))
        {
            Console.Error.WriteLine($"File not found {Path.GetFullPath(latestBatchPath)}");
            return;
        }

        Batch batch = Batch.Parser.ParseFrom(File.ReadAllBytes(latestBatchPath));

        Console.WriteLine(batch);
    }
}
