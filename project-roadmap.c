/* 16/05/2019
1- Remember the basics of C TCP/UDP communication (Done)
2- Quick review the code of the first project (Done)
3- Read Project 2 description (Done)
4- Learn about the specialities about UDP
  - Simple implementation of UDP client and server (Done)
  - Understand the ideia of using TCP concurrent and UDP iterative server
5- Implement UDP/TCP server using select
*/

/* Code rodemap
1- Implement a simple tcp client
2- Implement a simple tcp server
3- Establish the connection between computers in the same network
4- Establish the connection between computers in different networks
*/

/* Key points:
1- Measure elapsed time between requests and create a log for them.
*/

/* Main questions:
1- Can there be collisions between requests?
2- How to deal with concurrency?
*/

/* Joao
0- Implement simple tcp communication
1- C File handling
2- Decide how to build the database (Binary files, Text Files, sql)
3- Implement Crud operations
4- Implement server
*/

// Very nice website
https://github.com/angrave/SystemProgramming/wiki/Networking,-Part-2:-Using-getaddrinfo

// Concepts to support this software development:
- Unix programs do any sort of I/O by redaing or writing to a file descriptor

“Jeez, everything in Unix is a file!” What that person may have been talking about is the fact that when Unix programs do any sort of I/O, they do it by reading or writing to a file descriptor. A file descriptor is simply an integer associated with an open file. But (and here's the catch), that file can be a network connection, a FIFO, a pipe, a terminal, a real on-the-disk file, or just about anything else. Everything in Unix is a file! So when you want to communicate with another program over the Internet you're gonna do it through a file descriptor, you'd better believe it.

-“Where do I get this file descriptor for network communication, Mr. Smarty-Pants?” is probably the last question on your mind right now, but I'm going to answer it a
   You make a call to the socket() system routine. It returns the socket descriptor, and you communicate through it using the specialized send() and recv() (man send, man recv) socket calls.

*Two principal types of Internet socket:
  -“Stream Sockets” and “Datagram Sockets” - “SOCK_STREAM” and “SOCK_DGRAM”
  -Datagram sockets are sometimes called “connectionless sockets”. (Though they can be connect()'d if you really want. See connect(), below.)

  -StreamSockets: Stream sockets are reliable two-way connected communication streams. If you output two items into the socket in the order “1, 2”, they will arrive in the order “1, 2” at the opposite end. They will also be error-free.
  -How do stream sockets achieve this high level of data transmission quality? They use a protocol called “The Transmission Control Protocol”, otherwise known as “TCP”
    -TCP makes sure your data arrives sequentially and error-free.

  - if you send a datagram, it may arrive. It may arrive out of order. If it arrives, the data within the packet will be error-free.
  -Datagram sockets also use IP for routing, but they don't use TCP; they use the “User Datagram Protocol”, or “UDP” (see RFC 7688.)
  -Why are they connectionless? Well, basically, it's because you don't have to maintain an open connection as you do with stream sockets. You just build a packet, slap an IP header on it with destination information, and send it out. No connection needed. They are generally used either when a TCP stack is unavailable or when a few dropped packets here and there don't mean the end of the Universe. Sample applications: tftp (trivial file transfer protocol, a little brother to FTP), dhcpcd (a DHCP client), multiplayer games, streaming audio, video conferencing, etc.

  -!!!“Wait a minute! tftp and dhcpcd are used to transfer binary applications from one host to another! Data can't be lost if you expect the application to work when it arrives! What kind of dark magic is this?”!!!
    -Well, my human friend, tftp and similar programs have their own protocol on top of UDP. For example, the tftp protocol says that for each packet that gets sent, the recipient has to send back a packet that says, “I got it!” (an “ACK” packet.) If the sender of the original packet gets no reply in, say, five seconds, he'll re- transmit the packet until he finally gets an ACK. This acknowledgment procedure is very important when implementing reliable SOCK_DGRAM applications.
    -Why would you use an unreliable underlying protocol? Two reasons: speed and speed. It's way faster to fire-and-forget than it is to keep track of what has arrived safely and make sure it's in order and all that. If you're sending chat messages, TCP is great; if you're sending 40 positional updates per second of the players in the world, maybe it doesn't matter so much if one or two get dropped, and UDP is a good choice.

* Low level Nonsense and Network Theory:

  -Subnets:
    -For organizational reasons, it's sometimes convenient to declare that “this first part of this IP address up through this bit is the network portion of the IP address, and the remainder is the host portion.

    -For instance, with IPv4, you might have 192.0.2.12, and we could say that the first three bytes are the network and the last byte was the host. Or, put another way, we're talking about host 12 on network 192.0.2.0 (see how we zero out the byte that was the host.)

    -The network portion of the IP address is described by something called the netmask, which you bitwise- AND with the IP address to get the network number out of it.
      -The netmask usually looks something like 255.255.255.0. (E.g. with that netmask, if your IP is 192.0.2.12, then your network is 192.0.2.12 AND 255.255.255.0 which gives 192.0.2.0.)

  -Port Numbers:
    -Turns out that besides an IP address (used by the IP layer), there is another address that is used by TCP (stream sockets) and, coincidentally, by UDP (datagram sockets). It is the port number. It's a 16-bit number that's like the local address for the connection.
    -Think of the IP address as the street address of a hotel, and the port number as the room number. That's a decent analogy; maybe later I'll come up with one involving the automobile industry.
    -Say you want to have a computer that handles incoming mail AND web services—how do you differentiate between the two on a computer with a single IP address?
    -Well, different services on the Internet have different well-known port numbers. You can see them all in the Big IANA Port List12 or, if you're on a Unix box, in your /etc/services file. HTTP (the web) is port
    80, telnet is port 23, SMTP is port 25, the game DOOM13 used port 666, etc. and so on. Ports under 1024 are often considered special, and usually require special OS privileges to use.

  -Byte Order:
    -A lot of times when you're building packets or filling out data structures you'll need to make sure your two- and four-byte numbers are in Network Byte Order. But how can you do this if you don't know the native Host Byte Order?
    -Good news! You just get to assume the Host Byte Order isn't right, and you always run the value through a function to set it to Network Byte Order. The function will do the magic conversion if it has to, and this way your code is portable to machines of differing endianness.
    -Basically, you'll want to convert the numbers to Network Byte Order before they go out on the wire, and convert them to Host Byte Order as they come in off the wire.

  -Structs:
    -Socket descriptor: int

    struct addrinfo {
      int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
      int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
      int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
      int              ai_protocol;  // use 0 for "any"
      size_t           ai_addrlen;   // size of ai_addr in bytes
      struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
      char            *ai_canonname; // full canonical hostname
      struct addrinfo *ai_next;      // linked list, next node
    };

    -You'll load this struct up a bit, and then call getaddrinfo(). It'll return a pointer to a new linked list of these structures filled out with all the goodies you need.

    -You can force it to use IPv4 or IPv6 in the ai_family field, or leave it as AF_UNSPEC to use whatever. This is cool because your code can be IP version-agnostic.

    -Note that this is a linked list: ai_next points at the next element—there could be several results for you to choose from. I'd use the first result that worked, but you might have different business needs; I don't know everything, man!

    -You'll see that the ai_addr field in the struct addrinfo is a pointer to a struct sockaddr. This is where we start getting into the nitty-gritty details of what's inside an IP address structure.

    -you might not usually need to write to these structures; oftentimes, a call to getaddrinfo() to fill out your struct addrinfo for you is all you'll need. You will, however, have to peer inside these structs to get the values out, so I'm presenting them here.

   struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
   ;

  -Private (Or Disconnected) Networks:
    -Lots of places have a firewall that hides the network from the rest of the world for their own protection.

    -And often times, the firewall translates “internal” IP addresses to “external” (that everyone else in the world knows) IP addresses using a process called Network Address Translation, or NAT.

    -For instance, I have a firewall at home. I have two static IPv4 addresses allocated to me by the DSL company, and yet I have seven computers on the network. How is this possible? Two computers can't share the same IP address, or else the data wouldn't know which one to go to!

    -The answer is: they don't share the same IP addresses. They are on a private network with 24 million IP addresses allocated to it. They are all just for me. Well, all for me as far as anyone else is concerned. Here's what's happening:
      -If I log into a remote computer, it tells me I'm logged in from 192.0.2.33 which is the public IP address my ISP has provided to me. But if I ask my local computer what its IP address is, it says 10.0.0.5. Who is translating the IP address from one to the other? That's right, the firewall! It's doing NAT!
      -10.x.x.x is one of a few reserved networks that are only to be used either on fully disconnected networks, or on networks that are behind firewalls

*System Calls or Bust:

  -When you call one of these functions, the kernel takes over and does all the work for you automagically.

  -(Please note that for brevity, many code snippets below do not include necessary error checking. And they very commonly assume that the result from calls to getaddrinfo() succeed and return a valid entry in the linked list. Both of these situations are properly addressed in the stand-alone programs, though, so use those as a model.)

  -getaddrinfo()—Prepare to launch!
    -A tiny bit of history: it used to be that you would use a function called gethostbyname() to do DNS
    lookups. Then you'd load that information by hand into a struct sockaddr_in, and use that in your calls.

    -In these modern times, you now have the function getaddrinfo() that does all kinds of good stuff for you, including DNS and service name lookups, and fills out the structs you need, besides!

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>

    int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                    const char *service,  // e.g. "http" or port number
                    const struct addrinfo *hints,
                    struct addrinfo **res);

    // Finally, the hints parameter points to a struct addrinfo that you've already filled out with relevant information.

    // Here's a sample call if you're a server who wants to listen on your host's IP address, port 3490. Note that this doesn't actually do any listening or network setup; it merely sets up structures we'll use later:
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results
    memset(&hints, 0, sizeof hints); // make sure the struct is empty

    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
    // servinfo now points to a linked list of 1 or more struct addrinfos
    // ... do everything until you don't need servinfo anymore ....
    freeaddrinfo(servinfo); // free the linked-list

    -Finally, when we're eventually all done with the linked list that getaddrinfo() so graciously allocated for us, we can (and should) free it all up with a call to freeaddrinfo().

    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    // get ready to connect
    status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);
    // servinfo now points to a linked list of 1 or more struct addrinfos
    // etc.

    -Now that we have that under control, we'll use the results we get from getaddrinfo() to pass to other socket functions and, at long last, get our network connection established! Keep reading!

  - socket() - Get the file Descriptor!

  pg 23/19

  // ANGRAVE -  HOW do i connect to a TCP Server:
  getaddrinfo -- Determine the remote addresses of a remote host
  socket      -- Create a socket
  connect     -- Connect to the remote host using the socket and address information

  The getaddrinfo call if successful, creates a linked-list of addrinfo structs and sets the given pointer to point to the first one.

  The socket call creates an outgoing socket and returns a descriptor (sometimes called a 'file descriptor') that can be used with read and write etc. In this sense it is the network analog of open that opens a file stream - except that we haven't connected the socket to anything yet!

  // Pull out the socket address info from the addrinfo struct:
  connect(sockfd, p->ai_addr, p->ai_addrlen);

  As part of the clean up code call freeaddrinfo on the top-most addrinfo struct:
  void freeaddrinfo(struct addrinfo *ai);

  If getaddrinfo fails can I use strerror to print out the error?

  int result = getaddrinfo(...);
  if (result) {
      const char *mesg = gai_strerror(result);
      ...
  }

  // ANGRAVE: Building a simple TCP client:
  int socket(int domain, int socket_type, int protocol);

  Socket creates a socket with domain (e.g. AF_INET for IPv4 or AF_INET6 for IPv6), socket_type is whether to use UDP or TCP or other socket type, protocol is an optional choice of protocol configuration (for our examples this we can just leave this as 0 for default). This call creates a socket object in the kernel with which one can communicate with the outside world/network. You can use the result of getaddressinfo to fill in the socket parameters, or provide them manually.

  The socket call returns an integer - a file descriptor - and, for TCP clients, you can use it like a regular file descriptor i.e. you can use read and write to receive or send packets.

  TCP sockets are similar to pipes except that they allow full duplex communication i.e. you can send and receive data in both directions independently.

    //Steps:
    -getaddressinfo

    -connect:
      Pass connect the socket file descriptor, the address you want to connect to and the length in bytes of the address structure. To help identify errors and mistakes it is good practice to check the return value of all networking calls, including connect

    -read/write:
      Once we have a successful connection we can read or write like any old file descriptor. Keep in mind if you are connected to a website, you want to conform to the HTTP protocol specification in order to get any sort of meaningful results back. There are libraries to do this, usually you don't connect at the socket level because there are other libraries or packages around it.

      The number of bytes read or written may be smaller than expected. Thus it is important to check the return value of read and write.

  // Angrave building a simple tcp server
  What is htons and when is it used?

  Integers can be represented in least significant byte first or most-significant byte first. Either approach is reasonable as long as the machine itself is internally consistent. For network communications we need to standardize on agreed format.

  htons(xyz) returns the 16 bit unsigned integer 'short' value xyz in network byte order. htonl(xyz) returns the 32 bit unsigned integer 'long' value xyz in network byte order.

  These functions are read as 'host to network'; the inverse functions (ntohs, ntohl) convert network ordered byte values to host-ordered ordering. So, is host-ordering little-endian or big-endian? The answer is - it depends on your machine! It depends on the actual architecture of the host running the code. If the architecture happens to be the same as network ordering then the result of these functions is just the argument. For x86 machines, the host and network ordering is different.

  Summary: Whenever you read or write the low level C network structures (e.g. port and address information), remember to use the above functions to ensure correct conversion to/from a machine format. Otherwise the displayed or specified value may be incorrect.

  -- Big 4 network calls used to create a server:
    1 - socket()
    2 - bind()
    3 - listen()
    4 - accept()

  -- socket:
    - To create a endpoint for networking communication. A new socket by itself is not particularly useful; though we've specified either a packet or stream-based connections it is not bound to a particular network interface or port. Instead socket returns a network descriptor that can be used with later calls to bind, listen and accept.

  -- bind:
    - The bind call associates an abstract socket with an actual network interface and port. It is possible to call bind on a TCP client however it's unusually unnecessary to specify the outgoing port.

  -- listen:
    - The listen call specifies the queue size for the number of incoming, unhandled connections i.e. that have not yet been assigned a network descriptor by accept Typical values for a high performance server are 128 or more.

  -- Why are server sockets passive?
    - Server sockets do not actively try to connect to another host; instead they wait for incoming connections. Additionally, server sockets are not closed when the peer disconnects. Instead the client communicates with a separate active socket on the server that is specific to that connection.

    -Unique TCP connections are identified by the tuple (source ip, source port, destination ip, destination port) It is possible to have multiple connections from a web browser to the same server port (e.g. port 80) because the the source port on each arriving packet is unique. i.e. For a particular server port (e.g. port 80) there can be one passive server socket but multiple active sockets (one for each currently open connection) and the server's operating system maintains a lookup table that associates a unique tuple with active sockets, so that incoming packets can be correctly routed to the correct socket.

  -- What is the purpose of calling accept?
    - Once the server socket has been initialized the server calls accept to wait for new connections. Unlike socket bind and listen, this call will block. i.e. if there are no new connections, this call will block and only return when a new client connects. The returned TCP socket is associated with a particular tuple (client IP, client port, server IP, server port) and will be used for all future incoming and outgoing TCP packets that match this tuple.

    -Note the accept call returns a new file descriptor. This file descriptor is specific to a particular client. It is common programming mistake to use the original server socket descriptor for server I/O and then wonder why networking code has failed.

  -- The gotchas of creating a TCP-Server:
    - Using the socket descriptor of the passiver server socket
    - Not specifying SOCK_STREAM requirement for getaddrinfo
    - Not being able to re-use an existing port.
    - Not initializing the unused struct entries
    - The bind call will fail if the port is currently in use

  --
