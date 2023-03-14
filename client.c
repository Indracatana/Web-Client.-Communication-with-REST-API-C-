#include <ctype.h>
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
#include <stdbool.h>
int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    //variabila pentru a verifica daca clientul este logat
    bool connected=false;

    int id;
    char *token=NULL;
    char **cookies=NULL;
    char cmd[50];
    char username[100];
    char password[100];
    char *cookie;

    //intr-un while interpretez comenzile de la tastatura
    while(1)
    {
        scanf("%s", cmd);

        if(strcmp(cmd,"register\0") == 0)
        {
            //deschid conectiunea cu server-ul
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);
            char *serialized_string = NULL;

            //citesc username-ul si parola
            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string_pretty(root_value);

            //trimit cerere de tip POST pentru autentificare
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", serialized_string, NULL, NULL,0);
            //trimit mesajul catre server
            send_to_server(sockfd, message);
            //extrag raspunsul de la server
            response = receive_from_server(sockfd);

            char *serv_msj = basic_extract_json_response(response);
            //daca mesajul primit de la server nu contine nimic inseamna ca s-a realizat cu succes comanda
            if(serv_msj==NULL)
            {
                printf("200-OK-Utilizator inregistrat cu succes!\n");
            }
            //daca am obtinut eroare
            else
            {
                //afisez eroarea
                root_value=json_parse_string(serv_msj);
                root_object= json_value_get_object(root_value);
                printf("Nu va puteti inregistra: %s\n",json_object_get_string(root_object,"error"));
            }
            //eliberez memoria
            json_value_free(root_value);
            json_free_serialized_string(serialized_string);
        }

        if(strcmp(cmd,"login\0")==0)
        {

            //daca clientul este deja conectat trimit mesaj de eroare
            if(connected==true)
            {
                printf("Sunteti deja conectat\n");
                continue;
            }

            //deschid conexiunea cu server-ul
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);
            char *serialized_string = NULL;
            //citesc username-ul si parola
            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            json_object_set_string(root_object, "username", username);
            json_object_set_string(root_object, "password", password);
            serialized_string = json_serialize_to_string_pretty(root_value);

            //trimit cerere de tip POST pentru logare
            message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", serialized_string, NULL, NULL,0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *serv_msj = basic_extract_json_response(response);
            //daca mesajul primit de la server nu contine nimic inseamna ca s-a realizat cu succes comanda
            if(serv_msj==NULL)
            {
                connected = true;
                printf("200-OK-Bun venit!\n");

                //extrag cookie-ul din mesaj
                char aux[1000];
                strcpy(aux,response);
                char *s= strstr(aux,"Cookie:");
                char *s2=s+8;
                cookie=strtok(s2,";");
            }
            //daca obtin eroare de la server
            else
            {
                //afisez eroarea
                root_value=json_parse_string(serv_msj);
                root_object= json_value_get_object(root_value);
                printf("Nu va puteti loga: %s\n",json_object_get_string(root_object,"error"));
            }
            json_value_free(root_value);
            json_free_serialized_string(serialized_string);
        }

        if(strcmp(cmd,"enter_library\0")==0)
        {
            //daca clientul nu este logat
            if(connected==false)
            {
                continue;
            }
            //completez cookie-ul
            cookies=(char**)calloc(1,sizeof(char*));
            cookies[0]=malloc(strlen(cookie)*sizeof(char));
            strcpy(cookies[0],cookie);

            //deschid conexiunea cu server-ul
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            //verific daca clientul este conectat inainte de a-i permite accesul la biblioteca

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);

            //creez cerere de tip GET pentru acces biblioteca
            message=compute_get_request("34.241.4.235", "/api/v1/tema/library/access",NULL, NULL,cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *serv_msj = basic_extract_json_response(response);
            //daca mesajul primit de la server nu contine nimic inseamna ca s-a realizat cu succes comanda
            if(!(strstr(message,"error")))
            {
                //extrag token-ul de autentificare
                char *aux;
                aux=strstr(serv_msj,"{\"token\":");
                char *aux2=aux+10;
                token=calloc(strlen(serv_msj),sizeof(char));
                strncpy(token,aux2,(strlen(aux2)-2));
                printf("Aveti acces la biblioteca!\n");
            }
            //daca primesc eroare de la server
            else
            {
                root_value=json_parse_string(serv_msj);
                root_object= json_value_get_object(root_value);
                printf("Nu puteti accesa libraria: %s\n",json_object_get_string(root_object,"error"));
            }
            json_value_free(root_value);
        }

        if(strcmp(cmd,"get_books")==0)
        {
            //deschid conexiunea cu server-ul
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);
            JSON_Array *commits;
            JSON_Object *commit;

            //completez cookie-ul
            cookies=(char**)calloc(1,sizeof(char*));
            cookies[0]=malloc(strlen(cookie)*sizeof(char));
            strcpy(cookies[0],cookie);
            //creez cerere de tip GET pentru vizualizare informatii despre toate cartile
            message=compute_get_request("34.241.4.235", "/api/v1/tema/library/books",NULL,token,cookies,1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *serv_msj = basic_extract_json_response(response);
            //daca nu primesc nimic de la server inseamna ca nu exista carti in biblioteca
            if(serv_msj==NULL)
            {
                printf("Nu exista carti!\n");
            }
            //daca server-ul intoarce mesaj de eroare
            else
            {
                if(strstr(serv_msj,"error"))
                {
                    root_value=json_parse_string(serv_msj);
                    root_object= json_value_get_object(root_value);
                    printf("Eroare vizualizare carti! %s\n",json_object_get_string(root_object,"error"));
                }
                else
                {
                    //extrag json array-ul si afisez id-urile si titlurile cartilor din biblioteca
                    root_value=json_parse_string(serv_msj-1);
                    commits = json_value_get_array(root_value);
                    for (int i = 0; i < json_array_get_count(commits); i++)
                    {
                        commit = json_array_get_object(commits, i);
                        printf("Id:%.0f,\nTitlu:%s\n\n",
                               json_object_get_number(commit, "id"),
                               json_object_get_string(commit, "title"));
                    }

                }
            }
            json_value_free(root_value);
        }

        if(strcmp(cmd,"get_book")==0)
        {
            //construiesc cookie-ul
            cookies=(char**)calloc(1,sizeof(char*));
            cookies[0]=malloc(strlen(cookie)*sizeof(char));
            strcpy(cookies[0],cookie);
            //deschid conexiunea cu server-ul
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            char string_id[100];
            //citesc id-ul cartii
            printf("id=");
            scanf("%s", string_id);
            //verfic daca id-ul are formatul corect:numar
            for (int i = 0; string_id[i]!= '\0'; i++)
            {
                if (isdigit(string_id[i]) == 0)
                {
                    printf("Id incorect\n");
                    continue;
                }
            }

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);
            JSON_Array *commits;
            JSON_Object *commit;
            id=atoi(string_id);
            //construiesc url-ul corespunzator cererii de vizualizare a unei carti
            char book_req[100];
            sprintf(book_req,"/api/v1/tema/library/books/%d",id);

            //trimit cerere de tip GET pentru viziualizarea unei carti
            message=compute_get_request("34.241.4.235",book_req,NULL,token,cookies,1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *serv_msj = basic_extract_json_response(response);
            //daca am eroare de la server
            if(strstr(serv_msj,"error")!=NULL)
            {
                //afisez eroarea
                root_value=json_parse_string(serv_msj);
                root_object= json_value_get_object(root_value);
                printf("Eroare vizualizare carte! %s\n",json_object_get_string(root_object,"error"));
            }
            else
            {
                //afisez cartea avand formatul cerut
                root_value=json_parse_string(serv_msj-1);
                commits = json_value_get_array(root_value);
                commit = json_array_get_object(commits, 0);
                printf("Id:%d,\nTitlu:%s,\nAutor:%s,\nPublicatie:%s,\nGen:%s,\nNumar_pagini:%.0f\n\n",
                       id,
                       json_object_get_string(commit, "title"),
                       json_object_get_string(commit, "author"),
                       json_object_get_string(commit, "publisher"),
                       json_object_get_string(commit, "genre"),
                       json_object_get_number(commit, "page_count"));
            }
            json_value_free(root_value);
        }

        if(strcmp(cmd,"add_book")==0)
        {
            cookies=(char**)calloc(1,sizeof(char*));
            cookies[0]=malloc(strlen(cookie)*sizeof(char));
            strcpy(cookies[0],cookie);

            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);
            char *serialized_string = NULL;

            int ok=1;
            char title[100];
            char author[100];
            char genre[100];
            char publisher[100];
            char page_count_str[100];
            int page_count;

            //citesc campurile necesare adaugarii unei carti
            printf("title=");
            scanf("%s",title);

            printf("author=");
            scanf("%s",author);

            printf("genre=");
            scanf("%s",genre);

            printf("publisher=");
            scanf("%s",publisher);

            //citesc id-ul cartii
            printf("page_count=");
            scanf("%s", page_count_str);

            //verific daca numarul de pagini are formatul corespunzator:numar
            for (int i = 0; page_count_str[i]!= '\0'; i++)
            {
                if (isdigit(page_count_str[i]) == 0)
                {
                    printf("Numar pagini incorect\n");
                    ok=0;
                    break;
                }
            }
            //daca nu are formatul corespunzator
            if(ok==0)
            {
                continue;
            }

            page_count=atoi(page_count_str);

            json_object_set_string(root_object,"title",title);
            json_object_set_string(root_object,"author",author);
            json_object_set_string(root_object,"genre",genre);
            json_object_set_string(root_object,"publisher",publisher);
            json_object_set_number(root_object,"page_count",page_count);

            serialized_string=json_serialize_to_string_pretty(root_value);
            //compun cerere de tip POST pentru adaugarea unei carti
            message=compute_post_request("34.241.4.235","/api/v1/tema/library/books","application/json",serialized_string,token,cookies,1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *serv_msj = basic_extract_json_response(response);
            //daca obtin eroare de server
            if(serv_msj)
            {
                root_value=json_parse_string(serv_msj);
                root_object= json_value_get_object(root_value);
                printf("Eroare adaugare carte! %s\n",json_object_get_string(root_object,"error"));
            }
            else
            {
                printf("Cartea dvs a fost adaugata cu succes!\n");
            }
            json_value_free(root_value);
            json_free_serialized_string(serialized_string);
        }

        if(strcmp(cmd,"delete_book")==0)
        {
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);

            int ok=1;
            char string_id[100];
            //citesc id-ul cartii care trebuie stearsa
            printf("id=");
            scanf("%s",string_id);
            //verific daca id-ul are formatul corect:numar
            for (int i = 0; string_id[i]!= '\0'; i++)
            {
                if (isdigit(string_id[i]) == 0)
                    ok=0;
                break;
            }
            if(ok==1)
            {
                id=atoi(string_id);
                char book_req[100];
                sprintf(book_req,"/api/v1/tema/library/books/%d",id);

                //trimit cerere de tip GET pentru viziualizarea unei carti
                message=compute_delete_request("34.241.4.235",book_req,token,NULL,0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                char *serv_msj = basic_extract_json_response(response);
                //daca primesc eroare de la server
                if(serv_msj)
                {
                    //afisez eroarea
                    root_value=json_parse_string(serv_msj);
                    root_object= json_value_get_object(root_value);
                    printf("Eroare stergere carte! %s\n",json_object_get_string(root_object,"error"));
                }
                else
                {
                    printf("Carte stearsa cu succes!\n");
                }
            }
            else
            {
                printf("Id incorect!\n");
            }

            json_value_free(root_value);
        }

        if(strcmp(cmd,"logout")==0)
        {
            if(connected==false)
            {
                printf("Nu sunteti conectat!\n");
                continue;
            }

            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object= json_value_get_object(root_value);
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            //compun mesaj de tip GET pentru logout
            message= compute_get_request("34.241.4.235","api/v1/tema/auth/logout",NULL,token,cookies,1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char *serv_msj = basic_extract_json_response(response);
            //daca obtin eroare de la server
            if(serv_msj)
            {
                root_value=json_parse_string(serv_msj);
                root_object= json_value_get_object(root_value);
                printf("Eroare logout! %s\n",json_object_get_string(root_object,"error"));
            }
            else
            {
                printf("Ati fost delogat!\n");
                //client-ul nu mai este autentificat
                connected=false;
                //sterg cookie-urile
                cookies=NULL;
                //sterg token-ul de autentificare
                token=NULL;
                //inchid conexiunea
                close_connection(sockfd);
            }
            json_value_free(root_value);
        }

        if(strcmp(cmd,"exit")==0)
        {
            break;
        }
    }
    //eliberez memoria
    free(message);
    free(cookies);
    free(token);
}
