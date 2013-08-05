using System;
using System.IO;
using System.Security.Cryptography;
using System.Threading;

namespace NSass.Tool
{
	internal static class FileChecksum
	{
		private static readonly HashAlgorithm Crc32 = CRC32.Create();

		public static uint Calculate(string fullPath)
		{
			byte[] data = ReadAllBytes(fullPath);
			return BitConverter.ToUInt32(Crc32.ComputeHash(data), 0);
		}

		private static byte[] ReadAllBytes(string path)
		{
			byte[] bytes;
			using (FileStream fs = WaitForFile(path))
			{
				int index = 0;
				long fileLength = fs.Length;
				if (fileLength > Int32.MaxValue)
					throw new IOException("File too long");
				int count = (int)fileLength;
				bytes = new byte[count];
				while (count > 0)
				{
					int n = fs.Read(bytes, index, count);
					if (n == 0)
						throw new InvalidOperationException("End of file reached before expected");
					index += n;
					count -= n;
				}
				fs.Close();
			}
			return bytes;
		}

		// Not the best way to open file, but FileSystemWatcher raises created/change event even is a file is still copying
		private static FileStream WaitForFile(string path)
		{
			while (true)
			{
				try
				{
					return new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
				}
				catch (IOException)
				{
					Thread.Sleep(100);
				}
			}
		}
	}
}