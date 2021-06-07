using System;
using System.Net;

namespace srv
{
  class Program
  {
    static void Main(string[] args)
    {
      HttpListener listener = new HttpListener();
      listener.Prefixes.Add("http://localhost:7012/");
      listener.Start();
      HttpListenerContext context = listener.GetContext();
      HttpListenerRequest request = context.Request;
      HttpListenerResponse response = context.Response;
      string responseString = "Hello";
      byte[] buffer = System.Text.Encoding.UTF8.GetBytes(responseString);
      response.ContentLength64 = buffer.Length;
      System.IO.Stream output = response.OutputStream;
      output.Write(buffer,0,buffer.Length);
      output.Close();
      listener.Stop();
    }
  }
}
