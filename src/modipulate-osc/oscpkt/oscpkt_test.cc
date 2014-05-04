/**
   Test the robustness of oscpkt.

   build with: 

   g++ -O3 -Wall -W -I. oscpkt/oscpkt_test.cc 
   cl.exe /Zi /EHsc /I. oscpkt/oscpkt_test.cc 
 */

#define OSCPKT_TEST
#define OSCPKT_OSTREAM_OUTPUT
#define OSCPKT_TEST_UDP

//define OSCPKT_DEBUG
#include <iostream>
void hexdump(std::ostream &os, const void *s_void, size_t sz, size_t offset=0);
using std::cout;
using std::cerr;

#ifdef OSCPKT_TEST_UDP
#include "udp.hh"
#endif

#include "oscpkt.hh"
#include <cstdio>
#include <cstdlib>
#include <ctime>



using namespace oscpkt;

void hexdump(std::ostream &os, const void *s_void, size_t sz, size_t offset) {
  unsigned char *s = (unsigned char*)s_void;
  size_t nb = 16;
  for (size_t l=0; l < sz; l += nb) {
    char tmp[12];
    sprintf(tmp, "%08x ", (uint32_t)(l+offset)); os << tmp;
    for (size_t c = 0; c < nb; ++c) {
      if (l+c < sz) {
        sprintf(tmp, "%02x ", s[l+c]);
        os << tmp;
      } else os << "   ";
    }
    os << "| ";
    for (size_t c = 0; c < nb; ++c) {
      if (c+l < sz) {
        if (s[l+c] >= ' ' && s[l+c] < 127)
          os << s[l+c];
        else os << ".";
      } else os << " ";
    }
    os << "\n";
  }
}

void bintoc(void *s_void, size_t sz) {
  char *p = (char*)s_void;
  while (sz) {
    printf("\"");
    for (int j=0; sz && j < 16; ++j, --sz, ++p) {
      printf("\\x%02x", (char)(*p));
    }
    printf("\"\n");    
  }
}
      
void basicTests() {
  Message msg;
  
  PacketWriter wr;
  msg.init("/foot");
  wr.init().addMessage(msg);
  return;
  cout << "simplest message:\n"; hexdump(cout, wr.packetData(), wr.packetSize(), 0); cout << std::endl;
  assert(wr.isOk()); 

  wr.init().startBundle();
  wr.addMessage(msg.init("/foo").pushInt32(1000).pushInt32(-1).pushStr("hello").pushFloat(1.234f).pushFloat(5.678f));
  wr.endBundle();
  
  cout << "larger message:\n"; hexdump(cout, wr.packetData(), wr.packetSize(), 0); cout << std::endl;
  assert(wr.packetSize() == 0x3c && 
         memcmp(wr.packetData(), 
                "\x23\x62\x75\x6e\x64\x6c\x65\x00\x00\x00\x00\x00\x00\x00\x00\x01"
                "\x00\x00\x00\x28\x2f\x66\x6f\x6f\x00\x00\x00\x00\x2c\x69\x69\x73"
                "\x66\x66\x00\x00\x00\x00\x03\xe8\xff\xff\xff\xff\x68\x65\x6c\x6c"
                "\x6f\x00\x00\x00\x3f\x9d\xf3\xb6\x40\xb5\xb2\x2d", wr.packetSize()) == 0);
  //bintoc(wr.data(), wr.size());
  assert(wr.isOk());
  
  PacketReader pr(wr.packetData(), wr.packetSize()); cerr << "err:" << pr.getErr() << "\n"; assert(pr.isOk());
  Message *mr = pr.popMessage(); assert(mr);
  cout << "message received: " << *mr << "\n";
  { int i1,i2; std::string s; float f1, f2;
    if (!mr->arg().popInt32(i1).popInt32(i2).popStr(s).popFloat(f1).popFloat(f2).isOk()) assert(0);
    cerr << "i1=" << i1 << ", i2=" << i2 << ", s='" << s.size() << "'\n";
    assert(i1 == 1000); assert(i2 == -1); assert(s == "hello");
  }

  wr.init().startBundle().startBundle().endBundle().endBundle();
  hexdump(cout, wr.packetData(), wr.packetSize(), 0); cout << "\n";
  assert(wr.packetSize() == 40);

  pr.init(wr.packetData(), wr.packetSize());
  assert(pr.isOk()); assert(pr.popMessage() == 0);
}

uint32_t global_seed = 0;
static uint32_t prandom_state = 1;
void prandom_seed(uint32_t seed) { prandom_state = seed + global_seed; }
uint32_t prandom(uint32_t max_val) {
  prandom_state = prandom_state * 1103515245 + 12345;
  return ((uint32_t)(prandom_state/65536) % 32768) % max_val;
}

std::string randomString(size_t max_len) {
  size_t len = (prandom(max_len));
  std::string s;
  s.resize(len);
  for (size_t i=0; i <len; ++i) {
    s[i] = 'a' + (prandom(26));
  }
  return s;
}

#define check(ok) do { if (!(ok)) { cerr << "TEST#" << test_number << " FAILED, line " << __LINE__ << ", " << #ok << "\n"; return false; } } while (0)

bool makeRandomTest(int seed, int fuzz=0, bool verbose=false) {
  int test_number = seed;

  //cerr << "makeRandomTest(" << seed << ", fuzz=" << fuzz << ")\n";
  prandom_seed(seed);

  int bundle_recurs = 1;
  PacketWriter wr; 
  wr.startBundle();
  int nb_item = prandom(50)+1;
  if (verbose) cerr << "nb_item=" << nb_item << "\n";
  while (nb_item != 0 && bundle_recurs > 0) {
    if (prandom(7) == 0) { 
      if (verbose) cerr << "startBundle\n";
      ++bundle_recurs; wr.startBundle();
    } else if (prandom(11) == 0 && bundle_recurs) {
      if (verbose) cerr << "endBundle\n";
      --bundle_recurs; wr.endBundle();
    } else {
      std::string cmd = "/" + randomString(70);
      Message msg(cmd);
      int nb_arg = prandom(15);
      for (int i=0; i < nb_arg; ++i) {
        switch (prandom(8)) {
          case 0: msg.pushBool(prandom(2)); break;
          case 1: msg.pushInt32(11223344); break;
          case 2: msg.pushInt64(123456789012345ll); break;
          case 3: msg.pushFloat(0.123f); break;
          case 4: msg.pushDouble(3.14159265358979323846); break;
          case 5: { 
            std::string s = randomString(40); 
            if (s.size()>1) { s[0] = '<'; s[s.size()-1] = '>'; }
            msg.pushStr(s);
          } break;
          case 6: { 
            std::vector<char> b; b.resize(prandom(40), char(0x55));
            if (b.size() > 1) { b.front() = 0x44; b.back() = 0x66; }
            msg.pushBlob((b.size() ? &b[0] : 0), b.size());
          } break;
        }
      }
      if (verbose) cerr << "adding msg: " << msg << "\n";
      wr.addMessage(msg);
    }
    nb_item--;
  }
  while (bundle_recurs) { wr.endBundle(); --bundle_recurs; }

  check(wr.isOk());

  std::vector<char> data(wr.packetData(), wr.packetData()+wr.packetSize());
  for (int cnt=0; cnt < fuzz; ++cnt) {
    size_t pos = prandom(data.size());
    switch (prandom(4)) {
      case 0: data[pos]++; break;
      case 1: data[pos]--; break;
      default:
        data[pos] = (char)prandom(256);
    }
  }

  PacketReader pr(&data[0], data.size()); 
  check(pr.isOk() || fuzz);
  Message *msg;
  while ((msg = pr.popMessage())) {
    Message::ArgReader arg(msg->arg());
    while (arg.nbArgRemaining()) {
      if (arg.isBlob()) {
        std::vector<char> b; arg.popBlob(b); 
        if (b.size()>1) { check(b.front() == 0x44 || fuzz); check(b.back() == 0x66 || fuzz); }
      } else if (arg.isBool()) {
        bool b; arg.popBool(b);
      } else if (arg.isInt32()) {
        int i; arg.popInt32(i); check(i == 11223344 || fuzz);
      } else if (arg.isInt64()) {
        int64_t h; arg.popInt64(h); check(h == 123456789012345ll || fuzz);
      } else if (arg.isFloat()) {
        float f; arg.popFloat(f); check(f == 0.123f || fuzz);
      } else if (arg.isDouble()) {
        double d; arg.popDouble(d); check(d == 3.14159265358979323846 || fuzz);
      } else if (arg.isStr()) {
        std::string s; arg.popStr(s); 
        if (s.size() > 1) { check((s[0] == '<' && s[s.size()-1] == '>') || fuzz); }
      }
    }
    check(arg.isOk() || fuzz);
  }
  return true;
}


void randomTests(int nb_test, bool verbose) {
  cout << "doing random tests..." << std::endl;
  bool ok = true;
  //makeRandomTest(427,1,true);
  for (int fuzz=0; fuzz < 4; ++fuzz) {
    for (int i=0; ok && i < nb_test; ++i) {
      if (!makeRandomTest(i,fuzz, verbose)) ok = false;
    }
  }
  if (ok) cout << "all tests passed !\n";
}

#ifdef OSCPKT_TEST_UDP
#ifdef HAVE_LIBLO
#include "lo/lo.h"
static void lo_osc_error(int num, const char *msg, const char *path) {
  cerr << "Error: liblo server error " << num
       << " in path \"" << (path ? path : "(null)")
       << "\": " << msg << "\n";
}
static int plop_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *) {
    (void)path; (void)types; (void)argc; (void)argv; (void)data;
    cerr << "lo_server received a PLOP ! wouhou\n";
    return 0;
}
void libloTest() {
  UdpSocket sock;
  sock.bindTo(0);
  
  lo_address loaddr = lo_address_new(sock.localHostName().c_str(), sock.boundPortAsString().c_str());
  lo_send(loaddr, "/plop", "s", "HELLO FROM LIBLO");
  bool ok=sock.receiveNextPacket(100);
  cout << "received packet from liblo ? " << ok << ", send=" << sock.packetOrigin() << "\n";
  if (ok) {
    PacketReader pr(sock.packetData(), sock.packetSize());
    Message *msg;
    while ((msg = pr.popMessage())) {
      cout << "message from liblo: " << *msg << "\n";
    }
  }

  lo_server serv = lo_server_new(NULL, lo_osc_error);
  const char *url = lo_server_get_url(serv);
  cout << "started lo_server on " << url << "\n";
  lo_server_add_method(serv, "/plop", "", plop_handler, 0);

  PacketWriter pw; Message msg;
  pw.addMessage(msg.init("/plop"));
  oscpkt::UdpSocket sock2; sock2.connectTo(lo_url_get_hostname(url), lo_url_get_port(url));
  if (sock2.isOk()) {
    bool ok = sock2.sendPacket(pw.packetData(), pw.packetSize());
  }
  if (lo_server_recv_noblock(serv, 50) == 0) {
    cout << "reply not received by lo server\n";
  } else { 
    cout << "lo server got the reply!\n";
  }
}
#endif

void socketTests() {
#ifdef HAVE_LIBLO
  libloTest();
#endif

  UdpSocket sock1, sock2;

  sock1.bindTo(0);
  if (!sock1.isOk()) cerr << "sock1: " << sock1.errorMessage() << "\n";

  sock2.connectTo(sock1.localHostName(), sock1.boundPort());
  if (!sock2.isOk()) cerr << "sock2: " << sock2.errorMessage() << "\n";

  for (int sz=8*8; sz <= 256*1024; sz *= 2) {    
    std::string buf(sz-32, 'x');
    bool sent_ok = sock2.sendPacket(buf.data(), buf.size());
    if (!sock2.isOk()) cerr << "sock2: " << sock2.errorMessage() << "\n";
    cerr << "sz=" << buf.size() << ", sent_ok=" << sent_ok << " ";
    if (sent_ok) {
      bool recv_ok = sock1.receiveNextPacket(100);
      cerr << "recv_ok=" << recv_ok << ", sock1.receiveNextPacket: size=" << sock1.packetSize() << " from: " << sock1.packetOrigin();
      if (recv_ok) {
        for (size_t k=0; k < sock1.packetSize(); ++k) {
          assert(((char*)sock1.packetData())[k] == buf[k]);
        }
      }
    }
    cerr << "\n";
  }
  cerr << "sock1 is replying to sock2\n";
  bool ok;
  ok = sock1.sendPacketTo("plop", 5, sock1.packetOrigin()); assert(ok);
  cerr << "sock2 is receiving the reply..\n";
  ok = sock2.receiveNextPacket(2000); assert(ok);
  cerr << "sock2 has received the reply, sz=" << sock2.packetSize() << ": '" << ((char*)sock2.packetData()) << "'\n";
  assert(strcmp((char*)sock2.packetData(), "plop")==0);


  cerr << "flooding: "; 
  int nok = 0;
  for (int cnt=0; cnt < 128; ++cnt) {
    std::string buf = randomString(2048);
    bool sent_ok = sock2.sendPacket(buf.data(), buf.size());
    cerr << sent_ok;
    if (sent_ok) ++nok;
  }
  cerr << "\nreading:  ";
  while (nok) {
     bool recv_ok = sock1.receiveNextPacket(30);
     cerr << recv_ok;
     --nok;
  }
  cerr << "\nOK\n";
}
#endif // OSCPKT_TEST_UDP


void checkMatch(const char *pattern, const char *test, bool expected_match=true) {
  cout << "doing fullPatternMatch('" << pattern << "', '" << test << "'), expected result is : " 
       << (expected_match?"MATCH":"MISMATCH") << std::endl;
  bool m = fullPatternMatch(pattern, test);
  if (!expected_match) {
    if (m) { cerr << "unexpected match... " << pattern << " with " << test << "\n"; assert(0); }
  } else {
    if (!m) { cerr << "unexpected mismatch... " << pattern << " with " << test << "\n"; assert(0); }
    std::string tmp(test);
    while (tmp.size() && tmp[tmp.size()-1] != '/') tmp.resize(tmp.size()-1);
    assert(partialPatternMatch(pattern, tmp));
  }
  
  // a bit of fuzzing..
  for (size_t k=0; k < 1000; ++k) {
    std::string pat(pattern), t(test);
    int nb = prandom(5);
    for (int n=0; n < nb; ++n) {
      int idx = prandom((int)pat.size());
      const char *repl = "*{}[],!$#-/_";
      pat.at(idx) = repl[prandom(strlen(repl))];
      for (size_t kk=0; kk < t.size(); ++kk) {
        fullPatternMatch(pat, t.substr(0, kk));
      }
    }
  }
}

void patternTests() {
  cout << "Doing pattern tests" << std::endl;
  checkMatch("//bar", "bar", false);
  checkMatch("//bar", "/bar");
  checkMatch("//bar", "/foo/plop/bar");
  checkMatch("/foo//", "/foo/plop/df/");
  checkMatch("/foo///////bar", "/foo/plop/baz/bar");
  checkMatch("*", "bar");
  checkMatch("/foo/*", "/foo/bar");
  checkMatch("/{bar,fo}/b[aA]r", "/fo/bar");
  checkMatch("/{bar,fo}/b[aA]r", "/foo/bar", false);
  checkMatch("/fo{bar,}/ba[e-t]", "/fo/bar");
  checkMatch("/fo{bar,}/ba[t-z]", "/fo/bar", false);
  checkMatch("/f{,ioio,bar}o/?a[!a]", "/fo/bar");
  checkMatch("/foo/bar", "/foo/bar");
  checkMatch("/f*o/bar", "/foo/bar");
  checkMatch("/fo*o/bar", "/foo/bar");
  checkMatch("/*//bar", "/foo/bar");
  checkMatch("/*/bar", "/foo/bar");
  checkMatch("/*o/bar", "/foo/bar");
  checkMatch("/*/*/*/*a***/*/*/*/*/", "/foo/bar/foo/barrrr/foo/bar/foo/barrrr/");
  checkMatch("/*/*/*/**/*/*/*/*/q", "/foo/bar/foo/barrrr/foo/bar/foo/barrrr/p", false);
}

int main(int argc, char **argv) {
  srand(time(NULL));
  global_seed = rand();
  bool verbose = false;
  int nb_test = 5000;
  if (argc >= 2) {
    nb_test = atoi(argv[1]);
  }
  if (argc >= 3) {
    global_seed = atoi(argv[2]); verbose = true;
  }
  (void)argc; (void)argv;
  patternTests();
#ifdef OSCPKT_TEST_UDP
  //socketTests();
#endif
  basicTests();
  randomTests(nb_test, verbose);  
  cout << "OK it looks like everything works as expected!\n";
  return 0;
}



