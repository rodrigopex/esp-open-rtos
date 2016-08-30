#ifndef lob_h
#define lob_h

#include <stdint.h>
#include <stdlib.h>

typedef struct lob_struct
{
  // these are public but managed by accessors
  uint8_t *raw;
  uint8_t *body;
  size_t body_len;
  uint8_t *head;
  size_t head_len;
  
  // these are all for external use only
  uint32_t id;
  void *arg;

  // these are internal/private
  struct lob_struct *chain;
  char *cache; // edited copy of the json head

  // used only by the list utils
  struct lob_struct *next, *prev;

} *lob_t;

// these all allocate/free memory
lob_t lob_new();
lob_t lob_copy(lob_t p);
lob_t lob_free(lob_t p); // returns NULL for convenience

// creates a new parent packet chained to the given child one, so freeing the new packet also free's it
lob_t lob_chain(lob_t child);
// manually chain together two packets, returns parent, frees any existing child, creates parent if none
lob_t lob_link(lob_t parent, lob_t child);
// return a linked child if any
lob_t lob_linked(lob_t parent);
// returns child, unlinked
lob_t lob_unlink(lob_t parent);

// initialize head/body from raw, parses json
lob_t lob_parse(const uint8_t *raw, size_t len);

// return full encoded packet
uint8_t *lob_raw(lob_t p);
size_t lob_len(lob_t p);

// return null-terminated json header only
char *lob_json(lob_t p);

// creates temporarily cached string on lob (used by lob_json), free'd w/ lob or next cache call
char *lob_cache(lob_t p, size_t len);

// set/store these in the current packet
uint8_t *lob_head(lob_t p, uint8_t *head, size_t len);
uint8_t *lob_body(lob_t p, uint8_t *body, size_t len);
lob_t lob_append(lob_t p, uint8_t *chunk, size_t len);
lob_t lob_append_str(lob_t p, char *chunk);

// core accessors
size_t lob_head_len(lob_t p);
uint8_t *lob_head_get(lob_t p);
size_t lob_body_len(lob_t p);
uint8_t *lob_body_get(lob_t p);

// convenient json setters/getters, always return given lob so they're chainable
lob_t lob_set_raw(lob_t p, char *key, size_t klen, char *val, size_t vlen); // raw
lob_t lob_set(lob_t p, char *key, char *val); // escapes value
lob_t lob_set_len(lob_t p, char *key, size_t klen, char *val, size_t vlen); // same as lob_set
lob_t lob_set_int(lob_t p, char *key, int val);
lob_t lob_set_uint(lob_t p, char *key, unsigned int val);
lob_t lob_set_float(lob_t p, char *key, float val, uint8_t places);
lob_t lob_set_printf(lob_t p, char *key, const char *format, ...);
lob_t lob_set_base32(lob_t p, char *key, uint8_t *val, size_t vlen);

// copies keys from json into p
lob_t lob_set_json(lob_t p, lob_t json);

// count of keys
unsigned int lob_keys(lob_t p);

// alpha-sorts the json keys
lob_t lob_sort(lob_t p);

// 0 to match, !0 if different, compares only top-level json and body
int lob_cmp(lob_t a, lob_t b);

// the return uint8_t* is invalidated with any _set* operation!
char *lob_get(lob_t p, char *key);
int lob_get_int(lob_t p, char *key);
unsigned int lob_get_uint(lob_t p, char *key);
float lob_get_float(lob_t p, char *key);

char *lob_get_index(lob_t p, uint32_t i); // returns ["0","1","2","3"] or {"0":"1","2":"3"}

// just shorthand for util_cmp to match a key/value
int lob_get_cmp(lob_t p, char *key, char *val);

// get the raw value, must use get_len
char *lob_get_raw(lob_t p, char *key);
size_t lob_get_len(lob_t p, char *key);

// returns new packets based on values
lob_t lob_get_json(lob_t p, char *key); // creates new packet from key:object value
lob_t lob_get_array(lob_t p, char *key); // list of packet->next from key:[object,object]
lob_t lob_get_base32(lob_t p, char *key); // decoded binary is the return body

// TODO, this would be handy, js syntax to get a json value
// char *lob_eval(lob_t p, "foo.bar[0]['zzz']");

// manage a basic double-linked list of packets using ->next and ->prev
lob_t lob_pop(lob_t list); // returns last item, item->next is the new list
lob_t lob_push(lob_t list, lob_t append); // appends new item, returns new list
lob_t lob_shift(lob_t list); // returns first item, item->next is the new list
lob_t lob_unshift(lob_t list, lob_t prepend); // adds item, returns new list
lob_t lob_splice(lob_t list, lob_t extract); // removes item from list, returns new list
lob_t lob_insert(lob_t list, lob_t after, lob_t p); // inserts item in list after other item, returns new list
lob_t lob_freeall(lob_t list); // frees all
lob_t lob_match(lob_t list, char *key, char *value); // find the first packet in the list w/ the matching key/value
lob_t lob_next(lob_t list);
lob_t lob_array(lob_t list); // return json array of the list

#endif
#include <stddef.h>

#ifndef xht_h
#define xht_h

// simple string->void* hashtable, very static and bare minimal, but efficient

typedef struct xht_struct *xht_t;

// must pass a prime#
xht_t xht_new(unsigned int prime);

// caller responsible for key storage, no copies made (don't free it b4 xht_free()!)
// set val to NULL to clear an entry, memory is reused but never free'd (# of keys only grows to peak usage)
void xht_set(xht_t h, const char *key, void *val);

// ooh! unlike set where key/val is in caller's mem, here they are copied into xht_t and free'd when val is 0 or xht_free()
void xht_store(xht_t h, const char *key, void *val, size_t vlen);

// returns value of val if found, or NULL
void *xht_get(xht_t h, const char *key);

// free the hashtable and all entries
void xht_free(xht_t h);

// pass a function that is called for every key that has a value set
typedef void (*xht_walker)(xht_t h, const char *key, void *val, void *arg);
void xht_walk(xht_t h, xht_walker w, void *arg);

// iterator through all the keys (NULL to start), use get for values
char *xht_iter(xht_t h, char *key);

#endif

#ifndef e3x_cipher_h
#define e3x_cipher_h


// these are unique to each cipher set implementation
#define local_t void*
#define remote_t void*
#define ephemeral_t void*

// this is the overall holder for each cipher set, function pointers to cs specific implementations
typedef struct e3x_cipher_struct
{
  // these are common functions each one needs to support
  uint8_t *(*rand)(uint8_t *bytes, size_t len); // write len random bytes, returns bytes as well for convenience
  uint8_t *(*hash)(uint8_t *in, size_t len, uint8_t *out32); // sha256's the in, out32 must be [32] from caller
  uint8_t *(*err)(void); // last known crypto error string, if any

  // create a new keypair, save encoded to csid in each
  uint8_t (*generate)(lob_t keys, lob_t secrets);

  // our local identity
  local_t (*local_new)(lob_t keys, lob_t secrets);
  void (*local_free)(local_t local);
  lob_t (*local_decrypt)(local_t local, lob_t outer);
  lob_t (*local_sign)(local_t local, lob_t args, uint8_t *data, size_t len);
  
  // a remote endpoint identity
  remote_t (*remote_new)(lob_t key, uint8_t *token);
  void (*remote_free)(remote_t remote);
  uint8_t (*remote_verify)(remote_t remote, local_t local, lob_t outer);
  lob_t (*remote_encrypt)(remote_t remote, local_t local, lob_t inner);
  uint8_t (*remote_validate)(remote_t remote, lob_t args, lob_t sig, uint8_t *data, size_t len);
  
  // an active session to a remote for channel packets
  ephemeral_t (*ephemeral_new)(remote_t remote, lob_t outer);
  void (*ephemeral_free)(ephemeral_t ephemeral);
  lob_t (*ephemeral_encrypt)(ephemeral_t ephemeral, lob_t inner);
  lob_t (*ephemeral_decrypt)(ephemeral_t ephemeral, lob_t outer);

  uint8_t id, csid;
  char hex[3], *alg;
} *e3x_cipher_t;


// all possible cipher sets, as index into cipher_sets global
#define CS_1a 0
#define CS_1c 1
#define CS_2a 2
#define CS_3a 3
#define CS_MAX 4

extern e3x_cipher_t e3x_cipher_sets[]; // all created
extern e3x_cipher_t e3x_cipher_default; // just one of them for the rand/hash utils

// calls all e3x_cipher_init_*'s to fill in e3x_cipher_sets[]
uint8_t e3x_cipher_init(lob_t options);

// return by id or hex
e3x_cipher_t e3x_cipher_set(uint8_t csid, char *hex);

// init functions for each
e3x_cipher_t cs1a_init(lob_t options);
e3x_cipher_t cs1c_init(lob_t options);
e3x_cipher_t cs2a_init(lob_t options);
e3x_cipher_t cs3a_init(lob_t options);

#endif
#ifndef e3x_self_h
#define e3x_self_h


typedef struct e3x_self_struct
{
  lob_t keys[CS_MAX];
  local_t locals[CS_MAX];
} *e3x_self_t;

// load id secrets/keys to create a new local endpoint
e3x_self_t e3x_self_new(lob_t secrets, lob_t keys);
void e3x_self_free(e3x_self_t self); // any exchanges must have been free'd first

// try to decrypt any message sent to us, returns the inner
lob_t e3x_self_decrypt(e3x_self_t self, lob_t message);

// generate a signature for the data
lob_t e3x_self_sign(e3x_self_t self, lob_t args, uint8_t *data, size_t len);

#endif
#ifndef e3x_exchange_h
#define e3x_exchange_h

#include <stdint.h>

// apps should only use accessor functions for values in this struct
typedef struct e3x_exchange_struct
{
  e3x_cipher_t cs; // convenience
  e3x_self_t self;
  remote_t remote;
  ephemeral_t ephem;
  uint32_t in, out;
  uint32_t cid, last;
  uint8_t token[16], eid[16];
  uint8_t csid, order;
  char hex[3];
} *e3x_exchange_t;

// make a new exchange
// packet must contain the raw key in the body
e3x_exchange_t e3x_exchange_new(e3x_self_t self, uint8_t csid, lob_t key);
void e3x_exchange_free(e3x_exchange_t x);

// these are stateless async encryption and verification
lob_t e3x_exchange_message(e3x_exchange_t x, lob_t inner);
uint8_t e3x_exchange_verify(e3x_exchange_t x, lob_t outer);
uint8_t e3x_exchange_validate(e3x_exchange_t x, lob_t args, lob_t sig, uint8_t *data, size_t len);

// return the current incoming at value, optional arg to update it
uint32_t e3x_exchange_in(e3x_exchange_t x, uint32_t at);

// will return the current outgoing at value, optional arg to update it
uint32_t e3x_exchange_out(e3x_exchange_t x, uint32_t at);

// synchronize to incoming ephemeral key and set out at = in at, returns x if success, NULL if not
e3x_exchange_t e3x_exchange_sync(e3x_exchange_t x, lob_t outer);

// drops ephemeral state, out=0
e3x_exchange_t e3x_exchange_down(e3x_exchange_t x);

// generates handshake w/ current e3x_exchange_out value and ephemeral key
lob_t e3x_exchange_handshake(e3x_exchange_t x, lob_t inner);

// simple synchronous encrypt/decrypt conversion of any packet for channels
lob_t e3x_exchange_receive(e3x_exchange_t x, lob_t outer); // goes to channel, validates cid
lob_t e3x_exchange_send(e3x_exchange_t x, lob_t inner); // comes from channel 

// validate the next incoming channel id from the packet, or return the next avail outgoing channel id
uint32_t e3x_exchange_cid(e3x_exchange_t x, lob_t incoming);

// get the 16-byte token value to this exchange
uint8_t *e3x_exchange_token(e3x_exchange_t x);

#endif
#ifndef hashname_h
#define hashname_h


// overall type
typedef struct hashname_struct
{
  uint8_t bin[32];
} *hashname_t;

// only things that actually malloc/free
hashname_t hashname_dup(hashname_t hn);
hashname_t hashname_free(hashname_t hn);

// everything else returns a pointer to a static global for temporary use
hashname_t hashname_vchar(const char *str); // from a string
hashname_t hashname_vbin(const uint8_t *bin);
hashname_t hashname_vkeys(lob_t keys);
hashname_t hashname_vkey(lob_t key, uint8_t id); // key is body, intermediates in json

// accessors
uint8_t *hashname_bin(hashname_t hn); // 32 bytes
char *hashname_char(hashname_t hn); // 52 byte base32 string w/ \0 (TEMPORARY)

// utilities related to hashnames
int hashname_cmp(hashname_t a, hashname_t b);  // memcmp shortcut
uint8_t hashname_id(lob_t a, lob_t b); // best matching id (single byte)
lob_t hashname_im(lob_t keys, uint8_t id); // intermediate hashes in the json, optional id to set that as body

// working with short hashnames (5 bin bytes, 8 char bytes)
char *hashname_short(hashname_t hn); // 8 byte base32 string w/ \0 (TEMPORARY)
int hashname_scmp(hashname_t a, hashname_t b);  // short only comparison
hashname_t hashname_schar(const char *str); // 8 char string, temp hn
hashname_t hashname_sbin(const uint8_t *bin); // 5 bytes, temp hn
hashname_t hashname_isshort(hashname_t hn); // NULL unless is short

#endif
#ifndef mesh_h
#define mesh_h

typedef struct mesh_struct *mesh_t;
typedef struct link_struct *link_t;
typedef struct chan_struct *chan_t;




struct mesh_struct
{
  hashname_t id;
  lob_t keys, paths;
  e3x_self_t self;
  void *on; // internal list of triggers
  // shared network info
  uint16_t port_local, port_public;
  char *ipv4_local, *ipv4_public;
  link_t links;
};

mesh_t mesh_new(void);
mesh_t mesh_free(mesh_t mesh);

// must be called to initialize to a hashname from keys/secrets, return !0 if failed
uint8_t mesh_load(mesh_t mesh, lob_t secrets, lob_t keys);

// creates and loads a new random hashname, returns secrets if it needs to be saved/reused
lob_t mesh_generate(mesh_t mesh);

// simple accessors
hashname_t mesh_id(mesh_t mesh);
lob_t mesh_keys(mesh_t mesh);

// generate json of mesh keys and current paths
lob_t mesh_json(mesh_t mesh);

// generate json for all links, returns lob list
lob_t mesh_links(mesh_t mesh);

// creates a link from the json format of {"hashname":"...","keys":{},"paths":[]}
link_t mesh_add(mesh_t mesh, lob_t json);

// return only if this hashname (full or short) is currently linked (in any state)
link_t mesh_linked(mesh_t mesh, char *hn, size_t len);
link_t mesh_linkid(mesh_t mesh, hashname_t id); // TODO, clean this up

// remove this link, will event it down and clean up during next process()
mesh_t mesh_unlink(link_t link);

// processes incoming packet, it will take ownership of packet, returns link delivered to if success
link_t mesh_receive(mesh_t mesh, lob_t packet);

// process any unencrypted handshake packet
link_t mesh_receive_handshake(mesh_t mesh, lob_t handshake);

// process any channel timeouts based on the current/given time
mesh_t mesh_process(mesh_t mesh, uint32_t now);

// callback when the mesh is free'd
void mesh_on_free(mesh_t mesh, char *id, void (*free)(mesh_t mesh));

// callback when a path needs to be turned into a pipe
void mesh_on_path(mesh_t mesh, char *id, link_t (*path)(link_t link, lob_t path));
link_t mesh_path(mesh_t mesh, link_t link, lob_t path);

// callback when an unknown hashname is discovered
void mesh_on_discover(mesh_t mesh, char *id, link_t (*discover)(mesh_t mesh, lob_t discovered));
void mesh_discover(mesh_t mesh, lob_t discovered);

// callback when a link changes state created/up/down
void mesh_on_link(mesh_t mesh, char *id, void (*link)(link_t link));
void mesh_link(mesh_t mesh, link_t link);

// callback when a new incoming channel is requested
void mesh_on_open(mesh_t mesh, char *id, lob_t (*open)(link_t link, lob_t open));
lob_t mesh_open(mesh_t mesh, link_t link, lob_t open);


#endif
#ifndef link_h
#define link_h
#include <stdint.h>


struct link_struct
{
  // public link data
  hashname_t id;
  e3x_exchange_t x;
  mesh_t mesh;
  lob_t key;
  chan_t chans;

  // transport plumbing
  void *send_arg;
  link_t (*send_cb)(link_t link, lob_t packet, void *arg);
  
  // these are for internal link management only
  link_t next;
  uint8_t csid;
};

// these all create or return existing one from the mesh
link_t link_get(mesh_t mesh, hashname_t id);
link_t link_get_keys(mesh_t mesh, lob_t keys); // adds in the right key
link_t link_get_key(mesh_t mesh, lob_t key, uint8_t csid); // adds in from the body

// simple accessors
hashname_t link_id(link_t link);
lob_t link_key(link_t link);

// get link info json
lob_t link_json(link_t link);

// removes from mesh
void link_free(link_t link);

// load in the key to existing link
link_t link_load(link_t link, uint8_t csid, lob_t key);

// add a delivery pipe to this link
link_t link_pipe(link_t link, link_t (*send)(link_t link, lob_t packet, void *arg), void *arg);

// process a decrypted channel packet
link_t link_receive(link_t link, lob_t inner);

// process an incoming handshake
link_t link_receive_handshake(link_t link, lob_t handshake);

// try to deliver this encrypted packet
link_t link_send(link_t link, lob_t outer);

// encrypt and send this packet
link_t link_direct(link_t link, lob_t inner);

// return current handshake (caller free's)
lob_t link_handshake(link_t link);

// send current handshake(s) 
link_t link_sync(link_t link);

// force generate new encrypted handshake(s) and sync
link_t link_resync(link_t link);

// is the other endpoint connected and the link available, NULL if not
link_t link_up(link_t link);

// force link down, ends channels and generates events
link_t link_down(link_t link);

// create/track a new channel for this open
chan_t link_chan(link_t link, lob_t open);

// process any channel timeouts based on the current/given time
link_t link_process(link_t link, uint32_t now);

#endif
#ifndef chan_h
#define chan_h
#include <stdint.h>

enum chan_states { CHAN_ENDED, CHAN_OPENING, CHAN_OPEN };

// standalone channel packet management, buffering and ordering
// internal only structure, always use accessors
struct chan_struct
{
  link_t link; // so channels can be first-class
  chan_t next; // links keep lists
  uint32_t id; // wire id (not unique)
  char *type;
  lob_t in;

  // timer stuff
  uint32_t tsent, trecv; // last send, recv at
  uint32_t timeout; // when in the future to trigger timeout
  
  // direct handler
  void *arg;
  void (*handle)(chan_t c, void *arg);

  enum chan_states state;
};

// caller must manage lists of channels per e3x_exchange based on cid
chan_t chan_new(lob_t open); // open must be chan_receive or chan_send next yet
chan_t chan_free(chan_t c);

// sets when in the future this channel should timeout auto-error from no receive, returns current timeout
uint32_t chan_timeout(chan_t c, uint32_t at);

// returns current inbox cache
uint32_t chan_size(chan_t c);

// incoming packets
chan_t chan_receive(chan_t c, lob_t inner); // process into receiving queue
chan_t chan_sync(chan_t c, uint8_t sync); // false to force start timeouts (after any new handshake), true to cancel and resend last packet (after any e3x_exchange_sync)
lob_t chan_receiving(chan_t c); // get next avail packet in order, null if nothing

// outgoing packets
lob_t chan_oob(chan_t c); // id/ack/miss only headers base packet
lob_t chan_packet(chan_t c);  // creates a sequenced packet w/ all necessary headers, just a convenience
chan_t chan_send(chan_t c, lob_t inner); // encrypts and sends packet out link
chan_t chan_err(chan_t c, char *err); // generates local-only error packet for next chan_process()

// must be called after every send or receive, processes resends/timeouts, fires handlers
chan_t chan_process(chan_t c, uint32_t now);

// set up internal handler for all incoming packets on this channel
chan_t chan_handle(chan_t c, void (*handle)(chan_t c, void *arg), void *arg);

// convenience functions, accessors
chan_t chan_next(chan_t c); // c->next
uint32_t chan_id(chan_t c); // c->id
enum chan_states chan_state(chan_t c);



#endif
#ifndef util_chunks_h
#define util_chunks_h

#include <stdint.h>

// for list of incoming chunks
typedef struct util_chunk_struct
{
  struct util_chunk_struct *prev;
  uint8_t size;
  uint8_t data[];
} *util_chunk_t;

typedef struct util_chunks_struct
{

  util_chunk_t reading; // stacked linked list of incoming chunks

  lob_t writing;
  size_t writeat; // offset into lob_raw()
  uint16_t waitat; // gets to 256, offset into current chunk
  uint8_t waiting; // current writing chunk size;
  uint8_t readat; // always less than a max chunk, offset into reading

  uint8_t cap;
  uint8_t blocked:1, blocking:1, ack:1, err:1; // bool flags
} *util_chunks_t;


// size of each chunk, 0 == MAX (256)
util_chunks_t util_chunks_new(uint8_t size);

util_chunks_t util_chunks_free(util_chunks_t chunks);

// turn this packet into chunks and append, free's out
util_chunks_t util_chunks_send(util_chunks_t chunks, lob_t out);

// get any packets that have been reassembled from incoming chunks
lob_t util_chunks_receive(util_chunks_t chunks);

// bytes waiting to be sent
uint32_t util_chunks_writing(util_chunks_t chunks);


////// these are for a stream-based transport

// how many bytes are there ready to write
uint32_t util_chunks_len(util_chunks_t chunks);

// return the next block of data to be written to the stream transport, max len is util_chunks_len()
uint8_t *util_chunks_write(util_chunks_t chunks);

// advance the write this far, don't mix with util_chunks_out() usage
util_chunks_t util_chunks_written(util_chunks_t chunks, size_t len);

// queues incoming stream based data
util_chunks_t util_chunks_read(util_chunks_t chunks, uint8_t *block, size_t len);

////// these are for frame-based transport

// size of the next chunk, -1 when none, max is chunks size-1
int16_t util_chunks_size(util_chunks_t chunks);

// return the next chunk of data, use util_chunks_next to advance
uint8_t *util_chunks_frame(util_chunks_t chunks);

// peek into what the next chunk size will be, to see terminator ones
int16_t util_chunks_peek(util_chunks_t chunks);

// process incoming chunk
util_chunks_t util_chunks_chunk(util_chunks_t chunks, uint8_t *chunk, int16_t size);

// advance the write past the current chunk
util_chunks_t util_chunks_next(util_chunks_t chunks);


#endif
#ifndef util_frames_h
#define util_frames_h

#include <stdint.h>

// for list of incoming frames
typedef struct util_frame_struct
{
  struct util_frame_struct *prev;
  uint32_t hash;
  uint8_t data[];
} *util_frame_t;

typedef struct util_frames_struct
{

  lob_t inbox; // received packets waiting to be processed
  lob_t outbox; // current packet being sent out

  util_frame_t cache; // stacked linked list of incoming frames in progress

  uint32_t inbase; // last confirmed inbox hash
  uint32_t outbase; // last confirmed outbox hash

  uint8_t in; // number of incoming frames received/waiting
  uint8_t out; //  number of outgoing frames of outbox sent since outbase

  uint8_t size; // frame size
  uint8_t flush:1; // bool to signal a flush is needed
  uint8_t err:1; // unrecoverable failure
  uint8_t more:1; // last incoming meta said there was more

} *util_frames_t;


// size of each frame, min 16 max 128, multiple of 4
util_frames_t util_frames_new(uint8_t size);

util_frames_t util_frames_free(util_frames_t frames);

// turn this packet into frames and append, free's out
util_frames_t util_frames_send(util_frames_t frames, lob_t out);

// get any packets that have been reassembled from incoming frames
lob_t util_frames_receive(util_frames_t frames);

// total bytes in the inbox/outbox
size_t util_frames_inlen(util_frames_t frames);
size_t util_frames_outlen(util_frames_t frames);

// meta space is (size - 14) and only filled when receiving a meta frame
util_frames_t util_frames_inbox(util_frames_t frames, uint8_t *data, uint8_t *meta); // data=NULL is ready check

// fills data with the next frame, if no payload available always fills w/ meta frame, safe to re-run (idempotent)
util_frames_t util_frames_outbox(util_frames_t frames, uint8_t *data, uint8_t *meta); // data=NULL is ready-check

// this must be called immediately (no inbox interleaved) after last outbox is actually sent to advance payload or clear flush request, returns if more to send
util_frames_t util_frames_sent(util_frames_t frames);

// is just a check to see if there's data waiting to be sent
util_frames_t util_frames_waiting(util_frames_t frames);

// is there an expectation of an incoming frame
util_frames_t util_frames_await(util_frames_t frames);

// are we waiting to send/receive a frame (both waiting && await)
util_frames_t util_frames_busy(util_frames_t frames);

// is a frame pending to be sent immediately
util_frames_t util_frames_pending(util_frames_t frames);

// check error state and clearing it
util_frames_t util_frames_ok(util_frames_t frames);
util_frames_t util_frames_clear(util_frames_t frames);

#endif
/**
 * \file aes.h
 *
 * \brief AES block cipher
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#ifndef MBEDTLS_AES_H
#define MBEDTLS_AES_H


#include <stddef.h>
#include <stdint.h>

#define MBEDTLS_AES_ENCRYPT     1
#define MBEDTLS_AES_DECRYPT     0

#define MBEDTLS_ERR_AES_INVALID_KEY_LENGTH                -0x0020  /**< Invalid key length. */
#define MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH              -0x0022  /**< Invalid data input length. */

#define MBEDTLS_AES_ROM_TABLES

#ifdef __cplusplus
extern "C" {
#endif

// local wrapper
void aes_128_ctr(unsigned char *key, size_t length, unsigned char nonce_counter[16], const unsigned char *input, unsigned char *output);

/**
 * \brief          AES context structure
 *
 * \note           buf is able to hold 32 extra bytes, which can be used:
 *                 - for alignment purposes if VIA padlock is used, and/or
 *                 - to simplify key expansion in the 256-bit case by
 *                 generating an extra round key
 */
typedef struct
{
    int nr;                     /*!<  number of rounds  */
    uint32_t *rk;               /*!<  AES round keys    */
    uint32_t buf[68];           /*!<  unaligned data    */
}
mbedtls_aes_context;

/**
 * \brief          Initialize AES context
 *
 * \param ctx      AES context to be initialized
 */
void mbedtls_aes_init( mbedtls_aes_context *ctx );

/**
 * \brief          Clear AES context
 *
 * \param ctx      AES context to be cleared
 */
void mbedtls_aes_free( mbedtls_aes_context *ctx );

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keybits  must be 128, 192 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_enc( mbedtls_aes_context *ctx, const unsigned char *key,
                    unsigned int keybits );

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keybits  must be 128, 192 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_dec( mbedtls_aes_context *ctx, const unsigned char *key,
                    unsigned int keybits );

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     MBEDTLS_AES_ENCRYPT or MBEDTLS_AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if successful
 */
int mbedtls_aes_crypt_ecb( mbedtls_aes_context *ctx,
                    int mode,
                    const unsigned char input[16],
                    unsigned char output[16] );


/**
 * \brief               AES-CTR buffer encryption/decryption
 *
 * Warning: You have to keep the maximum use of your counter in mind!
 *
 * Note: Due to the nature of CTR you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * mbedtls_aes_setkey_enc() for both MBEDTLS_AES_ENCRYPT and MBEDTLS_AES_DECRYPT.
 *
 * \param ctx           AES context
 * \param length        The length of the data
 * \param nc_off        The offset in the current stream_block (for resuming
 *                      within current cipher stream). The offset pointer to
 *                      should be 0 at the start of a stream.
 * \param nonce_counter The 128-bit nonce and counter.
 * \param stream_block  The saved stream-block for resuming. Is overwritten
 *                      by the function.
 * \param input         The input data stream
 * \param output        The output data stream
 *
 * \return         0 if successful
 */
int mbedtls_aes_crypt_ctr( mbedtls_aes_context *ctx,
                       size_t length,
                       size_t *nc_off,
                       unsigned char nonce_counter[16],
                       unsigned char stream_block[16],
                       const unsigned char *input,
                       unsigned char *output );

/**
 * \brief           Internal AES block encryption function
 *                  (Only exposed to allow overriding it,
 *                  see MBEDTLS_AES_ENCRYPT_ALT)
 *
 * \param ctx       AES context
 * \param input     Plaintext block
 * \param output    Output (ciphertext) block
 */
void mbedtls_aes_encrypt( mbedtls_aes_context *ctx,
                          const unsigned char input[16],
                          unsigned char output[16] );

/**
 * \brief           Internal AES block decryption function
 *                  (Only exposed to allow overriding it,
 *                  see MBEDTLS_AES_DECRYPT_ALT)
 *
 * \param ctx       AES context
 * \param input     Ciphertext block
 * \param output    Output (plaintext) block
 */
void mbedtls_aes_decrypt( mbedtls_aes_context *ctx,
                          const unsigned char input[16],
                          unsigned char output[16] );

#ifdef __cplusplus
}
#endif

#endif /* aes.h */
// Base32 implementation
//
// Copyright 2010 Google Inc.
// Author: Markus Gutschke
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Encode and decode from base32 encoding using the following alphabet:
//   ABCDEFGHIJKLMNOPQRSTUVWXYZ234567
// This alphabet is documented in RFC 4668/3548
//
// We allow white-space and hyphens, but all other characters are considered
// invalid.
//
// All functions return the number of output bytes or -1 on error. If the
// output buffer is too small, the result will silently be truncated.

#ifndef _BASE32_H_
#define _BASE32_H_

#include <stdint.h>
#include <stddef.h>

size_t base32_decode(const char *encoded, size_t length, uint8_t *result, size_t bufSize);
size_t base32_encode(const uint8_t *data, size_t length, char *result, size_t bufSize);

// number of characters required, including null terminator
size_t base32_encode_length(size_t rawLength);

// number of bytes, rounded down (truncates extra bits)
size_t base32_decode_floor(size_t base32Length);

#endif /* _BASE32_H_ */
#ifndef b64_h
#define b64_h

#include <stddef.h>
#include <stdint.h>

// min buffer size for encoding/decoding
#define base64_encode_length(x) (8 * (((x) + 2) / 6)) + 3
#define base64_decode_length(x) ((((x) + 2) * 6) / 8)

// encode str of len into out (must be at least base64_encode_length(len) big), return actual encoded len
size_t base64_encoder(const uint8_t *str, size_t len, char *out);

// decode str of len into out (must be base64_decode_length(len) bit), return actual decoded len
size_t base64_decoder(const char *str, size_t len, uint8_t *out);

#endif

#ifndef _CHACHA20_H_
#define _CHACHA20_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// a convert-in-place utility
uint8_t *chacha20(uint8_t *key, uint8_t *nonce, uint8_t *bytes, uint32_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !_CHACHA20_H_ */
#ifndef chan_h
#define chan_h
#include <stdint.h>

enum chan_states { CHAN_ENDED, CHAN_OPENING, CHAN_OPEN };

// standalone channel packet management, buffering and ordering
// internal only structure, always use accessors
struct chan_struct
{
  link_t link; // so channels can be first-class
  chan_t next; // links keep lists
  uint32_t id; // wire id (not unique)
  char *type;
  lob_t in;

  // timer stuff
  uint32_t tsent, trecv; // last send, recv at
  uint32_t timeout; // when in the future to trigger timeout
  
  // direct handler
  void *arg;
  void (*handle)(chan_t c, void *arg);

  enum chan_states state;
};

// caller must manage lists of channels per e3x_exchange based on cid
chan_t chan_new(lob_t open); // open must be chan_receive or chan_send next yet
chan_t chan_free(chan_t c);

// sets when in the future this channel should timeout auto-error from no receive, returns current timeout
uint32_t chan_timeout(chan_t c, uint32_t at);

// returns current inbox cache
uint32_t chan_size(chan_t c);

// incoming packets
chan_t chan_receive(chan_t c, lob_t inner); // process into receiving queue
chan_t chan_sync(chan_t c, uint8_t sync); // false to force start timeouts (after any new handshake), true to cancel and resend last packet (after any e3x_exchange_sync)
lob_t chan_receiving(chan_t c); // get next avail packet in order, null if nothing

// outgoing packets
lob_t chan_oob(chan_t c); // id/ack/miss only headers base packet
lob_t chan_packet(chan_t c);  // creates a sequenced packet w/ all necessary headers, just a convenience
chan_t chan_send(chan_t c, lob_t inner); // encrypts and sends packet out link
chan_t chan_err(chan_t c, char *err); // generates local-only error packet for next chan_process()

// must be called after every send or receive, processes resends/timeouts, fires handlers
chan_t chan_process(chan_t c, uint32_t now);

// set up internal handler for all incoming packets on this channel
chan_t chan_handle(chan_t c, void (*handle)(chan_t c, void *arg), void *arg);

// convenience functions, accessors
chan_t chan_next(chan_t c); // c->next
uint32_t chan_id(chan_t c); // c->id
enum chan_states chan_state(chan_t c);



#endif
#ifndef e3x_h
#define e3x_h

#define E3X_VERSION_MAJOR 0
#define E3X_VERSION_MINOR 5
#define E3X_VERSION_PATCH 1
#define E3X_VERSION ((E3X_VERSION_MAJOR) * 10000 + (E3X_VERSION_MINOR) * 100 + (E3X_VERSION_PATCH))

#ifdef __cplusplus
extern "C" {
#endif


// ##### e3x - end-to-end encrypted exchange #####
//
// * intended to be wrapped/embedded in other codebases
// * includes some minimal crypto, but is primarily a wrapper around other crypto libraries
// * tries to minimize the integration points to send and receive encrypted packets over any transport
// * everything contains a '3' to minimize any naming conflicts when used with other codebases
//


// top-level library functions

// process-wide boot one-time initialization, !0 is error and lob_get(options,"err");
uint8_t e3x_init(lob_t options);

// return last known error string from anywhere, intended only for debugging/logging
uint8_t *e3x_err(void);

// generate a new local identity, secrets returned in the lob json and keys in the linked lob json
lob_t e3x_generate(void);

// random bytes, from a supported cipher set
uint8_t *e3x_rand(uint8_t *bytes, size_t len);

// set a random byte provider function
void e3x_random(uint8_t (*frand)(void));

// sha256 hashing, from one of the cipher sets
uint8_t *e3x_hash(uint8_t *in, size_t len, uint8_t *out32);


// local endpoint state management

// a single exchange (a session w/ local endpoint and remote endpoint)



//##############
/* binding notes

* app must have an index of the hashnames-to-exchange and tokens-to-exchange
* for each exchange, keep a list of active network transport sessions and potential transport paths
* also keep a list of active channels per exchange to track state
* one transport session may be delivering to multiple exchanges (not a 1:1 mapping)
* unreliable transport sessions must trigger a new handshake for any exchange when they need to be re-validated
* all transport sessions must signal when they are closed, which generates a new handshake to any other sessions
* always use the most recently validated-active transport session to deliver to for sending

*/

#ifdef __cplusplus
}
#endif

#endif
#ifndef e3x_cipher_h
#define e3x_cipher_h


// these are unique to each cipher set implementation
#define local_t void*
#define remote_t void*
#define ephemeral_t void*

// this is the overall holder for each cipher set, function pointers to cs specific implementations
typedef struct e3x_cipher_struct
{
  // these are common functions each one needs to support
  uint8_t *(*rand)(uint8_t *bytes, size_t len); // write len random bytes, returns bytes as well for convenience
  uint8_t *(*hash)(uint8_t *in, size_t len, uint8_t *out32); // sha256's the in, out32 must be [32] from caller
  uint8_t *(*err)(void); // last known crypto error string, if any

  // create a new keypair, save encoded to csid in each
  uint8_t (*generate)(lob_t keys, lob_t secrets);

  // our local identity
  local_t (*local_new)(lob_t keys, lob_t secrets);
  void (*local_free)(local_t local);
  lob_t (*local_decrypt)(local_t local, lob_t outer);
  lob_t (*local_sign)(local_t local, lob_t args, uint8_t *data, size_t len);
  
  // a remote endpoint identity
  remote_t (*remote_new)(lob_t key, uint8_t *token);
  void (*remote_free)(remote_t remote);
  uint8_t (*remote_verify)(remote_t remote, local_t local, lob_t outer);
  lob_t (*remote_encrypt)(remote_t remote, local_t local, lob_t inner);
  uint8_t (*remote_validate)(remote_t remote, lob_t args, lob_t sig, uint8_t *data, size_t len);
  
  // an active session to a remote for channel packets
  ephemeral_t (*ephemeral_new)(remote_t remote, lob_t outer);
  void (*ephemeral_free)(ephemeral_t ephemeral);
  lob_t (*ephemeral_encrypt)(ephemeral_t ephemeral, lob_t inner);
  lob_t (*ephemeral_decrypt)(ephemeral_t ephemeral, lob_t outer);

  uint8_t id, csid;
  char hex[3], *alg;
} *e3x_cipher_t;


// all possible cipher sets, as index into cipher_sets global
#define CS_1a 0
#define CS_1c 1
#define CS_2a 2
#define CS_3a 3
#define CS_MAX 4

extern e3x_cipher_t e3x_cipher_sets[]; // all created
extern e3x_cipher_t e3x_cipher_default; // just one of them for the rand/hash utils

// calls all e3x_cipher_init_*'s to fill in e3x_cipher_sets[]
uint8_t e3x_cipher_init(lob_t options);

// return by id or hex
e3x_cipher_t e3x_cipher_set(uint8_t csid, char *hex);

// init functions for each
e3x_cipher_t cs1a_init(lob_t options);
e3x_cipher_t cs1c_init(lob_t options);
e3x_cipher_t cs2a_init(lob_t options);
e3x_cipher_t cs3a_init(lob_t options);

#endif
#ifndef e3x_exchange_h
#define e3x_exchange_h

#include <stdint.h>

// apps should only use accessor functions for values in this struct
typedef struct e3x_exchange_struct
{
  e3x_cipher_t cs; // convenience
  e3x_self_t self;
  remote_t remote;
  ephemeral_t ephem;
  uint32_t in, out;
  uint32_t cid, last;
  uint8_t token[16], eid[16];
  uint8_t csid, order;
  char hex[3];
} *e3x_exchange_t;

// make a new exchange
// packet must contain the raw key in the body
e3x_exchange_t e3x_exchange_new(e3x_self_t self, uint8_t csid, lob_t key);
void e3x_exchange_free(e3x_exchange_t x);

// these are stateless async encryption and verification
lob_t e3x_exchange_message(e3x_exchange_t x, lob_t inner);
uint8_t e3x_exchange_verify(e3x_exchange_t x, lob_t outer);
uint8_t e3x_exchange_validate(e3x_exchange_t x, lob_t args, lob_t sig, uint8_t *data, size_t len);

// return the current incoming at value, optional arg to update it
uint32_t e3x_exchange_in(e3x_exchange_t x, uint32_t at);

// will return the current outgoing at value, optional arg to update it
uint32_t e3x_exchange_out(e3x_exchange_t x, uint32_t at);

// synchronize to incoming ephemeral key and set out at = in at, returns x if success, NULL if not
e3x_exchange_t e3x_exchange_sync(e3x_exchange_t x, lob_t outer);

// drops ephemeral state, out=0
e3x_exchange_t e3x_exchange_down(e3x_exchange_t x);

// generates handshake w/ current e3x_exchange_out value and ephemeral key
lob_t e3x_exchange_handshake(e3x_exchange_t x, lob_t inner);

// simple synchronous encrypt/decrypt conversion of any packet for channels
lob_t e3x_exchange_receive(e3x_exchange_t x, lob_t outer); // goes to channel, validates cid
lob_t e3x_exchange_send(e3x_exchange_t x, lob_t inner); // comes from channel 

// validate the next incoming channel id from the packet, or return the next avail outgoing channel id
uint32_t e3x_exchange_cid(e3x_exchange_t x, lob_t incoming);

// get the 16-byte token value to this exchange
uint8_t *e3x_exchange_token(e3x_exchange_t x);

#endif
#ifndef e3x_self_h
#define e3x_self_h


typedef struct e3x_self_struct
{
  lob_t keys[CS_MAX];
  local_t locals[CS_MAX];
} *e3x_self_t;

// load id secrets/keys to create a new local endpoint
e3x_self_t e3x_self_new(lob_t secrets, lob_t keys);
void e3x_self_free(e3x_self_t self); // any exchanges must have been free'd first

// try to decrypt any message sent to us, returns the inner
lob_t e3x_self_decrypt(e3x_self_t self, lob_t message);

// generate a signature for the data
lob_t e3x_self_sign(e3x_self_t self, lob_t args, uint8_t *data, size_t len);

#endif
#ifndef ext_h
#define ext_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#endif
#ifndef ext_block_h
#define ext_block_h


// add block channel support
mesh_t ext_block(mesh_t mesh);

// get the next incoming block, if any, packet->arg is the link it came from
lob_t ext_block_receive(mesh_t mesh);

// creates/reuses a single default block channel on the link
link_t ext_block_send(link_t link, lob_t block);

// TODO, handle multiple block channels per link, and custom packets on open

#endif
#ifndef ext_chat_h
#define ext_chat_h

/*

typedef struct chat_struct 
{
  char ep[32+1], id[32+1+64+1], idhash[9];
  hashname_t origin;
  switch_t s;
  chan_t hub;
  char rhash[9];
  uint8_t local, seed[4];
  uint16_t seq;
  lob_t roster;
  xht_t conn, log;
  lob_t msgs;
  char *join, *sent, *after;
} *chat_t;

chat_t ext_chat(chan_t c);

chat_t chat_get(switch_t s, char *id);
chat_t chat_free(chat_t chat);

// get the next incoming message (type state/message), caller must free
lob_t chat_pop(chat_t chat);

lob_t chat_message(chat_t chat);
chat_t chat_join(chat_t chat, lob_t join);
chat_t chat_send(chat_t chat, lob_t msg);
chat_t chat_add(chat_t chat, char *hn, char *val);

// get a participant or walk the list, returns the current state packet (immutable), online:true/false
lob_t chat_participant(chat_t chat, char *hn);
lob_t chat_iparticipant(chat_t chat, int index);

*/
#endif
#ifndef ext_connect_h
#define ext_connect_h


//void ext_connect(chan_t c);

#endif
#ifndef ext_path_h
#define ext_path_h


// send a path ping and get callback event
link_t path_ping(link_t link, void (*pong)(link_t link, lob_t status, void *arg), void *arg);

lob_t path_on_open(link_t link, lob_t open);

#endif
#ifndef ext_peer_h
#define ext_peer_h


// enables peer/connect handling for this mesh
mesh_t peer_enable(mesh_t mesh);

// become a router
mesh_t peer_route(mesh_t mesh);

// set this link as the default router for any peer
link_t peer_router(link_t router);

// try to connect this peer via this router (sends an ad-hoc peer request)
link_t peer_connect(link_t peer, link_t router);


#endif

#ifndef ext_sock_h
#define ext_sock_h
/*

#define SOCKC_NEW 0
#define SOCKC_OPEN 1
#define SOCKC_CLOSED 2

typedef struct sockc_struct 
{
  uint8_t state;
  uint32_t readable, writing;
  uint8_t *readbuf, *writebuf, *zwrite;
  lob_t opts;
  chan_t c;
  int fd; // convenience for app usage, initialized to -1
} *sockc_t;

// process channel for new incoming sock channel, any returned sockc must have a read or close function called
// state is SOCKC_OPEN or SOCKC_NEW (until sockc_accept())
// it's set to SOCKC_CLOSED after all the data is read on an ended channel or sockc_close() is called
sockc_t ext_sock(chan_t c);

// changes state from SOCKC_NEW to SOCKC_OPEN
void sockc_accept(sockc_t sc);

// create a sock channel to this hn, optional opts (ip, port), sets state=SOCKC_OPEN
sockc_t sockc_connect(switch_t s, char *hn, lob_t opts);

// tries to flush and end, cleans up, sets SOCKC_CLOSED
sockc_t sockc_close(sockc_t sock);

// must be called to free up sc resources (internally calls sockc_close to be sure)
sockc_t sockc_free(sockc_t sc);

// -1 on err, returns bytes read into buf up to len, sets SOCKC_CLOSED when channel ended
int sockc_read(sockc_t sock, uint8_t *buf, int len);

// -1 on err, returns len and will always buffer up to len 
int sockc_write(sockc_t sock, uint8_t *buf, int len);

// general flush outgoing buffer into a packet
void sockc_flush(chan_t c);

// advances readbuf by this len, for use when doing zero-copy reading directly from ->readbuf
// sets SOCKC_CLOSED when channel ended
void sockc_zread(sockc_t sc, int len);

// creates zero-copy buffer space of requested len at sc->zwrite or returns 0
int sockc_zwrite(sockc_t sc, int len);

// must use after writing anything to ->zwrite, adds len to outgoing buffer and resets sc->zwrite, returns len
int sockc_zwritten(sockc_t sc, int len);

// serial-style single character interface
int sockc_available(sockc_t sc);
uint8_t sockc_sread(sockc_t sc);
uint8_t sockc_swrite(sockc_t sc, uint8_t byte);
*/
#endif
#ifndef ext_thtp_h
#define ext_thtp_h

/*

void ext_thtp(chan_t c);

// optionally initialize thtp w/ an index, happens automatically too
void thtp_init(switch_t s, xht_t index);
void thtp_free(switch_t s);

// sends requests matching this glob ("/path" matches "/path/foo") to this note, most specific wins
void thtp_glob(switch_t s, char *glob, lob_t note);

// sends requests matching this exact path to this note
void thtp_path(switch_t s, char *path, lob_t note);

// generate an outgoing request, send the response attached to the note
chan_t thtp_req(switch_t s, lob_t note);
*/
#endif
#ifndef hashname_h
#define hashname_h


// overall type
typedef struct hashname_struct
{
  uint8_t bin[32];
} *hashname_t;

// only things that actually malloc/free
hashname_t hashname_dup(hashname_t hn);
hashname_t hashname_free(hashname_t hn);

// everything else returns a pointer to a static global for temporary use
hashname_t hashname_vchar(const char *str); // from a string
hashname_t hashname_vbin(const uint8_t *bin);
hashname_t hashname_vkeys(lob_t keys);
hashname_t hashname_vkey(lob_t key, uint8_t id); // key is body, intermediates in json

// accessors
uint8_t *hashname_bin(hashname_t hn); // 32 bytes
char *hashname_char(hashname_t hn); // 52 byte base32 string w/ \0 (TEMPORARY)

// utilities related to hashnames
int hashname_cmp(hashname_t a, hashname_t b);  // memcmp shortcut
uint8_t hashname_id(lob_t a, lob_t b); // best matching id (single byte)
lob_t hashname_im(lob_t keys, uint8_t id); // intermediate hashes in the json, optional id to set that as body

// working with short hashnames (5 bin bytes, 8 char bytes)
char *hashname_short(hashname_t hn); // 8 byte base32 string w/ \0 (TEMPORARY)
int hashname_scmp(hashname_t a, hashname_t b);  // short only comparison
hashname_t hashname_schar(const char *str); // 8 char string, temp hn
hashname_t hashname_sbin(const uint8_t *bin); // 5 bytes, temp hn
hashname_t hashname_isshort(hashname_t hn); // NULL unless is short

#endif

// key = string to match or null
// klen = key length (or 0), or if null key then len is the array offset value
// json = json object or array
// jlen = length of json
// vlen = where to store return value length
// returns pointer to value and sets len to value length, or 0 if not found or any error
char *js0n(char *key, size_t klen, char *json, size_t jlen, size_t *vlen);
#ifndef jwt_h
#define jwt_h

#include <stdint.h>
#include <stdbool.h>

// utils to parse and generate JWTs to/from LOBs

// one JWT is two LOB packets, jwt_claims() returns second one
//  token->head is the JWT header JSON
//  token->body is a raw LOB for the claims
//  claims->head is the JWT claims JSON
//  claims->body is the JWT signature

lob_t jwt_decode(char *encoded, size_t len); // base64 into lobs
char *jwt_encode(lob_t token); // returns new malloc'd base64 string, caller owns

lob_t jwt_parse(uint8_t *raw, size_t len); // from raw lobs
uint8_t *jwt_raw(lob_t token); // lob-encoded raw bytes of whole thing
uint32_t jwt_len(lob_t token); // length of raw bytes

lob_t jwt_claims(lob_t token); // just returns the claims from the token
lob_t jwt_verify(lob_t token, e3x_exchange_t x); // checks signature, optional x for pk algs
lob_t jwt_sign(lob_t token, e3x_self_t self); // attaches signature to the claims, optional self for pk algs

// checks if this alg is supported
char *jwt_alg(char *alg);

#endif
#ifndef link_h
#define link_h
#include <stdint.h>


struct link_struct
{
  // public link data
  hashname_t id;
  e3x_exchange_t x;
  mesh_t mesh;
  lob_t key;
  chan_t chans;

  // transport plumbing
  void *send_arg;
  link_t (*send_cb)(link_t link, lob_t packet, void *arg);
  
  // these are for internal link management only
  link_t next;
  uint8_t csid;
};

// these all create or return existing one from the mesh
link_t link_get(mesh_t mesh, hashname_t id);
link_t link_get_keys(mesh_t mesh, lob_t keys); // adds in the right key
link_t link_get_key(mesh_t mesh, lob_t key, uint8_t csid); // adds in from the body

// simple accessors
hashname_t link_id(link_t link);
lob_t link_key(link_t link);

// get link info json
lob_t link_json(link_t link);

// removes from mesh
void link_free(link_t link);

// load in the key to existing link
link_t link_load(link_t link, uint8_t csid, lob_t key);

// add a delivery pipe to this link
link_t link_pipe(link_t link, link_t (*send)(link_t link, lob_t packet, void *arg), void *arg);

// process a decrypted channel packet
link_t link_receive(link_t link, lob_t inner);

// process an incoming handshake
link_t link_receive_handshake(link_t link, lob_t handshake);

// try to deliver this encrypted packet
link_t link_send(link_t link, lob_t outer);

// encrypt and send this packet
link_t link_direct(link_t link, lob_t inner);

// return current handshake (caller free's)
lob_t link_handshake(link_t link);

// send current handshake(s) 
link_t link_sync(link_t link);

// force generate new encrypted handshake(s) and sync
link_t link_resync(link_t link);

// is the other endpoint connected and the link available, NULL if not
link_t link_up(link_t link);

// force link down, ends channels and generates events
link_t link_down(link_t link);

// create/track a new channel for this open
chan_t link_chan(link_t link, lob_t open);

// process any channel timeouts based on the current/given time
link_t link_process(link_t link, uint32_t now);

#endif
#ifndef lob_h
#define lob_h

#include <stdint.h>
#include <stdlib.h>

typedef struct lob_struct
{
  // these are public but managed by accessors
  uint8_t *raw;
  uint8_t *body;
  size_t body_len;
  uint8_t *head;
  size_t head_len;
  
  // these are all for external use only
  uint32_t id;
  void *arg;

  // these are internal/private
  struct lob_struct *chain;
  char *cache; // edited copy of the json head

  // used only by the list utils
  struct lob_struct *next, *prev;

} *lob_t;

// these all allocate/free memory
lob_t lob_new();
lob_t lob_copy(lob_t p);
lob_t lob_free(lob_t p); // returns NULL for convenience

// creates a new parent packet chained to the given child one, so freeing the new packet also free's it
lob_t lob_chain(lob_t child);
// manually chain together two packets, returns parent, frees any existing child, creates parent if none
lob_t lob_link(lob_t parent, lob_t child);
// return a linked child if any
lob_t lob_linked(lob_t parent);
// returns child, unlinked
lob_t lob_unlink(lob_t parent);

// initialize head/body from raw, parses json
lob_t lob_parse(const uint8_t *raw, size_t len);

// return full encoded packet
uint8_t *lob_raw(lob_t p);
size_t lob_len(lob_t p);

// return null-terminated json header only
char *lob_json(lob_t p);

// creates temporarily cached string on lob (used by lob_json), free'd w/ lob or next cache call
char *lob_cache(lob_t p, size_t len);

// set/store these in the current packet
uint8_t *lob_head(lob_t p, uint8_t *head, size_t len);
uint8_t *lob_body(lob_t p, uint8_t *body, size_t len);
lob_t lob_append(lob_t p, uint8_t *chunk, size_t len);
lob_t lob_append_str(lob_t p, char *chunk);

// core accessors
size_t lob_head_len(lob_t p);
uint8_t *lob_head_get(lob_t p);
size_t lob_body_len(lob_t p);
uint8_t *lob_body_get(lob_t p);

// convenient json setters/getters, always return given lob so they're chainable
lob_t lob_set_raw(lob_t p, char *key, size_t klen, char *val, size_t vlen); // raw
lob_t lob_set(lob_t p, char *key, char *val); // escapes value
lob_t lob_set_len(lob_t p, char *key, size_t klen, char *val, size_t vlen); // same as lob_set
lob_t lob_set_int(lob_t p, char *key, int val);
lob_t lob_set_uint(lob_t p, char *key, unsigned int val);
lob_t lob_set_float(lob_t p, char *key, float val, uint8_t places);
lob_t lob_set_printf(lob_t p, char *key, const char *format, ...);
lob_t lob_set_base32(lob_t p, char *key, uint8_t *val, size_t vlen);

// copies keys from json into p
lob_t lob_set_json(lob_t p, lob_t json);

// count of keys
unsigned int lob_keys(lob_t p);

// alpha-sorts the json keys
lob_t lob_sort(lob_t p);

// 0 to match, !0 if different, compares only top-level json and body
int lob_cmp(lob_t a, lob_t b);

// the return uint8_t* is invalidated with any _set* operation!
char *lob_get(lob_t p, char *key);
int lob_get_int(lob_t p, char *key);
unsigned int lob_get_uint(lob_t p, char *key);
float lob_get_float(lob_t p, char *key);

char *lob_get_index(lob_t p, uint32_t i); // returns ["0","1","2","3"] or {"0":"1","2":"3"}

// just shorthand for util_cmp to match a key/value
int lob_get_cmp(lob_t p, char *key, char *val);

// get the raw value, must use get_len
char *lob_get_raw(lob_t p, char *key);
size_t lob_get_len(lob_t p, char *key);

// returns new packets based on values
lob_t lob_get_json(lob_t p, char *key); // creates new packet from key:object value
lob_t lob_get_array(lob_t p, char *key); // list of packet->next from key:[object,object]
lob_t lob_get_base32(lob_t p, char *key); // decoded binary is the return body

// TODO, this would be handy, js syntax to get a json value
// char *lob_eval(lob_t p, "foo.bar[0]['zzz']");

// manage a basic double-linked list of packets using ->next and ->prev
lob_t lob_pop(lob_t list); // returns last item, item->next is the new list
lob_t lob_push(lob_t list, lob_t append); // appends new item, returns new list
lob_t lob_shift(lob_t list); // returns first item, item->next is the new list
lob_t lob_unshift(lob_t list, lob_t prepend); // adds item, returns new list
lob_t lob_splice(lob_t list, lob_t extract); // removes item from list, returns new list
lob_t lob_insert(lob_t list, lob_t after, lob_t p); // inserts item in list after other item, returns new list
lob_t lob_freeall(lob_t list); // frees all
lob_t lob_match(lob_t list, char *key, char *value); // find the first packet in the list w/ the matching key/value
lob_t lob_next(lob_t list);
lob_t lob_array(lob_t list); // return json array of the list

#endif
#ifndef mesh_h
#define mesh_h

typedef struct mesh_struct *mesh_t;
typedef struct link_struct *link_t;
typedef struct chan_struct *chan_t;




struct mesh_struct
{
  hashname_t id;
  lob_t keys, paths;
  e3x_self_t self;
  void *on; // internal list of triggers
  // shared network info
  uint16_t port_local, port_public;
  char *ipv4_local, *ipv4_public;
  link_t links;
};

mesh_t mesh_new(void);
mesh_t mesh_free(mesh_t mesh);

// must be called to initialize to a hashname from keys/secrets, return !0 if failed
uint8_t mesh_load(mesh_t mesh, lob_t secrets, lob_t keys);

// creates and loads a new random hashname, returns secrets if it needs to be saved/reused
lob_t mesh_generate(mesh_t mesh);

// simple accessors
hashname_t mesh_id(mesh_t mesh);
lob_t mesh_keys(mesh_t mesh);

// generate json of mesh keys and current paths
lob_t mesh_json(mesh_t mesh);

// generate json for all links, returns lob list
lob_t mesh_links(mesh_t mesh);

// creates a link from the json format of {"hashname":"...","keys":{},"paths":[]}
link_t mesh_add(mesh_t mesh, lob_t json);

// return only if this hashname (full or short) is currently linked (in any state)
link_t mesh_linked(mesh_t mesh, char *hn, size_t len);
link_t mesh_linkid(mesh_t mesh, hashname_t id); // TODO, clean this up

// remove this link, will event it down and clean up during next process()
mesh_t mesh_unlink(link_t link);

// processes incoming packet, it will take ownership of packet, returns link delivered to if success
link_t mesh_receive(mesh_t mesh, lob_t packet);

// process any unencrypted handshake packet
link_t mesh_receive_handshake(mesh_t mesh, lob_t handshake);

// process any channel timeouts based on the current/given time
mesh_t mesh_process(mesh_t mesh, uint32_t now);

// callback when the mesh is free'd
void mesh_on_free(mesh_t mesh, char *id, void (*free)(mesh_t mesh));

// callback when a path needs to be turned into a pipe
void mesh_on_path(mesh_t mesh, char *id, link_t (*path)(link_t link, lob_t path));
link_t mesh_path(mesh_t mesh, link_t link, lob_t path);

// callback when an unknown hashname is discovered
void mesh_on_discover(mesh_t mesh, char *id, link_t (*discover)(mesh_t mesh, lob_t discovered));
void mesh_discover(mesh_t mesh, lob_t discovered);

// callback when a link changes state created/up/down
void mesh_on_link(mesh_t mesh, char *id, void (*link)(link_t link));
void mesh_link(mesh_t mesh, link_t link);

// callback when a new incoming channel is requested
void mesh_on_open(mesh_t mesh, char *id, lob_t (*open)(link_t link, lob_t open));
lob_t mesh_open(mesh_t mesh, link_t link, lob_t open);


#endif
// local wrappers/additions
#include <stdint.h>

// murmurhash3 32bit
uint32_t murmur4(const uint8_t *data, uint32_t len);

// hex must be 8+\0
char *murmur8(const uint8_t *data, uint32_t len, char *hex);

// more convenient, caller must ensure 4-byte sizing
uint8_t *murmur(const uint8_t *data, uint32_t len, uint8_t *hash);


/*-----------------------------------------------------------------------------
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain.
 *
 * This implementation was written by Shane Day, and is also public domain.
 *
 * This is a portable ANSI C implementation of MurmurHash3_x86_32 (Murmur3A)
 * with support for progressive processing.
 */


/* ------------------------------------------------------------------------- */
/* Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

void PMurHash32_Process(uint32_t *ph1, uint32_t *pcarry, const void *key, int len);
uint32_t PMurHash32_Result(uint32_t h1, uint32_t carry, uint32_t total_length);
uint32_t PMurHash32(uint32_t seed, const void *key, int len);

void PMurHash32_test(const void *key, int len, uint32_t seed, void *out);

#ifdef __cplusplus
}
#endif
#ifndef net_loopback_h
#define net_loopback_h


typedef struct net_loopback_struct
{
  mesh_t a, b;
} *net_loopback_t;

// connect two mesh instances with each other for packet delivery
net_loopback_t net_loopback_new(mesh_t a, mesh_t b);
void net_loopback_free(net_loopback_t pair);

#endif
#ifndef net_serial_h
#define net_serial_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>


// overall server
typedef struct net_serial_struct
{
  mesh_t mesh;
  xht_t pipes;
} *net_serial_t;

// create a new serial hub
net_serial_t net_serial_new(mesh_t mesh, lob_t options);
void net_serial_free(net_serial_t net);

// add a named serial pipe and I/O callbacks for it
net_serial_t net_serial_add(net_serial_t net, const char *name, int (*read)(void), int (*write)(uint8_t *buf, size_t len), uint8_t buffer);

// manually send a packet down a named pipe (discovery, etc)
net_serial_t net_serial_send(net_serial_t net, const char *name, lob_t packet);

// check all pipes for data
net_serial_t net_serial_loop(net_serial_t net);

#ifdef __cplusplus
}
#endif

#endif
#ifndef net_tcp4_h
#define net_tcp4_h

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>


// overall server
typedef struct net_tcp4_struct
{
  int server;
  int port;
  mesh_t mesh;
  xht_t pipes;
  lob_t path;
} *net_tcp4_t;

// create a new listening tcp server
net_tcp4_t net_tcp4_new(mesh_t mesh, lob_t options);
void net_tcp4_free(net_tcp4_t net);

// check all sockets for work (just for testing, use libuv or such for production instead)
net_tcp4_t net_tcp4_loop(net_tcp4_t net);

#endif

#endif
#ifndef net_udp4_h
#define net_udp4_h

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>


// overall server
typedef struct net_udp4_struct *net_udp4_t;

// create a new listening udp server
net_udp4_t net_udp4_new(mesh_t mesh, lob_t options);
net_udp4_t net_udp4_free(net_udp4_t net);

// send/receive any waiting frames, delivers packets into mesh
net_udp4_t net_udp4_process(net_udp4_t net);

// return server socket handle / port
int net_udp4_socket(net_udp4_t net);
uint16_t net_udp4_port(net_udp4_t net);

// send a packet directly
net_udp4_t net_udp4_direct(net_udp4_t net, lob_t packet, char *ip, uint16_t port);

#endif // POSIX

#endif // net_udp4_h
#ifndef SHA256_H
#define SHA256_H

#ifdef __cplusplus
extern "C" {
#endif

  // use old api
/**
 * \brief          Output = SHA-256( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-224/256 checksum result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256( const unsigned char *input, size_t ilen,
           unsigned char output[32], int is224 );

/**
 * \brief          Output = HMAC-SHA-256( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-224/256 result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void sha256_hmac( const unsigned char *key, size_t keylen,
                  const unsigned char *input, size_t ilen,
                  unsigned char output[32], int is224 );

void hmac_256(const unsigned char *key, size_t keylen,
                  const unsigned char *input, size_t ilen,
                  unsigned char output[32]);

#ifdef __cplusplus
}
#endif

#endif /* sha256.h */
#ifndef socketio_h
#define socketio_h

// utilities to parse and generate minimal socket.io / engine.io packets

#include <stdint.h>

// https://github.com/Automattic/engine.io-protocol
#define SOCKETIO_ETYPE_OPEN 0
#define SOCKETIO_ETYPE_CLOSE 1
#define SOCKETIO_ETYPE_PING 2
#define SOCKETIO_ETYPE_PONG 3
#define SOCKETIO_ETYPE_MESSAGE 4
#define SOCKETIO_ETYPE_UPGRADE 5
#define SOCKETIO_ETYPE_NOOP 6

// https://github.com/Automattic/socket.io-protocol
#define SOCKETIO_PTYPE_CONNECT 0
#define SOCKETIO_PTYPE_DISCONNECT 1
#define SOCKETIO_PTYPE_EVENT 2
#define SOCKETIO_PTYPE_ACK 3
#define SOCKETIO_PTYPE_ERROR 4
#define SOCKETIO_PTYPE_BINARY_EVENT 5
#define SOCKETIO_PTYPE_BINARY_ACK 6


lob_t socketio_decode(lob_t data);

lob_t socketio_encode(uint8_t etype, uint8_t ptype, lob_t payload);

#endif
#ifndef telehash_h
#define telehash_h

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
#ifndef tmesh_h
#define tmesh_h

#include <stdbool.h>

/*

tempo_t is a synchronized comm thread
every tempo has a medium id that identifies the transceiver details
all tempos belong to a mote->community for grouping
two types of tempos: signals and streams
motes have one constant signal tempo and on-demand stream tempos
signal tempo
  - use shared secrets across the community
  - can be lost signal, derived secret and no neighbors
  - signal tempo should use limited medium to ease syncing
  - include neighbor info and stream requests
  - either side initiates stream tempo
  - 6*10-block format, +4 mmh
    - neighbors (+rssi)
    - stream req (+repeat)
    - signals (+hops)
stream tempo
  - use one-time secrets
  - initiated from signals
  - frames only
  - flush meta space for app use
  - inside channel sent link signal sync

mediums
  - limit # of channels
  - set rx tolerance
  - indicate power/lna for rssi comparisons
  - tm->tempo(tempo), sets window min/max, can change secret

stream tempos are lost until first rx
lost streams are reset each signal
lost signal hash is 8+50+4 w/ hash of just 50, rx can detect difference
!signal->lost after first good rx
must be commanded to change tx signal->lost, scheduled along w/ medium change

stream hold/lost/idle states
  * hold means don't schedule
  * idle means don't signal, dont schedule TX but do schedule RX (if !hold)
  * lost means signal (if !idle), hold = request, !hold || mote->signal->lost = accept
    - TX/RX signal accept is stream resync event
  - idle is cleared on any new send data, set after any TX/RX if !ready && !await
  - mote_gone() called by the driver based on missed RX, sets lost if !idle, hold if idle

TODO, move flags to explicit vs implicit:
  * do_schedule and !do_schedule
    - skip TX if !ready && !awaiting
    - set true on any new data
  * do_signal and !do_signal
    - do_schedule = true if mote->signal->lost
    - request if !do_schedule, else accept
  * mote_gone() called by the driver based on missed RX
    - sets do_schedule = false
    - sets do_signal = true if ready||awaiting

beacon > lost
  * tm->beacon alongside tm->signal
  * beacon is a stream, always starts w/ meta frame
    * meta contains sender nickname to ignore once linked (just for now, future is random id)
  * when no motes, beacon uses fast medium, if any other mote it goes slow
  * signal is off or slow when beaconing
  * only beacon open rx seeks
  * when mote looses signal it drops entirely, beacon goes fast
    * if stream request is not accepted in X signals, also drop
  * streams just go idle (not lost), low priori rx skip tx
  * after beacon stream exchanges handshake
    * start our signal to include in stream
    * create mote
    * move beacon to mote as stream
    * reset beacon after link is up
    * create their signal once received in stream meta
  * when routed packet is requested on a stream
    * cache from who on outgoing stream
    * include orig sender's full blocks in stream meta
    * faster way to establish signal to neighbor than waiting for slow beacon

... more notes
  * beacon (signal) advertises its shared stream
  * shared stream uses beacon id in secret to be unique
  * all tempo keys roll community name + other stuff + password

SECURITY TODOs
  * generate a temp hashname on boot, use that to do all beacon and shared stream handling

NEXT
  * use ad-hoc beacons for faster signal stream request/accept

*/

typedef struct tmesh_struct *tmesh_t; // joined community motes/signals
typedef struct mote_struct *mote_t; // local link info, signal and list of stream tempos
typedef struct tempo_struct *tempo_t; // single tempo, is a signal or stream
typedef struct knock_struct *knock_t; // single txrx action

// overall tmesh manager
struct tmesh_struct
{
  mesh_t mesh;
  uint32_t at; // last known
  
  // community deets
  char *community;
  char *password; // optional
  mote_t motes;
  tempo_t signal; // outgoing signal, unique to us
  tempo_t stream; // have an always-running shared stream, keyed from beacon for handshakes, RX for alerts
  tempo_t beacon; // only one of these, advertises our shared stream
  uint32_t route; // available for app-level routing logic

  // driver interface
  tempo_t (*sort)(tmesh_t tm, tempo_t a, tempo_t b);
  tmesh_t (*schedule)(tmesh_t tm); // called whenever a new knock is ready to be scheduled
  tmesh_t (*advance)(tmesh_t tm, tempo_t tempo, uint8_t seed[8]); // advances tempo to next window
  tmesh_t (*medium)(tmesh_t tm, tempo_t tempo, uint8_t seed[8], uint32_t medium); // driver can initialize/update a tempo's medium
  tmesh_t (*accept)(tmesh_t tm, hashname_t id, uint32_t route); // driver handles new neighbors, returns tm to continue or NULL to ignore
  tmesh_t (*free)(tmesh_t tm, tempo_t tempo); // driver can free any associated tempo resources
  knock_t knock;
  
  uint8_t seen[5]; // recently seen short hn from a beacon
};

// join a new tmesh community, pass optional
tmesh_t tmesh_new(mesh_t mesh, char *name, char *pass);
tmesh_t tmesh_free(tmesh_t tm);

// process knock that has been completed by a driver
// returns a tempo when successful rx in case there's new packets available
tempo_t tmesh_knocked(tmesh_t tm);

// at based on current cycle count since start or last rebase
tmesh_t tmesh_schedule(tmesh_t tm, uint32_t at);

// call before a schedule to rebase (subtract) given cycles off all at's (to prevent overflow)
tmesh_t tmesh_rebase(tmesh_t tm, uint32_t at);

// returns mote for this link, creating one if a stream is provided
mote_t tmesh_mote(tmesh_t tm, link_t link);

// drops and free's this mote (link just goes to down state if no other paths)
tmesh_t tmesh_demote(tmesh_t tm, mote_t mote);

// returns mote for this id if one exists
mote_t tmesh_moted(tmesh_t tm, hashname_t id);

// update/signal our current route value
tmesh_t tmesh_route(tmesh_t tm, uint32_t route);

// tempo state
struct tempo_struct
{
  tmesh_t tm; // mostly convenience
  mote_t mote; // parent mote (except for our outgoing signal) 
  void *driver; // for driver use, set during tm->tempo()
  util_frames_t frames; // r/w frame buffers for streams
  uint32_t qos_remote, qos_local; // last qos from/about this tempo
  uint32_t medium; // id
  uint32_t at; // cycles until next knock in current window
  uint32_t seq; // window increment part of nonce
  uint16_t c_tx, c_rx; // current counts
  uint16_t c_bad; // dropped bad frames
  int16_t last, best, worst; // rssi
  uint8_t secret[32];
  uint8_t c_miss, c_skip, c_idle; // how many of the last rx windows were missed (expected), skipped (scheduling), or idle
  uint8_t c_wait; // how many signals we've waited for
  uint8_t chan; // channel of next knock
  uint8_t priority; // next knock priority
  // a byte of state flags for each tempo type
  union
  {
    struct
    {
      uint8_t is_signal:1;
      uint8_t unused1:1;
      uint8_t qos_ping:1;
      uint8_t qos_pong:1;
      uint8_t seen:1; // beacon only
      uint8_t adhoc:1;
    };
    struct
    {
      uint8_t unused2:1;
      uint8_t is_stream:1;
      uint8_t requesting:1;
      uint8_t accepting:1;
      uint8_t direction:1; // 1==TX, 0==RX
    };
  } state;
};

// a single convenient knock request ready to go
struct knock_struct
{
  tempo_t tempo;
  uint32_t adhoc; // request driver to do an adhoc tx(immediate) or rx(seek until)
  uint32_t started, stopped; // actual times
  int16_t rssi, snr; // set by driver only after rx
  uint8_t frame[64];
  uint8_t nonce[8]; // convenience
  // boolean flags for state tracking, etc
  uint8_t is_active:1; // is actively transceiving
  uint8_t is_tx:1; // current window direction (copied from tempo for convenience)
  uint8_t do_err:1; // driver sets if failed
  uint8_t do_gone:1; // driver sets to fail the signal/stream
};

// mote state tracking
struct mote_struct
{
  mote_t next; // for lists
  mote_t via; // router mote
  tmesh_t tm;
  link_t link;
  tempo_t signal; // tracks their signal
  tempo_t stream; // is a private stream, optionally can track their shared stream (TODO)
  uint32_t route; // most recent route block from them
};

// return current mote appid
uint32_t mote_appid(mote_t mote);

// find a stream to send it to for this mote
mote_t mote_send(mote_t mote, lob_t packet);

// send this packet to this id via this router
mote_t mote_route(mote_t router, hashname_t to, lob_t packet);


#endif
/* Copyright 2014, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_H_
#define _UECC_H_

#include <stdint.h>

/* Platform selection options.
If uECC_PLATFORM is not defined, the code will try to guess it based on compiler macros.
Possible values for uECC_PLATFORM are defined below: */
#define uECC_arch_other 0
#define uECC_x86        1
#define uECC_x86_64     2
#define uECC_arm        3
#define uECC_arm_thumb  4
#define uECC_arm_thumb2 5
#define uECC_arm64      6
#define uECC_avr        7

/* If desired, you can define uECC_WORD_SIZE as appropriate for your platform (1, 4, or 8 bytes).
If uECC_WORD_SIZE is not explicitly defined then it will be automatically set based on your
platform. */

/* Optimization level; trade speed for code size.
   Larger values produce code that is faster but larger.
   Currently supported values are 0 - 3; 0 is unusably slow for most applications. */
#ifndef uECC_OPTIMIZATION_LEVEL
    #define uECC_OPTIMIZATION_LEVEL 2
#endif

/* uECC_SQUARE_FUNC - If enabled (defined as nonzero), this will cause a specific function to be
used for (scalar) squaring instead of the generic multiplication function. This can make things
faster somewhat faster, but increases the code size. */
#ifndef uECC_SQUARE_FUNC
    #define uECC_SQUARE_FUNC 0
#endif

/* uECC_VLI_NATIVE_LITTLE_ENDIAN - If enabled (defined as nonzero), this will switch to native
little-endian format for *all* arrays passed in and out of the public API. This includes public 
and private keys, shared secrets, signatures and message hashes. 
Using this switch reduces the amount of call stack memory used by uECC, since less intermediate
translations are required. 
Note that this will *only* work on native little-endian processors and it will treat the uint8_t
arrays passed into the public API as word arrays, therefore requiring the provided byte arrays 
to be word aligned on architectures that do not support unaligned accesses. */
#ifndef uECC_VLI_NATIVE_LITTLE_ENDIAN
    #define uECC_VLI_NATIVE_LITTLE_ENDIAN 0
#endif

/* Curve support selection. Set to 0 to remove that curve. */
#ifndef uECC_SUPPORTS_secp160r1
    #define uECC_SUPPORTS_secp160r1 1
#endif
#ifndef uECC_SUPPORTS_secp192r1
    #define uECC_SUPPORTS_secp192r1 0
#endif
#ifndef uECC_SUPPORTS_secp224r1
    #define uECC_SUPPORTS_secp224r1 0
#endif
#ifndef uECC_SUPPORTS_secp256r1
    #define uECC_SUPPORTS_secp256r1 1
#endif
#ifndef uECC_SUPPORTS_secp256k1
    #define uECC_SUPPORTS_secp256k1 1
#endif

/* Specifies whether compressed point format is supported.
   Set to 0 to disable point compression/decompression functions. */
#ifndef uECC_SUPPORT_COMPRESSED_POINT
    #define uECC_SUPPORT_COMPRESSED_POINT 1
#endif

struct uECC_Curve_t;
typedef const struct uECC_Curve_t * uECC_Curve;

#ifdef __cplusplus
extern "C"
{
#endif

#if uECC_SUPPORTS_secp160r1
uECC_Curve uECC_secp160r1(void);
#endif
#if uECC_SUPPORTS_secp192r1
uECC_Curve uECC_secp192r1(void);
#endif
#if uECC_SUPPORTS_secp224r1
uECC_Curve uECC_secp224r1(void);
#endif
#if uECC_SUPPORTS_secp256r1
uECC_Curve uECC_secp256r1(void);
#endif
#if uECC_SUPPORTS_secp256k1
uECC_Curve uECC_secp256k1(void);
#endif

/* uECC_RNG_Function type
The RNG function should fill 'size' random bytes into 'dest'. It should return 1 if
'dest' was filled with random data, or 0 if the random data could not be generated.
The filled-in values should be either truly random, or from a cryptographically-secure PRNG.

A correctly functioning RNG function must be set (using uECC_set_rng()) before calling
uECC_make_key() or uECC_sign().

Setting a correctly functioning RNG function improves the resistance to side-channel attacks
for uECC_shared_secret() and uECC_sign_deterministic().

A correct RNG function is set by default when building for Windows, Linux, or OS X.
If you are building on another POSIX-compliant system that supports /dev/random or /dev/urandom,
you can define uECC_POSIX to use the predefined RNG. For embedded platforms there is no predefined
RNG function; you must provide your own.
*/
typedef int (*uECC_RNG_Function)(uint8_t *dest, unsigned size);

/* uECC_set_rng() function.
Set the function that will be used to generate random bytes. The RNG function should
return 1 if the random data was generated, or 0 if the random data could not be generated.

On platforms where there is no predefined RNG function (eg embedded platforms), this must
be called before uECC_make_key() or uECC_sign() are used.

Inputs:
    rng_function - The function that will be used to generate random bytes.
*/
void uECC_set_rng(uECC_RNG_Function rng_function);

/* uECC_get_rng() function.

Returns the function that will be used to generate random bytes.
*/
uECC_RNG_Function uECC_get_rng(void);

/* uECC_curve_private_key_size() function.

Returns the size of a private key for the curve in bytes.
*/
int uECC_curve_private_key_size(uECC_Curve curve);

/* uECC_curve_public_key_size() function.

Returns the size of a public key for the curve in bytes.
*/
int uECC_curve_public_key_size(uECC_Curve curve);

/* uECC_make_key() function.
Create a public/private key pair.

Outputs:
    public_key  - Will be filled in with the public key. Must be at least 2 * the curve size
                  (in bytes) long. For example, if the curve is secp256r1, public_key must be 64
                  bytes long.
    private_key - Will be filled in with the private key. Must be as long as the curve order; this
                  is typically the same as the curve size, except for secp160r1. For example, if the
                  curve is secp256r1, private_key must be 32 bytes long.

                  For secp160r1, private_key must be 21 bytes long! Note that the first byte will
                  almost always be 0 (there is about a 1 in 2^80 chance of it being non-zero).

Returns 1 if the key pair was generated successfully, 0 if an error occurred.
*/
int uECC_make_key(uint8_t *public_key, uint8_t *private_key, uECC_Curve curve);

/* uECC_shared_secret() function.
Compute a shared secret given your secret key and someone else's public key.
Note: It is recommended that you hash the result of uECC_shared_secret() before using it for
symmetric encryption or HMAC.

Inputs:
    public_key  - The public key of the remote party.
    private_key - Your private key.

Outputs:
    secret - Will be filled in with the shared secret value. Must be the same size as the
             curve size; for example, if the curve is secp256r1, secret must be 32 bytes long.

Returns 1 if the shared secret was generated successfully, 0 if an error occurred.
*/
int uECC_shared_secret(const uint8_t *public_key,
                       const uint8_t *private_key,
                       uint8_t *secret,
                       uECC_Curve curve);

#if uECC_SUPPORT_COMPRESSED_POINT
/* uECC_compress() function.
Compress a public key.

Inputs:
    public_key - The public key to compress.

Outputs:
    compressed - Will be filled in with the compressed public key. Must be at least
                 (curve size + 1) bytes long; for example, if the curve is secp256r1,
                 compressed must be 33 bytes long.
*/
void uECC_compress(const uint8_t *public_key, uint8_t *compressed, uECC_Curve curve);

/* uECC_decompress() function.
Decompress a compressed public key.

Inputs:
    compressed - The compressed public key.

Outputs:
    public_key - Will be filled in with the decompressed public key.
*/
void uECC_decompress(const uint8_t *compressed, uint8_t *public_key, uECC_Curve curve);
#endif /* uECC_SUPPORT_COMPRESSED_POINT */

/* uECC_valid_public_key() function.
Check to see if a public key is valid.

Note that you are not required to check for a valid public key before using any other uECC
functions. However, you may wish to avoid spending CPU time computing a shared secret or
verifying a signature using an invalid public key.

Inputs:
    public_key - The public key to check.

Returns 1 if the public key is valid, 0 if it is invalid.
*/
int uECC_valid_public_key(const uint8_t *public_key, uECC_Curve curve);

/* uECC_compute_public_key() function.
Compute the corresponding public key for a private key.

Inputs:
    private_key - The private key to compute the public key for

Outputs:
    public_key - Will be filled in with the corresponding public key

Returns 1 if the key was computed successfully, 0 if an error occurred.
*/
int uECC_compute_public_key(const uint8_t *private_key, uint8_t *public_key, uECC_Curve curve);

/* uECC_sign() function.
Generate an ECDSA signature for a given hash value.

Usage: Compute a hash of the data you wish to sign (SHA-2 is recommended) and pass it in to
this function along with your private key.

Inputs:
    private_key  - Your private key.
    message_hash - The hash of the message to sign.
    hash_size    - The size of message_hash in bytes.

Outputs:
    signature - Will be filled in with the signature value. Must be at least 2 * curve size long.
                For example, if the curve is secp256r1, signature must be 64 bytes long.

Returns 1 if the signature generated successfully, 0 if an error occurred.
*/
int uECC_sign(const uint8_t *private_key,
              const uint8_t *message_hash,
              unsigned hash_size,
              uint8_t *signature,
              uECC_Curve curve);

/* uECC_HashContext structure.
This is used to pass in an arbitrary hash function to uECC_sign_deterministic().
The structure will be used for multiple hash computations; each time a new hash
is computed, init_hash() will be called, followed by one or more calls to
update_hash(), and finally a call to finish_hash() to produce the resulting hash.

The intention is that you will create a structure that includes uECC_HashContext
followed by any hash-specific data. For example:

typedef struct SHA256_HashContext {
    uECC_HashContext uECC;
    SHA256_CTX ctx;
} SHA256_HashContext;

void init_SHA256(uECC_HashContext *base) {
    SHA256_HashContext *context = (SHA256_HashContext *)base;
    SHA256_Init(&context->ctx);
}

void update_SHA256(uECC_HashContext *base,
                   const uint8_t *message,
                   unsigned message_size) {
    SHA256_HashContext *context = (SHA256_HashContext *)base;
    SHA256_Update(&context->ctx, message, message_size);
}

void finish_SHA256(uECC_HashContext *base, uint8_t *hash_result) {
    SHA256_HashContext *context = (SHA256_HashContext *)base;
    SHA256_Final(hash_result, &context->ctx);
}

... when signing ...
{
    uint8_t tmp[32 + 32 + 64];
    SHA256_HashContext ctx = {{&init_SHA256, &update_SHA256, &finish_SHA256, 64, 32, tmp}};
    uECC_sign_deterministic(key, message_hash, &ctx.uECC, signature);
}
*/
typedef struct uECC_HashContext {
    void (*init_hash)(const struct uECC_HashContext *context);
    void (*update_hash)(const struct uECC_HashContext *context,
                        const uint8_t *message,
                        unsigned message_size);
    void (*finish_hash)(const struct uECC_HashContext *context, uint8_t *hash_result);
    unsigned block_size; /* Hash function block size in bytes, eg 64 for SHA-256. */
    unsigned result_size; /* Hash function result size in bytes, eg 32 for SHA-256. */
    uint8_t *tmp; /* Must point to a buffer of at least (2 * result_size + block_size) bytes. */
} uECC_HashContext;

/* uECC_sign_deterministic() function.
Generate an ECDSA signature for a given hash value, using a deterministic algorithm
(see RFC 6979). You do not need to set the RNG using uECC_set_rng() before calling
this function; however, if the RNG is defined it will improve resistance to side-channel
attacks.

Usage: Compute a hash of the data you wish to sign (SHA-2 is recommended) and pass it to
this function along with your private key and a hash context. Note that the message_hash
does not need to be computed with the same hash function used by hash_context.

Inputs:
    private_key  - Your private key.
    message_hash - The hash of the message to sign.
    hash_size    - The size of message_hash in bytes.
    hash_context - A hash context to use.

Outputs:
    signature - Will be filled in with the signature value.

Returns 1 if the signature generated successfully, 0 if an error occurred.
*/
int uECC_sign_deterministic(const uint8_t *private_key,
                            const uint8_t *message_hash,
                            unsigned hash_size,
                            const uECC_HashContext *hash_context,
                            uint8_t *signature,
                            uECC_Curve curve);

/* uECC_verify() function.
Verify an ECDSA signature.

Usage: Compute the hash of the signed data using the same hash as the signer and
pass it to this function along with the signer's public key and the signature values (r and s).

Inputs:
    public_key   - The signer's public key.
    message_hash - The hash of the signed data.
    hash_size    - The size of message_hash in bytes.
    signature    - The signature value.

Returns 1 if the signature is valid, 0 if it is invalid.
*/
int uECC_verify(const uint8_t *public_key,
                const uint8_t *message_hash,
                unsigned hash_size,
                const uint8_t *signature,
                uECC_Curve curve);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _UECC_H_ */
/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_TYPES_H_
#define _UECC_TYPES_H_

#ifndef uECC_PLATFORM
    #if __AVR__
        #define uECC_PLATFORM uECC_avr
    #elif defined(__thumb2__) || defined(_M_ARMT) /* I think MSVC only supports Thumb-2 targets */
        #define uECC_PLATFORM uECC_arm_thumb2
    #elif defined(__thumb__)
        #define uECC_PLATFORM uECC_arm_thumb
    #elif defined(__arm__) || defined(_M_ARM)
        #define uECC_PLATFORM uECC_arm
    #elif defined(__aarch64__)
        #define uECC_PLATFORM uECC_arm64
    #elif defined(__i386__) || defined(_M_IX86) || defined(_X86_) || defined(__I86__)
        #define uECC_PLATFORM uECC_x86
    #elif defined(__amd64__) || defined(_M_X64)
        #define uECC_PLATFORM uECC_x86_64
    #else
        #define uECC_PLATFORM uECC_arch_other
    #endif
#endif

#ifndef uECC_WORD_SIZE
    #if uECC_PLATFORM == uECC_avr
        #define uECC_WORD_SIZE 1
    #elif (uECC_PLATFORM == uECC_x86_64 || uECC_PLATFORM == uECC_arm64)
        #define uECC_WORD_SIZE 8
    #else
        #define uECC_WORD_SIZE 4
    #endif
#endif

#if (uECC_WORD_SIZE != 1) && (uECC_WORD_SIZE != 4) && (uECC_WORD_SIZE != 8)
    #error "Unsupported value for uECC_WORD_SIZE"
#endif

#if ((uECC_PLATFORM == uECC_avr) && (uECC_WORD_SIZE != 1))
    #pragma message ("uECC_WORD_SIZE must be 1 for AVR")
    #undef uECC_WORD_SIZE
    #define uECC_WORD_SIZE 1
#endif

#if ((uECC_PLATFORM == uECC_arm || uECC_PLATFORM == uECC_arm_thumb || \
        uECC_PLATFORM ==  uECC_arm_thumb2) && \
     (uECC_WORD_SIZE != 4))
    #pragma message ("uECC_WORD_SIZE must be 4 for ARM")
    #undef uECC_WORD_SIZE
    #define uECC_WORD_SIZE 4
#endif

#if defined(__SIZEOF_INT128__) || ((__clang_major__ * 100 + __clang_minor__) >= 302)
    #define SUPPORTS_INT128 1
#else
    #define SUPPORTS_INT128 0
#endif

typedef int8_t wordcount_t;
typedef int16_t bitcount_t;
typedef int8_t cmpresult_t;

#if (uECC_WORD_SIZE == 1)

typedef uint8_t uECC_word_t;
typedef uint16_t uECC_dword_t;

#define HIGH_BIT_SET 0x80
#define uECC_WORD_BITS 8
#define uECC_WORD_BITS_SHIFT 3
#define uECC_WORD_BITS_MASK 0x07

#elif (uECC_WORD_SIZE == 4)

typedef uint32_t uECC_word_t;
typedef uint64_t uECC_dword_t;

#define HIGH_BIT_SET 0x80000000
#define uECC_WORD_BITS 32
#define uECC_WORD_BITS_SHIFT 5
#define uECC_WORD_BITS_MASK 0x01F

#elif (uECC_WORD_SIZE == 8)

typedef uint64_t uECC_word_t;
#if SUPPORTS_INT128
typedef unsigned __int128 uECC_dword_t;
#endif

#define HIGH_BIT_SET 0x8000000000000000ull
#define uECC_WORD_BITS 64
#define uECC_WORD_BITS_SHIFT 6
#define uECC_WORD_BITS_MASK 0x03F

#endif /* uECC_WORD_SIZE */

#endif /* _UECC_TYPES_H_ */
/* Copyright 2015, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _UECC_VLI_H_
#define _UECC_VLI_H_


/* Functions for raw large-integer manipulation. These are only available
   if uECC.c is compiled with uECC_ENABLE_VLI_API defined to 1. */
#ifndef uECC_ENABLE_VLI_API
    #define uECC_ENABLE_VLI_API 0
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if uECC_ENABLE_VLI_API

void uECC_vli_clear(uECC_word_t *vli, wordcount_t num_words);

/* Constant-time comparison to zero - secure way to compare long integers */
/* Returns 1 if vli == 0, 0 otherwise. */
uECC_word_t uECC_vli_isZero(const uECC_word_t *vli, wordcount_t num_words);

/* Returns nonzero if bit 'bit' of vli is set. */
uECC_word_t uECC_vli_testBit(const uECC_word_t *vli, bitcount_t bit);

/* Counts the number of bits required to represent vli. */
bitcount_t uECC_vli_numBits(const uECC_word_t *vli, const wordcount_t max_words);

/* Sets dest = src. */
void uECC_vli_set(uECC_word_t *dest, const uECC_word_t *src, wordcount_t num_words);

/* Constant-time comparison function - secure way to compare long integers */
/* Returns one if left == right, zero otherwise */
uECC_word_t uECC_vli_equal(const uECC_word_t *left,
                           const uECC_word_t *right,
                           wordcount_t num_words);

/* Constant-time comparison function - secure way to compare long integers */
/* Returns sign of left - right, in constant time. */
cmpresult_t uECC_vli_cmp(const uECC_word_t *left, const uECC_word_t *right, wordcount_t num_words);

/* Computes vli = vli >> 1. */
void uECC_vli_rshift1(uECC_word_t *vli, wordcount_t num_words);

/* Computes result = left + right, returning carry. Can modify in place. */
uECC_word_t uECC_vli_add(uECC_word_t *result,
                         const uECC_word_t *left,
                         const uECC_word_t *right,
                         wordcount_t num_words);

/* Computes result = left - right, returning borrow. Can modify in place. */
uECC_word_t uECC_vli_sub(uECC_word_t *result,
                         const uECC_word_t *left,
                         const uECC_word_t *right,
                         wordcount_t num_words);

/* Computes result = left * right. Result must be 2 * num_words long. */
void uECC_vli_mult(uECC_word_t *result,
                   const uECC_word_t *left,
                   const uECC_word_t *right,
                   wordcount_t num_words);

/* Computes result = left^2. Result must be 2 * num_words long. */
void uECC_vli_square(uECC_word_t *result, const uECC_word_t *left, wordcount_t num_words);

/* Computes result = (left + right) % mod.
   Assumes that left < mod and right < mod, and that result does not overlap mod. */
void uECC_vli_modAdd(uECC_word_t *result,
                     const uECC_word_t *left,
                     const uECC_word_t *right,
                     const uECC_word_t *mod,
                     wordcount_t num_words);

/* Computes result = (left - right) % mod.
   Assumes that left < mod and right < mod, and that result does not overlap mod. */
void uECC_vli_modSub(uECC_word_t *result,
                     const uECC_word_t *left,
                     const uECC_word_t *right,
                     const uECC_word_t *mod,
                     wordcount_t num_words);

/* Computes result = product % mod, where product is 2N words long.
   Currently only designed to work for mod == curve->p or curve_n. */
void uECC_vli_mmod(uECC_word_t *result,
                   uECC_word_t *product,
                   const uECC_word_t *mod,
                   wordcount_t num_words);

/* Calculates result = product (mod curve->p), where product is up to
   2 * curve->num_words long. */
void uECC_vli_mmod_fast(uECC_word_t *result, uECC_word_t *product, uECC_Curve curve);

/* Computes result = (left * right) % mod.
   Currently only designed to work for mod == curve->p or curve_n. */
void uECC_vli_modMult(uECC_word_t *result,
                      const uECC_word_t *left,
                      const uECC_word_t *right,
                      const uECC_word_t *mod,
                      wordcount_t num_words);

/* Computes result = (left * right) % curve->p. */
void uECC_vli_modMult_fast(uECC_word_t *result,
                           const uECC_word_t *left,
                           const uECC_word_t *right,
                           uECC_Curve curve);

/* Computes result = left^2 % mod.
   Currently only designed to work for mod == curve->p or curve_n. */
void uECC_vli_modSquare(uECC_word_t *result,
                        const uECC_word_t *left,
                        const uECC_word_t *mod,
                        wordcount_t num_words);

/* Computes result = left^2 % curve->p. */
void uECC_vli_modSquare_fast(uECC_word_t *result, const uECC_word_t *left, uECC_Curve curve);

/* Computes result = (1 / input) % mod.*/
void uECC_vli_modInv(uECC_word_t *result,
                     const uECC_word_t *input,
                     const uECC_word_t *mod,
                     wordcount_t num_words);

#if uECC_SUPPORT_COMPRESSED_POINT
/* Calculates a = sqrt(a) (mod curve->p) */
void uECC_vli_mod_sqrt(uECC_word_t *a, uECC_Curve curve);
#endif

/* Converts an integer in uECC native format to big-endian bytes. */
void uECC_vli_nativeToBytes(uint8_t *bytes, int num_bytes, const uECC_word_t *native);
/* Converts big-endian bytes to an integer in uECC native format. */
void uECC_vli_bytesToNative(uECC_word_t *native, const uint8_t *bytes, int num_bytes);

unsigned uECC_curve_num_words(uECC_Curve curve);
unsigned uECC_curve_num_bytes(uECC_Curve curve);
unsigned uECC_curve_num_bits(uECC_Curve curve);
unsigned uECC_curve_num_n_words(uECC_Curve curve);
unsigned uECC_curve_num_n_bytes(uECC_Curve curve);
unsigned uECC_curve_num_n_bits(uECC_Curve curve);

const uECC_word_t *uECC_curve_p(uECC_Curve curve);
const uECC_word_t *uECC_curve_n(uECC_Curve curve);
const uECC_word_t *uECC_curve_G(uECC_Curve curve);
const uECC_word_t *uECC_curve_b(uECC_Curve curve);

int uECC_valid_point(const uECC_word_t *point, uECC_Curve curve);

/* Multiplies a point by a scalar. Points are represented by the X coordinate followed by
   the Y coordinate in the same array, both coordinates are curve->num_words long. Note
   that scalar must be curve->num_n_words long (NOT curve->num_words). */
void uECC_point_mult(uECC_word_t *result,
                     const uECC_word_t *point,
                     const uECC_word_t *scalar,
                     uECC_Curve curve);

/* Generates a random integer in the range 0 < random < top.
   Both random and top have num_words words. */
int uECC_generate_random_int(uECC_word_t *random,
                             const uECC_word_t *top,
                             wordcount_t num_words);

#endif /* uECC_ENABLE_VLI_API */

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _UECC_VLI_H_ */
#ifndef util_h
#define util_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


// make sure out is 2*len + 1
char *util_hex(uint8_t *in, size_t len, char *out);
// out must be len/2
uint8_t *util_unhex(char *in, size_t len, uint8_t *out);
// hex string validator, NULL is invalid, else returns str
char *util_ishex(char *str, uint32_t len);

// safer string comparison (0 == same)
int util_cmp(char *a, char *b);

// portable sort
void util_sort(void *base, unsigned int nel, unsigned int width, int (*comp)(void *, const void *, const void *), void *arg);

// portable reallocf
void *util_reallocf(void *ptr, size_t size);

// get a "now" timestamp to do millisecond timers
uint64_t util_at(void); // only pass at into _since()
uint32_t util_since(uint64_t at); // get ms since the at

// Use a constant time comparison function to avoid timing attacks
int util_ct_memcmp(const void* s1, const void* s2, size_t n);

// embedded may not have strdup but it's a kinda handy shortcut
char *util_strdup(const char *str);
#ifndef strdup
#define strdup util_strdup
#endif

#endif
#ifndef util_chunks_h
#define util_chunks_h

#include <stdint.h>

// for list of incoming chunks
typedef struct util_chunk_struct
{
  struct util_chunk_struct *prev;
  uint8_t size;
  uint8_t data[];
} *util_chunk_t;

typedef struct util_chunks_struct
{

  util_chunk_t reading; // stacked linked list of incoming chunks

  lob_t writing;
  size_t writeat; // offset into lob_raw()
  uint16_t waitat; // gets to 256, offset into current chunk
  uint8_t waiting; // current writing chunk size;
  uint8_t readat; // always less than a max chunk, offset into reading

  uint8_t cap;
  uint8_t blocked:1, blocking:1, ack:1, err:1; // bool flags
} *util_chunks_t;


// size of each chunk, 0 == MAX (256)
util_chunks_t util_chunks_new(uint8_t size);

util_chunks_t util_chunks_free(util_chunks_t chunks);

// turn this packet into chunks and append, free's out
util_chunks_t util_chunks_send(util_chunks_t chunks, lob_t out);

// get any packets that have been reassembled from incoming chunks
lob_t util_chunks_receive(util_chunks_t chunks);

// bytes waiting to be sent
uint32_t util_chunks_writing(util_chunks_t chunks);


////// these are for a stream-based transport

// how many bytes are there ready to write
uint32_t util_chunks_len(util_chunks_t chunks);

// return the next block of data to be written to the stream transport, max len is util_chunks_len()
uint8_t *util_chunks_write(util_chunks_t chunks);

// advance the write this far, don't mix with util_chunks_out() usage
util_chunks_t util_chunks_written(util_chunks_t chunks, size_t len);

// queues incoming stream based data
util_chunks_t util_chunks_read(util_chunks_t chunks, uint8_t *block, size_t len);

////// these are for frame-based transport

// size of the next chunk, -1 when none, max is chunks size-1
int16_t util_chunks_size(util_chunks_t chunks);

// return the next chunk of data, use util_chunks_next to advance
uint8_t *util_chunks_frame(util_chunks_t chunks);

// peek into what the next chunk size will be, to see terminator ones
int16_t util_chunks_peek(util_chunks_t chunks);

// process incoming chunk
util_chunks_t util_chunks_chunk(util_chunks_t chunks, uint8_t *chunk, int16_t size);

// advance the write past the current chunk
util_chunks_t util_chunks_next(util_chunks_t chunks);


#endif
#ifndef util_frames_h
#define util_frames_h

#include <stdint.h>

// for list of incoming frames
typedef struct util_frame_struct
{
  struct util_frame_struct *prev;
  uint32_t hash;
  uint8_t data[];
} *util_frame_t;

typedef struct util_frames_struct
{

  lob_t inbox; // received packets waiting to be processed
  lob_t outbox; // current packet being sent out

  util_frame_t cache; // stacked linked list of incoming frames in progress

  uint32_t inbase; // last confirmed inbox hash
  uint32_t outbase; // last confirmed outbox hash

  uint8_t in; // number of incoming frames received/waiting
  uint8_t out; //  number of outgoing frames of outbox sent since outbase

  uint8_t size; // frame size
  uint8_t flush:1; // bool to signal a flush is needed
  uint8_t err:1; // unrecoverable failure
  uint8_t more:1; // last incoming meta said there was more

} *util_frames_t;


// size of each frame, min 16 max 128, multiple of 4
util_frames_t util_frames_new(uint8_t size);

util_frames_t util_frames_free(util_frames_t frames);

// turn this packet into frames and append, free's out
util_frames_t util_frames_send(util_frames_t frames, lob_t out);

// get any packets that have been reassembled from incoming frames
lob_t util_frames_receive(util_frames_t frames);

// total bytes in the inbox/outbox
size_t util_frames_inlen(util_frames_t frames);
size_t util_frames_outlen(util_frames_t frames);

// meta space is (size - 14) and only filled when receiving a meta frame
util_frames_t util_frames_inbox(util_frames_t frames, uint8_t *data, uint8_t *meta); // data=NULL is ready check

// fills data with the next frame, if no payload available always fills w/ meta frame, safe to re-run (idempotent)
util_frames_t util_frames_outbox(util_frames_t frames, uint8_t *data, uint8_t *meta); // data=NULL is ready-check

// this must be called immediately (no inbox interleaved) after last outbox is actually sent to advance payload or clear flush request, returns if more to send
util_frames_t util_frames_sent(util_frames_t frames);

// is just a check to see if there's data waiting to be sent
util_frames_t util_frames_waiting(util_frames_t frames);

// is there an expectation of an incoming frame
util_frames_t util_frames_await(util_frames_t frames);

// are we waiting to send/receive a frame (both waiting && await)
util_frames_t util_frames_busy(util_frames_t frames);

// is a frame pending to be sent immediately
util_frames_t util_frames_pending(util_frames_t frames);

// check error state and clearing it
util_frames_t util_frames_ok(util_frames_t frames);
util_frames_t util_frames_clear(util_frames_t frames);

#endif
#ifndef util_sys_h
#define util_sys_h

typedef uint32_t at_t;

// returns a number that increments in seconds for comparison (epoch or just since boot)
at_t util_sys_seconds();

// number of milliseconds since given epoch seconds value
unsigned long long util_sys_ms(long epoch);

unsigned short util_sys_short(unsigned short x);
unsigned long util_sys_long(unsigned long x);

// use the platform's best RNG
void util_sys_random_init(void);
long util_sys_random(void);

// -1 toggles debug, 0 disable, 1 enable
void util_sys_logging(int enabled);

// returns NULL for convenient return logging
void *util_sys_log(uint8_t level, const char *file, int line, const char *function, const char * format, ...);

// use syslog levels https://en.wikipedia.org/wiki/Syslog#Severity_level
#define LOG_LEVEL(level, fmt, ...) util_sys_log(level, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)

// default LOG is DEBUG level and compile-time optional
#ifdef NOLOG
#define LOG(...) NULL
#else
#define LOG(fmt, ...) util_sys_log(7, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#endif

// most things just need these
#define LOG_DEBUG LOG
#define LOG_INFO(fmt, ...) util_sys_log(6, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define LOG_WARN(fmt, ...) util_sys_log(4, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...) util_sys_log(3, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#define LOG_CRAZY(fmt, ...) util_sys_log(8, __FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)

#endif
#ifndef util_unix_h
#define util_unix_h

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))


// load a json file into packet
lob_t util_fjson(char *file);

// load an array of hashnames from a file and add them as links
mesh_t util_links(mesh_t mesh, char *file);

// simple sockets simpler
int util_sock_timeout(int sock, uint32_t ms); // blocking timeout

#endif

#endif
#ifndef util_uri_h
#define util_uri_h

#include <stdint.h>

// utils to get link info in/out of a uri

// parser
lob_t util_uri_parse(char *string);

// get keys from query
lob_t util_uri_keys(lob_t uri);

// get paths from host and query
lob_t util_uri_paths(lob_t uri); // chain

// validate any fragment from this peer
uint8_t util_uri_check(lob_t uri, uint8_t *peer);

// generators
lob_t util_uri_add_keys(lob_t uri, lob_t keys);
lob_t util_uri_add_path(lob_t uri, lob_t path);
lob_t util_uri_add_check(lob_t uri, uint8_t *peer, uint8_t *data, size_t len);
lob_t util_uri_add_data(lob_t uri, uint8_t *data, size_t len);

// serialize out from lob format to "uri" key and return it
char *util_uri_format(lob_t uri);

#endif
#ifndef version_h
#define version_h

#define TELEHASH_VERSION_MAJOR 3
#define TELEHASH_VERSION_MINOR 2
#define TELEHASH_VERSION_PATCH 0
#define TELEHASH_VERSION ((TELEHASH_VERSION_MAJOR) * 10000 + (TELEHASH_VERSION_MINOR) * 100 + (TELEHASH_VERSION_PATCH))

#endif
#include <stddef.h>

#ifndef xht_h
#define xht_h

// simple string->void* hashtable, very static and bare minimal, but efficient

typedef struct xht_struct *xht_t;

// must pass a prime#
xht_t xht_new(unsigned int prime);

// caller responsible for key storage, no copies made (don't free it b4 xht_free()!)
// set val to NULL to clear an entry, memory is reused but never free'd (# of keys only grows to peak usage)
void xht_set(xht_t h, const char *key, void *val);

// ooh! unlike set where key/val is in caller's mem, here they are copied into xht_t and free'd when val is 0 or xht_free()
void xht_store(xht_t h, const char *key, void *val, size_t vlen);

// returns value of val if found, or NULL
void *xht_get(xht_t h, const char *key);

// free the hashtable and all entries
void xht_free(xht_t h);

// pass a function that is called for every key that has a value set
typedef void (*xht_walker)(xht_t h, const char *key, void *val, void *arg);
void xht_walk(xht_t h, xht_walker w, void *arg);

// iterator through all the keys (NULL to start), use get for values
char *xht_iter(xht_t h, char *key);

#endif

