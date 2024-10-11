#define F_CPU 16000000UL
#define BAUD 9600
#define GPS_BAUD 9600
#define UBRR_VALUE ((F_CPU/16/BAUD)-1)
#define GPS_UBRR_VALUE ((F_CPU/16/GPS_BAUD)-1)

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define BUFFER_SIZE 160
#define Buzzer_PIN PB7
#define VIBRATION_PIN PE4
#define GPS_BUFFER_SIZE 200

volatile char received_string[BUFFER_SIZE];
volatile uint8_t received_string_index = 0;
volatile uint8_t message_received = 0;
volatile char gps_buffer[GPS_BUFFER_SIZE];
volatile uint8_t gps_buffer_index = 0;
volatile uint8_t gps_data_ready = 0;

void Config_USART0(void)
{
	UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
	UBRR0L = (unsigned char)UBRR_VALUE;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void Config_USART2(void)
{
	UBRR2H = (unsigned char)(GPS_UBRR_VALUE >> 8);
	UBRR2L = (unsigned char)GPS_UBRR_VALUE;
	UCSR2B = (1 << RXEN2) | (1 << TXEN2) | (1 << RXCIE2);
	UCSR2C = (1 << UCSZ21) | (1 << UCSZ20);
}

void USART0_Transmit(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void USART0_TransmitString(const char* str) {
	while (*str) {
		USART0_Transmit(*str++);
	}
}

void SIM800_Init(void) {
	_delay_ms(2000);
	USART0_TransmitString("AT\r\n");
	_delay_ms(1000);
	USART0_TransmitString("AT+CMGF=1\r\n");
	_delay_ms(1000);
	USART0_TransmitString("AT+CNMI=1,2,0,0,0\r\n");  
	_delay_ms(1000);
}

void SIM800_SendSMS(const char* number, const char* message) {
	char at_command[30];
	snprintf(at_command, sizeof(at_command), "AT+CMGS=\"%s\"\r\n", number);
	USART0_TransmitString(at_command);
	_delay_ms(1000);
	USART0_TransmitString(message);
	USART0_Transmit(0x1A);  
	_delay_ms(5000);  
}

void Buzzer_On() {
	PORTB |= (1 << Buzzer_PIN);
}

void Buzzer_Off() {
	PORTB &= ~(1 << Buzzer_PIN);
}

void Buzzer(void) {
	for (int i = 0; i < 20; i++) { 
		Buzzer_On();
		_delay_ms(500);
		Buzzer_Off();
		_delay_ms(500);
	}
}

void parse_gps_data(char* buffer, char* latitude, char* longitude) {
	char* token = strtok(buffer, ",");
	int field_count = 0;

	while (token != NULL) {
		if (field_count == 3) {
			// DDMM.MMMMM to decimal degrees
			float lat_deg = atof(token) / 100;
			int lat_int_deg = (int)lat_deg;
			float lat_min = (lat_deg - lat_int_deg) * 100;
			float lat_decimal = lat_int_deg + (lat_min / 60);

			
			if (lat_decimal > 0.1) {
				dtostrf(lat_decimal, 8, 6, latitude);  
				} else {
				strcpy(latitude, "0");  
			}
			} else if (field_count == 5) {
			// DDDMM.MMMMM to decimal degrees
			float lon_deg = atof(token) / 100;
			int lon_int_deg = (int)lon_deg;
			float lon_min = (lon_deg - lon_int_deg) * 100;
			float lon_decimal = lon_int_deg + (lon_min / 60);

			
			if (lon_decimal > 0.1) {
				dtostrf(lon_decimal, 8, 6, longitude); 
				} else {
				strcpy(longitude, "0");  
			}
			break;
		}
		token = strtok(NULL, ",");
		field_count++;
	}
}

ISR(USART0_RX_vect) {
	char received_char = UDR0;
	
	if (received_string_index < BUFFER_SIZE - 1) {
		received_string[received_string_index++] = received_char;
		received_string[received_string_index] = '\0';
		
		if (strstr((char*)received_string, "\r\n") != NULL) {
			message_received = 1;
		}
	}
}

ISR(USART2_RX_vect) {
	char received_char = UDR2;
	
	if (gps_buffer_index < GPS_BUFFER_SIZE - 1) {
		gps_buffer[gps_buffer_index++] = received_char;
		gps_buffer[gps_buffer_index] = '\0';
		
		if (strstr((char*)gps_buffer, "\r\n") != NULL) {
			// Process GPS data
			if (strstr((char*)gps_buffer, "$GPRMC") != NULL) {
				gps_data_ready = 1;
			}
		}
	}
}

int main(void)
{
	Config_USART0();  // For GSM module
	Config_USART2();  // For GPS module
	SIM800_Init();

	// Set Buzzer pin as output
	DDRB |= (1 << Buzzer_PIN);

	// Set PE4 as input for vibration sensor
	DDRE &= ~(1 << VIBRATION_PIN);

	sei();  // Enable global interrupts
	USART0_TransmitString("System ready. Monitoring for vibrations and SMS commands...\r\n");

	uint8_t sms_sent = 0;
	char sms_buffer[200];
	char latitude[15] = "0", longitude[15] = "0";

	while (1)
	{
		if (gps_data_ready) {
			parse_gps_data((char*)gps_buffer, latitude, longitude);
			gps_data_ready = 0;
			gps_buffer_index = 0;

			// Debug output
			USART0_TransmitString("GPS Data: ");
			USART0_TransmitString(latitude);
			USART0_TransmitString(", ");
			USART0_TransmitString(longitude);
			USART0_TransmitString("\r\n");
		}
	

		// Check for incoming SMS
		if (message_received) {
			if (strstr((char*)received_string, "Location") != NULL) {
				// Send current location
				if (strcmp(latitude, "0") != 0 && strcmp(longitude, "0") != 0) {
					snprintf(sms_buffer, sizeof(sms_buffer),
					"Current location: https://www.google.com/maps?q=%s,%s",
					latitude, longitude);
					} else {
					snprintf(sms_buffer, sizeof(sms_buffer), "Unable to get current location. GPS data not available.");
				}
				SIM800_SendSMS("+94717138078", sms_buffer);
				USART0_TransmitString("Location request received. SMS sent.\r\n");
				_delay_ms(1000);
			}
			message_received = 0;
			received_string_index = 0;
			memset(received_string, 0, BUFFER_SIZE);
		}

		// Check for vibration
		if (PINE & (1 << VIBRATION_PIN)) {
			if (!sms_sent) {
				if (strcmp(latitude, "0") != 0 && strcmp(longitude, "0") != 0) {
					// Construct Google Maps link with location
					snprintf(sms_buffer, sizeof(sms_buffer),
					"Accident signal detected! Location: https://www.google.com/maps?q=%s,%s",
					latitude, longitude);
					} else {
					// Send message without location
					snprintf(sms_buffer, sizeof(sms_buffer), "Accident signal detected! Location unavailable.");
				}

				SIM800_SendSMS("+94717138078", sms_buffer);
				USART0_TransmitString("Accident signal detected!. SMS sent.\r\n");
				sms_sent = 1;
			}
			Buzzer();  // Blink LED for 10 seconds
			} else {
			sms_sent = 0;  // Reset SMS flag when no vibration is detected
		}

		_delay_ms(50);  // Small delay to prevent constant checking
	}
	return 0;
}