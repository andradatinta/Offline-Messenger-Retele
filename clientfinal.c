#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#define PORT 8745

int checkLogin = 0;
void readCommand (char* comm, size_t maxBytes ) // reading the command given by the user 
{
    bzero(comm, maxBytes);
    fflush(stdout);
    read(0, comm, maxBytes);
    comm[strlen(comm)]='\0';
}

void sendCommand(char* comm, int sock, int length ) // writing the command given by the user  to the server
{
    write(sock, comm, length); 
}

void readResponse(char* response, int sock)
{
    bzero(response, 1000);
    read(sock, response, 1000);
    response[strlen(response)]='\0';
}

void getAnswer(void * answer, int sock, size_t length) // nu merge
{
    bzero(answer, length);
    read(sock, answer, length);
}

void printMessages(char * response, int sock_server)
{  

    if(strncmp(response, "login:success", strlen("login:success")) == 0)
    {
        if(checkLogin == 1)
        {
            printf("\n[Offline Messenger] -> You are already logged in as an user! Please log out first if you want to change the user!\n");

        }
        else
        {
            checkLogin = 1;
            printf("\n[Offline Messenger] -> You have successfully logged in!\n");
        }
    }

    if(strncmp(response, "login:failed", strlen("login:failed")) == 0)
    {
        printf("\n[Offline Messenger] -> No existing user found with this username, please sign up on Offline Messenger!\n");
    }

    if(strncmp(response, "status:active", strlen("status:active")) == 0)
    {
        if(checkLogin == 1)
        {
            printf("\n[Offline Messenger] -> The user you're looking for is active!\n");
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }
    }

    if(strncmp(response, "status:offline", strlen("status:offline")) == 0)
    {
        if(checkLogin == 1)
        {
            printf("\n[Offline Messenger] -> The user you're looking for is offline!\n");
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }

    }

    if(strncmp(response, "signup:failed", strlen("signup:failed")) == 0)
    {
        printf("\n[Offline Messenger] -> The username you wanted already exists! Try again with another username!\n");
    }

    if(strncmp(response, "signup:success", strlen("signup:success")) == 0)
    {
        printf("\n[Offline Messenger] -> You have succesfully signed up! Please log in now to use Offline Messenger!\n");
    }

    if(strncmp(response, "conversation:success", strlen("conversation:success")) == 0)
    {
        printf("\n[Offline Messenger] -> You have succesfully sent the messages! Please enter another command!\n");
    }

    if(strncmp(response, "\n HISTORY: \n ", strlen("\n HISTORY: \n ")) == 0)
    {
        if(checkLogin == 1)
        {
            printf("%s \n", response);
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }
    }

    if(strncmp(response, "history:failed", strlen("history:failed")) == 0)
    {
        printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
    }

    if(strncmp(response, "2022", strlen("2022")) == 0)
    {
         if(checkLogin == 1)
        {
            printf("%s \n", response);
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }
    }


    if(strncmp(response, "logout:success", strlen("logout:success")) == 0)
    {
         if(checkLogin == 1)
        {
            checkLogin = 0;
            printf("\n[Offline Messenger] -> Logged out succesfully. Please log back in to keep using Offline Messenger. \n");
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }
    }

    if(strncmp(response, "logout:failed", strlen("logout:failed")) == 0)
    {
         if(checkLogin == 1)
        {
            printf("\n[Offline Messenger] -> Wrong username for logout. \n");
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }
    }

    if(strncmp(response, "logout:nologin", strlen("logout:nologin")) == 0)
    {
        
        printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        
    }

    if(strncmp(response, "\n You have ", strlen("\n You have ")) == 0)
    {
        
        if(checkLogin == 1)
        {
            printf("%s \n", response);
        }
        else
        {
            printf("\n[Offline Messenger] -> You are NOT logged in! Please use the login command before anything else!\n");
        }
        
    }
    



     
}

int main ()
{
    int sock_server = -1;
    int check = -1;
    struct sockaddr_in server_address;
    char message[100];
    char response[1000];
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock_server < 0 )
    { 
        perror("Failed at socket()");
        return -1;
    }

    // Populating the server_address struct 

    server_address.sin_family= AF_INET;
    server_address.sin_port=htons(PORT); 
    server_address.sin_addr.s_addr=inet_addr("127.0.0.1"); 

    check = connect(sock_server, (struct sockaddr *) &server_address, sizeof(server_address));

    if (check < 0)
    { 
        perror ("Failed at connect ()");
        // close(sock_server);
        return -1;
    }

    // Communicating with the server

    while (1)
    {
    readCommand(message, sizeof(message));
    sendCommand(message, sock_server, strlen(message));

    // reading the response from the server 

    readResponse(response, sock_server);
    printf( "Mesajul de la server: %s \n", response);
    printMessages(response, sock_server);


   
    }

    // Closing the client 

    close(sock_server);
    return 0;
}