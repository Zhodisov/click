// -*- c-basic-offset: 4 -*-
#ifndef CLICK_KERNELTUN_HH
#define CLICK_KERNELTUN_HH
#include <click/element.hh>
#include <click/ipaddress.hh>
#include <click/task.hh>
#include <click/notifier.hh>
CLICK_DECLS

/*
=c

KernelTun(ADDR/MASK [, GATEWAY, I<keywords> HEADROOM, ETHER, MTU, IGNORE_QUEUE_OVERFLOWS])

=s comm

interface to /dev/tun or ethertap (user-level)

=d

Reads IP packets from and writes IP packets to a /dev/net/tun, /dev/tun*,
or /dev/tap* device.  This allows a user-level Click to hand packets to the
ordinary kernel IP processing code.  KernelTun will also install a routing
table entry so that the kernel can pass packets to the KernelTun device.

KernelTun produces and expects IP packets.  If, for some reason, the kernel
passes up a non-IP packet (or an invalid IP packet), KernelTun will emit
that packet on its second output, or drop it if there is no second output.

KernelTun allocates a /dev/net/tun, /dev/tun*, or /dev/tap* device (this
might fail) and runs ifconfig(8) to set the interface's local (i.e.,
kernel) address to ADDR and the netmask to MASK.  If a nonzero GATEWAY IP
address (which must be on the same network as the tun) is specified, then
KernelTun tries to set up a default route through that host.

When cleaning up, KernelTun attempts to bring down the device via
ifconfig(8).

Keyword arguments are:

=over 8

=item HEADROOM

Integer. The number of bytes left empty before the packet data to leave
room for additional encapsulation headers. Default is 28.

=item MTU

Integer. The interface's MTU. KernelTun will refuse to send packets larger
than the MTU. Default is 1500; not all operating systems allow MTU to be set.

=item ETHER

Ethernet address. Specifies the tunnel device's Ethernet address. Default is
00:01:02:03:04:05. On FreeBSD, any ETHER argument is silently ignored.

=item IGNORE_QUEUE_OVERFLOWS

Boolean.  If true, don't print more than one error message when
there are queue overflows error when sending/receiving packets
to/from the tun device (e.g. there was an ENOBUFS error).  Default
is false.

=item DEV_NAME

String. If specified, try to alloc a tun device with name DEV_NAME.
Otherwise, we'll just take the first virtual device we find. This option
only works with the Linux Universal TUN/TAP driver.

=back

=n

Make sure that your kernel has tun support enabled before running
KernelTun.  Initialization errors like "no such device" or "no such file or
directory" may indicate that your kernel isn't set up, or that some
required kernel module hasn't been loaded (on Linux, the relevant module is
"tun").

Packets sent to ADDR will be processed by the host kernel stack; packets sent
to any other address in ADDR/MASK will be sent to KernelTun.  Say you run this
configuration:

    tun :: KernelTun(1.0.0.1/8);
    tun -> IPClassifier(icmp type echo) -> ICMPPingResponder
        -> IPPrint -> tun;

If you then "C<ping 1.0.0.1>", I<your own kernel> will respond.  Click will
never see the packets, so it won't print anything.  But if you "C<ping
1.0.0.2>", the pings are sent to Click.  You should see printouts from Click,
and C<ping> should print Click's responses.

This element differs from KernelTap in that it produces and expects IP
packets, not IP-in-Ethernet packets.

=a

FromDevice.u, ToDevice.u, KernelTap, ifconfig(8) */

class KernelTun : public Element { public:
  
    KernelTun();
    ~KernelTun();
  
    const char *class_name() const	{ return "KernelTun"; }
    const char *port_count() const	{ return "0-1/1-2"; }
    const char *processing() const	{ return "a/h"; }
    const char *flow_code() const	{ return "x/y"; }
    const char *flags() const		{ return "S3"; }

    void *cast(const char *);
    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void cleanup(CleanupStage);
    void add_handlers();

    void selected(int fd);

    void push(int port, Packet *);
    bool run_task();

  private:

    enum { DEFAULT_MTU = 1500 };
    enum Type { LINUX_UNIVERSAL, LINUX_ETHERTAP, BSD_TUN, BSD_TAP, OSX_TUN,
		NETBSD_TUN, NETBSD_TAP };

    int _fd;
    int _mtu_in;
    int _mtu_out;
    Type _type;
    bool _tap;
    String _dev_name;
    IPAddress _near;
    IPAddress _mask;
    IPAddress _gw;
    EtherAddress _macaddr;
    int _headroom;
    Task _task;
    NotifierSignal _signal;
    
    bool _ignore_q_errs;
    bool _printed_write_err;
    bool _printed_read_err;

    static String print_dev_name(Element *e, void *);

#if HAVE_LINUX_IF_TUN_H
    int try_linux_universal(ErrorHandler *);
#endif
    int try_tun(const String &, ErrorHandler *);
    int alloc_tun(ErrorHandler *);
    int setup_tun(ErrorHandler *);
    int updown(IPAddress, IPAddress, ErrorHandler *);

    friend class KernelTap;
    
};

CLICK_ENDDECLS
#endif
