/*
    Copyright (c) 2007-2014 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testutil.hpp"

void print_frame(zmq_msg_t *msg) {
  unsigned int i;
  const char *p = (const char *)zmq_msg_data(msg);
  size_t len = zmq_msg_size(msg);
  fprintf(stderr, "frame = {");
  for(i = 0; i < len; ++i) {
    fprintf(stderr, "0x%02x '%c', ", (unsigned char)p[i], p[i]);
  }
  fprintf(stderr, "}\n");    
}

int main (void)
{
    setup_test_environment();
    void *ctx = zmq_ctx_new ();
    assert (ctx);
    
    //  Create server and bind to endpoint
    void *server = zmq_socket (ctx, ZMQ_ROUTER);
    assert (server);
    int rc;
    int mask = ZMQ_EVENT_ALL;
    rc = zmq_setsockopt (server, ZMQ_EVENT_MSGS, &mask, sizeof (mask));
    assert (rc == 0);
    rc = zmq_bind (server, "tcp://127.0.0.1:5560");
    assert (rc == 0);

    //  Create client and connect to server, doing a probe
    void *client = zmq_socket (ctx, ZMQ_DEALER);
    assert (client);
    rc = zmq_setsockopt (client, ZMQ_IDENTITY, "CLIENT", 6);
    assert (rc == 0);
    rc = zmq_connect (client, "tcp://localhost:5560");
    assert (rc == 0);

    //  Send a message to server now
    fprintf(stderr, "send message to server\n");
    rc = zmq_send (client, "", 0, ZMQ_SNDMORE);
    assert (rc == 0);
    rc = zmq_send (client, "Hello", 5, 0);
    assert (rc == 5);

    // We expect an event message
    fprintf(stderr, "expect an event message\n");
    zmq_msg_t id;
    rc = zmq_msg_init(&id);
    assert(rc == 0);
    rc = zmq_msg_recv(&id, server, 0);
    assert(rc != -1);
    print_frame(&id);
    zmq_msg_t msg;
    do {
      rc = zmq_msg_init(&msg);
      assert(rc == 0);
      rc = zmq_msg_recv(&msg, server, 0);
      assert(rc != -1);
      print_frame(&msg);
      rc = zmq_msg_close(&msg);
      assert(rc == 0);
    } while(zmq_msg_more(&msg));

    // We expect a message from client
    fprintf(stderr, "expect message from client\n");
    zmq_msg_t id2;
    rc = zmq_msg_init(&id2);
    assert(rc == 0);
    rc = zmq_msg_recv(&id2, server, 0);
    assert(rc != -1);
    print_frame(&id2);
    do {
      rc = zmq_msg_init(&msg);
      assert(rc == 0);
      rc = zmq_msg_recv(&msg, server, 0);
      assert(rc != -1);
      print_frame(&msg);
      rc = zmq_msg_close(&msg);
      assert(rc == 0);
    } while(zmq_msg_more(&msg));

    // Send a message to client now
    fprintf(stderr, "send message to client\n");
    rc = zmq_send (server, zmq_msg_data(&id), zmq_msg_size(&id), ZMQ_SNDMORE);
    assert (rc != -1);
    rc = zmq_send (server, "Hello", 6, 0);
    assert (rc == 6);

    // close id message
    rc = zmq_msg_close(&id);
    assert(rc == 0);

    // get message
    fprintf(stderr, "get message from client\n");
    char buffer[256];
    rc = zmq_recv (client, buffer, 255, 0);
    fprintf(stderr, "server says '%s'\n", buffer);
    assert (rc == 6);
    
    rc = zmq_close (server);
    assert (rc == 0);

    rc = zmq_close (client);
    assert (rc == 0);

    rc = zmq_ctx_term (ctx);
    assert (rc == 0);

    return 0 ;
}
