#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <dirent.h>
#ifdef USE_GLIB

#include <glib.h>
#define strlcpy g_strlcpy
#define strlcat g_strlcat
#endif
#define UNIX_PATH_MAX    108
#define WEBCGI_WEBCLIENT_SOCK "/tmp/qcmap_webclient_cgi_file"
#define WEBCLIENT_WEBCGI_SOCK "/tmp/cgi_webclient_file"
//Time to wait for socket responce in secs.
#define SOCK_TIMEOUT 50
//Returns Minimum value of both
#define MIN(a,b) (((a)<(b))?(a):(b))
//Max Buffer size for HTTP and Socket transactions
#define MAX_BUFFER_SIZE 65536

#define HTTP_RESPONCE_HEADER "Content-Type: text/html\nContent-Length: "
#define SYSTEM_ERR "System Error"
#define SYSTEM_ERR_LEN 12
#define GOT_NOTHING "Nothing"
#define GOT_NOTHING_LEN 7
#define MEMORY_ERROR "Memory Error"
#define MEMORY_ERROR_LEN 12
#define HTTP_READ_ERROR "HTTP Read Error"
#define HTTP_READ_ERROR_LEN 15
#define SOCK_OPEN_ERROR "{\"commit\":\"Socket Open Error\"}"
#define SOCK_OPEN_ERROR_LEN 30
#define SOCK_FD_ERROR "{\"commit\":\"Socket FD Error\"}"
#define SOCK_FD_ERROR_LEN 28
#define SOCK_BIND_ERROR "{\"commit\":\"Socket Bind Error\"}"
#define SOCK_BIND_ERROR_LEN 30
#define SOCK_PATH_ERROR "{\"commit\":\"Socket Path Error\"}"
#define SOCK_PATH_ERROR_LEN 30
#define SOCK_SEND_ERROR "{\"commit\":\"Socket Send Error\"}"
#define SOCK_SEND_ERROR_LEN 30
#define SOCK_SEND_COMPLETE_ERROR "{\"commit\":\"Unable to Send Complete "\
                                 "data through socket\"}"
#define SOCK_SEND_COMPLETE_ERROR_LEN 56
#define SOCK_TIMEOUT_ERROR "{\"commit\":\"Socket TimeOut\"}"
#define SOCK_TIMEOUT_ERROR_LEN 27
#define SOCK_RESPONSE_ERROR "{\"commit\":\"Socket Responce Error\"}"
#define SOCK_RESPONSE_ERROR_LEN 34
#define SOCK_RECEIVE_ERROR "{\"commit\":\"Socket Receive Error\"}"
#define SOCK_RECEIVE_ERROR_LEN 33
#define SUCCESS 0
#define FAIL    -1

#define LOG_BUFF_SIZE 28
#define MAX_HTTP_BUFFER_SIZE 100
#define IPV4_STRING_LEN 16
#define AUTH_FAIL "Content-Type: text/html\nContent-Length: \
                            22\n\n{\"result\":\"AUTH_FAIL\"}"
#define UPDATE_FILE_PATH "/tmp/"
#define UPLOAD_FILE_SUCCESS_LEN 36
#define UPLOAD_FILE_SUCCESS "{\"commit\":\"Success\",\"downgrade\":\"0\"}"
#define DOWNGRADE_FILE_SUCCESS "{\"commit\":\"Success\",\"downgrade\":\"1\"}"

#define LOG_MSG_INFO( fmtString, args ...)   syslog(LOG_INFO, fmtString, ## args);

#define LOG_MSG_ERROR(fmtString, args ...)   syslog (LOG_ERR, fmtString, ## args);

#define SYS_LOG(syslogLevel, msg, args ...) syslog(syslogLevel, msg, ## args);
typedef struct
{
  //To store last login time
  struct timeval log_time;
  //To store last login login IP 
  char ip[16];
  //To specify session timeout
  unsigned int timeout;
}loginfo;


int is_ipv4_address_valid(char *p_ipAddress)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, p_ipAddress, &(sa.sin_addr)) != 0;
}

void write_save_ip(char *ip_address)
{
    FILE *p_fp = NULL;

     p_fp = fopen("/var/www/html/box_ip_add", "w");
     if(p_fp)
    {
      fwrite(ip_address, 1, strlen(ip_address), p_fp);
      fclose(p_fp);
    }
}

int get_current_config(char *p_ipaddres, char *p_subnet, int *p_dhcp_mode, char *p_gateway, char *p_dns)
{
	FILE *fp = NULL;
	char lineBufF[256];
    char temp[32];
    int result = 0;

	fp = fopen("/etc/network/interfaces", "r");

	if (fp != NULL)
	{	
 		
	    while(fgets(lineBufF, sizeof(lineBufF), fp))
	    {
	        if (strstr(lineBufF, "inet dhcp"))
	        {
	            *p_dhcp_mode = 0;
	            result = 1;
	        }
	        else if(strstr(lineBufF, "inet static"))
	        {
	        	*p_dhcp_mode = 1;
	        	result = 1;
	        }
	        else  if ( strstr(lineBufF, "address") && !strstr(lineBufF, "hwaddress"))
	        {
	            sscanf(lineBufF, "%s %s", temp, p_ipaddres);
	        }
	        else if (strstr(lineBufF, "netmask"))
	        {
	        	sscanf(lineBufF, "%s %s", temp, p_subnet);
	        }
	        else if (strstr(lineBufF, "gateway"))
	        {
	        	sscanf(lineBufF, "%s %s", temp, p_gateway);
	        }
	 
	        else if (strstr(lineBufF, "dns-nameservers"))
	        {
	        	sscanf(lineBufF, "%s %s", temp, p_dns);	
	        }

	        memset(lineBufF, 0, sizeof(lineBufF));
	    }		
	}


	// printf(" \n \t 3434p_ipaddres %s, p_subnet %s, p_dhcp_mode %d, p_gateway %s, p_dns  %s\n", p_ipaddres, p_subnet, *p_dhcp_mode, p_gateway, p_dns);

	if(fp)
		fclose(fp);
	return result;
}

int get_box_ipp(char *ip_address)
{
    FILE *p_fHandle = NULL;
    char lineBuffer[128], tempIP;
    int  rc = -1;

    p_fHandle = popen("cat /var/www/html/box_ip_add", "r");

    if (p_fHandle != NULL)
    {
        memset(lineBuffer, 0, sizeof lineBuffer);
        rc = fread(lineBuffer, 1, sizeof lineBuffer, p_fHandle);

        if (is_ipv4_address_valid(lineBuffer) != 0)
        {
            strcpy(ip_address, lineBuffer);
            pclose(p_fHandle);
            return 1;
        }

        pclose(p_fHandle);
    }

    return 0;
}

int parse_network_config(char *html_cgi_buff, int *dhcp_mode, char *tp_ipaddres, char *tp_subnet, char *tp_gateway, char *tp_dns)
{
	char *sub;
	char s_dhcp[2];
	char p_ipaddres[32];
	char p_subnet[32];
	char p_gateway[32];
	char p_dns[32];
	int result = 0;


	memset(p_ipaddres, 0, sizeof(p_ipaddres));
	memset(p_subnet, 0, sizeof(p_subnet));
	memset(p_gateway, 0, sizeof(p_gateway));
	memset(p_dns, 0, sizeof(p_dns));
	memset(s_dhcp, 0, sizeof(s_dhcp));

	if(strstr(html_cgi_buff, "dhcp_mode=0"))
	{
		*dhcp_mode = 0;
		return;
	}
	else if (strstr(html_cgi_buff, "dhcp_mode=1"))
	{
		*dhcp_mode = 1;
	}
 

	//getIP
	sub = NULL;
	sub = strstr(html_cgi_buff,"ipaddress=");
	sub = sub + 10;
	for (int i = 0; i < sizeof(p_ipaddres); i++)
	{   
	    if ( sub[0] == '&')
	    {
	        break;
	    }
	    p_ipaddres[i]= sub[0];
	    sub = sub + 1;
	}

	strcpy(tp_ipaddres, p_ipaddres);

		//subnet
	sub = NULL;
	sub = strstr(html_cgi_buff,"subnet=");
	sub = sub + 7;
	for (int i = 0; i < sizeof(p_subnet); i++)
	{   
	    if ( sub[0] == '&')
	    {
	        break;
	    }
	    p_subnet[i]= sub[0];
	    sub = sub + 1;
	}

	strcpy(tp_subnet, p_subnet);


		//p_gateway
	sub = NULL;
	sub = strstr(html_cgi_buff,"gateway=");
	sub = sub + 8;
	for (int i = 0; i < sizeof(p_gateway); i++)
	{   
	    if ( sub[0] == '&')
	    {
	        break;
	    }
	    p_gateway[i]= sub[0];
	    sub = sub + 1;
	}
	strcpy(tp_gateway, p_gateway);

	//dns=
	sub = NULL;
	sub = strstr(html_cgi_buff,"dns=");
	sub = sub + 4;
	for (int i = 0; i < sizeof(p_dns); i++)
	{   
	    if ( sub[0] == '&' )
	    {	
	    	//printf("done \n");
	        break;
	    }
	    p_dns[i]=sub[0];
	    sub = sub + 1;
	}
	strcpy(tp_dns, p_dns);

	sub = NULL;

	return 1;
}
int webcgi_webcli_communication(char *webcgi_webcli_buff,
                                int webcgi_webcli_buff_size,
                                char *webcli_webcgi_buff,
                                int *webcli_webcgi_buff_size);

int send_to_webcli(char *webcgi_webcli_buff,
                   int webcgi_webcli_buff_size,
                   char *webcli_webcgi_buff,
                   int *webcli_webcgi_buff_size,
                   int *webcli_sockfd);

int recv_from_webcli(char *webcli_webcgi_buff,
                     int *webcli_webcgi_buff_size,
                     int *webcli_sockfd);

 int curl_get_all_info(char *box_info)
 {
     FILE *p_fHandle = NULL;
    char lineBuffer[2048];
    char *sub;
    int  rc = -1;
    int result = 0;

   // p_fHandle = popen("curl --header \"Content-Type: application/json\" --request GET http://192.168.1.16:8200/controller/getVoltage", "r");
    // curl --header "Content-Type: application/json" --request GET http://192.168.1.16:8200/getallInfo/
    memset(lineBuffer, 0, sizeof(lineBuffer));
    sprintf(lineBuffer, "curl --header \"Content-Type: application/json\" --request GET http://localhost:8200/controller/getAllInfo");

    p_fHandle = popen(lineBuffer, "r");


    if (p_fHandle != NULL)
    {
        memset(lineBuffer, 0, sizeof lineBuffer);
        rc = fread(lineBuffer, 1, sizeof lineBuffer, p_fHandle);
        if(rc > 0)
        {
            sub = strstr(lineBuffer, "FPGA");
          //  printf("%s\n", sub);
            result = 1;
        }

        strcpy(box_info, lineBuffer);
   


        pclose(p_fHandle);
    }
 return result;
 }

int get_current_mac(char *p_mac)
{
	FILE *p_fp = NULL;
	int rc = -1;
	int lineBuff[256];
	int result = -1;

	memset(lineBuff, 0, sizeof(lineBuff));
	p_fp = popen("ifconfig -a | grep -Po \'HWaddr \\K.*$\'", "r");
	if (p_fp != NULL)
	{
		rc = fread(lineBuff, 1, sizeof(lineBuff), p_fp);
		if (rc > 0 )
		{   
			sscanf(lineBuff,"%s", p_mac );
			result = 1;
		}

		pclose(p_fp);
	}

	return result;
}

int set_network_config(int dhcp_mode, char *p_ipaddres, char *p_subnet, char *p_gateway, char *p_dns)
{
	FILE *fp = NULL;
	char mac_add[64];
	char lineBuff[128];
	FILE *p_fp = NULL;

	memset(mac_add, 0, sizeof(mac_add));
	memset(lineBuff, 0, sizeof(lineBuff));

	if (get_current_mac(mac_add) == 0)
		return 0;

    fp = fopen("/etc/network/interfaces", "w");

    if (dhcp_mode == 0) //mode hcp
    {
	    if (fp)
	    {

	    	system("sudo ifconfig eth0 down");
 	        system("sudo /etc/init.d/networking stop");
	        fprintf(fp, "source-directory /etc/network/interfaces.d\n");
	        fprintf(fp, "auto eth0\n");
	        fprintf(fp, "iface eth0 inet dhcp\n");
	        fprintf(fp, "hwaddress ether %s\n", mac_add);
	      //  system("sudo cp  /etc/network/interfaces /tmp/setautochdpc");
	     //   system("sudo ifconfig eth0 up");
	       // system("sudo /etc/init.d/networking restart");
	    }    	
    }
    else //mode static 
    {
            
            system("sudo ifconfig eth0 down");
	        system("sudo /etc/init.d/networking stop");


    		fprintf(fp, "source-directory /etc/network/interfaces.d\n");
	        fprintf(fp, "auto eth0\n");
	        fprintf(fp, "iface eth0 inet static\n");
	        fprintf(fp, "address %s\n", p_ipaddres);
	        fprintf(fp, "netmask %s\n", p_subnet);
	        fprintf(fp, "gateway %s\n", p_gateway);
	        fprintf(fp, "dns-nameservers %s\n", p_dns);
	        fprintf(fp, "hwaddress ether %s\n", mac_add);
	        sleep(2);
	     //   system("sudo /etc/init.d/networking restart");
	      //  system("sudo ifconfig eth0 up");

    }
    
    if(fp)
    {
    	 fclose(fp);
    }

 	memset(lineBuff, 0, sizeof(lineBuff)) ;
 	strcpy(lineBuff, "curl --header \"Content-Type: application/json\" --request GET http://localhost:8200/controller/reboot");	
 	p_fp = popen(lineBuff, "r");	 
    		 

    return 1;

}


int main(void) 
{	
    //To parse incoming HTTP Traffic Length.
    char *http_inc_traffic_len = NULL;
    //To store Min buffer length required to read HTTP,
    //afterwards this variable can be used as temp. buffer.
    char cgi_html_buff[MAX_BUFFER_SIZE]={0}; //To Generate HTTP Response.
    //To store Min buffer length required to read HTTP,
    //afterwards this variable can be used as temp. buffer.
    char *html_cgi_buff; //To store HTTP Request
	int html_cgi_buff_len = 0; 
	int res = -1;
	int cgi_html_buff_len = 0;
    char *pDest = NULL;
    char *p1Dest = NULL;  
    int boundaryLen = 0;
    int i = 0;
    int j = 0;
	char writeFile[128] = "";
	char writeFileTmp[128] = "";
    char command[256] = {0};
    char action[32] = {0};
    FILE *fileFd = NULL;
    char lengthDebug[256] = {0};
    char *p_action = NULL;

	http_inc_traffic_len = getenv("CONTENT_LENGTH");
	
	char *sub;
	char newip[128];
	char oldip[128];
	char responce[1024];
	char box_info_all[1024];


	char mac_add[64];
	int dhcp_mode = 0;
	char p_ipaddres[64];
	char p_subnet[64];
	char p_gateway[64];
	char p_dns[64];

	if (http_inc_traffic_len)
	{
		memset(cgi_html_buff, 0, MAX_BUFFER_SIZE);
   		html_cgi_buff_len = atoi(http_inc_traffic_len);

   		if (html_cgi_buff_len > 0)
   		{

				html_cgi_buff = 0;
				html_cgi_buff = (char *)calloc(html_cgi_buff_len,1);
				if (html_cgi_buff == 0)
				{
					printf("%s%d\n\n", HTTP_RESPONCE_HEADER, HTTP_READ_ERROR_LEN);
					printf("%s", HTTP_READ_ERROR);
					return FAIL;
				}
				memset(html_cgi_buff, 0, html_cgi_buff_len);
				res = fread( html_cgi_buff, 1, html_cgi_buff_len, stdin);
				if ( res != html_cgi_buff_len)
				{
					free((void *)html_cgi_buff);
					printf("%s%d\n\n", HTTP_RESPONCE_HEADER, HTTP_READ_ERROR_LEN);
					printf("%s", HTTP_READ_ERROR);
					return FAIL;
				}

				if (strstr(html_cgi_buff, "filename" ))
				{
					pDest = strstr(html_cgi_buff, "filename=");
					if (pDest != NULL)
					{
						pDest += strlen("filename=");
						if (*pDest == '"' && *(pDest + 1) == '"')
						{
							free((void *)html_cgi_buff);
							printf("%s%d\n\n",HTTP_RESPONCE_HEADER,GOT_NOTHING_LEN);
							printf("%s",GOT_NOTHING);
							return FAIL;
						}
						else
						{
							i = 0;
							while(*(pDest + i + 1) != '"')
							{
							    writeFile[i] = *(pDest + i + 1);
							    i++;
							}
							p1Dest = strstr(pDest, "\n");
							if ((*(p1Dest + 1) != 0xd) && (*(p1Dest + 2) != 0xa))
							{
							   pDest = strstr(p1Dest + 1, "\n");
							   pDest += 1;
							}
							else
							{
							    pDest = p1Dest + 1;
							}

							boundaryLen = pDest - html_cgi_buff;
							pDest += 2;

							// Get filename and action
							strcpy(writeFileTmp, writeFile);

							p_action = strtok(writeFile, " ");
							strcpy(action, p_action);
							strcpy(writeFile, writeFileTmp + strlen(action) + 1);

							snprintf(lengthDebug, sizeof lengthDebug, "%s%s", UPDATE_FILE_PATH, writeFile);
							fileFd = fopen(lengthDebug, "w+");
							if (fileFd != NULL)
							{
							    i = 0;
							    while(i <= html_cgi_buff_len - boundaryLen)
							    {
							        if ((*pDest == 0xd) && (*(pDest + 1) == 0xa) && (i > (html_cgi_buff_len - boundaryLen - 512)))
							        {
							            break;
							        }

							        fwrite(pDest, 1, 1, fileFd);
							        pDest += 1;
							        i += 1;
							    }

							    fclose(fileFd);

							    if (strcmp(action, "upgrade") == 0)
							    {
							    }
							    else if (strcmp(action, "upgradebit") == 0)
							    {   
							    }

							    free((void *)html_cgi_buff);
							    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,UPLOAD_FILE_SUCCESS_LEN);
							    printf("%s", UPLOAD_FILE_SUCCESS);
							  return SUCCESS;
							}
						}
					}
					else
					{
						free((void *)html_cgi_buff);
						printf("%s%d\n\n",HTTP_RESPONCE_HEADER,SOCK_OPEN_ERROR_LEN);
						printf("%s", SOCK_OPEN_ERROR);
						return FAIL;
					}
				}

				else if (strstr(html_cgi_buff, "getDracaenaIPaddress" ))
				{
					free((void *)html_cgi_buff);
					char respone_ip[256];
					char ip_address[256];
					memset(ip_address, 0, sizeof(ip_address));

					if (get_box_ipp(ip_address))
					{
					  	memset(respone_ip, 0, sizeof(respone_ip));
					  	sprintf(respone_ip,"{\"page\":\"getDracaenaIPaddress\", \"box_ip_address\":\"%s\", \"result\":\"SUCCESS\"}", ip_address);						
					}
					else
				    {
						memset(respone_ip, 0, sizeof(respone_ip));
						strcpy(respone_ip,"{\"page\":\"getDracaenaIPaddress\", \"result\":\"FAIL\"}");				    	
				    }



					printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(respone_ip));
					printf("%s", respone_ip);
					return SUCCESS;				
				}

				else if (strstr(html_cgi_buff, "getDracaenaNetworkinfo"))
				{
					free((void *)html_cgi_buff);
					memset(responce, 0, sizeof(responce));

					if ( get_current_config(p_ipaddres, p_subnet, &dhcp_mode, p_gateway, p_dns))
					{
						sprintf(responce,"{\"page\":\"getDracaenaNetworkinfo\", \"ipaddress\":\"%s\",\"subnet\":\"%s\",\"dhcp_mode\":\"%d\",\"gateway\":\"%s\", \"dns\":\"%s\",\"result\":\"SUCCESS\"}", p_ipaddres, p_subnet,dhcp_mode,p_gateway, p_dns);
					}
					else
					{
						strcpy(responce,"{\"page\":\"getDracaenaNetworkinfo\", \"result\":\"FAIL\"}");				    	
					}
					printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
					printf("%s", responce);
					return SUCCESS;	
				}



				else if (strstr(html_cgi_buff, "setDracaenaNetwork"))
				{
					    parse_network_config(html_cgi_buff, &dhcp_mode,  p_ipaddres,  p_subnet,  p_gateway,  p_dns);
						set_network_config( dhcp_mode, p_ipaddres, p_subnet, p_gateway, p_dns);
						memset(responce, 0, sizeof(responce));
						strcpy(responce,"{\"page\":\"setDracaenaNetwork\", \"result\":\"SUCCESS\", \"status\":\"SUCCESS\"}");	
					    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
					    printf("%s", responce);
					    return SUCCESS;	
				}




				else if (strstr(html_cgi_buff, "setDracaenaResetFPGA"))
				{
					 free((void *)html_cgi_buff);
				    memset(responce, 0, sizeof(responce));
				    strcpy(responce, "curl --header \"Content-Type: application/json\" --request POST --data \'{\"voltage_vccint\":850,\"voltage_hbm\":1265,\"boardId\":3}\' http://localhost:8200/controller/setVoltage");

				    //printf("\n \n %s \n \n", lineBuffer);

				    //system(lineBuffer);

				    FILE *p_fHandle = NULL;
				   // p_fHandle = popen("curl --header \"Content-Type: application/json\" --request GET http://192.168.1.16:8200/controller/getVoltage", "r");
				    // curl --header "Content-Type: application/json" --request GET http://192.168.1.16:8200/controller/getTemperature
				    p_fHandle = popen(responce, "r");
				    if (p_fHandle != NULL)
				    {
				        pclose(p_fHandle);
				    }

					memset(responce, 0, sizeof(responce));
					strcpy(responce,"{\"page\":\"setDracaenaResetFPGA\", \"result\":\"SUCCESS\"}");	
				    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
				    printf("%s", responce);
				    return SUCCESS;	
				}

				else if (strstr(html_cgi_buff, "getDracaenaBoxInfo" ))
				{  
                     
                     
/*
					memset(newip, 0, sizeof(newip));
					memset(oldip, 0, sizeof(oldip));*/
					memset(responce, 0, sizeof(responce));

/*
					sub = strstr(html_cgi_buff, "ip_address=");
					sub = sub + 11;
					strcpy(newip, sub);*/
                    
                    free((void *)html_cgi_buff);

/*					if (!is_ipv4_address_valid(newip))
					{
						memset(responce, 0, sizeof(responce));
						strcpy(responce,"{\"page\":\"getDracaenaBoxInfo\", \"result\":\"FAIL\", \"status\":\"invalid\"}");	
					    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
					    printf("%s", responce);
					    return SUCCESS;	

					}*/

/*
					get_box_ipp(oldip);

					if (strcmp(oldip, newip) !=0)
					{
						//need to update new IP
						write_save_ip(newip);
					}*/

					memset(box_info_all, 0, sizeof(box_info_all));

					curl_get_all_info(box_info_all);

                    
                    if ( curl_get_all_info(box_info_all) )
                    {

					   	memset(responce, 0, sizeof(responce));
					    //strcpy(box_info_all, "{\"FPGA\":[{\"boardId\":0,\"status\":0,\"chipTemp\":90,\"boardTemp\":90,\"voltage\":0,\"fanLevel\":1},{\"boardId\":1,\"status\":0,\"chipTemp\":90,\"boardTemp\":90,\"voltage\":0,\"fanLevel\":1},{\"boardId\":2,\"status\":1,\"chipTemp\":31,\"boardTemp\":24,\"voltage\":0,\"fanLevel\":2}],\"curentFanLevel\":2}");
					  	sprintf(responce,"{\"page\":\"getDracaenaBoxInfo\", \"box_info_all\":%s, \"result\":\"SUCCESS\"}", box_info_all);		
					  	//strcpy(responce, box_info_all);				
	
						printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
						printf("%s", responce);
						return SUCCESS;	                  	
                    }
                    else
                    {
                        memset(responce, 0, sizeof(responce));
						strcpy(responce,"{\"page\":\"getDracaenaBoxInfo\", \"result\":\"FAIL\", \"status\":\"offline\"}");	
					    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
					    printf("%s", responce);
					    return SUCCESS;	
                    }

						
				}

				else if (strstr(html_cgi_buff, "setDracaenaSetvoltage" ))
				{
 
                    char lineBuffer[512];
                    char voltage_vccint[50];
                    char boardId[2];
                    char voltage_hbm[50];

					//memset(newip, 0, sizeof(newip));
					//memset(oldip, 0, sizeof(oldip));
					memset(responce, 0, sizeof(responce));

					//get IP

/*					sub = strstr(html_cgi_buff, "ip_address=");
					sub = sub + 11;
					strcpy(newip, sub);*/

					//get vccint

					sub = NULL;
					sub = strstr(html_cgi_buff, "voltage_vccint=");
					sub = sub+15;

					for (int i = 0; i < sizeof(voltage_vccint); i++)
					{   
					    if ( sub[0] == '&')
					    {
					        break;
					    }
					    voltage_vccint[i]= sub[0];
					    sub = sub + 1;
					}


					//get vcc hbm
					sub = NULL;
					sub = strstr(html_cgi_buff, "voltage_hbm=");
					sub = sub+12;

					for (int i = 0; i < sizeof(voltage_hbm); i++)
					{   
					    if ( sub[0] == '&')
					    {
					        break;
					    }
					    voltage_hbm[i]= sub[0];
					    sub = sub + 1;
					}

					//get boardID
					sub = NULL;
					sub = strstr(html_cgi_buff, "board_id=");
					sub = sub+9;

					for (int i = 0; i < sizeof(boardId); i++)
					{   
					    if ( sub[0] == '&')
					    {
					        break;
					    }
					    boardId[i]= sub[0];
					    sub = sub + 1;
					}


/*					sprintf(lineBuffer, "echo \"voltage_vccint=|%s|  voltage_hbm=|%s|    boardId=|%s|   \" > /tmp/stringinpu", voltage_vccint, voltage_hbm,   boardId);
					system(lineBuffer);*/
                    
                    free((void *)html_cgi_buff);

/*					if (!is_ipv4_address_valid(newip))
					{
						memset(responce, 0, sizeof(responce));
						strcpy(responce,"{\"page\":\"setDracaenaSetvoltage\", \"result\":\"FAIL\", \"status\":\"invalid\"}");	
					    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
					    printf("%s", responce);
					    return SUCCESS;	
					}


					//get_box_ipp(oldip);

					if (strcmp(oldip, newip) !=0)
					{
						//need to update new IP
						write_save_ip(newip);
					}*/

					//get voltage 

					
				    memset(lineBuffer, 0, sizeof(lineBuffer));

				    sprintf(lineBuffer, "curl --header \"Content-Type: application/json\" --request POST --data \'{\"voltage_vccint\":%s,\"voltage_hbm\":%s,\"boardId\":%s}\' http://localhost:8200/controller/setVoltage",voltage_vccint,voltage_hbm,boardId);


				    //printf("\n \n %s \n \n", lineBuffer);

				    //system(lineBuffer);

				    FILE *p_fHandle = NULL;
				   // p_fHandle = popen("curl --header \"Content-Type: application/json\" --request GET http://192.168.1.16:8200/controller/getVoltage", "r");
				    // curl --header "Content-Type: application/json" --request GET http://192.168.1.16:8200/controller/getTemperature
				    p_fHandle = popen(lineBuffer, "r");
				    if (p_fHandle != NULL)
				    {
				        memset(lineBuffer, 0, sizeof lineBuffer);
				        fread(lineBuffer, 1, sizeof lineBuffer, p_fHandle);
				        pclose(p_fHandle);
				    }

					memset(responce, 0, sizeof(responce));
					strcpy(responce,"{\"page\":\"setDracaenaSetvoltage\", \"result\":\"SUCCESS\", \"status\":\"SUCCESS\"}");	
				    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,strlen(responce));
				    printf("%s", responce);
				    return SUCCESS;	


				}

		    	webcgi_webcli_communication(html_cgi_buff, html_cgi_buff_len,
		                                cgi_html_buff, &cgi_html_buff_len);

		    //system("sudo killall ethminer")
		    free((void *)html_cgi_buff);
		    html_cgi_buff = 0;
		    printf("%s%d\n\n", HTTP_RESPONCE_HEADER,cgi_html_buff_len);
		    printf("%s", cgi_html_buff);
   		}
	}

	return 0;
}


/*===========================================================================
  FUNCTION webcgi_webcli_communication
===========================================================================*/
/*!
@brief
  This function acts like proxy by sending data received from HTTP request
  to Webclient and returning the responce from Webclient as HTTP responce.

@return
  int  - Returns whether Socket communication is successfull or failure.
      0 - Success
      1 - Failed
@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int webcgi_webcli_communication(char *webcgi_webcli_buff,
                                int webcgi_webcli_buff_size,
                                char *webcli_webcgi_buff,
                                int *webcli_webcgi_buff_size)
{
	//Socket FD for Webclient socket. Used to communicate with Webclient socket.
	int webcli_sockfd = 0;

	//Send data to WEB_CLI socket
	if (send_to_webcli(webcgi_webcli_buff, webcgi_webcli_buff_size,
	webcli_webcgi_buff, webcli_webcgi_buff_size,
	&webcli_sockfd) == 0)
	{
	//Receive data from Webclient
	recv_from_webcli(webcli_webcgi_buff, webcli_webcgi_buff_size,
	&webcli_sockfd);
	}
	return SUCCESS;
}

/*=========================================================================*/
int send_to_webcli(char *webcgi_webcli_buff,
                   int webcgi_webcli_buff_size,
                   char *webcli_webcgi_buff,
                   int *webcli_webcgi_buff_size,
                   int *webcli_sockfd)
{
	//Socket Address to store address of webclient and web cgi sockets.
	struct sockaddr_un webcli_webcgi_sock, webcgi_webcli_sock;
	//Variables to track data received and sent.
	int bytes_sent_to_cli = 0;
	int sock_addr_len = 0;

	memset(&webcli_webcgi_sock,0,sizeof(struct sockaddr_un));
	memset(&webcgi_webcli_sock,0,sizeof(struct sockaddr_un));
	if ((*webcli_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		*webcli_webcgi_buff_size = SOCK_OPEN_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_OPEN_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	if (fcntl(*webcli_sockfd, F_SETFD, FD_CLOEXEC) < 0)
	{
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_FD_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_FD_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	webcli_webcgi_sock.sun_family = AF_UNIX;
	if (strlen(WEBCLIENT_WEBCGI_SOCK) >= UNIX_PATH_MAX )
	{
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_PATH_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_PATH_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	strlcat(webcli_webcgi_sock.sun_path, WEBCLIENT_WEBCGI_SOCK,UNIX_PATH_MAX);
	unlink(webcli_webcgi_sock.sun_path);
	sock_addr_len = strlen(webcli_webcgi_sock.sun_path) + sizeof(webcli_webcgi_sock.sun_family);
	if (bind(*webcli_sockfd, (struct sockaddr *)&webcli_webcgi_sock, sock_addr_len) == -1)
	{
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_BIND_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_BIND_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	webcgi_webcli_sock.sun_family = AF_UNIX;
	if (strlen(WEBCGI_WEBCLIENT_SOCK) >= UNIX_PATH_MAX )
	{
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_PATH_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_PATH_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	strlcat(webcgi_webcli_sock.sun_path, WEBCGI_WEBCLIENT_SOCK,UNIX_PATH_MAX);
	/*d rwx rwx rwx = dec
	0 110 110 110 = 666
	*/
	system("chmod 777 /tmp/cgi_webclient_file");
	sock_addr_len = strlen(webcgi_webcli_sock.sun_path) + sizeof(webcgi_webcli_sock.sun_family);
	if ((bytes_sent_to_cli = sendto(*webcli_sockfd, webcgi_webcli_buff, webcgi_webcli_buff_size, 0,
	(struct sockaddr *)&webcgi_webcli_sock, sock_addr_len)) == -1)
	{
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_SEND_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_SEND_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	if (bytes_sent_to_cli == webcgi_webcli_buff_size)
	{
		return SUCCESS;
	}
	else
	{
		close((int)*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_SEND_COMPLETE_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_SEND_COMPLETE_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
}
/*===========================================================================
  FUNCTION recv_from_webcli
===========================================================================*/
/*!
@brief
  This function frames HTTP response message with data received from WEB_CLI
  socket.

@return
  int  - Returns whether Socket communication is successfull or failure.
      0 - Success
      1 - Failed
@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/



int recv_from_webcli(char *webcli_webcgi_buff,
              int *webcli_webcgi_buff_size,
              int *webcli_sockfd)
{
	//FD set used to hold sockets for select.
	fd_set webcgi_sockfd;
	//Time out for webcgi response.
	struct timeval webcli_sock_timeout;
	//To evaluate webclient socket responce(timed out, error, ....)
	int webcli_sock_resp_status = 0;
	//Variables to track data received and sent.
	int bytes_recv_from_cli = 0;

	FD_ZERO(&webcgi_sockfd);
	if ( *webcli_sockfd < 0 )
	{
		*webcli_webcgi_buff_size = SOCK_FD_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_FD_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	FD_SET(*webcli_sockfd, &webcgi_sockfd);
	webcli_sock_timeout.tv_sec = SOCK_TIMEOUT;
	webcli_sock_timeout.tv_usec = 0;
	webcli_sock_resp_status = select(((int)(*webcli_sockfd))+1,
	                           &webcgi_sockfd, NULL, NULL,
	                           &webcli_sock_timeout);
	if (webcli_sock_resp_status == 0)
	{
		FD_CLR(*webcli_sockfd, &webcgi_sockfd);
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_TIMEOUT_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_TIMEOUT_ERROR, MAX_BUFFER_SIZE);
		return FAIL;  //Time out
	}
	else if  (webcli_sock_resp_status == -1)
	{
		FD_CLR(*webcli_sockfd, &webcgi_sockfd);
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_RESPONSE_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_RESPONSE_ERROR, MAX_BUFFER_SIZE);
		return FAIL;  //Error
	}
	memset(webcli_webcgi_buff, 0, MAX_BUFFER_SIZE);
	bytes_recv_from_cli = recv(*webcli_sockfd, webcli_webcgi_buff, MAX_BUFFER_SIZE, 0);

	if (bytes_recv_from_cli == -1)
	{
		FD_CLR(*webcli_sockfd, &webcgi_sockfd);
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = SOCK_RECEIVE_ERROR_LEN;
		strlcat(webcli_webcgi_buff, SOCK_RECEIVE_ERROR, MAX_BUFFER_SIZE);
		return FAIL;
	}
	else
	{
		FD_CLR(*webcli_sockfd, &webcgi_sockfd);
		close(*webcli_sockfd);
		*webcli_webcgi_buff_size = bytes_recv_from_cli;
		webcli_webcgi_buff[bytes_recv_from_cli] = 0;
		return SUCCESS;
	}
}