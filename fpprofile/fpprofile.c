// fpprofile.cpp : Defines the entry point for the application.
//

#include "fpprofile.h"
#include "netcode.h"
#include "sodium.h"

#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#define CONNECT_TOKEN_EXPIRY 30
#define CONNECT_TOKEN_TIMEOUT 5
#define PROTOCOL_ID 0x1122334455667788

static uint8_t private_key[NETCODE_KEY_BYTES] = { 0x60, 0x6a, 0xbe, 0x6e, 0xc9, 0x19, 0x10, 0xea,
												  0x9a, 0x65, 0x62, 0xf6, 0x6f, 0x2b, 0x30, 0xe4,
												  0x43, 0x71, 0xd6, 0x2c, 0xd1, 0x99, 0x27, 0x26,
												  0x6b, 0x3c, 0x60, 0xf4, 0xb7, 0x15, 0xab, 0xa1 };

static volatile int quit = 0;

void interrupt_handler(int signal)
{
	(void)signal;
	quit = 1;
}

#define INSTRUCTION_ADD 0
#define INSTRUCTION_SUB 1
#define INSTRUCTION_MUL 2
#define INSTRUCTION_DIV 3
#define INSTRUCTION_POW 4
#define INSTRUCTION_SQRT 5
#define INSTRUCTION_TAN 6
#define INSTRUCTION_ATAN 7

#define INSTRUCTION_MAX_OP_NUM 8

#define SEQUENCE_COUNT 4
#define SEQUENCE_ENTRY_SIZE (sizeof(float) + sizeof(int8_t))
#define SEQUENCE_SIZE (sizeof(float) + (SEQUENCE_COUNT * SEQUENCE_ENTRY_SIZE) + sizeof(float))

int8_t random_operation()
{
	return (int8_t)(randombytes_random() % INSTRUCTION_MAX_OP_NUM);
}

float random_float()
{
	// Generate a random uint32, then treat it as a float to just get something (potentially
	// an invalid float, but we want to see how things respond to any value).
	return (float)randombytes_random();
}

float execute_instruction(float current, float input, int operation)
{
	switch (operation)
	{
	case INSTRUCTION_ADD:
		return current + input;
	case INSTRUCTION_SUB:
		return current - input;
	case INSTRUCTION_MUL:
		return current * input;
	case INSTRUCTION_DIV:
		return current / input;
	case INSTRUCTION_POW:
		return powf(current, input);
	case INSTRUCTION_SQRT:
		return sqrtf(current);
	case INSTRUCTION_TAN:
		return tanf(current);
	case INSTRUCTION_ATAN:
		return atanf(current);
	default:
		return current;
	}
}

char* generate_instruction_sequence()
{
	char* sequence = (char*)malloc(SEQUENCE_SIZE);
	if (sequence == NULL)
	{
		return NULL;
	}

	float initial = random_float();
	memcpy(sequence, &initial, sizeof(float));

	float chain = initial;

	for (int i = 0; i < SEQUENCE_COUNT; i++)
	{
		int8_t operation = random_operation();
		float next = random_float();

		memcpy(sequence + sizeof(float) + (i * SEQUENCE_ENTRY_SIZE), &operation, sizeof(int8_t));
		memcpy(sequence + sizeof(float) + (i * SEQUENCE_ENTRY_SIZE) + sizeof(int8_t), &next, sizeof(float));

		chain = execute_instruction(chain, next, operation);
	}

	memcpy(sequence + sizeof(float) + (SEQUENCE_COUNT * SEQUENCE_ENTRY_SIZE), &chain, sizeof(float));

	return sequence;
}

bool verify_instruction_sequence(char* sequence, int sequence_bytes, float* local_result, float* remote_result)
{
	if (sequence_bytes != SEQUENCE_SIZE)
	{
		return false;
	}

	float initial;
	memcpy(&initial, sequence, sizeof(float));

	float chain = initial;

	for (int i = 0; i < SEQUENCE_COUNT; i++)
	{
		int8_t operation;
		float next;

		memcpy(&operation, sequence + sizeof(float) + (i * SEQUENCE_ENTRY_SIZE), sizeof(int8_t));
		memcpy(&next, sequence + sizeof(float) + (i * SEQUENCE_ENTRY_SIZE) + sizeof(int8_t), sizeof(float));

		chain = execute_instruction(chain, next, operation);
	}

	memcpy(remote_result, sequence + sizeof(float) + (SEQUENCE_COUNT * SEQUENCE_ENTRY_SIZE), sizeof(float));

	*local_result = chain;

	// compare the floats at a bit level - make sure they're exactly the same bits
	return (uint32_t)*local_result == (uint32_t)*remote_result;
}

int main(int argc, char* argv[])
{
	if (netcode_init() != NETCODE_OK)
	{
		printf("error: failed to initialize netcode.io\n");
		return 1;
	}

	netcode_log_level(NETCODE_LOG_LEVEL_ERROR);

	double time = 0.0;
	double delta_time = 1.0 / 60.0;

	if (argc != 2)
	{
		printf("error: expected either 'server' or server address to connect to\n");
		return 1;
	}

	bool is_server = argc >= 2 && (strcmp(argv[1], "server") == 0 || strcmp(argv[1], "dual") == 0);
	bool is_client = argc >= 2 && strcmp(argv[1], "server") != 0;

	char* server_address = "127.0.0.1:40000";
	char* client_server_address = argv[1];

	struct netcode_server_t* server;
	struct netcode_client_t* client;

	if (is_server)
	{
		printf("acting as server\n");

		struct netcode_server_config_t server_config;
		netcode_default_server_config(&server_config);
		server_config.protocol_id = PROTOCOL_ID;
		memcpy(&server_config.private_key, private_key, NETCODE_KEY_BYTES);

		server = netcode_server_create(server_address, &server_config, time);

		if (!server)
		{
			printf("error: failed to create server\n");
			return 1;
		}

		netcode_server_start(server, 64);

		printf("server started on %s\n", server_address);
	}
	
	if (is_client)
	{
		printf("acting as client\n");

		if (strcmp(argv[1], "dual") == 0)
		{
			client_server_address = "127.0.0.1:40000";
		}

		struct netcode_client_config_t client_config;
		netcode_default_client_config(&client_config);
		client = netcode_client_create("0.0.0.0:0", &client_config, time);

		if (!client)
		{
			printf("error: failed to create client\n");
			return 1;
		}

		uint8_t connect_token[NETCODE_CONNECT_TOKEN_BYTES];

		uint64_t client_id = 0;
		netcode_random_bytes((uint8_t*)&client_id, 8);
		printf("client id is %.16" PRIx64 "\n", client_id);

		uint8_t user_data[NETCODE_USER_DATA_BYTES];
		netcode_random_bytes(user_data, NETCODE_USER_DATA_BYTES);

		if (netcode_generate_connect_token(1, (NETCODE_CONST char**) & client_server_address, (NETCODE_CONST char**) & server_address, CONNECT_TOKEN_EXPIRY, CONNECT_TOKEN_TIMEOUT, client_id, PROTOCOL_ID, private_key, user_data, connect_token) != NETCODE_OK)
		{
			printf("error: failed to generate connect token\n");
			return 1;
		}

		printf("client connecting to %s\n", client_server_address);

		netcode_client_connect(client, connect_token);
	}

	signal(SIGINT, interrupt_handler);

	int valid_sequences = 0;
	int invalid_sequences = 0;

	while (!quit)
	{
		if (is_server)
		{
			netcode_server_update(server, time);

			char* sequence = generate_instruction_sequence();
			if (sequence != NULL)
			{
				for (int i = 0; i < netcode_server_max_clients(server); i++)
				{
					if (netcode_server_client_connected(server, i))
					{
						netcode_server_send_packet(server, 0, sequence, SEQUENCE_SIZE);
					}
				}

				free(sequence);
			}
		}
		
		if (is_client)
		{
			netcode_client_update(client, time);

			while (1)
			{
				int packet_bytes;
				uint64_t packet_sequence;
				void* packet = netcode_client_receive_packet(client, &packet_bytes, &packet_sequence);
				if (!packet)
				{
					break;
				}

				float local_result, remote_result;

				if (!verify_instruction_sequence((char*)packet, packet_bytes, &local_result, &remote_result))
				{
					invalid_sequences++;

					printf("deterministic floating point sequence failed, got local result %f with remote result %f\n\n", local_result, remote_result);
				}
				else
				{
					valid_sequences++;
				}

				printf("sequences valid: %i invalid: %i\n", valid_sequences, invalid_sequences);

				netcode_client_free_packet(client, packet);
			}

			if (!is_server)
			{
				if (valid_sequences + invalid_sequences > 1000)
				{
					if (invalid_sequences > 0)
					{
						// at least one failure, exit with failure
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}
		}

		netcode_sleep(delta_time);

		time += delta_time;
	}
	
	return 0;
}
