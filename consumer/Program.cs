using CPlusSharpQueue.Telemetry;
using System.Reflection;

public class Program()
{
    static void Main(string[] args)
    {
        Console.WriteLine(System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly()!.Location));

        Batch batch = Batch.Parser.ParseFrom(File.ReadAllBytes("../../../../data/batch.bin"));

        Console.WriteLine(batch);
    }
}
