#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <cmsis_os.h>
#include <wifi_login_server.h>

#include "builtin_led.h"
#include "uart_isr_rx_tx.h"
#include "flash_write_read_data.h"

#define FLESH_SECTOR_ADDRESS 0x0800C100
#define WRITED_FLASH_FLAG 10113

struct ssid_pswd_data_t{
	char ssid[32];
	char pswd[32];
	uint32_t flag;
} sp_data;

static int sp_data_words()
{
	return sizeof(struct ssid_pswd_data_t) / 4;
}

#define OK_EXPR "OK\r\n"
#define STATIC_IP "192.168.1.1"

static char *ap_register_page = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'> \
<title>REGISTER ROUTER</title> \
//<link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css'> \
</head><style> html {height: 100%;} body {height: 100%;} \
.global-container {height: 100%;display: flex;align-items: center;justify-content: center;background-color: #f5f5f5;} \
form {padding-top: 10px; font-size: 14px; margin-top: 30px;} \
.card-title {font-weight: 300;} \
.btn {font-size: 14px; margin-top: 20px;}\
.login-form { width: 330px; margin: 20px;}\
.sign-up {text-align: center; padding: 20px 0 0;}\
.alert {margin-bottom: -30px; font-size: 13px; margin-top: 20px;}\
</style><body>\
<div class='pt-5'>\
  <div class='global-container'>\
    <div class='card login-form'>\
    <div class='card-body'>\
        <h3 class='card-title text-center'> REGISTER ROUTER </h3>\
        <div class='card-text'>\
            <form action='/data'>\
                <div class='form-group'>\
                    <label for='ssid1'>ssid:  </label>\
                    <input type='form' class='form-control form-control-sm' id='ssid1' name='ssid1'>\
                </div>\
                <div class='form-group'>\
                    <label for='pswd1'>pswd:</label>\
                    <input type='password' class='form-control form-control-sm' id='pswd1' name='pswd1'>\
                </div>\
                <button type='submit' class='btn btn-primary btn-block'> Enter </button>\
            </form>\
        </div>\
    </div>\
</div>\
</div>\
</body></html>";

static bool command(char* str, char* expr_to_wait)
{
	uart_isr_flush();
	uart_isr_sendstring(str);

	int result = 0;
	while(result == 0){
		result = uart_isr_wait_for(expr_to_wait);
		if(result == UART_TIMEOUT_RET){
			error_blink();
			return false;
		}
	}
	return true;
}

static bool sta_mode(char *ssid, char *pswd, char *sta_ip)
{
	char data[64];
	if(!command("AT+CWMODE_CUR=1\r\n", OK_EXPR))return false;

	sprintf(data, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pswd);
	if(!command(data, OK_EXPR))return false;

	uart_isr_flush();
	uart_isr_sendstring("AT+CIFSR\r\n");
	while (!(uart_isr_wait_for("CIFSR:STAIP,\"")));
	char buffer[20] = {0};
	while (!(uart_isr_copy_upto("\"",buffer)));
	while (!(uart_isr_wait_for(OK_EXPR)));

	if(!command("AT+CIPMUX=1\r\n", OK_EXPR)) return false;
	if(!command("AT+CIPSERVER=1,80\r\n", OK_EXPR)) return false;

	return true;
}

static bool ap_mode(char* ap_ip)
{
	char data[64];
	if(!command("AT+CWMODE=2\r\n", "Ai-Thinker")){
		osDelay(1000);
		return false;
	}
	if(!command("AT+CWDHCP=0,1\r\n", OK_EXPR)) return false;
	if(!command("AT+CWSAP_CUR=\"IoTWebService\",\"12345678\",8,4\r\n", OK_EXPR)) return false;
	if(!command("AT+CIPMODE=0\r\n", OK_EXPR)) return false;
	sprintf(data, "AT+CIPAP_CUR=\"%s\"\r\n", ap_ip);
	if(!command(data, OK_EXPR)) return false;
	if(!command("AT+CIPMUX=1\r\n", OK_EXPR)) return false;
	if(!command("AT+CIPSERVER=1,80\r\n", OK_EXPR)) return false;

	return true;
}

static bool send_to_client_html(char* string, int id)
{
	char data[80];
	char datatosend[4096] = {0};
	sprintf(datatosend, string);

	uart_isr_flush();
	sprintf(data, "AT+CIPSEND=%d,%d\r\n", id, strlen(datatosend));
	if(!command(data, ">"))
		return false;
	if(!command(datatosend, "SEND OK"))
		return false;

	sprintf(data, "AT+CIPCLOSE=%d\r\n", id);
	if(!command(data, OK_EXPR))
		return false;

	return true;
}

bool start_http_web_server()
{
	uart_isr_init();
	uart_isr_sendstring("AT+RST\r\n"); osDelay(2000);
	if(!command("AT\r\n", OK_EXPR))
		return false;
	flash_data_read(FLESH_SECTOR_ADDRESS, (uint32_t*)&sp_data, sp_data_words());

	if(sp_data.flag == WRITED_FLASH_FLAG)
		return sta_mode(sp_data.ssid, sp_data.pswd, STATIC_IP);
	else
		return ap_mode(STATIC_IP);

	return false;
}

bool proc_http_web_server()
{
	char id;
	int result = 0;
	while (!result){
		result = uart_isr_get_after("+IPD,", 1, &id);
		if(result == UART_TIMEOUT_RET)
			return false;
	}
	id -= 48;

	result = 0;
	char buff[256] = {0};
	while (!result){
		result = uart_isr_copy_upto(" HTTP/1.1", buff);
		if(result == UART_TIMEOUT_RET)
			return false;
	}

	if(sp_data.flag != WRITED_FLASH_FLAG)
	{
		if(uart_isr_look_for("/data", buff) == 1)
		{
			char ssid[32];
			char pswd[32];

			uart_isr_getDataFromBuffer("ssid1=", "&", buff, ssid);
			uart_isr_getDataFromBuffer("pswd1=", " HTTP", buff, pswd);

			if(strlen(ssid) && strlen(pswd) > 7)
			{
				uart_isr_sendstring("AT+RST\r\n"); osDelay(2000);
				if(sta_mode(ssid, pswd, STATIC_IP))
				{
					sprintf(sp_data.ssid, ssid);
					sprintf(sp_data.pswd, pswd);
					sp_data.flag = WRITED_FLASH_FLAG;

					flash_data_write(FLESH_SECTOR_ADDRESS, (uint32_t*)&sp_data, sp_data_words());
				}
				return start_http_web_server();
			}
		}
		send_to_client_html(ap_register_page, id);
	}
	else
	{
		/*------------------------------------*/
		/*-----------YOU-CODE-HERE------------*/
		/*------------------------------------*/
	}

	return true;
}
