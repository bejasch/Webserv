-> http://www.mi.uni-koeln.de/c/mirror/www.codeguru.com/cpp/tic/tic_html.zip/tic0305.html

to genereralize webtraffic:
	-> use straight HTML (which has a very limited way to collect and submit information from the client) and 
	-> common gateway interface (CGI) programs that are run on the server. The Web server takes an encoded request submitted via an HTML page and responds by invoking a CGI program and handing it the encoded data from the request. This request is classified as either a “GET” or a “POST”


CGI: quite straightforward:
		takes its input from environment variables and standard input
		sends its output to standard output. However, there is some decoding that must be done in order to extract the data that’s been sent to you from the client’s web page. In this section you’ll get a crash course in CGI programming, and we’ll develop tools that will perform the decoding for the two different types of CGI submissions (GET and POST). These tools will allow you to easily write a CGI program to solve any problem. Since C++ exists on virtually all machines that have Web servers (and you can get GNU C++ free for virtually any platform), the solution presented here is quite portable. 
