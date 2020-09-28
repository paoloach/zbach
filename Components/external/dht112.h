#ifndef __DHT_112__H__
#define __DHT_112__H__

void dht112_init(uint8 taskid);
void dht112_loop(uint8 taskid);

extern uint16 temp;
extern uint16 humidity;

#endif