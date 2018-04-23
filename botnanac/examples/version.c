#include <stdio.h>
#include "botnana.h"

unsigned int version_count = 0;
unsigned int message_count = 0;

void handle_meaasge (const char * src)
{
	message_count = message_count + 1;
	printf("handle_meaasge (%u): %s \n", message_count, src);

}


void handle_version (const char * src)
{
	version_count = version_count + 1;
	printf("handle_version (%u): %s \n", version_count, src);

}


int main() {

	struct Botnana * botnana = botnana_connect("192.168.7.2", handle_meaasge);
	botnana_attach_event(botnana, "version", 12, handle_version);

	while (1)
	{
		botnana_get_version(botnana);
		sleep(1);
	}
	return 0;

}
