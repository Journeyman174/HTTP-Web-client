#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <ctype.h>      /* isdigit */



int main(int argc, char *argv[])
{
    char *message = NULL;
    char *response = NULL;
    char *info = NULL;				//buffer in care se citeste de la tastatura
    char *temp = NULL;				//zona temporara in care se pastreaza raspunsul de la server
    char *token_jwt = NULL;    	//zona in care se memoreaza tokenul jwt transmis de server
	char *token_jwt_rez = NULL; 	//zona in care se memoreaza tokenul jwt transmis de server
    char * cooky = NULL;		 	//memoreaza cookies transmisi de server
							//folosite la parsarea raspunsului json de la server
    JSON_Value *error = NULL;
    JSON_Value *token = NULL;
    JSON_Object *commit = NULL;
    JSON_Value *root_value = NULL;
    JSON_Array *commits = NULL;
    size_t i;
    int sockfd, logout = 0, login = 0, acces = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr/*, servaddr_weather*/;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("34.241.4.235");
    servaddr.sin_port = htons(8080);
    
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

//sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
//fac open_connection la fiecare c-da

    char **form_data = calloc(6, sizeof(char *));	//parametrii de cerere
    for (int i = 0; i < 6; i++)
    {
        form_data[i] = calloc(LINELEN, sizeof(char));
    }

		char **json_data = calloc(1, sizeof(char *));
		json_data[0] = calloc(BUFLEN, sizeof(char));
		info = calloc(80, sizeof(char));
		temp = calloc(4096, sizeof(char));
		token_jwt = calloc(4096, sizeof(char));
		token_jwt_rez = calloc(4096, sizeof(char));
		char *cmd = calloc(80, sizeof(char));
		char *tmp = calloc(4096, sizeof(char));
		char **cookies = calloc(1, sizeof(char *));
    	cookies[0] = calloc(LINELEN, sizeof(char));
    	cooky = calloc(LINELEN, sizeof(char));

    while (1) {
		printf("Introduceti comanda!\n ");	//invitatie
        // se citeste de la tastatura
	    memset(info, 0, 80);	//pregatesc bufferul de citire de la tastatura
        fgets(info, 79, stdin);
//-----------------------
        if (strncmp(info, "exit", 4) == 0) {
			break;
		}
//-----------------------
        if (strncmp(info, "login", 5) == 0)   {
			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

			if(strlen(cookies[0]) != 0 && logout==0)
			{
				printf("Already logged on !\n");
				continue;
			}
			//se citeste username si password
		    printf("username : ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info)-1]=='\n') info[strlen(info)-1]='\0';
		    strcpy(form_data[0], "\n\t\"username\":");//pregatesc username si paswword in format json
		    strcat(form_data[0], "\"");
	        strcat(form_data[0], info);
		    strcat(form_data[0], "\"");

		    memset(info, 0, 80);
		    printf("password : ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info)-1]=='\n') info[strlen(info)-1]='\0';
		    strcpy(form_data[1], "\n\t\"password\":");
		    strcat(form_data[1], "\"");
	        strcat(form_data[1], info);
		    strcat(form_data[1], "\"\n");
		    message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", form_data, 2, NULL, 0, NULL);
	    	send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
			
			strcpy(temp, response);	//pastrez raspunsul serverului pentru eventuale prelucrari
			
			char *raspuns = strtok(temp,"\r\n"); //extrag prima linie din raspuns
			if(strcmp(raspuns , "HTTP/1.1 200 OK") != 0) 
			{ 
				// serverul nu a acceptat cererea si raspunde cu mesaj JSON
   			 	strcpy(json_data[0], basic_extract_json_response(response)); //extrag linia cu json adica incepe cu {:"
				error = json_parse_string(json_data[0]); //in vectorul json_data[0]
				commit = json_value_get_object(error); //creez obiect json cu valoarea primita
				if (strcmp(json_object_get_string(commit, "error"),"No account with this username!") == 0)
					printf("No account with this username! Va rugam sa executati comanda  register\n");
				
				else if (strcmp(json_object_get_string(commit, "error"),"Credentials are not good!") == 0)
					printf("Credentials are not good! Va rugam sa reluati login corect de data aceasta !\n");
			}

			else {	//c-da acceptata de server analizez raspunsul
				while (raspuns != NULL) {
        				if(strstr(raspuns,"Set-Cookie:") != NULL){ 
						/*daca contine cookies ii pastrez intregul in cook si ii extrag valoarea in 
						cookies[0]*/
						char * cook = strstr(raspuns,"Set-Cookie:")+11;
						cookies[0] = strtok(cook,";");
						break;
						}
        				
						raspuns = strtok(NULL, "\r\n");
    			} 

				logout = 0; //initializez variabila de logout
				strcpy(cooky,cookies[0]); //memorez cookies
				login = 1;
				printf("Logare reusita ! Bun venit !\n");
			}

			close_connection(sockfd);
			continue;				
		}
//------------------------------------
        if (strncmp(info, "register", 8) == 0)  {
			if (strlen(cookies[0]) != 0)
			{
				printf("Already logged on !\n");
				continue;
			}

			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
		
			// se citeste username si password
		    printf("username : ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info)-1] == '\n') info[strlen(info)-1] = '\0';
		    strcpy(form_data[0], "\n\t\"username\":");
		    strcat(form_data[0], "\"");
	        strcat(form_data[0], info);
		    strcat(form_data[0], "\"");

		    memset(info, 0, 80);
		    printf("password : ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info)-1] == '\n') info[strlen(info)-1] = '\0';
		    strcpy(form_data[1], "\n\t\"password\":");
		    strcat(form_data[1], "\"");
	        strcat(form_data[1],info);
		    strcat(form_data[1], "\"\n");
		    message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", form_data, 2, NULL, 0, NULL);
	    	send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);

			//a primit raspuns, verific daca serverul a acceptat comanda
			strcpy(temp, response);
			char *raspuns = strtok(temp,"\r\n");
			if (strcmp(raspuns , "HTTP/1.1 201 Created") != 0)
			{
				//serverul nu a acceptat cererea
				if (strcmp(raspuns , "HTTP/1.1 429 Too Many Requests") == 0) {
					printf("Too many requests, please try again later!\n");
					close(sockfd);
					continue;
				}
				else {
					//analizez raspunsul json
		    		strcpy(json_data[0], basic_extract_json_response(response));
					error = json_parse_string(json_data[0]);
					commit = json_value_get_object(error);
					if (json_object_get_string(commit, "error") != NULL)
						printf("%s !Va rugam sa executati comanda : register cu alt nume de cont !\n",json_object_get_string(commit, "error"));
				}
			}
			else {
				printf("Inregistrare cont reusita !\n");
    			close(sockfd);
				continue;
			}
		}
//------------------------------
		if (strncmp(info, "enter_library", 12) == 0)  {
			if(logout == 1) {
				printf("Ati dat Logout ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if (login == 0) {
				printf("NU ati dat login ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			strcpy(cooky,cookies[0]);
			message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, cookies, 1,NULL);
    		send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
			strcpy(temp, response);
			char *raspuns = strtok(temp,"\r\n");
			strcpy(json_data[0], basic_extract_json_response(response));

			if (strcmp(raspuns , "HTTP/1.1 429 Too Many Requests") == 0) {
				printf("Too many requests, please try again later!Dati logout si introduceti din nou cartea\n");
				close(sockfd);
				continue;
				}

			if(strcmp(raspuns , "HTTP/1.1 200 OK") != 0) {
				error = json_parse_string(json_data[0]);
				commit = json_value_get_object(error);
				if (strcmp(json_object_get_string(commit, "error"),"You are not logged in!") == 0)
					printf("%s !Va rugam sa executati comanda : login\n",json_object_get_string(commit, "error"));
			}

			else {
				token = json_parse_string(json_data[0]); //dupa acceptarea cererii de acces biblioteca
				commit = json_value_get_object(token);	//serverul trimite token JWT(JSON Web Token)
				strcpy(token_jwt,json_object_get_string(commit, "token"));//il pastrez pentru utilizari ulterioare in variabila token_jwt

				printf("Acces in biblioteca  reusit !\n");
				acces = 1;
			}

			strcpy(cookies[0], cooky);
			continue;

		}
//----------------------------
		if (strncmp(info, "get_books", 9) == 0)  {
			if (logout == 1) {
				printf("Ati dat Logout ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if (login == 0) {
				printf("NU ati dat login ! Nu aveti acces la biblioteca !\n");
				continue;
			}
			
			if (acces == 0) {
				printf("Tastati c-da : enter_library ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, cookies, 1,token_jwt);
			strcpy(token_jwt_rez,token_jwt);
			send_to_server(sockfd, message);
			response = receive_from_server(sockfd);
			strcpy(temp, response);
			char *raspuns = strtok(temp, "\r\n");
			if (basic_extract_json_response(response) == NULL) 
			{
				puts("Nu aveti nici o carte in biblioteca !");
				continue;
			}
			
			strcpy(json_data[0], basic_extract_json_response(response));
			sprintf(tmp,"[ %s",json_data[0]);
			json_data[0] = strdup(tmp);

			if (strcmp(raspuns , "HTTP/1.1 429 Too Many Requests") == 0) {
				printf("Too many requests, please try again later!Dati logout si introduceti din nou cartea\n");
				close(sockfd);
				continue;
				}

			if (strcmp(raspuns , "HTTP/1.1 200 OK") != 0)
				printf("Nu aveti acces la biblioteca\n");
				
			else {
				printf("Informatii sumare despre toate cartile din biblioteca :\n");
				root_value = json_parse_string(json_data[0]);
    			
				//analizeaza array-ul cu datele cartilor din biblioteca
    			commits = json_value_get_array(root_value);

    			printf("%10s  %s\n", "Id", "Titlu");
    			for (i = 0; i < json_array_get_count(commits); i++) {
        			commit = json_array_get_object(commits, i);
        			printf("%10.f  %s\n",
               		json_object_get_number(commit, "id"),
               		json_object_get_string(commit, "title"));
    			}
			}

			continue;
		}
//------------------------------------
		if (strncmp(info, "get_book", 8) == 0)  {
			if(logout == 1) {
				printf("Ati dat Logout ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if (login == 0) {
				printf("NU ati dat login ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if (acces == 0) {
				printf("Tastati c-da : enter_library ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			printf("id = ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info) -1] =='\n') info[strlen(info) -1] = '\0';
			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			strcpy(cmd, "/api/v1/tema/library/books/");
			strcat(cmd, info);
			strcpy(cookies[0], cooky);
			message = compute_get_request("34.241.4.235", cmd, NULL, cookies, 1, token_jwt);
			send_to_server(sockfd, message);
			response = receive_from_server(sockfd);
			strcpy(temp, response);
			char *raspuns = strtok(temp,"\r\n");
			strcpy(json_data[0], basic_extract_json_response(response));

			if (strcmp(raspuns , "HTTP/1.1 429 Too Many Requests") == 0) {
				printf("Too many requests, please try again later!Dati logout si introduceti din nou cartea\n");
				close(sockfd);
				continue;
				}

			if (strcmp(raspuns , "HTTP/1.1 200 OK") != 0) 
				printf("Nu aveti acces la biblioteca sau id carte eronat !\n");

			else {
				error = json_parse_string(json_data[0]);//in vectorul json_data[0]
				commit=json_value_get_object(error);//creez obiect json cu valoarea primita
				printf("Informatii sumare despre cartea din biblioteca :\n");
				printf("Titlu : %s\n",json_object_get_string(commit, "title"));
				printf("Autor : %s\n",json_object_get_string(commit, "author"));
				printf("Gen : %s\n",json_object_get_string(commit, "genre"));
				printf("Nr.pag. : %.f\n",json_object_get_number(commit, "page_count"));
				}
			strcpy(cookies[0],cooky);
			//cooky memoreaza cooky initial pentru utilizari in autentificare
			continue;
		}
//----------------
		if (strncmp(info, "add_book", 8) == 0)  {
			if(logout == 1) {
				printf("Ati dat Logout ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if(login == 0) {
				printf("NU ati dat login ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if(acces == 0) {
				printf("Tastati c-da : enter_library ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			printf("title : ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info) -1] == '\n') info[strlen(info) -1] = '\0';
		    strcpy(form_data[0], "\n\t\"title\":");
		    strcat(form_data[0], "\"");
	        strcat(form_data[0],info);
		    strcat(form_data[0], "\"");
		    memset(info, 0, 80);
		    printf("author: ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info)-1] == '\n') info[strlen(info)-1] = '\0';
		    strcpy(form_data[1], "\n\t\"author\":");
		    strcat(form_data[1], "\"");
	        strcat(form_data[1],info);
		    strcat(form_data[1], "\"");

			printf("genre: ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info) -1] == '\n') info[strlen(info) -1] = '\0';
		    strcpy(form_data[2], "\n\t\"genre\":");
		    strcat(form_data[2], "\"");
	        strcat(form_data[2], info);
		    strcat(form_data[2], "\"");
			
			//accept doar numere la nr. de pagini
			while (1) {
		    	memset(info, 0, 80);
		    	printf("page_count: ");
		    	fgets(info, 79, stdin);
		      	if(info[strlen(info) -1] == '\n') info[strlen(info) -1] = '\0';
				int digit = 1;
			  	for(int j = 0; info[j] != '\0'; j++)
			  	{
			  		if(isdigit(info[j]) == 0) 
				  	{
     					digit = 0;
					  	break;
				  	}
			  	}

				if(digit != 0)
					break;
				else
					puts("Introduceti cifre ! Este vorba de nr. de pagini!");
			}

		    strcpy(form_data[3], "\n\t\"page_count\":");
	        strcat(form_data[3], info);

			printf("publisher: ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info)-1] == '\n') info[strlen(info) -1] = '\0';
		    strcpy(form_data[4], "\n\t\"publisher\":");
		    strcat(form_data[4], "\"");
	        strcat(form_data[4], info);
		    strcat(form_data[4], "\"\n");
			strcpy(cookies[0], cooky);

//			puts(cookies[0]);

		    message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json", form_data, 5, cookies, 1,token_jwt);
	    	send_to_server(sockfd, message);
    		response = receive_from_server(sockfd);
			strcpy(temp, response);
			char *raspuns = strtok(temp,"\r\n");
//			puts(raspuns);
			if (strcmp(raspuns , "HTTP/1.1 429 Too Many Requests") == 0) {
				printf("Too many requests, please try again later!Dati logout si introduceti din nou cartea\n");
				close(sockfd);
				continue;
				}

			if(strcmp(raspuns , "HTTP/1.1 200 OK") != 0) 
				{
					printf("Nu aveti acces la biblioteca sau informatii  carte eronate !\n");
				}
			else 	
				printf("Adaugare carte reusita !\n");
//			puts(cookies[0]);
			strcpy(cookies[0], cooky);
			//cooky = memoreaza cooky initial pentru utilizari in autentificare
			
			continue;
		}
//----------------		

		if (strncmp(info, "delete_book", 11) == 0)  {
			if(logout == 1) {
				printf("Ati dat Logout ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if(acces == 0) {
				printf("Tastati c-da : enter_library ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			if(login == 0) {
				printf("NU ati dat login ! Nu aveti acces la biblioteca !\n");
				continue;
			}

			printf("id = ");
		    fgets(info, 79, stdin);
		    if(info[strlen(info) -1] == '\n') info[strlen(info) -1] = '\0';
			sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			strcpy(cmd,"/api/v1/tema/library/books/");
			strcat(cmd, info);
			strcpy(cookies[0], cooky);
			message = compute_delete_request("34.241.4.235", cmd, NULL, cookies, 1, token_jwt);
			send_to_server(sockfd, message);
			response = receive_from_server(sockfd);
			strcpy(temp, response);
			char *raspuns = strtok(temp,"\r\n");

			if (strcmp(raspuns , "HTTP/1.1 429 Too Many Requests") == 0) {
				printf("Too many requests, please try again later!Dati logout si introduceti din nou cartea\n");
				close(sockfd);
				continue;
				}

			if(strcmp(raspuns , "HTTP/1.1 200 OK") != 0) 
				printf("Nu aveti acces la biblioteca sau id carte eronat !\n");
			else 	
				printf("S-a sters cartea !\n");

			continue;
			
		}

//----------------
		if (strncmp(info, "logout", 6 ) == 0)  {
			if(strlen(cookies[0]) == 0)
			{
				printf("Nu ati data login! Nu sunteti autentificat!\n");
				continue;
			}
	
			close_connection(sockfd);
			memset(cookies[0], 0, LINELEN);
			memset(cooky, 0, LINELEN);
			memset(token_jwt, 0, 1024);
			memset(temp, 0, 1024);

			logout = 1;
			login = 0;
			acces = 0;
			printf("La revedere !\n");
			continue;
		}

		printf("Comanda eronata !\n");
		fflush(stdin);
               
	}

    // exit
    free(message);
    free(response);	
    free(info);
	free(json_data[0]);
	free(temp);
	free(token_jwt);

	free(form_data);
	free(token_jwt_rez);
	free(json_data[0]);
	free(cmd);
	free(tmp);
	json_value_free(root_value);
	for (int i = 0; i < 6; i++)
    {
        free(form_data[i]);
    }
    return 0;
}